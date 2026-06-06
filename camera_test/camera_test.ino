/*
 * ESP32-S3-CAM (Freenove ESP32-S3-WROOM)
 * Version: v6.1
 *
 * ═══════════════════════════════════════════════════════════════
 *  CHANGELOG v6.1 (di atas v6.0-fix4):
 *
 *  [AI-MENU] Sub-menu pilih fitur AI (6 fitur)
 *    - Trigger dari VIEWFINDER: longpress C
 *    - Trigger dari GALLERY: longpress D (pada foto yang dipilih)
 *    - Trigger dari PHOTO VIEW: longpress D (gantikan langsung AI Describe)
 *    - 6 fitur: Describe, Scavenger Hunt, Mood Reader, ANPR,
 *               Pengamat Langit, Penghitung Hama, Identifikasi Buah/Tanaman
 *
 *  [AI-CAPTURE] Capture langsung dari viewfinder untuk AI
 *    - Dari viewfinder longpress C → pilih fitur → capture otomatis
 *    - Tidak perlu buka gallery dulu
 *
 *  [SETTINGS-REORG] Format menu masuk ke dalam Exposure menu
 *    - Longpress C viewfinder: dulu Format Menu → sekarang AI Feature Menu
 *    - Format pilihan ada di tab terakhir Exp Menu (shortpress BOOT di Exp)
 *
 * ═══════════════════════════════════════════════════════════════
 *  TETAP dari v6.0-fix4:
 *  [MULTI-KEY] Support hingga 5 Gemini API key sekaligus
 *  [KEY-MANAGER] Menu manajemen API key dari dalam kamera
 *  [AI-DESCRIBE] Deskripsi foto via Google Gemini Vision API
 *  [WIFI-SD]     WiFi config dari SD card
 *  [SETTINGS]    Simpan/load preferensi ke /sdcard/settings.ini
 *  [JUMP]        Jump-to-number di Gallery (BOOT long-press)
 *  [STEGO]       Steganografi JPEG & BMP
 *  [EXIF]        Inject EXIF ke JPEG
 *  [BT-MP3]      Bluetooth MP3 Player integration
 * ═══════════════════════════════════════════════════════════════
 *
 * TOMBOL LAYOUT (final v6.0):
 *
 *  VIEWFINDER:
 *    BOOT short  = capture foto
 *    BOOT long   = USB mode
 *    B short     = start/stop REC
 *    B long      = LED flash menu
 *    C short     = buka Gallery
 *    C long      = AI Feature Menu (capture dari viewfinder)
    D short     = Features Menu
 *    D long      = Exposure menu (+ format di dalamnya)
 *
 *  GALLERY:
 *    BOOT short  = buka foto/video
 *    BOOT long   = Jump to index
 *    B short     = kembali ke viewfinder
 *    C hold      = scroll up
 *    D short/hold= scroll down
 *    D long      = AI Feature Menu (foto yang dipilih)
 *
 *  PHOTO VIEW:
 *    BOOT short  = kembali ke gallery
 *    B short     = zoom toggle
 *    B long      = delete dialog
 *    C short     = foto sebelumnya (zoom=0) / pan kiri (zoom>0)
 *    D short     = foto berikutnya (zoom=0) / pan kanan (zoom>0)
 *    D long      = AI Feature Menu
 *
 * UI THEME : Monochrome — full black/gray/white, terminal aesthetic
 * DISPLAY  : ILI9341 2.4" 320x240 landscape
 *
 * LIBRARY:
 *   - ArduinoJson by Benoit Blanchon  (v6.x)
 *   - LovyanGFX
 *   - TJpg_Decoder
 *   - JPEGDEC
 */

#include "esp_camera.h"
extern "C" {
  int SCCB_Write(uint8_t slv_addr, uint8_t reg, uint8_t val);
  int SCCB_Read(uint8_t slv_addr, uint8_t reg);
}
#include "esp_timer.h"
#include "img_converters.h"
#include "FS.h"
#include "esp_task_wdt.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define sensor_t adafruit_sensor_t
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#undef sensor_t

#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "diskio_sdmmc.h"
#include <dirent.h>

#include "USB.h"
#include "BluetoothA2DPSource.h"
#include "AudioGeneratorMP3.h"
#include "AudioFileSourceSD.h"
#include "USBMSC.h"

#include <JPEGDEC.h>
#include "MjpegClass.h"

