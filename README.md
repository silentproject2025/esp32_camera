```
╔═══════════════════════════════════════════════════════════╗
║   ███████╗ █████╗ ███╗   ██╗███████╗██╗  ██╗ ██████╗    ║
║  ██╔════╝██╔══██╗████╗  ██║╚══███╔╝╚██╗██╔╝██╔════╝    ║
║  ███████╗███████║██╔██╗ ██║  ███╔╝  ╚███╔╝ ██║         ║
║  ╚════██║██╔══██║██║╚██╗██║ ███╔╝   ██╔██╗ ██║         ║
║  ███████║██║  ██║██║ ╚████║███████╗██╔╝ ██╗╚██████╗    ║
║  ╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝ ╚═════╝    ║
║                                                           ║
║  v5.3-nonblocking-FIXED  ·  ESP32-S3  ·  4-BTN edition  ║
╚═══════════════════════════════════════════════════════════╝
```

> **ESP32-S3 camera system** — live viewfinder, foto & video capture ke SD, galeri built-in, USB mass storage, dan face detection. Monochrome terminal aesthetic. No cloud. No WiFi. Just shoot.

---

## `$ system --info`

```
BOARD    : Freenove ESP32-S3-WROOM
DISPLAY  : ILI9341 2.4" 320×240 landscape
STORAGE  : MicroSD via SDMMC 1-bit
SENSOR   : GC2145 / OV3660 (auto-detect)
BUTTONS  : 4x tactile — BOOT / B / C / D
UI       : Monochrome · black/gray/white · terminal
VERSION  : v5.3-nonblocking-FIXED
```

---

## `$ features --list`

| Module | Status | Keterangan |
|---|---|---|
| Live Viewfinder | `[ACTIVE]` | RGB565 full-frame, FPS counter real-time |
| Foto Capture | `[ACTIVE]` | JPEG ke SD, auto-numbered `photo_XXXX.jpg` |
| Video Record | `[ACTIVE]` | MJPEG ke SD, timer overlay, `video_XXXX.mjpeg` |
| Gallery Browser | `[ACTIVE]` | Scroll dengan hold-to-accelerate |
| Photo Viewer | `[ACTIVE]` | Zoom 1×/2×/4×, pan, prev/next, delete |
| MJPEG Player | `[ACTIVE]` | Play/pause, loop, speed 0.5×/1×/2× |
| Face Detection | `[ACTIVE]` | MSR01 + MNP01 two-stage, bracket overlay |
| LED Flash | `[ACTIVE]` | GPIO 2, toggle via menu |
| Exposure Preset | `[ACTIVE]` | AUTO / MOON / NIGHT / MANUAL + bar adjustment |
| USB Mass Storage | `[ACTIVE]` | SD card mount ke PC tanpa cabut kartu |
| Non-blocking BTN | `[ACTIVE]` | Debounce 80ms, long press 1500ms, cooldown 100ms |

---

## `$ hardware --pinout`

### Display — SPI2

```
MOSI  ──── GPIO 45
MISO  ──── GPIO 42
SCLK  ──── GPIO 47
DC    ──── GPIO 14
CS    ──── GPIO 21
RST   ──── GPIO  1
```

### SD Card — SDMMC 1-bit

```
CLK   ──── GPIO 39
CMD   ──── GPIO 38
D0    ──── GPIO 40
```

### Camera

```
XCLK  ──── GPIO 15      VSYNC ──── GPIO  6
SIOD  ──── GPIO  4      HREF  ──── GPIO  7
SIOC  ──── GPIO  5      PCLK  ──── GPIO 13
Y9    ──── GPIO 16      Y8    ──── GPIO 17
Y7    ──── GPIO 18      Y6    ──── GPIO 12
Y5    ──── GPIO 10      Y4    ──── GPIO  8
Y3    ──── GPIO  9      Y2    ──── GPIO 11
```

### Buttons & LED

