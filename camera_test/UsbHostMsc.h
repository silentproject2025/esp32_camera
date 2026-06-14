#ifndef USB_HOST_MSC_H
#define USB_HOST_MSC_H

#include <Arduino.h>

// [SANZXCAM v6.1] USB Host MSC Driver
// Uses ESP-IDF USB Host stack to mount external USB Flash Drives to /usb

extern bool usbReady;

void initUsbHost();
void usbHostTick();
void stopUsbHost();

// Forward declaration for UI feedback
void notifyUsbState(bool connected);

#endif