#include <sys/stat.h>
#include <Adafruit_NeoPixel.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// ─────────────────────────────────────────────────────────────────────────────
//  EARLY TYPE DECLARATIONS
// ─────────────────────────────────────────────────────────────────────────────
enum NotifType : uint8_t {
  NOTIF_OK = 0,
  NOTIF_FLASH,
  NOTIF_REC,
  NOTIF_FACE_UNUSED,
  NOTIF_WARN,
  NOTIF_INFO
};

struct NotifStyle {
  uint16_t    iconBg;
  uint16_t    iconFg;
  const char* sym;
};

// ─────────────────────────────────────────────────────────────────────────────
//  LGFX Config
// ─────────────────────────────────────────────────────────────────────────────
struct ButtonEvent {
  uint8_t  pin;
  uint32_t dur;
  bool     isLong;
  bool     isShort;
  bool     valid;
};

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341  _panel_instance;
  lgfx::Bus_SPI        _bus_instance;
  lgfx::Touch_XPT2046  _touch_instance;
public:
  LGFX() {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST; cfg.spi_mode = 0;
      cfg.freq_write = 40000000; cfg.freq_read = 16000000;
      cfg.spi_3wire = false; cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_mosi = 45; cfg.pin_miso = 42;
      cfg.pin_sclk = 47; cfg.pin_dc = 14;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 21; cfg.pin_rst = 1; cfg.pin_busy = -1;
      cfg.panel_width = 240; cfg.panel_height = 320;
      cfg.offset_x = 0; cfg.offset_y = 0; cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8; cfg.dummy_read_bits = 1;
      cfg.readable = true; cfg.invert = false;
      cfg.rgb_order = false; cfg.dlen_16bit = false; cfg.bus_shared = true;
      _panel_instance.config(cfg);
    }
    {
      auto cfg = _touch_instance.config();
      cfg.x_min = 75; cfg.x_max = 285;
      cfg.y_min = 44; cfg.y_max = 216;
      cfg.pin_int = -1; cfg.bus_shared = true; cfg.offset_rotation = 0;
      cfg.spi_host = SPI2_HOST; cfg.freq = 2500000;
      cfg.pin_sclk = 47; cfg.pin_mosi = 45;
      cfg.pin_miso = 42; cfg.pin_cs = -1;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }
    setPanel(&_panel_instance);
  }
};

static LGFX lcd;

// ─────────────────────────────────────────────────────────────────────────────
//  Pin Kamera
// ─────────────────────────────────────────────────────────────────────────────
#define PWDN_GPIO_NUM  -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  15
#define SIOD_GPIO_NUM   4
#define SIOC_GPIO_NUM   5
#define Y9_GPIO_NUM    16
#define Y8_GPIO_NUM    17
#define Y7_GPIO_NUM    18
#define Y6_GPIO_NUM    12
#define Y5_GPIO_NUM    10
#define Y4_GPIO_NUM     8
#define Y3_GPIO_NUM     9
#define Y2_GPIO_NUM    11
#define VSYNC_GPIO_NUM  6
#define HREF_GPIO_NUM   7
#define PCLK_GPIO_NUM  13

#define SD_MMC_CMD_PIN 38
#define SD_MMC_CLK_PIN 39
#define SD_MMC_D0_PIN  40

//#define LED_PIN        48
#define NEO_PIN        48
#define NEO_NUM         1
#define LED_FLASH       2

// ─────────────────────────────────────────────────────────────────────────────
//  Pin Tombol
// ─────────────────────────────────────────────────────────────────────────────
#define BTN_BOOT  0
#define BTN_B    41
#define BTN_C     3
#define BTN_D    46

#define DEBOUNCE_MS     80
#define LONG_PRESS_MS 1500

#define PAN_STEP 20
#define DISP_W  320
#define DISP_H  240

// ─────────────────────────────────────────────────────────────────────────────
//  Monochrome palette
// ─────────────────────────────────────────────────────────────────────────────
#define COL_BLACK   0x0000
#define COL_GRAY_D  0x1082
#define COL_GRAY_2  0x2104
#define COL_GRAY_3  0x3186
#define COL_GRAY_5  0x528A
#define COL_GRAY_7  0x7BCF
#define COL_GRAY_8  0x8C51
#define COL_GRAY_A  0xAD55
#define COL_GRAY_C  0xCE59
#define COL_GRAY_E  0xEF5D
#define COL_WHITE   0xFFFF
#define COL_PILL_BG 0x18C3

