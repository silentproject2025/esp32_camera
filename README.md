# SANZXCAM v5.9 — README

## Deskripsi Proyek

Firmware kamera berbasis ESP32-S3 dengan display ILI9341 2.4" (320×240 landscape). Mendukung dua sensor kamera (GC2145 dan OV3660), menyimpan foto ke SD card dalam format JPG atau BMP, merekam video MJPEG, dan menyertakan fitur steganografi pada setiap file foto.

---

## Hardware

| Komponen | Detail |
|---|---|
| MCU | ESP32-S3 (Freenove ESP32-S3-WROOM) |
| Display | ILI9341 2.4", 320×240, landscape, via SPI |
| Touch | XPT2046, SPI shared |
| Kamera | GC2145 (utama) / OV3660 |
| Storage | SD Card via SDMMC 1-bit |
| LED | Pin 48 (indikator), Pin 2 (flash) |
| Tombol | BOOT (pin 0), B (pin 41), C (pin 3), D (pin 46) |

---

## Pin Map

### Kamera (DVP)
| Signal | Pin |
|---|---|
| XCLK | 15 |
| SIOD (SDA) | 4 |
| SIOC (SCL) | 5 |
| Y2–Y9 | 11,9,8,10,12,18,17,16 |
| VSYNC | 6 |
| HREF | 7 |
| PCLK | 13 |

### Display SPI
| Signal | Pin |
|---|---|
| MOSI | 45 |
| MISO | 42 |
| SCLK | 47 |
| DC | 14 |
| CS | 21 |
| RST | 1 |

### SD Card (SDMMC 1-bit)
| Signal | Pin |
|---|---|
| CLK | 39 |
| CMD | 38 |
| D0 | 40 |

---

## Mode Aplikasi

| Mode | Keterangan |
|---|---|
| MODE_VIEWFINDER | Live preview kamera, FPS counter, pill info |
| MODE_GALLERY | Daftar file JPG/BMP/VIDEO di SD card |
| MODE_PHOTO_VIEW | Lihat foto dengan zoom 1×/2×/4× dan pan |
| MODE_MJPEG_PLAYER | Putar video MJPEG dengan kontrol speed/loop |
| MODE_MENU_LED | Toggle flash LED saat capture |
| MODE_MENU_EXP | Pilih preset exposure (AUTO/MOON/NIGHT/MANUAL) |
| MODE_MENU_EXP_ADJ | Adjust manual exposure value dan gain |
| MODE_DIALOG_DELETE | Konfirmasi hapus file, timeout 8 detik |
| MODE_MENU_FORMAT | Pilih format foto GC2145: BMP atau JPG |

---

## Kontrol Tombol

### Viewfinder
| Tombol | Short Press | Long Press |
|---|---|---|
| BOOT | Ambil foto | Masuk USB MSC mode |
| B | Start/Stop rekam video | Menu LED flash |
| C | Buka Gallery | Menu Format (GC2145 saja) |
| D | Toggle Face Detect | Menu Exposure |

### Gallery
| Tombol | Aksi |
|---|---|
| BOOT | Buka file terpilih (foto/video) |
| B | Kembali ke Viewfinder |
| C (hold) | Scroll ke atas (akselerasi) |
| D (hold) | Scroll ke bawah (akselerasi) |

### Photo View
| Tombol | Short Press | Long Press |
|---|---|---|
| BOOT | Kembali ke Gallery | — |
| B | Cycle zoom (1×→2×→4×→1×) | Buka dialog hapus |
| C | Foto sebelumnya / pan kiri (zoom) | Pan ke atas (zoom) |
| D | Foto berikutnya / pan kanan (zoom) | Pan ke bawah (zoom) |

### MJPEG Player
| Tombol | Aksi |
|---|---|
| BOOT | Stop & kembali ke Gallery |
| B | Pause / Resume |
| C | Toggle loop |
| D | Cycle speed (0.5× / 1× / 2×) |

---

## Format File

### Foto JPG
- Kompresi JPEG quality 85
- Inject EXIF APP1: Make=SANZXCAM, Model=sensor, Software=v5.9, Timestamp
- Inject steganografi: COM marker `SANZXCAM|NNNN|v5.9`
- Nama file: `photo_NNNN.jpg`

### Foto BMP (GC2145 only)
- Raw RGB565 dikonversi ke BGR888 24-bit
- Fix byte-swap: big-endian ESP32 → little-endian standar BMP
- Inject steganografi: LSB bit Blue channel setiap pixel
- Nama file: `photo_NNNN.bmp`

