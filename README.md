# 📸 SANZXCAM v6.0

Firmware kamera kustom berbasis **ESP32-S3** dengan fokus pada estetika retro-terminal, fitur eksperimental (MPU6050), dan integrasi AI.

---

## 🎮 Kontrol & Navigasi

### 📺 Mode: VIEWFINDER (Utama)

| Tombol | Tekan Singkat (Short) | Tekan Lama (Long) |
|---|---|---|
| **BOOT** | 📸 Ambil Foto (Tap 2x jika HDR ON) | 🔌 Masuk USB Mode |
| **B** | ⏺️ Mulai / Stop Recording | 💡 Menu LED Flash |
| **C** | 🖼️ Buka Gallery | 🤖 AI Feature Menu |
| **D** | 🛠️ Features Menu (Exp) | ⚡ Menu Exposure |

---

### 🖼️ Mode: GALLERY

| Tombol | Tekan Singkat (Short) | Tekan Lama (Long) |
|---|---|---|
| **BOOT** | ✅ Buka file (Foto/Video) | 🚀 Jump to Index |
| **B** | ← Kembali ke Viewfinder | 🔲 Toggle Grid / List |
| **C** | ▲ Naik | 🗑️ **MULTI-DELETE MODE** |
| **D** | ▼ Turun | 🤖 AI Feature (Foto) |

**Multi-Delete Mode:**
- **BOOT**: Pilih / Batal pilih file
- **B (Long)**: Konfirmasi hapus semua yang dipilih
- **B (Short)**: Keluar mode tanpa menghapus

---

### 🛠️ Menu: EXPERIMENTAL FEATURES (D-Short)

Menu untuk mengatur fitur tingkat lanjut dan sensor MPU6050:
- **EIS**: Electronic Image Stabilization (mengurangi goyangan).
- **HDR**: Triple capture dengan exposure berbeda (Under/Normal/Over).
- **AUTO-ROTATE**: Rotasi otomatis file BMP/JPG berdasarkan kemiringan.
- **MPU LOG**: Log data sensor ke `/sdcard/mpu_log.csv`.
- **HUD**: Toggle overlay UI pada viewfinder.
- **CALIBRATE MPU**: Kalibrasi gyroscope (simpan ke `mpu_cal.ini`).
- **HD CAPTURE**: Kualitas JPEG tinggi (menggunakan PSRAM lebih besar).
- **KALMAN/DLPF**: Pengaturan filter sensor untuk stabilitas tilt.
- **CLEAR THUMB**: Hapus cache thumbnail di SD card.

---

### ⚡ Menu: EXPOSURE (D-Long)

UI baru berbasis **Arc Meter** dengan 6 preset:
1. **AUTO**: Kendali otomatis penuh.
2. **GRAY**: Efek hitam putih real-time.
3. **MOON**: Exposure rendah untuk objek sangat terang.
4. **NIGHT**: Gain tinggi untuk kondisi gelap (< 5 lux).
5. **N-BW**: Malam dengan filter grayscale.
6. **MANUAL**: Atur Exposure & Gain secara manual.

---

## 🔐 Steganografi & Metadata

Setiap foto yang diambil menyertakan data tersembunyi:
- **JPEG**: Payload di COM marker (`0xFE`) + EXIF tags (Make, Model, Software).
- **BMP**: Payload disisipkan di **LSB Blue channel** (24-bit).
- **Payload**: `SANZXCAM|XXXX|v6.0` (XXXX = index foto).

---

## 📁 Konfigurasi SD Card

- `wifi.ini`: Menyimpan kredensial WiFi (SSID & PASS).
- `settings.ini`: Menyimpan preferensi menu (Flash, EIS, HDR, dll).
- `mpu_cal.ini`: Menyimpan offset kalibrasi sensor.
- `/thumbnails/`: Folder cache untuk grid gallery (otomatis dibuat).

---

## 📦 Dependensi Utama

- `LovyanGFX`: Driver display & akselerasi grafis.
- `TJpg_Decoder` & `JPEGDEC`: Engine rendering gambar/video.
- `FastLED`: Kontrol status LED NeoPixel (GPIO 48).
- `Adafruit MPU6050`: Integrasi sensor gerak.

---

<div align="center">

**SANZXCAM v6.0** — Terminal Aesthetic ESP32-S3 Camera

*Stability. Control. Stealth.*

```
[████████████████████████████████] READY
```

</div>