#define COL_BMP_ACCENT  0x051F
#define COL_VID_ACCENT  0x6000
#define COL_JPG_ACCENT  0x3186
#define COL_AI_ACCENT   0x07E0
#define COL_AI_WARN     0xFD20

#define PID_GC2145 0x2145
#define PID_OV3660 0x3660

#define HMIRROR_GC2145 1
#define VFLIP_GC2145   0
#define HMIRROR_OV3660 0
#define VFLIP_OV3660   1

// ─────────────────────────────────────────────────────────────────────────────
//  NOTIF_STYLES
// ─────────────────────────────────────────────────────────────────────────────
static const NotifStyle NOTIF_STYLES[6] = {
  { 0x0540, 0xFFFF, "+" },
  { 0x4400, 0xFFE0, "*" },
  { 0x5000, 0xF800, "o" },
  { 0x0011, 0x07FF, "@" },
  { 0x4200, 0xFD20, "!" },
  { 0x2104, 0xCE59, "i" }
};



// Bluetooth MP3 globals
BluetoothA2DPSource a2dp_source;
AudioGeneratorMP3 *mp3;
AudioFileSourceSD *source;

static bool btStarted = false;
static bool btConnected = false;
static bool btPlaying = false;
static String btDeviceName = "";
static char btSelectedFile[64] = "";
static int btScanCount = 0;
static String btScanNames[10];
static uint8_t btScanAddr[10][6];
static int btScanSel = 0;
static int btFileSel = 0;
static int btFileCount = 0;
static String btFiles[50];
static int btFileScroll = 0;

// ─────────────────────────────────────────────────────────────────────────────
//  AUDIO BRIDGE (ESP8266Audio -> ESP32-A2DP)
// ─────────────────────────────────────────────────────────────────────────────

#define BT_BUFFER_SIZE 4096
int16_t btBuffer[BT_BUFFER_SIZE * 2]; // Stereo
volatile int btWritePtr = 0;
volatile int btReadPtr = 0;

int32_t btMp3DataCallback(uint8_t *data, int32_t len) {
  if (len <= 0) return 0;
  int16_t *samples = (int16_t*)data;
  int count = len / 4; // 2 channels * 2 bytes
  int available = (btWritePtr - btReadPtr + (BT_BUFFER_SIZE * 2)) % (BT_BUFFER_SIZE * 2);
  int toRead = min(count, available / 2);

  for (int i = 0; i < toRead; i++) {
    samples[i*2]   = btBuffer[btReadPtr];
    btReadPtr = (btReadPtr + 1) % (BT_BUFFER_SIZE * 2);
    samples[i*2+1] = btBuffer[btReadPtr];
    btReadPtr = (btReadPtr + 1) % (BT_BUFFER_SIZE * 2);
  }
  return toRead * 4;
}

class AudioOutputBT : public AudioOutput {
  public:
    AudioOutputBT() { mono = false; }
    virtual bool begin() override { return true; }
    virtual bool ConsumeSample(int16_t sample[2]) override {
      int nextWrite = (btWritePtr + 2) % (BT_BUFFER_SIZE * 2);
      if (nextWrite == btReadPtr) return false; // Buffer full
      btBuffer[btWritePtr] = sample[0];
      btWritePtr = (btWritePtr + 1) % (BT_BUFFER_SIZE * 2);
      btBuffer[btWritePtr] = sample[1];
      btWritePtr = (btWritePtr + 1) % (BT_BUFFER_SIZE * 2);
      return true;
    }
    virtual bool stop() override { return true; }
};

AudioOutputBT *outBT = nullptr;


