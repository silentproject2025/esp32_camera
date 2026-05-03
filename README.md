# 📷 SANZXCAM

> **ESP32-S3 Camera System** — Terminal aesthetic, monochrome UI, steganografi tersembunyi.

```
 ███████╗ █████╗ ███╗   ██╗███████╗██╗  ██╗ ██████╗ █████╗ ███╗   ███╗
 ██╔════╝██╔══██╗████╗  ██║╚══███╔╝╚██╗██╔╝██╔════╝██╔══██╗████╗ ████║
 ███████╗███████║██╔██╗ ██║  ███╔╝  ╚███╔╝ ██║     ███████║██╔████╔██║
 ╚════██║██╔══██║██║╚██╗██║ ███╔╝   ██╔██╗ ██║     ██╔══██║██║╚██╔╝██║
 ███████║██║  ██║██║ ╚████║███████╗██╔╝ ██╗╚██████╗██║  ██║██║ ╚═╝ ██║
 ╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝╚═╝     ╚═╝
                                                              v5.9-fix
```

[![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue?style=flat-square)](https://www.espressif.com/en/products/socs/esp32-s3)
[![Display](https://img.shields.io/badge/display-ILI9341%20320×240-lightgrey?style=flat-square)](#hardware)
[![Sensor](https://img.shields.io/badge/sensor-GC2145%20%7C%20OV3660-green?style=flat-square)](#hardware)
[![Battery](https://img.shields.io/badge/battery-3.7V%202300mAh-yellow?style=flat-square)](#-baterai--power)
[![Version](https://img.shields.io/badge/version-v5.9--fix-orange?style=flat-square)](#changelog)
[![UI](https://img.shields.io/badge/UI-monochrome%20terminal-black?style=flat-square)](#ui-theme)

---

## Daftar Isi

- [Tentang](#-tentang)
- [Hardware](#-hardware)
- [Baterai & Power](#-baterai--power)
- [Pinout](#-pinout)
- [Fitur Lengkap](#-fitur-lengkap)
- [Peta Tombol](#-peta-tombol)
- [Mode Aplikasi](#-mode-aplikasi)
- [Steganografi](#-steganografi)
- [Format Foto](#-format-foto)
- [Boot Animation](#-boot-animation)
- [Dynamic Island](#-dynamic-island)
- [Changelog v5.9-fix](#-changelog-v59-fix)
- [Dependensi](#-dependensi)

---

## 🎯 Tentang

SANZXCAM adalah firmware kamera berbasis **Freenove ESP32-S3-WROOM** dengan display ILI9341 2.4". Didesain dengan estetika **terminal monochrome** — full hitam/abu/putih — dan dilengkapi fitur-fitur seperti steganografi otomatis, injeksi EXIF, perekaman video MJPEG, face detection, dan USB Mass Storage.

---

## 🔧 Hardware

| Komponen | Spesifikasi |
|---|---|
| **Board** | Freenove ESP32-S3-WROOM |
| **CPU** | ESP32-S3 @ 240MHz dual-core |
| **PSRAM** | 8MB |
| **Display** | ILI9341 2.4" — 320×240 landscape |
| **Touchscreen** | XPT2046 (resistif) |
| **Sensor Kamera** | GC2145 atau OV3660 (auto-detect) |
| **Resolusi** | QVGA 320×240 (RGB565) |
| **Storage** | MicroSD via SDMMC 1-bit |
| **USB** | USB Mass Storage (MSC) |
| **LED** | Status LED (pin 48) + Flash LED (pin 2) |
| **Baterai** | Li-ion 3.7V 2300mAh |
| **Power Module** | MH-CD42 (IP5306) — charge + boost + proteksi |
| **Output Power** | 5V 2.1A ke board |

---

## 🔋 Baterai & Power

### Skema Power Chain

```
[USB 5V] ──► [MH-CD42] ──► [5V OUT] ──► [ESP32-S3]
                │                              │
          [Li-ion 3.7V]              (+ Display + SD + LED)
           2300mAh
```

### Baterai

| Spesifikasi | Nilai |
|---|---|
| **Kimia** | Li-ion (Lithium Ion) |
| **Tegangan Nominal** | 3.7V |
| **Tegangan Penuh** | 4.2V |
| **Kapasitas** | 2300mAh |
| **Energi** | ~8.5Wh |

### MH-CD42 Power Module

MH-CD42 menggunakan chip **IP5306** — solusi power management yang menangani charging, boosting, dan proteksi dalam satu modul berukuran 16×25mm.

| Spesifikasi | Nilai |
|---|---|
| **Chip** | IP5306 (atau klon FM5324GA) |
| **Input Charging** | DC 4.5V – 5.5V (rekomendasi 5V) |
| **Arus Charging** | hingga 2.1A |
| **Output** | 5V fixed |
| **Arus Output Maks** | 2.1A continuous |
| **Cut-off Charge** | 4.2V ±1% |
| **Cut-off Discharge** | 2.8–2.9V (over-discharge protection) |
| **Ukuran Modul** | 16 × 25mm |

### Fitur Proteksi MH-CD42

MH-CD42 dilengkapi proteksi overcharge, over-discharge, dan short-circuit — aman untuk penggunaan jangka panjang selama dioperasikan dalam batas spesifikasi.

| Proteksi | Status |
|---|---|
| Overcharge | ✅ |
| Over-discharge | ✅ (cutoff di 2.8–2.9V) |
| Short-circuit | ✅ |
| Over-temperature (chip) | ✅ |

### Indikator Baterai (4 LED SMD)

Empat LED merah SMD menampilkan status charging dan state of charge baterai dalam increment 25%.

| Kondisi | LED | Keterangan |
|---|---|---|
| < 25% | 1 LED blink | Baterai hampir habis |
| 25–49% | 2 LED (1 solid + 1 blink) | — |
| 50–74% | 3 LED | — |
| 75–100% | 4 LED solid | Penuh |
| Charging | LED blink bertahap | Menandakan proses charge |

### ⚠️ Catatan Penting

> Saat charging, chip membutuhkan power supply eksternal mampu deliver hingga **22W / 4.4A** karena charging baterai (2.1A) dan output ke beban (2.4A) berjalan bersamaan dari VIN.

> Jika beban sangat ringan (< 45mA) selama lebih dari **32 detik**, modul akan **auto power-off** — ini adalah fitur deteksi beban otomatis dari IP5306. Pastikan ESP32-S3 selalu menarik arus cukup saat beroperasi.

> Saat power supply disambung atau dicabut, akan ada **interupsi singkat** pada output karena MH-CD42 perlu switch power path antara VIN dan baterai.

### Estimasi Daya Tahan Baterai

| Kondisi | Estimasi |
|---|---|
| Viewfinder aktif (no flash) | ~2–2.5 jam |
| Recording MJPEG aktif | ~1.5–2 jam |
| Flash LED sering dipakai | ~1–1.5 jam |
| Idle / menu navigasi | ~3–4 jam |

> *Estimasi kasar. Aktual tergantung kondisi baterai, suhu, dan efisiensi boost converter (~85–90%).*

---

## 📌 Pinout

### Kamera

| Sinyal | GPIO |
|---|---|
| XCLK | 15 |
| SIOD (SDA) | 4 |
| SIOC (SCL) | 5 |
| VSYNC | 6 |
| HREF | 7 |
| PCLK | 13 |
| Y2–Y9 | 11, 9, 8, 10, 12, 18, 17, 16 |

### Display SPI (ILI9341 + XPT2046)

| Sinyal | GPIO |
|---|---|
| MOSI | 45 |
| MISO | 42 |
| SCLK | 47 |
| DC | 14 |
| CS (LCD) | 21 |
| RST | 1 |

### SD Card (SDMMC 1-bit)

| Sinyal | GPIO |
|---|---|
| CMD | 38 |
| CLK | 39 |
| D0 | 40 |

### Tombol & LED

| Fungsi | GPIO |
|---|---|
| BTN BOOT | 0 |
| BTN B | 41 |
| BTN C | 3 |
| BTN D | 46 |
| LED Status | 48 |
| LED Flash | 2 |

---

## ✨ Fitur Lengkap

### 📸 Kamera & Capture
- **Live viewfinder** 320×240 real-time
- **Auto-detect sensor**: GC2145 atau OV3660
- **Format foto pilihan** (GC2145): BMP atau JPEG
- **Flash LED** saat capture (on/off)
- **EXIF injection** ke JPEG: Make, Model, Software, DateTime, ExifVersion
- **Counter foto** otomatis (`photo_0001`, `photo_0002`, ...)

### 🎬 Video Recording (MJPEG)
- Rekam video format `.mjpeg` ke SD card
- Counter video otomatis (`video_0001`, ...)
- Indikator REC live dengan timer dan frame counter
- Putar ulang via MJPEG Player built-in

### 🔍 Face Detection
- Two-stage face detection: **MSR01** + **MNP01**
- Overlay kotak deteksi wajah dengan corner brackets
- 5 keypoint per wajah (mata, hidung, mulut)
- Counter wajah terdeteksi live di viewfinder

### 🖼️ Gallery & Foto Viewer
- Browser file SD card (JPG, BMP, MJPEG)
- Sorting alfabetis otomatis
- Navigasi halaman (8 item per halaman)
- Scrollbar visual
- Zoom 1× / 2× / 4× dengan pan

### 🎞️ MJPEG Player
- Putar file video `.mjpeg` dari SD
- **Speed control**: 0.5× / 1× / 2×
- **Loop mode** on/off
- Pause / Resume
- HUD speed + loop indicator

### 🔌 USB Mass Storage
- SD card mount sebagai USB drive ke PC/host
- Read & Write via USB tanpa cabut SD
- Live status screen saat USB mode aktif

### 💡 Dynamic Island Notifikasi
- Animasi slide-in/out dari atas layar
- Stack notifikasi (hingga 3 sekaligus)
- 6 tipe notif: OK, FLASH, REC, FACE, WARN, INFO
- Blink indikator saat recording aktif

### 🔐 Steganografi (Stego)
- **JPEG**: Embed payload ke marker `COM (0xFE)` — tak terlihat di viewer biasa
- **BMP**: Embed ke LSB byte Blue pixel langsung saat `saveBMP()`
- Payload format: `SANZXCAM|XXXX|v5.9`
- Extract & tampilkan saat preview foto (caption bar)

### ⚡ Exposure Control
| Preset | Deskripsi |
|---|---|
| **AUTO** | Kamera atur sendiri (AEC + AGC on) |
| **MOON** | Objek terang / bulan (exposure rendah) |
| **NIGHT** | Malam gelap (exposure & gain tinggi) |
| **MANUAL** | Atur `aec_value` 0–1200 dan `agc_gain` 0–30 |

---

## 🕹️ Peta Tombol

> **Short press** = tekan < 1500ms | **Long press** = tekan ≥ 1500ms

### 📷 Mode: VIEWFINDER

| Tombol | Short Press | Long Press |
|---|---|---|
| **BOOT** | 📸 Capture foto | 🔌 Masuk USB Mode |
| **B** | ⏺️ Mulai / Stop recording | 💡 Menu LED Flash |
| **C** | 🖼️ Buka Gallery | 🗂️ Menu Format Foto (GC2145) |
| **D** | 👤 Toggle Face Detect ON/OFF | ⚡ Menu Exposure |

---

### 🖼️ Mode: GALLERY

| Tombol | Aksi |
|---|---|
| **BOOT** | ✅ Buka foto/video yang dipilih |
| **B** | ← Kembali ke Viewfinder |
| **C** | ▲ Naik (tahan = scroll cepat) |
| **D** | ▼ Turun (tahan = scroll cepat) |

> Semakin lama tahan C/D, semakin cepat scroll (interval adaptif: 200ms → 100ms → 50ms)

---

### 🔍 Mode: PHOTO VIEW (Zoom 1×)

| Tombol | Short Press | Long Press |
|---|---|---|
| **BOOT** | ← Kembali ke Gallery | — |
| **B** | 🔍 Zoom in (1× → 2× → 4× → 1×) | 🗑️ Buka dialog hapus |
| **C** | ◀ Foto sebelumnya | — |
| **D** | ▶ Foto berikutnya | — |

### 🔍 Mode: PHOTO VIEW (Zoom 2× / 4×)

| Tombol | Short Press | Long Press | Tahan |
|---|---|---|---|
| **BOOT** | ← Kembali ke Gallery | — | — |
| **B** | 🔍 Zoom in/reset | 🗑️ Hapus foto | — |
| **C** | ◀ Pan kiri | ▲ Pan atas | Pan kiri terus |
| **D** | ▶ Pan kanan | ▼ Pan bawah | Pan kanan terus |

---

### 🎬 Mode: MJPEG PLAYER

| Tombol | Aksi |
|---|---|
| **BOOT** | ← Kembali ke Gallery |
| **B** | ⏯️ Pause / Resume |
| **C** | 🔁 Toggle Loop ON/OFF |
| **D** | ⏩ Ganti speed (0.5× → 1× → 2×) |

---

### 💡 Menu: LED FLASH

| Tombol | Aksi |
|---|---|
| **BOOT** | ✅ Konfirmasi pilihan |
| **B** | ✖ Batal / Kembali |
| **C / D** | Toggle LED ON / OFF |

---

### 🗂️ Menu: FORMAT FOTO (GC2145)

| Tombol | Aksi |
|---|---|
| **BOOT** | ✅ Simpan pilihan format |
| **B** | ✖ Batal |
| **C / D** | Toggle BMP / JPG |

> Format BMP: raw RGB565, warna asli, stego di LSB pixel  
> Format JPG: kompresi, EXIF + stego di COM marker

---

### ⚡ Menu: EXPOSURE

| Tombol | Aksi |
|---|---|
| **BOOT** | ✅ Terapkan preset (MANUAL → masuk adj) |
| **B** | ✖ Batal |
| **C** | ▲ Preset sebelumnya |
| **D** | ▼ Preset berikutnya |

### ⚡ Menu: EXPOSURE ADJ (Manual)

| Tombol | Aksi |
|---|---|
| **BOOT** | ✅ Simpan & kembali ke viewfinder |
| **B** | ➕ Naikkan Gain (0–30, wrap) |
| **C** | ➖ Kurangi Exposure (step adaptif) |
| **D** | ➕ Tambah Exposure (maks 1200) |

> Step exposure adaptif: `val < 100` → step 10 | `val < 400` → step 25 | `val ≥ 400` → step 50

---

### 🗑️ Dialog: HAPUS FILE

| Tombol | Aksi |
|---|---|
| **BOOT** | ✅ Konfirmasi hapus |
| **B / C / D** | ✖ Batal |
| *(timeout 8 detik)* | Auto batal |

---

### 🔌 Mode: USB MASS STORAGE

| Tombol | Aksi |
|---|---|
| **BOOT** | Keluar USB mode, remount SD |

> ⚠️ Pastikan **eject dari host terlebih dahulu** sebelum keluar USB mode untuk menghindari korupsi data.

---

## 🗂️ Mode Aplikasi

```
┌─────────────────────────────────────────────────────────────────┐
│                        SANZXCAM State Flow                      │
│                                                                 │
│   [BOOT]──────────────────────────────────────┐                │
│      │                                         ▼                │
│   VIEWFINDER ──(C)──► GALLERY ──(BOOT)──► PHOTO VIEW           │
│      │                   │                    │                 │
│      │                   └──(BOOT)──► MJPEG PLAYER             │
│      │                                                          │
│      ├──(B short)──► RECORDING ──(B)──► VIEWFINDER             │
│      ├──(B long) ──► MENU_LED                                   │
│      ├──(C long) ──► MENU_FORMAT  (GC2145 only)                │
│      ├──(D short)──► face detect toggle                         │
│      └──(D long) ──► MENU_EXP ──(MANUAL)──► MENU_EXP_ADJ      │
│                                                                 │
│   PHOTO VIEW ──(B long)──► DIALOG_DELETE                       │
│   BOOT (long) ──────────────────────────► USB MODE             │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🔐 Steganografi

SANZXCAM secara otomatis menyisipkan watermark tersembunyi ke setiap foto.

### Format Payload

```
SANZXCAM|XXXX|v5.9
```

| Field | Keterangan |
|---|---|
| `SANZXCAM` | Magic identifier |
| `XXXX` | Nomor foto (4 digit, zero-padded) |
| `v5.9` | Versi firmware |

### JPEG Steganografi

Payload disisipkan ke **COM marker (`0xFE`)** tepat setelah `0xD8 0xD9` header JPEG. Tidak terlihat di viewer foto biasa — hanya terbaca lewat parser khusus atau kode `stegoExtractFromJpeg()`.

```
[FF D8] [FF FE] [len_hi len_lo] [SANZXCAM|0001|v5.9] [... sisa JPEG ...]
```

### BMP Steganografi — v5.9-fix

Payload disisipkan ke **LSB (bit ke-0) dari komponen Blue** tiap pixel pada file BMP 24-bit BGR888. Iterasi mengikuti urutan penulisan BMP (bottom-up: `y = H-1` turun ke `y = 0`) sehingga extract dan embed selalu sinkron.

```
Blue byte pixel ke-N → bit ke-7 dari char ke-0 payload
Blue byte pixel ke-7 → bit ke-0 dari char ke-0 payload
... dst per 8 pixel = 1 karakter
```

### Membaca Stego

Saat membuka foto di **Photo View**, caption bar bawah akan otomatis menampilkan:

```
< 3 / 12 >  photo_0003.bmp [BMP] [#0003 v5.9]
```

---

## 🖼️ Format Foto

### GC2145 — Format Pilihan

| Format | Ekstensi | Stego | EXIF | Keterangan |
|---|---|---|---|---|
| **BMP** | `.bmp` | ✅ LSB Blue | ✗ | Raw RGB, warna paling akurat |
| **JPEG** | `.jpg` | ✅ COM marker | ✅ APP1 | Terkompresi, ukuran lebih kecil |

### OV3660 — Selalu JPEG

| Format | Ekstensi | Stego | EXIF |
|---|---|---|---|
| **JPEG** | `.jpg` | ✅ COM marker | ✅ APP1 |

### EXIF Tags yang Diinject

| Tag | ID | Isi |
|---|---|---|
| ImageDescription | 0x010E | `photo_XXXX` |
| Make | 0x010F | `SANZXCAM` |
| Model | 0x0110 | `GC2145` / `OV3660` |
| Software | 0x0131 | `v5.9` |
| ExifVersion | 0x9000 | `0220` |
| DateTimeOriginal | 0x9003 | `2025:01:01 HH:MM:SS` |

---

## 🚀 Boot Animation

Boot sequence SANZXCAM terdiri dari **2 fase**:

### Fase 1 — Splash Glitch (~2.5 detik)

```
1. Teks acak muncul (8 char random)
2. Glitch bars flash (6 bar)
3. Scramble text: random → "SANZXCAM" (8 step, 50ms/step)
4. Chromatic aberration: ghost merah + biru ±3px
5. Teks utama normal (GRAY_E)
6. Jeda 400ms
7. Glitch bars lagi (4 bar)
8. Subtitle muncul: "ESP32-S3  CAMERA SYSTEM"
9. Version: "v5.9-fix"
10. Divider line
11. Jeda 600ms
12. Glitch akhir (3 bar + chromatic ±2px)
13. Restore teks normal
14. Jeda 800ms
15. Wipe fade ke hitam (stripe 8px, 10ms/stripe)
```

### Fase 2 — Boot Log (~1.5 detik)

```
SANZXCAM                                           v5.9-fix
──────────────────────────────────────────────────────────

CAMERA
  SENSOR PID ·················· 0x2145  GC2145
  XCLK ························ 20MHz  OK
  RESOLUTION ················· 320x240  QVGA
  FORMAT ····················· RGB565  BMP

STORAGE
  SD CARD ···················· OK  3814MB
  FILESYSTEM ················· FAT32
  PHOTOS ····················· 0042 files

SYSTEM
  CPU FREQ ··················· 240MHz  OK
  PSRAM ······················ 8MB  OK
  HEAP FREE ·················· 248KB

BUILD
  VERSION ···················· v5.9-fix
  STEGO-BMP ·················· embed in saveBMP OK
  FEATURES ··················· EXIF+STEGO+FACE+REC

[████████████████████████████████████████████] READY
```

---

## 🏝️ Dynamic Island

Sistem notifikasi dinamis yang muncul dari atas layar:

| Tipe | Simbol | Warna | Durasi | Contoh |
|---|---|---|---|---|
| `NOTIF_OK` | `+` | Hijau gelap | 1000ms | `SAVED BMP #0042` |
| `NOTIF_FLASH` | `*` | Kuning gelap | 1000ms | `FLASH ON` |
| `NOTIF_REC` | `o` | Merah gelap | Selama REC | `REC  #0001` ← blink |
| `NOTIF_FACE` | `@` | Cyan gelap | 1000ms | `FACE DETECT  ON` |
| `NOTIF_WARN` | `!` | Oranye gelap | 2000ms | `WRITE ERR` |
| `NOTIF_INFO` | `i` | Abu | 800ms | `FORMAT: BMP+stego` |

- Stack hingga **3 notifikasi** sekaligus
- Animasi **ease-out cubic** slide in, **ease-in cubic** slide out
- Notif REC memiliki **blink dot merah** di kanan

---

## 📋 Changelog v5.9-fix

### 🐛 [FIX-STEGO-BMP] — Dua bug steganografi BMP diperbaiki

**Bug 1 — Bit loss saat RGB565 → BGR888:**
```
SEBELUM: embed ke LSB rgb565Buf[p*2+1]
         → saveBMP() expand: b = (px & 0x1F) << 3
         → bit bergeser ke posisi bit-3, bukan bit-0
         → stegoBmpExtract() membaca bit salah

SESUDAH: embed dilakukan DALAM saveBMP(), langsung ke
         byte Blue BGR888 final → b = (b & 0xFE) | bit
         → bit-0 selalu benar
```

**Bug 2 — Urutan pixel terbalik:**
```
SEBELUM: embed iterasi p=0,1,... = row y=0,1,... (top-down)
         saveBMP() tulis BMP bottom-up (y=H-1 pertama)
         extract() baca dari row=bmpH-1 turun
         → urutan embed ≠ urutan extract → payload korup

SESUDAH: embed dilakukan di dalam loop y=H-1..0 yang sama
         dengan penulisan BMP → urutan selalu sinkron
```

### ✨ [BOOT-GLITCH] — Durasi boot animation diperpanjang ~1.2s → ~2.5s

| Bagian | Sebelum | Sesudah | Delta |
|---|---|---|---|
| `glitchScrambleText` step delay | 30ms | 50ms | +~160ms |
| Jeda setelah teks normal | 0ms | 400ms | +400ms |
| Jeda sebelum subtitle | 100ms | 600ms | +500ms |
| Jeda setelah glitch akhir | 300ms | 800ms | +500ms |
| Wipe stripe delay | 4ms | 10ms | +~180ms |

---

## 📦 Dependensi

| Library | Fungsi |
|---|---|
| `esp_camera` | Driver kamera ESP32 |
| `LovyanGFX` | Driver display ILI9341 + XPT2046 |
| `JPEGDEC` | Decode JPEG untuk MJPEG player |
| `TJpg_Decoder` | Decode JPEG ke pixel buffer |
| `MjpegClass` | Parser stream MJPEG |
| `esp_vfs_fat` | Filesystem FAT SD card |
| `USBMSC` | USB Mass Storage Class |
| `human_face_detect_msr01` | Stage 1 face detection |
| `human_face_detect_mnp01` | Stage 2 face detection |
| `esp_task_wdt` | Watchdog timer reset |

---

## 📁 Struktur File SD Card

```
/sdcard/
├── photo_0001.jpg     ← foto JPEG + EXIF + stego
├── photo_0002.bmp     ← foto BMP 24-bit + stego
├── photo_0003.jpg
├── ...
├── video_0001.mjpeg   ← video rekaman MJPEG
├── video_0002.mjpeg
└── ...
```

---

## 💡 Tips Penggunaan

```
┌──────────────────────────────────────────────────────────────┐
│  ⚡ QUICK TIPS                                               │
│                                                              │
│  • Selalu eject dari host sebelum keluar USB mode           │
│  • Flash LED membuat 2 frame dummy untuk exposure adapt     │
│  • Manual exposure: step makin besar di nilai tinggi        │
│  • Tahan C/D di Gallery untuk scroll turbo                  │
│  • Zoom 2× / 4× pan bisa pakai short press atau hold       │
│  • Stego payload terbaca otomatis di caption foto           │
│  • NIGHT mode bagus untuk pencahayaan < 5 lux              │
│  • Format BMP cocok untuk analisis pixel-level              │
└──────────────────────────────────────────────────────────────┘
```

---

<div align="center">

**SANZXCAM v5.9-fix** — built for ESP32-S3

*Monochrome. Terminal. Covert.*

```
[████████████████████████████████] READY
```

</div>
