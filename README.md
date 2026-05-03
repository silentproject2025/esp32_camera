```
╔═══════════════════════════════════════════════════════════╗
║   ███████╗ █████╗ ███╗   ██╗███████╗██╗  ██╗ ██████╗    ║
║  ██╔════╝██╔══██╗████╗  ██║╚══███╔╝╚██╗██╔╝██╔════╝    ║
║  ███████╗███████║██╔██╗ ██║  ███╔╝  ╚███╔╝ ██║         ║
║  ╚════██║██╔══██║██║╚██╗██║ ███╔╝   ██╔██╗ ██║         ║
║  ███████║██║  ██║██║ ╚████║███████╗██╔╝ ██╗╚██████╗    ║
║  ╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝ ╚═════╝    ║
║                                                           ║
║  v5.8           ·  ESP32-S3  ·  4-BTN edition  ║
╚═══════════════════════════════════════════════════════════╝
```

> **ESP32-S3 camera system** — Live viewfinder, high-speed BMP/JPG capture, video recording, internal gallery, USB Mass Storage, and local face detection. Monochrome terminal aesthetic. Zero cloud. Just hardware.

---

## `$ system --info`

```
BOARD    : Freenove ESP32-S3-WROOM (N16R8)
DISPLAY  : ILI9341 2.4" 320×240 landscape (SPI2)
STORAGE  : MicroSD via SDMMC 1-bit
SENSOR   : GC2145 / OV3660 (auto-detect)
BUTTONS  : 4x tactile — BOOT / B / C / D
UI       : Monochrome · Dynamic Island Notifications
VERSION  : v5.8
```

---

## `$ features --list`

| Module | Status | Keterangan |
|---|---|---|
| **Dynamic Island** | `[NEW]` | Smooth non-blocking UI notifications for system events |
| **BMP Capture** | `[NEW]` | Direct RGB565 to BMP24 for GC2145 (lossless quality) |
| **EXIF & Stego** | `[NEW]` | APP1 EXIF injection + COM marker steganography (JPG) |
| **Gallery Pro** | `[UPD]` | Triple format support: JPG / BMP / VIDEO with icons |
| Live Viewfinder | `[ACTIVE]` | RGB565 full-frame, real-time FPS & Sensor ID |
| Video Record | `[ACTIVE]` | MJPEG to SD, timer overlay, `video_XXXX.mjpeg` |
| Photo Viewer | `[ACTIVE]` | Zoom 1×/2×/4×, pan, delete dialog |
| MJPEG Player | `[ACTIVE]` | Play/pause, loop, variable speed (0.5×/1×/2×) |
| Face Detection | `[ACTIVE]` | MSR01 + MNP01 two-stage, bracket & keypoint overlay |
| USB Mass Storage| `[ACTIVE]` | Mount SD card directly to PC via USB-OTG |

---

## `$ hardware --pinout`

### Display — SPI2
```
MOSI  ──── GPIO 45      DC    ──── GPIO 14
MISO  ──── GPIO 42      CS    ──── GPIO 21
SCLK  ──── GPIO 47      RST   ──── GPIO  1
```

### SD Card — SDMMC 1-bit
```
CLK   ──── GPIO 39      CMD   ──── GPIO 38
D0    ──── GPIO 40
```

### Camera (QVGA RGB565)
```
XCLK  ──── GPIO 15      VSYNC ──── GPIO  6
SIOD  ──── GPIO  4      HREF  ──── GPIO  7
SIOC  ──── GPIO  5      PCLK  ──── GPIO 13
D2-D9 ──── GPIO 11, 9, 8, 10, 12, 18, 17, 16
```

### Buttons & LED
```
BTN_BOOT  ──── GPIO  0    (Shutter / USB Mode)
BTN_B     ──── GPIO 41    (Record / Flash / Zoom)
BTN_C     ──── GPIO  3    (Gallery / Face / Prev)
BTN_D     ──── GPIO 46    (Face / Exposure / Next)
LED_IND   ──── GPIO 48    (Status Blink)
LED_FLASH ──── GPIO  2    (Flash Capture)
```