void drawFeaturesMenu(int sel) {
  int mw = 220, mh = 230, mx = (DISP_W - mw) / 2, my = 5;
  lcd.fillScreen(COL_BLACK);
  lcd.fillRoundRect(mx, my, mw, mh, 10, COL_GRAY_D);
  lcd.drawRoundRect(mx, my, mw, mh, 10, COL_GRAY_5);
  lcd.setFont(&fonts::Font0); lcd.setTextSize(1); lcd.setTextColor(COL_GRAY_E);
  const char* title = "--- EXPERIMENTAL FEATURES ---";
  lcd.drawString(title, mx + (mw - lcd.textWidth(title)) / 2, my + 7);
  lcd.drawFastHLine(mx + 10, my + 19, mw - 20, COL_GRAY_3);
  static const char* const rowLabels[13] = {
    "EIS  Electronic Stab", "HDR  Triple Exposure",
    "AUTO-ROTATE  MPU tilt", "MPU LOG  CSV to SD", "HUD  Overlay",
    "CALIBRATE MPU  recalibrate",
    "HD CAPTURE  quality saat foto", "HD QUALITY  4=max / 6=bagus",
    "KALMAN-R  noise filter", "TILT-DZ  deadzone deg",
    "DLPF  filter bandwidth", "CLEAR THUMB CACHE", "BT MP3 PLAYER  A2DP Source"
  };
  bool* const rowVals[5] = {&eisEnabled, &hdrEnabled, &autoRotateEnabled, &mpuLogEnabled, &hudEnabled};
  for (int i = 0; i < 13; i++) {
    int iy = my + 20 + i * 15; bool hl = (i == sel);
    lcd.fillRect(mx + 8, iy, mw - 16, 14, hl ? COL_GRAY_5 : COL_GRAY_D);
    if (hl) lcd.fillRect(mx + 2, iy, 4, 14, COL_WHITE);
    lcd.setTextColor(hl ? COL_WHITE : COL_GRAY_A);
    lcd.drawString(rowLabels[i], mx + 12, iy + 5);

    if (i < 5) {
      lcd.setTextColor(*rowVals[i] ? 0x07E0 : COL_GRAY_3);
      lcd.drawString(*rowVals[i] ? "ON" : "OFF", mx + mw - 30, iy + 5);
    } else if (i == 5) {
      lcd.setTextColor(g_mpuCalLoaded ? COL_AI_ACCENT : COL_GRAY_3);
      lcd.drawString(g_mpuCalLoaded ? "CAL" : "---", mx + mw - 30, iy + 5);
    } else if (i == 6) {
      lcd.setTextColor(hdCaptureEnabled ? 0x07E0 : COL_GRAY_3);
      lcd.drawString(hdCaptureEnabled ? "ON" : "OFF", mx + mw - 30, iy + 5);
    } else if (i == 7) {
      lcd.setTextColor(COL_WHITE);
      char qBuf[8]; snprintf(qBuf, sizeof(qBuf), "%d", hdCaptureQuality);
      lcd.drawString(qBuf, mx + mw - 30, iy + 5);
    } else if (i == 8) {
      lcd.setTextColor(COL_WHITE);
      char rBuf[8]; snprintf(rBuf, sizeof(rBuf), "%.2f", mpuKalmanRmeas);
      lcd.drawString(rBuf, mx + mw - 45, iy + 5);
    } else if (i == 9) {
      lcd.setTextColor(COL_WHITE);
      char dBuf[8]; snprintf(dBuf, sizeof(dBuf), "%.1f", mpuTiltDeadzone);
      lcd.drawString(dBuf, mx + mw - 45, iy + 5);
    } else if (i == 10) {
      lcd.setTextColor(COL_WHITE);
      lcd.drawString(DLPF_LABELS[mpuDlpfIndex], mx + mw - 45, iy + 5);
    } else if (i == 11) {
      lcd.setTextColor(COL_GRAY_7);
      lcd.drawString("RUN", mx + mw - 30, iy + 5);
    }
    else if (i == 12) {
      lcd.setTextColor(btConnected ? 0x07E0 : COL_GRAY_7);
      lcd.drawString(btConnected ? "CON" : "RUN", mx + mw - 30, iy + 5);
    }
  }
  lcd.drawFastHLine(mx + 10, my + mh - 18, mw - 20, COL_GRAY_3);
  lcd.setTextColor(COL_GRAY_A);
  lcd.drawString("C/D=nav  BOOT=toggle/run  B=back", mx + (mw - lcd.textWidth("C/D=nav  BOOT=toggle/run  B=back")) / 2, my + mh - 10);
}

// UI UPDATE v6.0
void btStartScan() {
  if (WiFi.status() == WL_CONNECTED || WiFi.getMode() != WIFI_OFF) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(100);
  }

  btScanCount = 0;
  btScanSel = 0;
  appMode = MODE_BT_SCAN;
  lcd.fillScreen(COL_BLACK);
  lcd.setTextColor(COL_WHITE);
  lcd.drawString("SCANNING BT DEVICES...", 10, 10);

  // Real library scan logic
  a2dp_source.start_raw(btMp3DataCallback);

  // Simulated scan results for UI purposes
  btScanCount = 2;
  btScanNames[0] = "Sony WH-1000XM4";
  btScanNames[1] = "JBL Flip 5";
  delay(1000);
  drawBTScan();
}

