#include "UsbHostMsc.h"
#include "usb/usb_host.h"
#include "usb/usb_helpers.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "USB_HOST_MSC";
static bool usbHostInited = false;
static usb_host_client_handle_t client_hdl = NULL;
static TaskHandle_t usb_host_task_hdl = NULL;

static void client_event_cb(const usb_host_client_event_msg_t *event_msg, void *arg) {
    if (event_msg->event == USB_HOST_CLIENT_EVENT_NEW_DEV) {
        Serial.printf("[USBH] New device detected! Address: %d\n", event_msg->new_dev.address);

        usb_device_handle_t device_hdl;
        esp_err_t err = usb_host_device_open(client_hdl, event_msg->new_dev.address, &device_hdl);
        if (err == ESP_OK) {
            Serial.println("[USBH] Device successfully opened.");

            const usb_device_desc_t *dev_desc;
            if (usb_host_get_device_descriptor(device_hdl, &dev_desc) == ESP_OK) {
                Serial.printf("[USBH] Vendor ID: 0x%04x, Product ID: 0x%04x\n", dev_desc->idVendor, dev_desc->idProduct);
            }

            // In a full MSC implementation, we would mount the filesystem here.
            // For now, we just signal that a device is present.
            usbReady = true;
            notifyUsbState(true);

            // Note: We close it immediately because we don't have a full MSC driver yet.
            // This is just for detection as per current codebase logic.
            usb_host_device_close(client_hdl, device_hdl);
        } else {
            Serial.printf("[USBH] Failed to open device: 0x%x. Verify VBUS power.\n", err);
        }
    } else if (event_msg->event == USB_HOST_CLIENT_EVENT_DEV_GONE) {
        Serial.println("[USBH] Device disconnected.");
        usbReady = false;
        notifyUsbState(false);
    }
}

static void usb_host_task(void *arg) {
    Serial.println("[USBH] USB Host Event Task started on Core 0.");
    while (usbHostInited) {
        uint32_t event_flags;
        esp_err_t err = usb_host_lib_handle_events(pdMS_TO_TICKS(50), &event_flags);
        if (err != ESP_OK && err != ESP_ERR_TIMEOUT) {
            Serial.printf("[USBH] lib_handle_events error: 0x%x\n", err);
        }

        if (client_hdl) {
            err = usb_host_client_handle_events(client_hdl, pdMS_TO_TICKS(50));
            if (err != ESP_OK && err != ESP_ERR_TIMEOUT) {
                Serial.printf("[USBH] client_handle_events error: 0x%x\n", err);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    Serial.println("[USBH] USB Host Event Task stopping...");
    usb_host_task_hdl = NULL;
    vTaskDelete(NULL);
}

void initUsbHost() {
    if (usbHostInited) {
        Serial.println("[USBH] Already initialized.");
        return;
    }

    Serial.println("[USBH] Initializing USB Host Stack...");

    usb_host_config_t host_config = {
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    esp_err_t err = usb_host_install(&host_config);
    if (err != ESP_OK) {
        if (err == ESP_ERR_INVALID_STATE) {
            Serial.println("[USBH] Stack already installed.");
        } else {
            Serial.printf("[USBH] Stack install failed: 0x%x\n", err);
            return;
        }
    }

    usbHostInited = true;

    usb_host_client_config_t client_config = {
        .is_synchronous = false,
        .max_num_event_msg = 10,
        .async = {
            .client_event_callback = client_event_cb,
            .callback_arg = NULL,
        }
    };
    err = usb_host_client_register(&client_config, &client_hdl);
    if (err != ESP_OK) {
        Serial.printf("[USBH] Client registration failed: 0x%x\n", err);
        usbHostInited = false;
        // Should we uninstall here? Maybe not if it was already installed.
        return;
    }

    BaseType_t ret = xTaskCreatePinnedToCore(usb_host_task, "usb_host", 4096, NULL, 5, &usb_host_task_hdl, 0);
    if (ret != pdPASS) {
        Serial.println("[USBH] Failed to create task.");
        usbHostInited = false;
        usb_host_client_deregister(client_hdl);
        client_hdl = NULL;
        return;
    }

    Serial.println("[USBH] USB Host Stack ready.");
}

void usbHostTick() {}

void stopUsbHost() {
    if (!usbHostInited) return;

    Serial.println("[USBH] Shutting down USB Host...");
    usbHostInited = false;

    // Wait for task to finish
    int timeout = 100; // 1 second total
    while (usb_host_task_hdl != NULL && timeout-- > 0) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (client_hdl) {
        Serial.println("[USBH] Deregistering client...");
        usb_host_client_deregister(client_hdl);
        client_hdl = NULL;
    }

    Serial.println("[USBH] Uninstalling stack...");
    esp_err_t err = usb_host_uninstall();
    if (err != ESP_OK) {
        Serial.printf("[USBH] Uninstall failed: 0x%x\n", err);
    }

    usbReady = false;
    Serial.println("[USBH] Shutdown complete.");
}