### Video MJPEG
- Stream frame JPEG tanpa container header
- Frame rate target: 15 FPS
- Nama file: `video_NNNN.mjpeg`

---

## Fitur Steganografi

### JPEG — COM Marker
Payload disisipkan pada marker `0xFE` (COM) tepat setelah SOI (`0xFFD8`):

```
Format: SANZXCAM|NNNN|v5.9
Contoh: SANZXCAM|0042|v5.9
```

Ekstraksi menggunakan fungsi `stegoExtractFromJpeg()` yang men-scan marker sequence JPEG.

### BMP — LSB Blue Channel
Payload disisipkan ke bit LSB channel Blue (byte 0 dalam triplet BGR) dari setiap pixel, 1 bit per pixel, MSB first.

```
Kapasitas: lebar × tinggi bit (320×240 = 76.800 bit ≈ 9.600 karakter)
Max payload: 64 karakter
Format: SANZXCAM|NNNN|v5.9
```

---

## ⚠️ BUG KRITIS: Steganografi BMP Tidak Berfungsi Benar

Terdapat dua bug serius pada implementasi steganografi BMP di v5.9 yang menyebabkan **payload tidak dapat diekstrak kembali** dari file BMP yang sudah disimpan.

### Bug 1 — Konversi Bit Loss saat RGB565 → BGR888

**Lokasi:** `stegoBmpEmbed()` vs `saveBMP()`

`stegoBmpEmbed()` menyisipkan bit ke LSB dari `rgb565Buf[p*2+1]`:
- Byte ini adalah byte "lo" dalam pasangan big-endian kamera
- Dalam format RGB565: bit 0 dari byte lo = bit 0 dari field Blue (5 bit)

Kemudian `saveBMP()` melakukan konversi:
```cpp
uint16_t px = (raw << 8) | (raw >> 8);     // byte-swap
uint8_t b   = (px & 0x1F) << 3;            // expand 5-bit ke 8-bit, << 3
rowBuf[x*3] = b;
```

Operasi `<< 3` menyebabkan bit 0 dari field Blue (yang sudah dimodifikasi oleh embed) bergeser ke posisi bit 3 pada byte BGR888. Sehingga `rowBuf[x*3+0] & 0x01` di `stegoBmpExtract()` **tidak membaca bit yang sama** dengan yang di-embed. Bit LSB BGR888 sebenarnya berasal dari bit 3 asli field Blue RGB565, bukan bit yang di-embed.

### Bug 2 — Urutan Pixel Terbalik saat Embed vs Extract

**Lokasi:** `stegoBmpEmbed()` vs `stegoBmpExtract()` + `saveBMP()`

`stegoBmpEmbed()` mengiterasi pixel dalam urutan:
```
p=0 → row y=0 kamera → pixel[0,0]
p=1 → row y=0 kamera → pixel[0,1]
...
p=W → row y=1 kamera → pixel[1,0]
```

`saveBMP()` menulis dalam urutan BMP bottom-up:
```
row y=H-1 → ditulis pertama (offset awal data)
row y=H-2 → ditulis kedua
...
row y=0   → ditulis terakhir (offset akhir data)
```

`stegoBmpExtract()` membaca mulai dari `row = bmpH-1` turun ke 0:
```
row=bmpH-1 → offset awal data → row y=H-1 kamera (SALAH: seharusnya y=0)
```

Akibatnya, urutan pixel saat extract **terbalik** dibanding saat embed. Jika payload memiliki panjang > 1 karakter, bit akan direkonstruksi dalam urutan yang salah, menghasilkan payload yang korup.

### Solusi yang Benar

Ada dua pendekatan untuk memperbaiki kedua bug sekaligus:

**Opsi A — Embed setelah konversi ke BGR888 (di dalam saveBMP):**
```cpp
// Di saveBMP(), setelah hitung nilai b, g, r:
// Embed ke bit 0 (LSB) dari byte Blue (b) sebelum tulis ke rowBuf
int pixelIdx = (h - 1 - y) * w + x;  // karena saveBMP iterasi y dari h-1 ke 0
if (pixelIdx < totalPayloadBits) {
    int charIdx = pixelIdx / 8;
    int bitPos  = 7 - (pixelIdx % 8);
    uint8_t bit = (payload[charIdx] >> bitPos) & 0x01;
    b = (b & 0xFE) | bit;
}
rowBuf[x*3+0] = b;
```
Dengan cara ini, embed dan extract bekerja pada representasi yang sama (BGR888, urutan pixel identik dengan yang dibaca `stegoBmpExtract()`).