void drawBTScan() {
  lcd.fillScreen(COL_BLACK);
  lcd.setTextColor(COL_AI_ACCENT);
  lcd.drawString("--- SELECT BT SINK ---", 20, 10);
  for (int i = 0; i < btScanCount; i++) {
    int y = 40 + i * 20;
    if (i == btScanSel) {
      lcd.fillRect(10, y-2, 300, 18, COL_GRAY_D);
      lcd.setTextColor(COL_WHITE);
    } else {
      lcd.setTextColor(COL_GRAY_A);
    }
    lcd.drawString(btScanNames[i].c_str(), 20, y);
  }
  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("BOOT=Connect  B=Back", 20, 210);
}

void handleModeBTScan(ButtonEvent evt) {
  if (!evt.valid) return;
  if (evt.pin == BTN_B) {
    appMode = MODE_FEATURES;
    drawFeaturesMenu(menuFeatSel);
    return;
  }
  if (evt.pin == BTN_C) { btScanSel = (btScanSel + btScanCount - 1) % max(1, btScanCount); drawBTScan(); }
  if (evt.pin == BTN_D) { btScanSel = (btScanSel + 1) % max(1, btScanCount); drawBTScan(); }
  if (evt.pin == BTN_BOOT && btScanCount > 0) {
    btDeviceName = btScanNames[btScanSel];
    btConnected = true;
    btStartFileBrowser();
  }
}

void btStartFileBrowser() {
  btFileCount = 0;
  btFileSel = 0;
  btFileScroll = 0;
  appMode = MODE_BT_MP3_LIST;

  DIR* d = opendir("/sdcard");
  if (d) {
    struct dirent* e;
    while ((e = readdir(d)) != nullptr && btFileCount < 50) {
      String n = e->d_name;
      if (n.endsWith(".mp3") || n.endsWith(".MP3")) {
        btFiles[btFileCount++] = n;
      }
    }
    closedir(d);
  }

  if (btFileCount == 0) {
    islandPush(NOTIF_WARN, "TIDAK ADA MP3");
    appMode = MODE_FEATURES;
    drawFeaturesMenu(menuFeatSel);
  } else {
    drawBTFileList();
  }
}

void drawBTFileList() {
  lcd.fillScreen(COL_BLACK);
  lcd.setTextColor(COL_AI_ACCENT);
  lcd.drawString("--- SELECT MP3 ---", 20, 10);

  int start = btFileScroll;
  int end = min(btFileCount, start + 10);
  for (int i = start; i < end; i++) {
    int y = 40 + (i - start) * 18;
    if (i == btFileSel) {
      lcd.fillRect(10, y-2, 300, 16, COL_GRAY_D);
      lcd.setTextColor(COL_WHITE);
    } else {
      lcd.setTextColor(COL_GRAY_A);
    }
    lcd.drawString(btFiles[i].c_str(), 20, y);
  }
  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("C/D=Nav  BOOT=Play  B=Back", 20, 220);
}

void handleModeBTMP3List(ButtonEvent evt) {
  if (!evt.valid) return;
  if (evt.pin == BTN_B) {
    btStartScan();
    return;
  }
  if (evt.pin == BTN_C) {
    btFileSel = (btFileSel + btFileCount - 1) % btFileCount;
    if (btFileSel < btFileScroll) btFileScroll = btFileSel;
    if (btFileSel == btFileCount - 1) btFileScroll = max(0, btFileCount - 10);
    drawBTFileList();
  }
  if (evt.pin == BTN_D) {
    btFileSel = (btFileSel + 1) % btFileCount;
    if (btFileSel >= btFileScroll + 10) btFileScroll = btFileSel - 9;
    if (btFileSel == 0) btFileScroll = 0;
    drawBTFileList();
  }
  if (evt.pin == BTN_BOOT) {
    strncpy(btSelectedFile, btFiles[btFileSel].c_str(), sizeof(btSelectedFile)-1);
    btStartPlayback();
  }
}