```
BTN_BOOT  ──── GPIO  0    (INPUT_PULLUP)
BTN_B     ──── GPIO 41    (INPUT_PULLUP)
BTN_C     ──── GPIO  3    (INPUT_PULLUP)
BTN_D     ──── GPIO 46    (INPUT_PULLUP)
LED_IND   ──── GPIO 48    (status blink)
LED_FLASH ──── GPIO  2    (flash capture)
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

Install via Arduino Library Manager:

```
LovyanGFX
JPEGDEC
TJpg_Decoder
esp32-camera        (Espressif)
MjpegClass          (moononournation)
esp-dl              (Espressif, untuk face detection)
```

Board package:
```
esp32 by Espressif  >=  v3.x
```

---

## `$ build --flash`

```bash
# 1. Buka Arduino IDE 2.x

# 2. Board settings:
#    Board          : Freenove ESP32-S3-WROOM
#    PSRAM          : OPI PSRAM
#    Flash Size     : 8MB
#    USB Mode       : USB-OTG
#    Partition      : 8M Flash (3MB APP/OTA)
#    CPU Frequency  : 240MHz

# 3. Upload
#    Kalau tidak auto-reset → tahan BOOT → tekan RESET → lepas keduanya
```

Serial monitor untuk debug: **115200 baud**

```
=== Sanzxcam v5.3-nonblocking-FIXED 4-BTN ===
SD CARD ............ OK  32MB
CAM PROBE .......... 20MHz
SENSOR PID ......... 0x2145  GC2145
XCLK ............... 20MHz  OK
BUTTONS ............ BOOT/B/C/D  non-blocking
MJPEG PLAY ......... MjpegClass+JPEGDEC
READY
```

---

## `$ usage --guide`

### Mode: VIEWFINDER
*Default mode setelah boot. Live preview dari kamera.*

```
┌─────────────────────────────────────┐
│ [fps pill]          [sensor pill]   │
│                                     │
│   ┌─                           ─┐   │
│   │   << LIVE VIEWFINDER >>    │   │
│   │                             │   │
│   └─                           ─┘   │
│                                     │
│ [shot# pill]         [SD pill]      │
└─────────────────────────────────────┘

  BOOT (tap)   → capture foto → preview 2 detik → simpan ke SD
  BOOT (hold)  → masuk USB Mass Storage mode
  B    (tap)   → start/stop recording video MJPEG
  B    (hold)  → menu LED flash (ON/OFF)
  C    (tap)   → buka galeri
  C    (hold)  → toggle face detection
  D    (tap)   → toggle face detection
  D    (hold)  → menu exposure preset
```

### Mode: GALLERY
*Browser file di SD card — foto dan video.*

```
┌─────────────────────────────────────┐
│ GALLERY  12 item              B=BACK│
├─────────────────────────────────────┤
│   1  □ photo_0001.jpg            >  │
│   2  ▶ video_0001.mjpeg          >  │  ← ▶ = video
│   3  □ photo_0002.jpg            >  │
│  [4  □ photo_0003.jpg            >] │  ← [ ] = selected
│   5  □ photo_0004.jpg            >  │
│  ...                                │
└─────────────────────────────────────┘

  BOOT       → buka file terpilih
  B          → kembali ke viewfinder
  C (hold)   → scroll naik — makin lama makin cepat
  D (hold)   → scroll turun — makin lama makin cepat
```

### Mode: PHOTO VIEWER
*Lihat foto full-screen dari SD card.*

```
  B=zoom                      BOOT=back
  ┌──────────────────────────────────┐
  │                                  │
  │         [ foto ]                 │
  │                                  │
  └──────────────────────────────────┘
  C=prev                      D=next
         < 3 / 8 >  photo_0003.jpg

  B (tap)    → cycle zoom: 1× → 2× → 4× → 1×
  B (hold)   → dialog hapus file (konfirmasi BOOT=YES)
  C / D      → prev / next foto   (saat zoom 1×)
  C / D      → pan kiri / kanan   (saat zoom 2× / 4×)
  C/D (hold) → pan atas / bawah   (saat zoom 2× / 4×)
  BOOT       → kembali ke galeri
```

### Mode: MJPEG PLAYER
*Putar video hasil rekaman.*

```
  [1.0×  -]
  ┌──────────────────────────────────┐
  │                                  │
  │         [ video ]                │
  │                                  │
  └──────────────────────────────────┘

  BOOT   → stop, kembali ke galeri
  B      → play / pause
  C      → toggle loop (LOOP ON / LOOP OFF)
  D      → ganti kecepatan: 0.5× → 1.0× → 2.0×
```

### Mode: EXPOSURE MENU
*D long press dari viewfinder.*

```
  AUTO    → kamera atur otomatis (default)
  MOON    → exposure rendah, untuk objek terang / bulan
  NIGHT   → exposure tinggi, untuk kondisi gelap
  MANUAL  → atur sendiri via slider EXP + GAIN

  C / D  → pilih preset
  BOOT   → konfirmasi
  B      → batal
```

### Mode: USB MASS STORAGE
*BOOT long press dari viewfinder. SD card mount ke PC.*

```
  ┌──────────────────────────────────┐
  │       USB MASS STORAGE           │
  │          [USB icon]              │
  │        SD CONNECTED              │
  │       to host via USB            │
  │                                  │
  │  SENSOR  GC2145                  │
  │  STORAGE 32MB  FAT32             │
  │  PHOTOS  0012 files              │
  │                                  │
  │   eject from host first          │
  │   [ BOOT ]  exit usb mode        │
  └──────────────────────────────────┘

  ! Eject dari PC dulu sebelum tekan BOOT untuk keluar
```

---

## `$ storage --structure`

```
/sdcard/
├── photo_0001.jpg
├── photo_0002.jpg
├── photo_XXXX.jpg    ← auto-numbered, tidak overwrite
├── video_0001.mjpeg
├── video_0002.mjpeg
└── video_XXXX.mjpeg
```

---

## `$ sensors --supported`

```
PID 0x2145  GC2145  ·  XCLK 20MHz  ·  hmirror ON   vflip OFF
PID 0x3660  OV3660  ·  XCLK 24MHz  ·  hmirror OFF  vflip ON
PID 0xXXXX  UNKNOWN ·  XCLK 20MHz  ·  default settings
```

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
      → Eject dari PC dulu sebelum keluar USB mode

[WARN] Face detection — FPS turun ke 3–5fps
       → Normal. MSR01+MNP01 berat di CPU.
       → Matikan dengan D tap atau C hold saat tidak perlu.

[WARN] MJPEG player patah-patah
       → Coba kecepatan 0.5× dulu
       → File dari sumber lain mungkin frame rate-nya berbeda
       → File hasil rekaman device ini sendiri paling optimal
```

---

## `$ button --reference`

```
╔══════════╦═══════════╦════════════════════════════════════╗
║  BUTTON  ║   TYPE    ║  ACTION                            ║
╠══════════╬═══════════╬════════════════════════════════════╣
║          ║ tap       ║ capture foto                       ║
║   BOOT   ║ hold      ║ USB mass storage mode              ║
║          ║ (dialog)  ║ konfirmasi hapus file              ║
╠══════════╬═══════════╬════════════════════════════════════╣
║          ║ tap       ║ start/stop record (viewfinder)     ║
║    B     ║ hold      ║ menu LED flash                     ║
║          ║ tap       ║ zoom cycle (photo view)            ║
║          ║ hold      ║ dialog hapus (photo view)          ║
╠══════════╬═══════════╬════════════════════════════════════╣
║          ║ tap       ║ buka galeri (viewfinder)           ║
║    C     ║ hold      ║ toggle face detect (viewfinder)    ║
║          ║ hold      ║ scroll / pan                       ║
╠══════════╬═══════════╬════════════════════════════════════╣
║          ║ tap       ║ toggle face detect (viewfinder)    ║
║    D     ║ hold      ║ menu exposure                      ║
║          ║ tap/hold  ║ scroll / pan                       ║
╚══════════╩═══════════╩════════════════════════════════════╝
```

---

*SANZXCAM — built for fun, runs offline, shoots in the dark.*