**Opsi B — Ubah stegoBmpExtract() agar membaca dalam urutan terbalik:**
Extract saat ini membaca dari `row=bmpH-1` turun ke 0 (= row y=H-1..0 kamera). Harus diubah membaca dari `row=0` naik ke `bmpH-1` (= row y=H-1..0 dari kamera... wait, ini masih terbalik). Yang benar: extract harus membaca dari `row=0` ke `bmpH-1` agar mendapat pixel dalam urutan yang sama dengan embed (p=0,1,2,...).

Tapi Bug 1 (konversi bit) tetap harus diperbaiki terpisah.

**Rekomendasi: Opsi A** — lebih bersih karena embed dilakukan tepat sebelum tulis, langsung pada nilai BGR888 final, tanpa perlu memikirkan byte order RGB565.

---

## Exposure Presets

| Preset | AEC | AEC2 | AEC Value | AGC | Gain | Ceiling | AE Level |
|---|---|---|---|---|---|---|---|
| AUTO | ON | ON | 300 | ON | 0 | 2 | 0 |
| MOON | OFF | OFF | 20 | OFF | 0 | 0 | -2 |
| NIGHT | OFF | ON | 1200 | OFF | 5 | 6 | +2 |
| MANUAL | OFF | OFF | user | OFF | user | 0 | 0 |

---

## Dynamic Island

Notifikasi animasi slide-in/out dari atas layar, mendukung stack hingga 3 pesan.

| Tipe | Ikon | Keterangan |
|---|---|---|
| NOTIF_OK | + | Operasi berhasil |
| NOTIF_FLASH | * | Flash LED toggle |
| NOTIF_REC | o | Recording aktif (blink dot) |
| NOTIF_FACE | @ | Face detect toggle |
| NOTIF_WARN | ! | Peringatan/error |
| NOTIF_INFO | i | Informasi umum |

Durasi tampil: 1000ms + animasi masuk 150ms + animasi keluar 150ms.

---

## USB Mass Storage

BOOT long press dari Viewfinder mengaktifkan mode USB MSC. SD card di-expose ke host sebagai USB drive. Keluar dengan BOOT press setelah host eject. VFS di-remount setelah keluar.

---

## EXIF Metadata (JPEG)

| Tag | ID | Nilai |
|---|---|---|
| ImageDescription | 0x010E | `photo_NNNN` |
| Make | 0x010F | `SANZXCAM` |
| Model | 0x0110 | Nama sensor |
| Software | 0x0131 | `v5.9` |
| DateTimeOriginal | 0x9003 | Timestamp dari millis() |

Format timestamp: `2025:01:01 HH:MM:SS` (relatif dari boot, bukan waktu nyata karena tidak ada RTC).

---

## Dependensi Library

| Library | Fungsi |
|---|---|
| esp32-camera | Driver kamera DVP |
| LovyanGFX v1 | Display ILI9341 + touch XPT2046 |
| TJpg_Decoder | Decode JPEG ke pixel buffer |
| MjpegClass | Stream MJPEG dari file |
| JPEGDEC | Decode frame MJPEG |
| human_face_detect | MSR01 + MNP01 face detection |
| USBMSC | USB Mass Storage Class |
| esp_vfs_fat | FAT filesystem via SDMMC |

---

## Changelog v5.9

- **[FIX-BMP-COLOR]** Perbaikan warna BMP dengan byte-swap RGB565 sebelum konversi ke BGR888
- **[FORMAT-SELECT]** Menu pilih format foto GC2145 (BMP/JPG) via C long press
- **[STEGO-BMP]** Implementasi steganografi LSB Blue channel untuk file BMP (lihat bug notes di atas)

---

## Changelog v5.8 (ringkasan)

- Simpan foto GC2145 sebagai BMP langsung dari raw RGB565
- Gallery mendukung tiga tipe file: JPG, BMP, VIDEO
- Animasi Dynamic Island smooth tanpa artefak kotak hitam
- Inject EXIF APP1 ke JPEG
- Steganografi via COM marker JPEG
- Fix: applyExpPreset() mempertahankan hmirror/vflip sensor