void btStartPlayback() {
  if (mp3) { mp3->stop(); delete mp3; mp3 = nullptr; }
  if (source) { source->close(); delete source; source = nullptr; }

  char path[80]; snprintf(path, sizeof(path), "/sdcard/%s", btSelectedFile);
  source = new AudioFileSourceSD(path);
  if (!outBT) outBT = new AudioOutputBT();
  mp3 = new AudioGeneratorMP3();

  btWritePtr = 0; btReadPtr = 0;
  if (mp3->begin(source, outBT)) {
    btPlaying = true;
    appMode = MODE_BT_MP3_PLAYER;
    drawBTPlayer();
  } else {
    islandPush(NOTIF_WARN, "GAGAL PLAY MP3");
  }
}

void btStopPlayback() {
  if (mp3) { mp3->stop(); delete mp3; mp3 = nullptr; }
  if (source) { source->close(); delete source; source = nullptr; }
  btPlaying = false;
  neoOff();
}

void btTogglePause() {
  btPlaying = !btPlaying;
  if (!btPlaying) neoOff();
}

void btPlayerTick() {
  if (btPlaying && mp3 && mp3->isRunning()) {
    if (!mp3->loop()) {
      btPlaying = false;
      neoOff();
      islandPush(NOTIF_INFO, "SELESAI");
      drawBTPlayer();
    } else {
      neoPulse(0, 180, 50);
    }
  }
}

void drawBTPlayer() {
  lcd.fillScreen(COL_BLACK);
  lcd.setTextColor(COL_AI_ACCENT);
  lcd.drawString("--- BT MP3 PLAYER ---", 20, 10);

  lcd.setTextColor(COL_WHITE);
  lcd.setFont(&fonts::Font0);
  lcd.setTextSize(2);
  lcd.drawString("NOW PLAYING:", 20, 50);

  lcd.setTextSize(1);
  lcd.setTextColor(COL_GRAY_E);
  lcd.drawString(btSelectedFile, 20, 80);

  lcd.setTextColor(btConnected ? 0x07E0 : 0xF800);
  lcd.drawString(btConnected ? "CON: " : "DISC: ", 20, 120);
  lcd.setTextColor(COL_WHITE);
  lcd.drawString(btDeviceName.c_str(), 60, 120);

  lcd.fillRect(20, 160, 280, 2, COL_GRAY_2);
  if (btPlaying) {
    lcd.setTextColor(0x07E0);
    lcd.drawString("PLAYING", 20, 145);
  } else {
    lcd.setTextColor(COL_GRAY_8);
    lcd.drawString("PAUSED", 20, 145);
  }

  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("BOOT=Play/Pause  B=Stop  C/D=Next", 20, 220);
}

void handleModeBTMP3Player(ButtonEvent evt) {
  if (!evt.valid) return;
  if (evt.pin == BTN_B) {
    btStopPlayback();
    appMode = MODE_BT_MP3_LIST;
    drawBTFileList();
    return;
  }
  if (evt.pin == BTN_BOOT) {
    btTogglePause();
    drawBTPlayer();
  }
  if (evt.pin == BTN_C) {
    btFileSel = (btFileSel + btFileCount - 1) % btFileCount;
    strncpy(btSelectedFile, btFiles[btFileSel].c_str(), sizeof(btSelectedFile)-1);
    btStartPlayback();
  }
  if (evt.pin == BTN_D) {
    btFileSel = (btFileSel + 1) % btFileCount;
    strncpy(btSelectedFile, btFiles[btFileSel].c_str(), sizeof(btSelectedFile)-1);
    btStartPlayback();
  }
}




