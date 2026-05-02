# 📷 SANZXCAM v5.3

> ESP32-S3 camera project dengan viewfinder live, galeri foto/video, dan USB Mass Storage — dibangun di atas Freenove ESP32-S3-WROOM.

---

## Fitur

- **Live viewfinder** RGB565 full-frame ke display ILI9341 320×240
- **Capture foto** ke SD card (JPEG, auto-numbered `photo_XXXX.jpg`)
- **Record video** MJPEG ke SD card (`video_XXXX.mjpeg`)
- **Galeri** — browse foto & video langsung dari SD, scroll hold-to-fast-scroll
- **Photo viewer** — zoom 1×/2×/4×, pan, prev/next, delete
- **MJPEG player** — play/pause, loop, speed 0.5×/1×/2×
- **Face detection** — two-stage MSR01 + MNP01, overlay bracket di viewfinder
- **LED flash** — GPIO 2, on/off via menu
- **Exposure preset** — AUTO / MOON / NIGHT / MANUAL (dengan bar adjustment)
- **USB Mass Storage** — SD card mount ke PC via USB, tanpa cabut kartu
- **UI monochrome** — terminal aesthetic, full black/gray/white, pill labels, FPS counter

---

## Hardware

### Komponen

| Komponen | Keterangan |
|---|---|
| Freenove ESP32-S3-WROOM | Board utama dengan PSRAM |
| Display ILI9341 2.4" | 320×240, touchscreen XPT2046 |
| MicroSD card | Format FAT32 |
| 4× Tactile button | BOOT, B, C, D |
| LED flash | GPIO 2 (opsional) |

### Pin Display (SPI2)

| Signal | GPIO |
|---|---|
| MOSI | 45 |
| MISO | 42 |
| SCLK | 47 |
| DC | 14 |
| CS | 21 |
| RST | 1 |

### Pin SD Card (SDMMC 1-bit)

| Signal | GPIO |
|---|---|
| CLK | 39 |
| CMD | 38 |
| D0 | 40 |

### Pin Kamera

| Signal | GPIO |
|---|---|
| XCLK | 15 |
| SIOD (SDA) | 4 |
| SIOC (SCL) | 5 |
| VSYNC | 6 |
| HREF | 7 |
| PCLK | 13 |
| Y2–Y9 | 11,9,8,10,12,18,17,16 |

### Pin Tombol

| Tombol | GPIO |
|---|---|
| BOOT | 0 |
| B | 41 |
| C | 3 |
| D | 46 |

### Pin LED

| | GPIO |
|---|---|
| LED indikator | 48 |
| LED flash | 2 |

---

## Wiring Tips

> **Penting:** Jauhkan kabel tombol dari kabel clock kamera (PCLK, XCLK) dan SPI display (SCLK). Jarak minimal 3–5mm, hindari sejajar panjang.

PCLK kamera berjalan di 20MHz dan SPI display di 40MHz. Kabel tombol yang terlalu dekat bisa pickup noise frekuensi tinggi ini dan menghasilkan ghost press — tombol terasa menekan sendiri. Kalau kabel tombol harus panjang, twist kabel sinyal bersama kabel GND-nya.

Kalau masih ada ghost press setelah routing dirapikan, tambahkan kapasitor 100nF dari pin tombol ke GND sebagai hardware debounce.

---

## Dependencies (Arduino Library)

Install via Arduino Library Manager atau PlatformIO:

```
LovyanGFX
JPEGDEC
TJpg_Decoder
esp32-camera         (Espressif)
MjpegClass           (moononournation)
esp-dl               (untuk face detection)
```

Board package: **esp32 by Espressif** — pilih board `Freenove ESP32-S3-WROOM`.

---

## Cara Build & Flash

1. Install Arduino IDE 2.x atau PlatformIO
2. Install board package **esp32 by Espressif** versi ≥ 3.x
3. Install semua library di atas
4. Buka file `.ino`
5. Pilih board: **Freenove ESP32-S3-WROOM** (atau ESP32S3 Dev Module)
6. Setting penting:
   - **PSRAM:** OPI PSRAM
   - **Flash Size:** 8MB (atau sesuai board)
   - **USB Mode:** USB-OTG (untuk USB MSC)
   - **Partition Scheme:** 8M Flash (3MB APP / OTA)
   - **CPU Freq:** 240MHz
