# 📸 SANZXCAM v6.1 — Terminal Aesthetic Camera
> **Advanced ESP32-S3 Firmware with AI, Motion Sensing, and Retro Intelligence.**

[![Platform: ESP32-S3](https://img.shields.io/badge/Platform-ESP32--S3-orange.svg)](https://www.espressif.com/en/products/socs/esp32-s3)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-00979D.svg)](https://www.arduino.cc/)

**SANZXCAM** bukan sekadar kamera digital biasa. Ini adalah perpaduan antara estetika *retro-terminal*, kecerdasan buatan Gemini AI, dan optimasi hardware tingkat tinggi pada platform ESP32-S3. Dirancang untuk stabilitas, kontrol penuh, dan fitur "stealth" steganografi.

---

## 🚀 Fitur Unggulan

### 🧠 Intelligence (Gemini AI Integration)
Integrasi langsung dengan **Gemini 2.5 Flash lite** untuk analisis gambar real-time:
- **Describe**: Deskripsi visual dalam Bahasa Indonesia.
- **Scavenger Hunt**: Game interaktif berbasis objek yang ditemukan kamera.
- **Mood Reader**: Analisis ekspresi wajah dan suasana foto.
- **ANPR**: Pengenalan pelat nomor otomatis.
- **Sky Watch**: Analisis awan, cuaca, dan fenomena langit.
- **Pest Count**: Identifikasi hama dan serangga untuk pertanian.
- **Produce ID**: Identifikasi buah/sayur dan estimasi nilai gizi.

### 🛡️ Motion & Stability (MPU6050)
- **EIS (Electronic Image Stabilization)**: Mengurangi goyangan pada viewfinder menggunakan data gyroscope.
- **Auto-Rotate**: Menyesuaikan orientasi file JPEG/BMP secara fisik berdasarkan gravitasi.
- **HDR (High Dynamic Range)**: Triple capture (Under/Normal/Over exposure) untuk detail cahaya yang lebih luas.
- **Kalman Filtering**: Fusi sensor tingkat lanjut untuk estimasi kemiringan (*tilt*) yang presisi.

### 📼 Multimedia & Audio
- **FM Radio**: Integrasi chip RDA5807M dengan RDS text dan volume digital.
- **Bluetooth MP3 Player**: Streaming audio dari SD Card ke speaker/headphone Bluetooth (A2DP Source).
- **MJPEG Player**: Pemutaran video format MJPEG langsung dari SD Card.

### 🕵️ Stealth & Steganography
Menyisipkan data rahasia ke dalam setiap jepretan:
- **JPEG**: Payload tersembunyi di dalam *COM Marker* (`0xFE`) dan tag EXIF.
- **BMP**: Steganografi **LSB (Least Significant Bit)** pada channel Blue (24-bit).

---

## ⌨️ Kontrol & Antarmuka

| Tombol | Mode: Viewfinder | Mode: Gallery |
| :--- | :--- | :--- |
| **BOOT (Double Tap)** | 🎞️ HDR Triple Capture | 🚀 Jump to Index |
| **BOOT (Long)** | 🔌 USB MSC Mode | - |
| **B (Short)** | ⏺️ Record Video (MJPEG) | ⬅️ Back to Viewfinder |
| **B (Long)** | 💡 Flash/Neo Menu | 🔲 Toggle Grid/List |
| **C (Short)** | 🖼️ Open Gallery | ▲ Navigate Up |
| **C (Long)** | 🤖 AI Feature Menu | 🗑️ **MULTI-DELETE MODE** |
| **D (Short)** | 🛠️ Features Menu (Exp) | ▼ Navigate Down |
| **D (Long)** | ⚡ Arc-Meter Exposure | 🤖 AI Analysis (Photo) |

---

## 🛠️ Arsitektur Kode (Deep Dive)

### 🧩 Manajemen Memori (PSRAM)
Firmware ini memanfaatkan **8MB PSRAM** secara agresif:
- **Double Buffering**: Untuk frame camera dan rendering LovyanGFX.
- **Large Image Buffer**: Mendukung capture resolusi tinggi (HD Capture) tanpa *crash*.
- **Thumbnail Cache**: Menyimpan preview gallery untuk rendering instan.

### 📱 Dynamic Island UI
Sistem notifikasi non-blocking yang terinspirasi dari modern UI:
- Menggunakan **Sprite-based restoration**: Menyimpan background sebelum menggambar island, lalu mengembalikannya agar tidak merusak frame kamera.
- Menampilkan status SD Card, WiFi, Error, dan konfirmasi capture.

### 📐 Navigasi Gallery Hardware-Scrolled
Sistem gallery grid menggunakan **Hardware Vertical Scroll** pada ILI9341:
- Rendering row-by-row untuk efisiensi memori.
- Navigasi super mulus tanpa *screen-tearing*.
- Mendukung multi-select delete untuk manajemen file massal.

---

## ⚙️ Pin Out (Hardware Mapping)

| Komponen | Pin | Deskripsi |
| :--- | :--- | :--- |
| **NeoPixel** | GPIO 48 | Status Indicator (Breathing/Pulse) |
| **Flash LED** | GPIO 2 | High-brightness focus light |
| **Buttons** | 0, 46, 3, 41 | BOOT, D, C, B (Active LOW) |
| **I2C (Bus 0)** | 43 (SDA), 44 (SCL) | MPU6050 Sensor |
| **I2C (Bus 1)** | 43 (SDA), 42 (SCL) | RDA5807M FM Radio |
| **Camera** | ESP32-S3 Cam | 16-bit DVP Interface |

---

## 📦 Instalasi & Kompilasi

### Persyaratan
- **Arduino IDE 2.x** atau **arduino-cli**.
- **Board Package**: `esp32` by Espressif (v2.0.17+).
- **Libraries**:
  - `LovyanGFX`, `TJpg_Decoder`, `JPEGDEC`, `ArduinoJson`.
  - `Adafruit MPU6050`, `Adafruit NeoPixel`.
  - `ESP32-A2DP`, `ESP8266Audio`, `Radio`.

### Konfigurasi Build
- **Board**: `ESP32S3 Dev Module`
- **Flash Size**: `16MB`
- **Partition Scheme**: `16MB (3MB APP / 9MB FAT)`
- **PSRAM**: `OPI PSRAM`

---

<div align="center">

**SANZXCAM v6.1** — *Stability. Control. Stealth.*

```text
[ SYSTEM STATUS: OPTIMIZED ]
[ AI CORE: CONNECTED ]
[ MOTION ENGINE: ACTIVE ]
```

Developed with ❤️ for the ESP32 Community.

</div>