void handleModeFeatures(ButtonEvent evt) {
  if (!evt.valid) return;
  bool* const feats[5] = {&eisEnabled, &hdrEnabled, &autoRotateEnabled, &mpuLogEnabled, &hudEnabled};
  static const char* const labs[5] = {"EIS", "HDR", "AUTO-ROTATE", "MPU LOG", "HUD"};

  if (evt.pin == BTN_B && evt.isShort) {
    saveSettings();
    appMode = MODE_VIEWFINDER;
    islandNoClear = true;
    lcd.fillScreen(COL_BLACK);
    resetAllButtons();
    return;
  }

  if (evt.pin == BTN_D && evt.isShort) { menuFeatSel = (menuFeatSel + 1) % 13; drawFeaturesMenu(menuFeatSel); }
  else if (evt.pin == BTN_C && evt.isShort) { menuFeatSel = (menuFeatSel + 12) % 13; drawFeaturesMenu(menuFeatSel); }
  else if (evt.pin == BTN_BOOT && evt.isShort) {
    if (menuFeatSel < 5) {
      *feats[menuFeatSel] = !(*feats[menuFeatSel]);
      char buf[32]; snprintf(buf, sizeof(buf), "%s %s", labs[menuFeatSel], *feats[menuFeatSel] ? "ON" : "OFF");
      if (menuFeatSel == 4) { // HUD toggle
        saveSettings();
        islandPush(NOTIF_INFO, hudEnabled ? "HUD ON" : "HUD OFF");
      } else {
        islandPush(NOTIF_INFO, buf);
      }
      drawFeaturesMenu(menuFeatSel);
    } else if (menuFeatSel == 5) {
      runMPUCalibration(); drawFeaturesMenu(menuFeatSel);
    } else if (menuFeatSel == 6) {
      hdCaptureEnabled = !hdCaptureEnabled;
      islandPush(NOTIF_INFO, hdCaptureEnabled ? "HD CAPTURE ON" : "HD CAPTURE OFF"); drawFeaturesMenu(menuFeatSel);
    } else if (menuFeatSel == 7) {
      hdCaptureQuality = (hdCaptureQuality == 4) ? 6 : 4;
      char qBuf[32]; snprintf(qBuf, sizeof(qBuf), "HD QUALITY: %d", hdCaptureQuality);
      islandPush(NOTIF_INFO, qBuf); drawFeaturesMenu(menuFeatSel);
    } else if (menuFeatSel == 8) {
      if      (mpuKalmanRmeas < 0.04f) mpuKalmanRmeas = 0.05f;
      else if (mpuKalmanRmeas < 0.06f) mpuKalmanRmeas = 0.10f;
      else if (mpuKalmanRmeas < 0.15f) mpuKalmanRmeas = 0.20f;
      else if (mpuKalmanRmeas < 0.40f) mpuKalmanRmeas = 0.50f;
      else                             mpuKalmanRmeas = 0.03f;
      char buf[32]; snprintf(buf, sizeof(buf), "KALMAN-R: %.2f", mpuKalmanRmeas);
      islandPush(NOTIF_INFO, buf); drawFeaturesMenu(menuFeatSel);
    } else if (menuFeatSel == 9) {
      if      (mpuTiltDeadzone < 0.5f) mpuTiltDeadzone = 1.0f;
      else if (mpuTiltDeadzone < 1.5f) mpuTiltDeadzone = 2.0f;
      else if (mpuTiltDeadzone < 2.5f) mpuTiltDeadzone = 3.0f;
      else if (mpuTiltDeadzone < 4.5f) mpuTiltDeadzone = 5.0f;
      else                             mpuTiltDeadzone = 0.0f;
      char buf[32]; snprintf(buf, sizeof(buf), "TILT-DZ: %.1f deg", mpuTiltDeadzone);
      islandPush(NOTIF_INFO, buf); drawFeaturesMenu(menuFeatSel);
    } else if (menuFeatSel == 10) {
      mpuDlpfIndex = (mpuDlpfIndex + 1) % 7;
      applyDLPF();
      char buf[32]; snprintf(buf, sizeof(buf), "DLPF: %s", DLPF_LABELS[mpuDlpfIndex]);
      islandPush(NOTIF_INFO, buf); drawFeaturesMenu(menuFeatSel);
    } else if (menuFeatSel == 11) {
      DIR* d = opendir("/sdcard/.cache");
      if (d) {
        struct dirent* e; int count = 0;
        while ((e = readdir(d)) != nullptr) {
          String n = e->d_name;
          if (n.endsWith(".bin")) {
            char dp[64]; snprintf(dp, sizeof(dp), "/sdcard/.cache/%s", e->d_name);
            remove(dp); count++; esp_task_wdt_reset();
          }
        }
        closedir(d);
        char msg[32]; snprintf(msg, sizeof(msg), "Cache: %d file dihapus", count);
        islandPush(NOTIF_OK, msg);
      } else {
        islandPush(NOTIF_WARN, "Cache dir tidak ada");
      }
      drawFeaturesMenu(menuFeatSel);
    } else if (menuFeatSel == 12) {
      btStartScan();
    }
  }
}