---

## `$ wiring --warnings`

```
⚠  CROSSTALK ALERT
   Jauhkan kabel tombol dari kabel clock kamera (PCLK/XCLK)
   dan SPI display (SCLK). Jarak minimal 3–5mm.
   Kabel nempel 1cm saja sudah cukup bikin ghost press.

   PCLK kamera  : 20 MHz  ─┐
   XCLK kamera  : 20 MHz   ├─ sumber noise utama
   SPI display  : 40 MHz  ─┘

   FIX #1 : pisah routing, jangan sejajar panjang
   FIX #2 : twist kabel tombol + GND bersama
   FIX #3 : kapasitor 100nF dari pin tombol ke GND
```

---

## `$ dependencies --install`

### Required Libraries
1. **LovyanGFX** (v1.x) - High performance display driver.
2. **JPEGDEC** - Fast JPEG decoding for MJPEG and Photo View.
3. **TJpg_Decoder** - Used for high-quality full image decoding.
4. **esp32-camera** - Built-in Espressif camera driver.
5. **esp-dl** - Required for Face Detection (MSR01/MNP01).

### Board Settings (Arduino IDE)
- **Board**: Freenove ESP32-S3-WROOM
- **PSRAM**: OPI PSRAM (Crucial for 320x240 RGB565 processing)
- **Flash Size**: 16MB (or at least 8MB)
- **USB Mode**: USB-OTG (Required for Mass Storage)
- **Partition Scheme**: 16M Flash (3MB APP / 9MB FAT) or similar.

---

## `$ usage --guide`

### 1. Viewfinder Mode (Default)
*   **BOOT (Tap)**: Capture Photo (GC2145 saves as **.bmp**, others as **.jpg**).
*   **BOOT (Hold)**: Enter **USB Mass Storage** mode.
*   **B (Tap)**: Start/Stop Video Recording (**MJPEG**).
*   **B (Hold)**: Flash LED Menu.
*   **C (Tap)**: Open Gallery.
*   **C (Hold)**: Toggle Face Detection.
*   **D (Tap)**: Toggle Face Detection.
*   **D (Hold)**: Exposure Preset Menu (**AUTO/MOON/NIGHT/MANUAL**).

### 2. Gallery Mode
*   **C / D (Hold)**: Scroll up/down with acceleration.
*   **BOOT**: Open selected file (Photo or Video).
*   **B**: Back to Viewfinder.

---

## `$ troubleshoot --run`

```
[ERR] Ghost press — tombol tekan sendiri
      → Cek routing kabel tombol vs kabel clock
      → Pisah minimal 3–5mm, jangan sejajar
      → Tambah kapasitor 100nF dari pin tombol ke GND

[ERR] SD card not found
      → Pastikan format FAT32
      → Coba turunkan SDMMC freq ke SDMMC_FREQ_PROBING
      → Cek pull-up: SDMMC_SLOT_FLAG_INTERNAL_PULLUP harus aktif

[ERR] Camera init failed — layar hitam
      → Cek kabel XCLK dan SIOD/SIOC (I2C kamera)
      → Buka Serial Monitor 115200 — lihat log PID
      → PID 0x0000 = koneksi I2C putus

[ERR] USB MSC tidak muncul di PC
      → Build harus pakai USB Mode: USB-OTG
      → Pakai kabel data (bukan kabel charge only)

[WARN] Face detection — FPS turun ke 3–5fps
       → Normal. MSR01+MNP01 berat di CPU.
```

---

## `$ technical --details`

### BMP Capture (v5.8)
For the GC2145 sensor, the system skips JPEG compression and writes the RGB565 buffer directly to a **BMP24** file. This preserves maximum detail and avoids compression artifacts.

### EXIF & Steganography
JPEGs captured by other sensors include:
- **EXIF Data**: Injected APP1 segment containing Make (SANZXCAM), Model, and Timestamp.
- **Steganography**: Hidden payload in the COM marker (FF FE) containing the device version and photo serial number.

---

*SANZXCAM — Local. Secure. Raw.*
