#include "UsbHostMsc.h"
#include "usb/usb_host.h"
#include "esp_log.h"

static const char *TAG = "USB_HOST_MSC";
static bool usbHostInited = false;
static usb_host_client_handle_t client_hdl = NULL;

static void client_event_cb(const usb_host_client_event_msg_t *event_msg, void *arg) {
    if (event_msg->event == USB_HOST_CLIENT_EVENT_NEW_DEV) {
        ESP_LOGI(TAG, "New device connected");
        // In a full implementation, we would open the device and check for MSC class
        // For now, we signal that a device might be available
        usbReady = true;
    } else if (event_msg->event == USB_HOST_CLIENT_EVENT_DEV_GONE) {
        ESP_LOGI(TAG, "Device disconnected");
        usbReady = false;
    }
}

void initUsbHost() {
    if (usbHostInited) return;

    ESP_LOGI(TAG, "Installing USB Host Library");
    usb_host_config_t host_config = {
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    esp_err_t err = usb_host_install(&host_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install USB Host: %d", err);
        return;
    }

    usb_host_client_config_t client_config = {
        .is_synchronous = false,
        .max_num_event_msg = 5,
        .async = {
            .client_event_callback = client_event_cb,
            .callback_arg = NULL,
        }
    };
    err = usb_host_client_register(&client_config, &client_hdl);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register client: %d", err);
        return;
    }

    usbHostInited = true;
}

void usbHostTick() {
    if (!usbHostInited) return;

    uint32_t event_flags;
    usb_host_lib_handle_events(0, &event_flags);
    usb_host_client_handle_events(client_hdl, 0);
}

void stopUsbHost() {
    if (!usbHostInited) return;

    // Simplification: We don't fully uninstall to avoid crash on re-init
    // But we stop the client to release the USB peripheral for Device mode
    if (client_hdl) usb_host_client_deregister(client_hdl);
    usb_host_uninstall();

    client_hdl = NULL;
    usbHostInited = false;
    usbReady = false;
}
