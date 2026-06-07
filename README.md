# 📸 SANZXCAM v6.1

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

### 📻 Mode: FM RADIO

| Tombol | Tekan Singkat (Short) | Tekan Lama (Long) |
|---|---|---|
| **BOOT** | 🔇 Mute / Unmute | - |
| **B** | 🔊 Volume (+) | ⬅️ Keluar (Back to Menu) |
| **C** | 🔍 Seek Down | - |
| **D** | 🔍 Seek Up | - |

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
- **HD QUALITY**: Pengaturan tingkat kompresi (4=maksimal, 6=seimbang).
- **KALMAN-R**: Tuning noise filter untuk stabilitas tilt.
- **TILT-DZ**: Pengaturan deadzone kemiringan (derajat).
- **DLPF**: Digital Low Pass Filter bandwidth (5Hz - 260Hz).
- **CLEAR THUMB**: Hapus cache thumbnail di SD card.
- **FM RADIO**: Membuka antarmuka Radio RDA5807M.

---

### ⚡ Menu: EXPOSURE (D-Long)

UI berbasis **Arc Meter** dengan 6 preset:
1. **AUTO**: Kendali otomatis penuh.
2. **GRAY**: Efek hitam putih real-time.
3. **MOON**: Exposure rendah untuk objek sangat terang.
4. **NIGHT**: Gain tinggi untuk kondisi gelap (< 5 lux).
5. **N-BW**: Malam dengan filter grayscale.
6. **MANUAL**: Atur Exposure & Gain secara manual.

---

## 🤖 Fitur AI (Gemini Vision)

Integrasi AI untuk analisis gambar secara real-time:
1. **Describe**: Deskripsi detail isi gambar dalam Bahasa Indonesia.
2. **Scavenger Hunt**: Tantangan interaktif menebak objek.
3. **Mood Reader**: Analisis emosi wajah atau suasana gambar.
4. **ANPR**: Pembacaan plat nomor kendaraan otomatis.
5. **Sky Watch**: Analisis kondisi awan dan fenomena langit.
6. **Pest Count**: Identifikasi dan perhitungan hama/serangga.
7. **Produce ID**: Identifikasi produk pertanian dan nilai gizi.

---

## 🔐 Steganografi & Metadata

Setiap foto yang diambil menyertakan data tersembunyi:
- **JPEG**: Payload di COM marker (`0xFE`) + EXIF tags (Make, Model, Software).
- **BMP**: Payload disisipkan di **LSB Blue channel** (24-bit).
- **Payload**: `SANZXCAM|XXXX|v6.1` (XXXX = index foto).

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
- `Adafruit NeoPixel`: Kontrol status LED NeoPixel (GPIO 48).
- `Adafruit MPU6050`: Integrasi sensor gerak.
- `Radio`: Library untuk modul RDA5807M.

---

<div align="center">

**SANZXCAM v6.1** — Terminal Aesthetic ESP32-S3 Camera

*Stability. Control. Stealth.*

```
[████████████████████████████████] READY
```

</div>
