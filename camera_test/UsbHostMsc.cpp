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
        Serial.printf("[USBH] Connection detected! Address: %d\n", event_msg->new_dev.address);

        usb_device_handle_t device_hdl;
        esp_err_t err = usb_host_device_open(client_hdl, event_msg->new_dev.address, &device_hdl);
        if (err == ESP_OK) {
            Serial.println("[USBH] Device verified and opened.");

            const usb_device_desc_t *dev_desc;
            if (usb_host_get_device_descriptor(device_hdl, &dev_desc) == ESP_OK) {
                Serial.printf("[USBH] VID: 0x%04x, PID: 0x%04x\n", dev_desc->idVendor, dev_desc->idProduct);
            }

            usbReady = true;
            notifyUsbState(true);
            usb_host_device_close(client_hdl, device_hdl);
        } else {
            Serial.printf("[USBH] Open failed: %d. Check VBUS power.\n", err);
        }
    } else if (event_msg->event == USB_HOST_CLIENT_EVENT_DEV_GONE) {
        Serial.println("[USBH] Device removed");
        usbReady = false;
        notifyUsbState(false);
    }
}

static void usb_host_task(void *arg) {
    Serial.println("[USBH] Task Running on Core 0");
    while (usbHostInited) {
        uint32_t event_flags;
        usb_host_lib_handle_events(pdMS_TO_TICKS(50), &event_flags);
        if (client_hdl) {
            usb_host_client_handle_events(client_hdl, pdMS_TO_TICKS(50));
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    Serial.println("[USBH] Task Exiting");
    vTaskDelete(NULL);
}

void initUsbHost() {
    if (usbHostInited) return;

    Serial.println("[USBH] Initializing USB Host Stack...");

    usb_host_config_t host_config = {
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    esp_err_t err = usb_host_install(&host_config);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        Serial.printf("[USBH] Install error: %d\n", err);
        return;
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
        Serial.printf("[USBH] Register error: %d\n", err);
        usbHostInited = false;
        return;
    }

    xTaskCreatePinnedToCore(usb_host_task, "usb_host", 4096, NULL, 5, &usb_host_task_hdl, 0);
    Serial.println("[USBH] Setup complete.");
}

void usbHostTick() {}

void stopUsbHost() {
    if (!usbHostInited) return;

    Serial.println("[USBH] Shutting down...");
    usbHostInited = false;
    vTaskDelay(pdMS_TO_TICKS(100));

    if (client_hdl) {
        usb_host_client_deregister(client_hdl);
        client_hdl = NULL;
    }
    usb_host_uninstall();

    usbReady = false;
}
