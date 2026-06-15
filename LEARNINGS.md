# Radio Mode Enhancements (v6.1+)

- Manual frequency tuning implemented for RDA5807M by modifying `radioFreq` global and calling `radio.setFrequency()`.
- Short press of BTN_C/BTN_D adjusts frequency by 0.1 MHz (represented as 10 units in `radioFreq` which is in 10kHz units, e.g., 10070 = 100.70 MHz).
- Long press of BTN_C/BTN_D triggers hardware seek using `radio.seekDown(true)` and `radio.seekUp(true)`.
- UI hint updated to "C/D: Tune (Hold:Seek) B:Vol (Hold:Exit)".

- Manual frequency input dialog for FM Radio (MODE_RADIO_FREQ_INPUT) allows users to set specific frequencies (50.00 - 115.00 MHz) using a 5-digit interface.
- Replaced auto-scan (long-press BOOT) with manual frequency input to give users precise control over the RDA5807M receiver.
- Frequency range 50-115 MHz enabled by setting the RDA5807M band to 3 (`radio.setBand(3)`).

## SANZXCAM v6.2 Learnings
- **Dynamic Camera Resolution:** Temporarily switching `framesize_t` during capture allows high-quality photos/videos while maintaining a low-latency, low-resolution viewfinder. Always ensure a sufficient delay and frame flushing after a resolution change.
- **USB Host Precedence:** When implementing multi-storage systems on microcontrollers, defining a `storageRoot` global and using it in all `fopen`/`opendir` calls simplifies redirection.
- **LovyanGFX API:** The `pushImageRotateZoom` function signature is: `pushImageRotateZoom(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, data)`.
- **Merged Directory Scanning:** Use a lambda or helper function to scan multiple roots (`/sdcard`, `/usb`) and aggregate results into a single index to support unified galleries.

## USB Host Force Detection
- Added "Force Detect USB" as the 14th item in the Experimental Features menu in `camera_test.ino`.
- The feature resets the USB Host stack by calling `stopUsbHost()`, `delay(300)`, and `initUsbHost()`.
- UI displays "READY" if `usbReady` is true, otherwise "WAIT".
- Menu layout was adjusted by reducing `startY` from 25 to 22 to accommodate the additional item on the 240px vertical display.