7. Upload — setelah upload tekan tombol BOOT di board kalau tidak auto-reset

---

## Kontrol Tombol

### Mode Viewfinder

| Tombol | Aksi |
|---|---|
| BOOT (short) | Capture foto |
| BOOT (long) | Masuk USB Mass Storage mode |
| B (short) | Start/Stop recording video |
| B (long) | Menu LED flash |
| C (short) | Buka galeri |
| C (long) | Toggle face detection |
| D (short) | Toggle face detection |
| D (long) | Menu exposure |

### Mode Galeri

| Tombol | Aksi |
|---|---|
| BOOT | Buka file terpilih (foto/video) |
| B | Kembali ke viewfinder |
| C (hold) | Scroll naik, makin lama makin cepat |
| D (hold) | Scroll turun, makin lama makin cepat |

### Mode Photo View

| Tombol | Aksi |
|---|---|
| BOOT | Kembali ke galeri |
| B (short) | Ganti zoom level (1× → 2× → 4× → 1×) |
| B (long) | Dialog hapus file |
| C / D | Prev/Next foto (zoom 1×), Pan kiri/kanan (zoom 2×/4×) |
| C long / D long | Pan atas/bawah (saat zoom > 1×) |

### Mode MJPEG Player

| Tombol | Aksi |
|---|---|
| BOOT | Kembali ke galeri |
| B | Play / Pause |
| C | Toggle loop |
| D | Ganti kecepatan (0.5× → 1× → 2×) |

---

## Sensor Kamera yang Didukung

| Sensor | PID | XCLK | Keterangan |
|---|---|---|---|
| GC2145 | 0x2145 | 20MHz | Default, hmirror ON |
| OV3660 | 0x3660 | 24MHz | vflip ON |
| Lain | auto-detect | 20MHz | Setting default |

---

## Struktur File SD Card

```
/sdcard/
  photo_0001.jpg
  photo_0002.jpg
  ...
  video_0001.mjpeg
  video_0002.mjpeg
  ...
```

Penamaan auto-numbered, tidak akan overwrite file lama selama nomor urut masih ada.

---

## Troubleshooting

### Ghost press / tombol tekan sendiri
Penyebab paling umum: **kabel tombol terlalu dekat dengan kabel clock kamera atau SPI display.**
Fix: pisahkan routing kabel, jarak minimal 3–5mm, hindari sejajar. Kalau perlu tambah kapasitor 100nF dari pin tombol ke GND.

### SD card tidak terdeteksi
- Pastikan format FAT32
- Coba kurangi `host.max_freq_khz` dari `SDMMC_FREQ_DEFAULT` ke `SDMMC_FREQ_PROBING`
- Pastikan pull-up internal aktif (`SDMMC_SLOT_FLAG_INTERNAL_PULLUP`)

### Kamera tidak init / layar hitam
- Cek koneksi XCLK dan SIOD/SIOC
- Lihat Serial Monitor 115200 baud — ada log PID sensor
- Kalau PID `0x0000`, kemungkinan koneksi I2C kamera putus

### USB MSC tidak muncul di PC
- Pastikan build dengan **USB Mode: USB-OTG**
- Gunakan kabel data (bukan kabel charge saja)
- Eject dulu dari PC sebelum keluar dari USB mode di device

### Face detection lambat / FPS turun drastis
Normal — face detection (MSR01 + MNP01) berat. FPS bisa turun ke 3–5fps saat aktif. Matikan dengan tombol D atau C long press saat tidak dibutuhkan.

### MJPEG player patah-patah
- Coba kecepatan 0.5× dulu
- File MJPEG yang di-record dari device ini sendiri sudah optimal untuk playback-nya
- File MJPEG dari sumber lain mungkin frame rate atau resolusinya berbeda

---

## Lisensi

Proyek ini dibuat untuk keperluan pribadi/eksperimen. Library yang digunakan memiliki lisensi masing-masing — cek repositori library terkait.
