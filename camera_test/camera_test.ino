/*
 * ESP32-S3-CAM (Freenove ESP32-S3-WROOM) - VIEWFINDER + SD CARD CAPTURE
 * Version: v4.3-ILI9341  [FIXED: touch zones & capture area]
 *
 * UI THEME: Monochrome — full black/gray/white, terminal aesthetic
 *
 * DISPLAY: ILI9341 2.4" 320×240 landscape + XPT2046 resistive touch
 *
 * Touch controls (viewfinder):
 *  Tap pojok kiri atas  (0..70 x 0..70)   → buka Gallery
 *  Tap pojok kanan atas (250..320 x 0..70) → toggle face detect
 *  Long tap (>1.5s)                        → masuk USB mode
 *  Tap 1/4 kanan layar  (x > 240)          → capture foto
 *
 * Touch controls (gallery):
 *  Swipe atas/bawah     → scroll list
 *  Tap nama file        → lihat foto full screen
 *  Tap pojok kanan      → back ke viewfinder
 *
 * Touch controls (photo view):
 *  Tap pojok kanan      → back ke gallery
 *
 * Tombol fisik:
 *  BOOT singkat        → capture foto ke SD
 *  BOOT tahan 2s       → masuk USB Mass Storage mode
 *  BOOT (di USB)       → keluar dari USB mode
 *  Pin 41 singkat      → mulai/stop rekam video MJPEG
 *  Pin 41 tahan 2s     → toggle face detection mode
 */

#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "FS.h"
#include "esp_task_wdt.h"

#include "human_face_detect_msr01.hpp"
#include "human_face_detect_mnp01.hpp"

#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "diskio_sdmmc.h"
#include <dirent.h>

#include "USB.h"
#include "USBMSC.h"

#include <TJpg_Decoder.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// ─────────────────────────────────────────────────────────────────────────────
//  LGFX Config
// ─────────────────────────────────────────────────────────────────────────────
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341  _panel_instance;
  lgfx::Bus_SPI        _bus_instance;
  lgfx::Touch_XPT2046  _touch_instance;

public:
  LGFX() {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host    = SPI2_HOST;
      cfg.spi_mode    = 0;
      cfg.freq_write  = 40000000;
      cfg.freq_read   = 16000000;
      cfg.spi_3wire   = false;
      cfg.use_lock    = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_mosi    = 45;
      cfg.pin_miso    = 42;
      cfg.pin_sclk    = 47;
      cfg.pin_dc      = 14;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs           = 21;
      cfg.pin_rst          = 1;
      cfg.pin_busy         = -1;
      cfg.panel_width      = 240;
      cfg.panel_height     = 320;
      cfg.offset_x         = 0;
      cfg.offset_y         = 0;
      cfg.offset_rotation  = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits  = 1;
      cfg.readable         = true;
      cfg.invert           = false;
      cfg.rgb_order        = false;
      cfg.dlen_16bit       = false;
      cfg.bus_shared       = true;
      _panel_instance.config(cfg);
    }
    {
      auto cfg = _touch_instance.config();
      cfg.x_min           = 75;
      cfg.x_max           = 285;
      cfg.y_min           = 44;
      cfg.y_max           = 216;
      cfg.pin_int         = 3;
      cfg.bus_shared      = true;
      cfg.offset_rotation = 0;
      cfg.spi_host        = SPI2_HOST;
      cfg.freq            = 2500000;
      cfg.pin_sclk        = 47;
      cfg.pin_mosi        = 45;
      cfg.pin_miso        = 42;
      cfg.pin_cs          = 2;
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
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    15
#define SIOD_GPIO_NUM     4
#define SIOC_GPIO_NUM     5
#define Y9_GPIO_NUM      16
#define Y8_GPIO_NUM      17
#define Y7_GPIO_NUM      18
#define Y6_GPIO_NUM      12
#define Y5_GPIO_NUM      10
#define Y4_GPIO_NUM       8
#define Y3_GPIO_NUM       9
#define Y2_GPIO_NUM      11
#define VSYNC_GPIO_NUM    6
#define HREF_GPIO_NUM     7
#define PCLK_GPIO_NUM    13

// ─────────────────────────────────────────────────────────────────────────────
//  SD Card Pins
// ─────────────────────────────────────────────────────────────────────────────
#define SD_MMC_CMD_PIN   38
#define SD_MMC_CLK_PIN   39
#define SD_MMC_D0_PIN    40

// ─────────────────────────────────────────────────────────────────────────────
//  LED & Tombol
// ─────────────────────────────────────────────────────────────────────────────
#define LED_PIN          48
#define BOOT_BTN_PIN      0
#define REC_BTN_PIN      41

// ─────────────────────────────────────────────────────────────────────────────
//  Display
// ─────────────────────────────────────────────────────────────────────────────
#define DISP_W          320
#define DISP_H          240

// ─────────────────────────────────────────────────────────────────────────────
//  Monochrome palette (RGB565)
// ─────────────────────────────────────────────────────────────────────────────
#define COL_BLACK       0x0000
#define COL_GRAY_D      0x1082
#define COL_GRAY_2      0x2104
#define COL_GRAY_3      0x3186
#define COL_GRAY_5      0x528A
#define COL_GRAY_7      0x7BCF
#define COL_GRAY_8      0x8C51
#define COL_GRAY_A      0xAD55
#define COL_GRAY_C      0xCE59
#define COL_GRAY_E      0xEF5D
#define COL_WHITE       0xFFFF
#define COL_PILL_BG     0x18C3

// ─────────────────────────────────────────────────────────────────────────────
//  Sensor PID
// ─────────────────────────────────────────────────────────────────────────────
#define PID_GC2145  0x2145
#define PID_OV3660  0x3660

// ─────────────────────────────────────────────────────────────────────────────
//  Konstanta
// ─────────────────────────────────────────────────────────────────────────────
#define DEBOUNCE_MS         400
#define LONG_PRESS_MS      2000
#define REC_LONG_PRESS_MS  2000
#define LONG_TAP_MS        1500

// ─────────────────────────────────────────────────────────────────────────────
//  Mirror & Flip
// ─────────────────────────────────────────────────────────────────────────────
#define HMIRROR_GC2145   1
#define VFLIP_GC2145     0
#define HMIRROR_OV3660   0
#define VFLIP_OV3660     1

// ─────────────────────────────────────────────────────────────────────────────
//  App mode
// ─────────────────────────────────────────────────────────────────────────────
enum AppMode { MODE_VIEWFINDER, MODE_GALLERY, MODE_PHOTO_VIEW };
AppMode appMode = MODE_VIEWFINDER;

// ─────────────────────────────────────────────────────────────────────────────
//  Gallery state
// ─────────────────────────────────────────────────────────────────────────────
#define GALLERY_MAX_FILES  200
#define GALLERY_ITEMS_PAGE   8
#define GALLERY_ITEM_H      26

char     galleryFiles[GALLERY_MAX_FILES][32];
int      galleryCount    = 0;
int      galleryScroll   = 0;
char     photoViewPath[48];

// ─────────────────────────────────────────────────────────────────────────────
//  Touch zones  [FIXED]
//  - TOUCH_ZONE_SZ diperbesar 50 → 70 agar lebih mudah kena tap di pojok
//  - inZoneCapture: hanya 1/4 kanan layar (x > 240) yang trigger capture
// ─────────────────────────────────────────────────────────────────────────────
#define TOUCH_ZONE_SZ   70   // ukuran pojok touch zone

bool inZoneTopLeft(int32_t x, int32_t y)  { return x < TOUCH_ZONE_SZ && y < TOUCH_ZONE_SZ; }
bool inZoneTopRight(int32_t x, int32_t y) { return x > DISP_W - TOUCH_ZONE_SZ && y < TOUCH_ZONE_SZ; }
bool inZoneCapture(int32_t x, int32_t y)  { return x > DISP_W * 3 / 4; }  // x > 240

// ─────────────────────────────────────────────────────────────────────────────
//  Video recording state
// ─────────────────────────────────────────────────────────────────────────────
bool          recActive     = false;
FILE*         recFile       = nullptr;
int           recFrameCount = 0;
int           recVideoCount = 0;
unsigned long recStartMs    = 0;

// ─────────────────────────────────────────────────────────────────────────────
//  Face Detection state
// ─────────────────────────────────────────────────────────────────────────────
bool faceDetectMode  = false;
int  faceDetectCount = 0;

// ─────────────────────────────────────────────────────────────────────────────
//  USB MSC
// ─────────────────────────────────────────────────────────────────────────────
USBMSC msc;
bool   usbModeActive = false;

// ─────────────────────────────────────────────────────────────────────────────
//  SD Card state
// ─────────────────────────────────────────────────────────────────────────────
sdmmc_card_t* sdCard         = nullptr;
bool          sdReady         = false;
uint32_t      sdTotalSectors  = 0;
bool          sdmmcDriverInit = false;
uint64_t      sdSizeMB        = 0;

// ─────────────────────────────────────────────────────────────────────────────
//  Global state
// ─────────────────────────────────────────────────────────────────────────────
int           photoCount     = 0;
unsigned long lastBtnPress   = 0;
unsigned long lastTapMs      = 0;
uint16_t      detectedSensor = 0;
char          sensorName[16] = "UNKNOWN";

// ─────────────────────────────────────────────────────────────────────────────
//  FPS meter
// ─────────────────────────────────────────────────────────────────────────────
unsigned long fpsLastTime   = 0;
int           fpsFrameCount = 0;
float         fpsValue      = 0.0f;

// ─────────────────────────────────────────────────────────────────────────────
//  UI constants
// ─────────────────────────────────────────────────────────────────────────────
#define BRACKET_LEN     10
#define BRACKET_W        2
#define PILL_H          13
#define PILL_R           6
#define PILL_PAD_X       5

// ─────────────────────────────────────────────────────────────────────────────
//  TJpgDec callback — output ke lcd
// ─────────────────────────────────────────────────────────────────────────────
bool tjpgdecOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  lcd.pushImage(x, y, w, h, bitmap);
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helper: corner brackets
// ─────────────────────────────────────────────────────────────────────────────
void drawCornerBrackets(uint16_t col = COL_WHITE) {
  int x0 = 3, y0 = 3, x1 = DISP_W - 4, y1 = DISP_H - 4;
  int L  = BRACKET_LEN;
  lcd.drawFastHLine(x0, y0, L, col);
  lcd.drawFastVLine(x0, y0, L, col);
  lcd.drawFastHLine(x1 - L + 1, y0, L, col);
  lcd.drawFastVLine(x1, y0, L, col);
  lcd.drawFastHLine(x0, y1, L, col);
  lcd.drawFastVLine(x0, y1 - L + 1, L, col);
  lcd.drawFastHLine(x1 - L + 1, y1, L, col);
  lcd.drawFastVLine(x1, y1 - L + 1, L, col);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helper: pill
// ─────────────────────────────────────────────────────────────────────────────
void drawPill(int cx, int cy, const char* text,
              uint16_t bgCol, uint16_t fgCol) {
  lcd.setFont(&fonts::Font0);
  lcd.setTextSize(1);
  int tw = lcd.textWidth(text);
  int pw = tw + PILL_PAD_X * 2;
  int px = cx - pw / 2;
  int py = cy - PILL_H / 2;
  lcd.fillRoundRect(px, py, pw, PILL_H, PILL_R, bgCol);
  lcd.drawRoundRect(px, py, pw, PILL_H, PILL_R, COL_GRAY_3);
  lcd.setTextColor(fgCol);
  lcd.drawString(text, px + PILL_PAD_X, py + 2);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helper: RSSI dots
// ─────────────────────────────────────────────────────────────────────────────
void drawRSSIDots(int rssi) {
  uint16_t cols[4] = { COL_GRAY_5, COL_GRAY_7, COL_GRAY_A, COL_GRAY_C };
  int dotY   = DISP_H - 7;
  int startX = DISP_W / 2 - 14;
  for (int i = 0; i < 4; i++) {
    uint16_t c = (i < rssi) ? cols[i] : COL_GRAY_2;
    lcd.fillCircle(startX + i * 9, dotY, 2, c);
  }
}

int getSignalBars() { return 4; }

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers misc
// ─────────────────────────────────────────────────────────────────────────────
void blinkLED(int n, int on_ms = 100, int off_ms = 100) {
  for (int i = 0; i < n; i++) {
    digitalWrite(LED_PIN, HIGH); delay(on_ms);
    digitalWrite(LED_PIN, LOW);  if (i < n - 1) delay(off_ms);
    esp_task_wdt_reset();
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  SD Mount / Unmount / Remount
// ─────────────────────────────────────────────────────────────────────────────
bool mountSDFull() {
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.max_freq_khz = SDMMC_FREQ_DEFAULT;
  sdmmc_slot_config_t slot = SDMMC_SLOT_CONFIG_DEFAULT();
  slot.clk   = (gpio_num_t)SD_MMC_CLK_PIN;
  slot.cmd   = (gpio_num_t)SD_MMC_CMD_PIN;
  slot.d0    = (gpio_num_t)SD_MMC_D0_PIN;
  slot.width = 1;
  slot.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
  esp_vfs_fat_sdmmc_mount_config_t mountCfg = {
    .format_if_mount_failed = false,
    .max_files              = 5,
    .allocation_unit_size   = 16 * 1024
  };
  esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot, &mountCfg, &sdCard);
  if (ret != ESP_OK) {
    Serial.printf("✗ SD Mount Failed: %s\n", esp_err_to_name(ret));
    sdCard = nullptr; sdTotalSectors = 0; sdmmcDriverInit = false; sdSizeMB = 0;
    return false;
  }
  sdTotalSectors  = sdCard->csd.capacity;
  sdmmcDriverInit = true;
  sdSizeMB = (uint64_t)sdTotalSectors * sdCard->csd.sector_size / (1024 * 1024);
  Serial.printf("✓ SD Ready  %lluMB  (%lu sectors)\n", sdSizeMB, sdTotalSectors);
  return true;
}

bool remountVFSOnly() {
  if (!sdCard || !sdmmcDriverInit) return mountSDFull();
  ff_diskio_register_sdmmc(0, sdCard);
  static FATFS fatfs;
  char drv[3] = {'0', ':', 0};
  FRESULT fr = f_mount(&fatfs, drv, 1);
  if (fr != FR_OK) {
    Serial.printf("✗ f_mount gagal: %d\n", fr);
    return false;
  }
  FATFS* fatfsPtr = &fatfs;
  esp_err_t err = esp_vfs_fat_register("/sdcard", "", 5, &fatfsPtr);
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
    Serial.printf("✗ vfs_fat_register gagal: %s\n", esp_err_to_name(err));
    return false;
  }
  Serial.println("✓ SD VFS re-mounted");
  return true;
}

void unmountVFSOnly() {
  esp_vfs_fat_unregister_path("/sdcard");
  ff_diskio_register_sdmmc(0, NULL);
  Serial.println("✓ SD VFS unmounted");
}

void scanPhotoCount() {
  photoCount = 0;
  DIR* dir = opendir("/sdcard");
  if (!dir) return;
  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    String name = entry->d_name;
    if (name.startsWith("photo_") && name.endsWith(".jpg")) {
      int num = name.substring(6, name.length() - 4).toInt();
      if (num > photoCount) photoCount = num;
    }
  }
  closedir(dir);
  Serial.printf("✓ Scan SD done. Next: #%04d\n", photoCount + 1);
}

void scanVideoCount() {
  recVideoCount = 0;
  DIR* dir = opendir("/sdcard");
  if (!dir) return;
  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    String name = entry->d_name;
    if (name.startsWith("video_") && name.endsWith(".mjpeg")) {
      int num = name.substring(6, name.length() - 6).toInt();
      if (num > recVideoCount) recVideoCount = num;
    }
  }
  closedir(dir);
  Serial.printf("✓ Video scan done. Next: #%04d\n", recVideoCount + 1);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Gallery: scan file foto dari SD
// ─────────────────────────────────────────────────────────────────────────────
void scanGalleryFiles() {
  galleryCount  = 0;
  galleryScroll = 0;
  DIR* dir = opendir("/sdcard");
  if (!dir) return;
  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr && galleryCount < GALLERY_MAX_FILES) {
    String name = entry->d_name;
    if (name.endsWith(".jpg") || name.endsWith(".JPG")) {
      strncpy(galleryFiles[galleryCount], entry->d_name, 31);
      galleryFiles[galleryCount][31] = '\0';
      galleryCount++;
    }
  }
  closedir(dir);
  // Sort sederhana (bubble sort by name)
  for (int i = 0; i < galleryCount - 1; i++) {
    for (int j = 0; j < galleryCount - i - 1; j++) {
      if (strcmp(galleryFiles[j], galleryFiles[j+1]) > 0) {
        char tmp[32];
        strncpy(tmp, galleryFiles[j], 31);
        strncpy(galleryFiles[j], galleryFiles[j+1], 31);
        strncpy(galleryFiles[j+1], tmp, 31);
      }
    }
    esp_task_wdt_reset();
  }
  Serial.printf("✓ Gallery: %d foto ditemukan\n", galleryCount);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Gallery: render list
// ─────────────────────────────────────────────────────────────────────────────
void drawGallery() {
  lcd.fillScreen(COL_BLACK);

  // Header
  lcd.fillRect(0, 0, DISP_W, 20, COL_GRAY_D);
  lcd.drawFastHLine(0, 20, DISP_W, COL_GRAY_3);
  lcd.setFont(&fonts::Font0);
  lcd.setTextColor(COL_GRAY_E);
  char hdr[32];
  snprintf(hdr, sizeof(hdr), "GALLERY  %d foto", galleryCount);
  int hw = lcd.textWidth(hdr);
  lcd.drawString(hdr, (DISP_W - hw) / 2, 6);

  // Back button pojok kanan atas
  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("BACK >", DISP_W - 44, 6);

  if (galleryCount == 0) {
    lcd.setTextColor(COL_GRAY_5);
    int ew = lcd.textWidth("SD kosong / tidak ada foto");
    lcd.drawString("SD kosong / tidak ada foto", (DISP_W - ew) / 2, DISP_H / 2);
    return;
  }

  // List item
  int visibleEnd = min(galleryScroll + GALLERY_ITEMS_PAGE, galleryCount);
  for (int i = galleryScroll; i < visibleEnd; i++) {
    int rowY = 24 + (i - galleryScroll) * GALLERY_ITEM_H;
    bool isEven = (i % 2 == 0);
    lcd.fillRect(0, rowY, DISP_W, GALLERY_ITEM_H - 1, isEven ? COL_GRAY_D : COL_BLACK);
    lcd.drawFastHLine(0, rowY + GALLERY_ITEM_H - 1, DISP_W, COL_GRAY_2);

    // Nomor
    lcd.setFont(&fonts::Font0);
    lcd.setTextColor(COL_GRAY_5);
    char num[6];
    snprintf(num, sizeof(num), "%3d", i + 1);
    lcd.drawString(num, 6, rowY + 8);

    // Nama file
    lcd.setTextColor(COL_GRAY_C);
    lcd.drawString(galleryFiles[i], 30, rowY + 8);

    // Panah
    lcd.setTextColor(COL_GRAY_3);
    lcd.drawString(">", DISP_W - 12, rowY + 8);
  }

  // Scroll indicator
  if (galleryCount > GALLERY_ITEMS_PAGE) {
    int barH = DISP_H - 22;
    int indH = max(10, barH * GALLERY_ITEMS_PAGE / galleryCount);
    int indY = 22 + (barH - indH) * galleryScroll / max(1, galleryCount - GALLERY_ITEMS_PAGE);
    lcd.fillRect(DISP_W - 4, 22, 4, barH, COL_GRAY_2);
    lcd.fillRect(DISP_W - 4, indY, 4, indH, COL_GRAY_7);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Photo view: decode & tampil JPEG dari SD
// ─────────────────────────────────────────────────────────────────────────────
void showPhotoView(const char* filename) {
  lcd.fillScreen(COL_BLACK);

  char path[48];
  snprintf(path, sizeof(path), "/sdcard/%s", filename);

  // Baca file ke buffer
  FILE* f = fopen(path, "rb");
  if (!f) {
    lcd.setFont(&fonts::Font0);
    lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("file tidak bisa dibuka", 20, DISP_H / 2);
    delay(1500);
    return;
  }

  fseek(f, 0, SEEK_END);
  size_t fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  uint8_t* buf = (uint8_t*)malloc(fsize);
  if (!buf) {
    fclose(f);
    lcd.setFont(&fonts::Font0);
    lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("out of memory", 20, DISP_H / 2);
    delay(1500);
    return;
  }

  fread(buf, 1, fsize, f);
  fclose(f);

  // Decode & render
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tjpgdecOutput);
  uint16_t w = 0, h = 0;
  TJpgDec.getJpgSize(&w, &h, buf, fsize);

  // Center jika lebih kecil dari layar
  int ox = (DISP_W - min((int)w, DISP_W)) / 2;
  int oy = (DISP_H - min((int)h, DISP_H)) / 2;
  TJpgDec.drawJpg(ox, oy, buf, fsize);

  free(buf);

  // Overlay: nama file + back button
  lcd.fillRect(0, DISP_H - 16, DISP_W, 16, COL_GRAY_D);
  lcd.setFont(&fonts::Font0);
  lcd.setTextColor(COL_GRAY_A);
  int nw = lcd.textWidth(filename);
  lcd.drawString(filename, (DISP_W - nw) / 2, DISP_H - 13);

  // Back button pojok kanan atas
  lcd.fillRect(DISP_W - 50, 0, 50, 16, COL_GRAY_D);
  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("BACK >", DISP_W - 44, 3);

  strncpy(photoViewPath, filename, sizeof(photoViewPath) - 1);
}

// ─────────────────────────────────────────────────────────────────────────────
//  UI: REC indicator overlay
// ─────────────────────────────────────────────────────────────────────────────
void drawRecIndicator() {
  if (!recActive) return;
  unsigned long elapsed = (millis() - recStartMs) / 1000;
  unsigned long mm = elapsed / 60;
  unsigned long ss = elapsed % 60;
  char timeBuf[12];
  snprintf(timeBuf, sizeof(timeBuf), "%02lu:%02lu", mm, ss);

  bool blink = (millis() / 500) % 2;
  uint16_t dotCol = blink ? COL_WHITE : COL_GRAY_5;
  lcd.fillCircle(DISP_W / 2 - 22, 10, 4, dotCol);

  lcd.setFont(&fonts::Font0);
  lcd.setTextColor(COL_WHITE);
  lcd.drawString(timeBuf, DISP_W / 2 - 14, 6);

  char fBuf[12];
  snprintf(fBuf, sizeof(fBuf), "%df", recFrameCount);
  lcd.setTextColor(COL_GRAY_7);
  lcd.drawString(fBuf, DISP_W / 2 + 24, 6);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Start / Stop recording
// ─────────────────────────────────────────────────────────────────────────────
void startRecording() {
  if (!sdReady) { Serial.println("[REC] Batal: SD tidak siap"); return; }
  if (faceDetectMode) { faceDetectMode = false; faceDetectCount = 0; }

  recVideoCount++;
  char path[48];
  snprintf(path, sizeof(path), "/sdcard/video_%04d.mjpeg", recVideoCount);
  recFile = fopen(path, "wb");
  if (!recFile) { recVideoCount--; return; }
  recFrameCount = 0;
  recStartMs    = millis();
  recActive     = true;
  Serial.printf("● REC START: %s\n", path);

  lcd.setFont(&fonts::Font0);
  char buf[20];
  snprintf(buf, sizeof(buf), "REC  #%04d", recVideoCount);
  int tw = lcd.textWidth(buf);
  int pw = tw + 14;
  int px = (DISP_W - pw) / 2;
  lcd.fillRoundRect(px, 6, pw, 15, 7, COL_WHITE);
  lcd.setTextColor(COL_BLACK);
  lcd.drawString(buf, px + 7, 9);
  delay(600);
}

void stopRecording() {
  if (!recActive || !recFile) return;
  fclose(recFile);
  recFile   = nullptr;
  recActive = false;
  unsigned long dur = (millis() - recStartMs) / 1000;
  Serial.printf("■ REC STOP: %d frames, %lus\n", recFrameCount, dur);

  lcd.setFont(&fonts::Font0);
  char buf[28];
  snprintf(buf, sizeof(buf), "SAVED  %df  %02lu:%02lu", recFrameCount, dur / 60, dur % 60);
  int tw = lcd.textWidth(buf);
  int pw = tw + 14;
  int px = (DISP_W - pw) / 2;
  lcd.fillRoundRect(px, 6, pw, 15, 7, COL_WHITE);
  lcd.setTextColor(COL_BLACK);
  lcd.drawString(buf, px + 7, 9);
  blinkLED(3, 100, 80);
  delay(2000);
  lcd.fillScreen(COL_BLACK);
  fpsLastTime = millis(); fpsFrameCount = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Record satu frame
// ─────────────────────────────────────────────────────────────────────────────
void recordFrame() {
  if (!recActive || !recFile) return;
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) { esp_task_wdt_reset(); return; }

  uint8_t *jpg_buf = nullptr;
  size_t   jpg_len = 0;
  bool     ok      = false;

  if (fb->format == PIXFORMAT_JPEG) {
    jpg_buf = fb->buf; jpg_len = fb->len; ok = true;
  } else {
    ok = frame2jpg(fb, 70, &jpg_buf, &jpg_len);
  }

  if (ok && jpg_buf) {
    fwrite(jpg_buf, 1, jpg_len, recFile);
    recFrameCount++;
    if (fb->format != PIXFORMAT_JPEG) free(jpg_buf);
  }

  if (recFrameCount % 3 == 0) {
    if (fb->format == PIXFORMAT_RGB565 && fb->width == DISP_W) {
      lcd.pushImage(0, 0, DISP_W, DISP_H, (uint16_t*)fb->buf);
      drawCornerBrackets(COL_GRAY_E);
      drawRecIndicator();
    }
  }

  esp_camera_fb_return(fb);
  esp_task_wdt_reset();
}

// ─────────────────────────────────────────────────────────────────────────────
//  USB MSC Callbacks
// ─────────────────────────────────────────────────────────────────────────────
static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
  (void)offset;
  if (!sdCard || !sdmmcDriverInit) return -1;
  return (sdmmc_read_sectors(sdCard, buffer, lba, bufsize / 512) == ESP_OK) ? (int32_t)bufsize : -1;
}
static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
  (void)offset;
  if (!sdCard || !sdmmcDriverInit) return -1;
  return (sdmmc_write_sectors(sdCard, buffer, lba, bufsize / 512) == ESP_OK) ? (int32_t)bufsize : -1;
}

// ─────────────────────────────────────────────────────────────────────────────
//  UI: STARTUP SCREEN
// ─────────────────────────────────────────────────────────────────────────────
void drawCameraIcon(int cx, int cy, uint16_t col) {
  lcd.drawRoundRect(cx - 22, cy - 14, 44, 30, 4, col);
  lcd.drawCircle(cx, cy - 1, 10, col);
  lcd.drawCircle(cx, cy - 1, 5, COL_GRAY_5);
  lcd.drawRect(cx - 16, cy - 18, 12, 5, col);
  lcd.drawPixel(cx + 4, cy - 5, COL_GRAY_7);
}

void bootLogLine(int y, const char* label, const char* status,
                 uint16_t statusCol = COL_GRAY_E) {
  lcd.setFont(&fonts::Font0);
  lcd.setTextSize(1);
  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString(label, 28, y);
  int lx = 28 + lcd.textWidth(label) + 3;
  for (int dx = lx; dx < 268; dx += 4) lcd.drawPixel(dx, y + 5, COL_GRAY_3);
  lcd.setTextColor(statusCol);
  lcd.drawString(status, 272, y);
}

void runBootSequence(bool sdOK, uint64_t sdMB,
                     bool pidOK, uint16_t pid,
                     bool xclkOK, uint32_t xclkHz) {
  lcd.fillScreen(COL_BLACK);
  lcd.drawFastHLine(0, 0, DISP_W, COL_GRAY_3);
  lcd.drawFastHLine(0, DISP_H - 1, DISP_W, COL_GRAY_3);

  lcd.setFont(&fonts::Font0);
  lcd.setTextColor(COL_GRAY_3);
  lcd.drawString("ESP32-S3", 6, 4);

  drawCameraIcon(DISP_W / 2, 55, COL_GRAY_7);

  lcd.setFont(&fonts::FreeSansBold9pt7b);
  lcd.setTextColor(COL_GRAY_E);
  lcd.drawString("SANZXCAM", DISP_W / 2 - 57, 85);

  lcd.setFont(&fonts::Font0);
  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("v4.3  ILI9341 + XPT2046", DISP_W / 2 - 72, 103);

  lcd.drawFastHLine(20, 116, DISP_W - 40, COL_GRAY_2);

  esp_task_wdt_reset();
  delay(200);

  char buf[32];
  if (sdOK) snprintf(buf, sizeof(buf), "OK  %lluMB", sdMB);
  else      snprintf(buf, sizeof(buf), "NOT FOUND");
  bootLogLine(122, "SD CARD", buf, sdOK ? COL_GRAY_E : COL_GRAY_5);
  delay(120); esp_task_wdt_reset();

  bootLogLine(134, "CAM PROBE", "20MHz", COL_GRAY_E);
  delay(120); esp_task_wdt_reset();

  if (pidOK) snprintf(buf, sizeof(buf), "0x%04X  %s", pid, sensorName);
  else       snprintf(buf, sizeof(buf), "0x%04X  ???", pid);
  bootLogLine(146, "SENSOR PID", buf, pidOK ? COL_GRAY_E : COL_GRAY_5);
  delay(120); esp_task_wdt_reset();

  snprintf(buf, sizeof(buf), "%uMHz  OK", xclkHz / 1000000);
  bootLogLine(158, "XCLK", buf, xclkOK ? COL_GRAY_E : COL_GRAY_5);
  delay(120); esp_task_wdt_reset();

  bootLogLine(170, "TOUCH", "XPT2046  OK", COL_GRAY_E);
  delay(120); esp_task_wdt_reset();

  lcd.drawRect(28, 188, DISP_W - 56, 4, COL_GRAY_3);
  int barW = DISP_W - 60;
  for (int i = 0; i <= barW; i += 4) {
    lcd.fillRect(30, 189, i, 2, COL_GRAY_7);
    if (i % 16 == 0) esp_task_wdt_reset();
  }
  lcd.fillRect(30, 189, barW, 2, COL_GRAY_C);

  lcd.setFont(&fonts::Font0);
  lcd.setTextColor(COL_GRAY_A);
  lcd.drawString("READY", DISP_W / 2 - 15, 198);

  delay(800);
  esp_task_wdt_reset();
}

// ─────────────────────────────────────────────────────────────────────────────
//  UI: USB MODE SCREEN
// ─────────────────────────────────────────────────────────────────────────────
void drawUSBIcon(int cx, int cy, uint16_t col) {
  lcd.drawFastVLine(cx, cy - 24, 28, col);
  lcd.drawCircle(cx, cy - 27, 4, col);
  lcd.drawFastHLine(cx - 18, cy - 12, 36, col);
  lcd.drawFastVLine(cx - 18, cy - 12, 12, col);
  lcd.drawCircle(cx - 18, cy + 2, 4, col);
  lcd.drawFastVLine(cx + 18, cy - 12, 10, col);
  lcd.drawRect(cx + 13, cy - 2, 10, 7, col);
}

void drawUSBModeScreen() {
  lcd.fillScreen(COL_BLACK);
  lcd.fillRect(0, 0, DISP_W, 18, COL_GRAY_D);
  lcd.drawFastHLine(0, 18, DISP_W, COL_GRAY_3);

  lcd.setFont(&fonts::Font0);
  lcd.setTextColor(COL_GRAY_3);
  lcd.drawString("ESP32-S3", 6, 4);
  lcd.setTextColor(COL_GRAY_5);
  int lw = lcd.textWidth("USB MASS STORAGE");
  lcd.drawString("USB MASS STORAGE", (DISP_W - lw) / 2, 4);
  lcd.setTextColor(COL_GRAY_3);
  lcd.drawString("v4.3", DISP_W - 24, 4);

  drawUSBIcon(DISP_W / 2, 85, COL_GRAY_7);

  lcd.setFont(&fonts::FreeSansBold9pt7b);
  lcd.setTextColor(COL_GRAY_E);
  int mw = lcd.textWidth("SD CONNECTED");
  lcd.drawString("SD CONNECTED", (DISP_W - mw) / 2, 118);

  lcd.setFont(&fonts::Font0);
  lcd.setTextColor(COL_GRAY_5);
  int sw = lcd.textWidth("to host via USB");
  lcd.drawString("to host via USB", (DISP_W - sw) / 2, 136);
  lcd.drawFastHLine(40, 150, DISP_W - 80, COL_GRAY_2);

  auto infoRow = [&](int y, const char* key, const char* val) {
    lcd.setFont(&fonts::Font0);
    lcd.setTextColor(COL_GRAY_5);
    lcd.drawString(key, 44, y);
    lcd.setTextColor(COL_GRAY_C);
    int vw = lcd.textWidth(val);
    lcd.drawString(val, DISP_W - 44 - vw, y);
  };

  char buf[32];
  infoRow(156, "SENSOR", sensorName);
  if (sdSizeMB < 1024) snprintf(buf, sizeof(buf), "%lluMB  FAT32", sdSizeMB);
  else                  snprintf(buf, sizeof(buf), "%lluGB  FAT32", sdSizeMB / 1024);
  infoRow(170, "STORAGE", buf);
  snprintf(buf, sizeof(buf), "%04d files", photoCount);
  infoRow(184, "PHOTOS", buf);

  lcd.fillRect(0, DISP_H - 18, DISP_W, 18, COL_GRAY_D);
  lcd.drawFastHLine(0, DISP_H - 18, DISP_W, COL_GRAY_3);
  lcd.setFont(&fonts::Font0);
  lcd.setTextColor(COL_GRAY_5);
  int ew = lcd.textWidth("eject from host first");
  lcd.drawString("eject from host first", (DISP_W - ew) / 2, DISP_H - 15);
  lcd.drawRoundRect((DISP_W / 2) - 68, DISP_H - 32, 136, 13, 2, COL_GRAY_3);
  int hw = lcd.textWidth("[ BOOT ]  exit usb mode");
  lcd.drawString("[ BOOT ]  exit usb mode", (DISP_W - hw) / 2, DISP_H - 30);
}

// ─────────────────────────────────────────────────────────────────────────────
//  USB Mode enter / exit
// ─────────────────────────────────────────────────────────────────────────────
void enterUSBMode() {
  if (!sdReady) { Serial.println("[USB] Batal: SD tidak siap."); return; }
  unmountVFSOnly();
  sdReady = false;
  esp_task_wdt_reset();
  msc.mediaPresent(true);
  esp_task_wdt_reset();
  drawUSBModeScreen();
  usbModeActive = true;
  blinkLED(3, 200, 100);
}

void exitUSBMode() {
  usbModeActive = false;
  msc.mediaPresent(false);
  esp_task_wdt_reset();

  lcd.fillScreen(COL_BLACK);
  lcd.setFont(&fonts::Font0);
  lcd.setTextColor(COL_GRAY_5);
  int tw = lcd.textWidth("reconnecting sd...");
  lcd.drawString("reconnecting sd...", (DISP_W - tw) / 2, DISP_H / 2 - 6);

  sdReady = remountVFSOnly();
  if (sdReady) {
    scanPhotoCount();
    lcd.fillRect(0, DISP_H / 2 - 10, DISP_W, 20, COL_BLACK);
    char buf[32];
    snprintf(buf, sizeof(buf), "sd ok  next #%04d", photoCount + 1);
    lcd.setTextColor(COL_GRAY_C);
    int bw = lcd.textWidth(buf);
    lcd.drawString(buf, (DISP_W - bw) / 2, DISP_H / 2 - 6);
  } else {
    lcd.fillRect(0, DISP_H / 2 - 10, DISP_W, 20, COL_BLACK);
    lcd.setTextColor(COL_GRAY_5);
    int fw = lcd.textWidth("sd mount failed");
    lcd.drawString("sd mount failed", (DISP_W - fw) / 2, DISP_H / 2 - 6);
  }

  esp_task_wdt_reset();
  blinkLED(2, 150, 100);
  delay(1000);
  lcd.fillScreen(COL_BLACK);
  appMode = MODE_VIEWFINDER;
  fpsLastTime = millis(); fpsFrameCount = 0; fpsValue = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
//  FPS
// ─────────────────────────────────────────────────────────────────────────────
void updateFPS() {
  fpsFrameCount++;
  unsigned long elapsed = millis() - fpsLastTime;
  if (elapsed >= 500) {
    fpsValue = fpsFrameCount * 1000.0f / elapsed;
    fpsFrameCount = 0; fpsLastTime = millis();
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Face Detection
// ─────────────────────────────────────────────────────────────────────────────
void runFaceDetect(camera_fb_t *fb) {
  if (!fb || fb->format != PIXFORMAT_RGB565) return;

  HumanFaceDetectMSR01 msr(0.1f, 0.5f, 10, 0.2f);
  HumanFaceDetectMNP01 mnp(0.5f, 0.3f, 5);

  std::list<dl::detect::result_t> &candidates = msr.infer(
    (uint16_t *)fb->buf, {(int)fb->height, (int)fb->width, 3}
  );
  std::list<dl::detect::result_t> &results = mnp.infer(
    (uint16_t *)fb->buf, {(int)fb->height, (int)fb->width, 3}, candidates
  );

  faceDetectCount = results.size();

  for (auto &res : results) {
    int x1 = constrain(res.box[0], 0, DISP_W - 1);
    int y1 = constrain(res.box[1], 0, DISP_H - 1);
    int x2 = constrain(res.box[2], 0, DISP_W - 1);
    int y2 = constrain(res.box[3], 0, DISP_H - 1);
    int bw = x2 - x1, bh = y2 - y1;
    if (bw < 4 || bh < 4) continue;
    int CL = min(12, min(bw, bh) / 3);
    uint16_t fc = COL_WHITE;
    lcd.drawFastHLine(x1, y1, CL, fc); lcd.drawFastVLine(x1, y1, CL, fc);
    lcd.drawFastHLine(x2 - CL, y1, CL, fc); lcd.drawFastVLine(x2, y1, CL, fc);
    lcd.drawFastHLine(x1, y2, CL, fc); lcd.drawFastVLine(x1, y2 - CL, CL, fc);
    lcd.drawFastHLine(x2 - CL, y2, CL, fc); lcd.drawFastVLine(x2, y2 - CL, CL, fc);
    if (res.keypoint.size() >= 10) {
      for (int k = 0; k < 10; k += 2) {
        int kx = constrain(res.keypoint[k],   0, DISP_W - 1);
        int ky = constrain(res.keypoint[k+1], 0, DISP_H - 1);
        lcd.fillCircle(kx, ky, 2, COL_GRAY_C);
      }
    }
  }
}

void toggleFaceDetect() {
  faceDetectMode = !faceDetectMode;
  faceDetectCount = 0;
  Serial.printf("Face Detect: %s\n", faceDetectMode ? "ON" : "OFF");

  lcd.setFont(&fonts::Font0);
  const char* msg = faceDetectMode ? "FACE DETECT  ON" : "FACE DETECT  OFF";
  int tw = lcd.textWidth(msg);
  int pw = tw + 14;
  int px = (DISP_W - pw) / 2;
  lcd.fillRoundRect(px, DISP_H / 2 - 8, pw, 15, 7,
                    faceDetectMode ? COL_WHITE : COL_GRAY_3);
  lcd.setTextColor(faceDetectMode ? COL_BLACK : COL_GRAY_5);
  lcd.drawString(msg, px + 7, DISP_H / 2 - 5);
  delay(800);
  lcd.fillRect(0, DISP_H / 2 - 12, DISP_W, 22, COL_BLACK);
}

// ─────────────────────────────────────────────────────────────────────────────
//  UI: VIEWFINDER
// ─────────────────────────────────────────────────────────────────────────────
void renderViewfinder() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) return;

  if (fb->format == PIXFORMAT_RGB565 && fb->width == DISP_W && fb->height == DISP_H) {
    lcd.pushImage(0, 0, DISP_W, DISP_H, (uint16_t*)fb->buf);
    drawCornerBrackets(COL_GRAY_E);

    if (faceDetectMode) runFaceDetect(fb);

    char fpsBuf[12];
    snprintf(fpsBuf, sizeof(fpsBuf), "%.0f fps", fpsValue);
    drawPill(32, 10, fpsBuf, COL_PILL_BG, COL_GRAY_A);
    drawPill(DISP_W - 35, 10, sensorName, COL_PILL_BG, COL_GRAY_A);

    char shotBuf[10];
    snprintf(shotBuf, sizeof(shotBuf), "#%04d", photoCount + 1);
    drawPill(30, DISP_H - 10, shotBuf, COL_PILL_BG, COL_GRAY_8);

    const char* sdLabel = sdReady ? "SD  OK" : "SD  --";
    drawPill(DISP_W - 36, DISP_H - 10, sdLabel, COL_PILL_BG,
             sdReady ? COL_GRAY_8 : COL_GRAY_5);

    if (faceDetectMode) {
      char faceBuf[12];
      snprintf(faceBuf, sizeof(faceBuf),
               faceDetectCount > 0 ? "FACE  %d" : "FACE  --", faceDetectCount);
      drawPill(DISP_W / 2, DISP_H - 10, faceBuf, COL_PILL_BG, COL_GRAY_C);
    } else {
      drawRSSIDots(getSignalBars());
    }

    updateFPS();
  } else {
    lcd.fillScreen(COL_BLACK);
    lcd.setFont(&fonts::Font0);
    lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("format not rgb565", 10, 110);
  }
  esp_camera_fb_return(fb);
}

// ─────────────────────────────────────────────────────────────────────────────
//  UI: CAPTURE FEEDBACK
// ─────────────────────────────────────────────────────────────────────────────
void showSavedFeedback(bool saved) {
  lcd.setFont(&fonts::Font0);
  if (saved) {
    char buf[16];
    snprintf(buf, sizeof(buf), "SAVED  #%04d", photoCount);
    int tw = lcd.textWidth(buf);
    int pw = tw + 14;
    int px = (DISP_W - pw) / 2;
    lcd.fillRoundRect(px, 6, pw, 15, 7, COL_WHITE);
    lcd.setTextColor(COL_BLACK);
    lcd.drawString(buf, px + 7, 9);
  } else {
    const char* msg = sdReady ? "WRITE ERR" : "NO SD";
    int tw = lcd.textWidth(msg);
    int pw = tw + 14;
    int px = (DISP_W - pw) / 2;
    lcd.fillRoundRect(px, 6, pw, 15, 7, COL_GRAY_3);
    lcd.setTextColor(COL_GRAY_5);
    lcd.drawString(msg, px + 7, 9);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Capture & Simpan ke SD
// ─────────────────────────────────────────────────────────────────────────────
void captureAndPreview() {
  digitalWrite(LED_PIN, HIGH);
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("✗ Gagal ambil frame");
    digitalWrite(LED_PIN, LOW);
    blinkLED(5, 50, 50);
    return;
  }
  digitalWrite(LED_PIN, LOW);

  if (fb->format == PIXFORMAT_RGB565 && fb->width == DISP_W) {
    lcd.pushImage(0, 0, DISP_W, DISP_H, (uint16_t*)fb->buf);
    drawCornerBrackets(COL_GRAY_E);
  }

  bool saved = false;
  if (sdReady) {
    uint8_t *jpg_buf = nullptr;
    size_t   jpg_len = 0;
    bool     ok      = false;

    if (fb->format == PIXFORMAT_JPEG) {
      jpg_buf = fb->buf; jpg_len = fb->len; ok = true;
    } else {
      ok = frame2jpg(fb, 85, &jpg_buf, &jpg_len);
      if (!ok) Serial.println("✗ frame2jpg gagal");
    }

    if (ok && jpg_buf) {
      photoCount++;
      char path[40];
      snprintf(path, sizeof(path), "/sdcard/photo_%04d.jpg", photoCount);
      FILE* f = fopen(path, "wb");
      if (f) {
        size_t w = fwrite(jpg_buf, 1, jpg_len, f);
        fclose(f);
        saved = (w == jpg_len);
        if (saved) Serial.printf("✓ Saved: %s (%u bytes)\n", path, jpg_len);
      }
      if (fb->format != PIXFORMAT_JPEG) free(jpg_buf);
    }
  }

  esp_camera_fb_return(fb);
  showSavedFeedback(saved);
  if (saved) blinkLED(2, 150, 80);
  else       blinkLED(5, 50, 50);
  delay(2000);
  fpsLastTime = millis(); fpsFrameCount = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Apply sensor settings
// ─────────────────────────────────────────────────────────────────────────────
void applySensorSettings(sensor_t *s, uint16_t pid) {
  s->set_brightness(s, 0); s->set_contrast(s, 0); s->set_saturation(s, 0);
  s->set_special_effect(s, 0); s->set_whitebal(s, 1); s->set_awb_gain(s, 1);
  s->set_wb_mode(s, 0); s->set_exposure_ctrl(s, 1); s->set_aec2(s, 1);
  s->set_ae_level(s, 0); s->set_gain_ctrl(s, 1); s->set_agc_gain(s, 0);

  if (pid == PID_GC2145) {
    s->set_aec_value(s, 300); s->set_gainceiling(s, GAINCEILING_4X);
    s->set_hmirror(s, HMIRROR_GC2145); s->set_vflip(s, VFLIP_GC2145);
  } else if (pid == PID_OV3660) {
    s->set_aec_value(s, 400); s->set_gainceiling(s, GAINCEILING_4X);
    s->set_hmirror(s, HMIRROR_OV3660); s->set_vflip(s, VFLIP_OV3660);
  } else {
    s->set_aec_value(s, 300); s->set_gainceiling(s, GAINCEILING_2X);
    s->set_hmirror(s, 0); s->set_vflip(s, 0);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Camera Init
// ─────────────────────────────────────────────────────────────────────────────
bool initCamera() {
  camera_config_t cfg;
  cfg.ledc_channel = LEDC_CHANNEL_0; cfg.ledc_timer = LEDC_TIMER_0;
  cfg.pin_d0 = Y2_GPIO_NUM; cfg.pin_d1 = Y3_GPIO_NUM;
  cfg.pin_d2 = Y4_GPIO_NUM; cfg.pin_d3 = Y5_GPIO_NUM;
  cfg.pin_d4 = Y6_GPIO_NUM; cfg.pin_d5 = Y7_GPIO_NUM;
  cfg.pin_d6 = Y8_GPIO_NUM; cfg.pin_d7 = Y9_GPIO_NUM;
  cfg.pin_xclk = XCLK_GPIO_NUM; cfg.pin_pclk = PCLK_GPIO_NUM;
  cfg.pin_vsync = VSYNC_GPIO_NUM; cfg.pin_href = HREF_GPIO_NUM;
  cfg.pin_sccb_sda = SIOD_GPIO_NUM; cfg.pin_sccb_scl = SIOC_GPIO_NUM;
  cfg.pin_pwdn = PWDN_GPIO_NUM; cfg.pin_reset = RESET_GPIO_NUM;
  cfg.pixel_format = PIXFORMAT_RGB565;
  cfg.frame_size   = FRAMESIZE_QVGA;
  cfg.grab_mode    = CAMERA_GRAB_LATEST;

  if (psramFound()) {
    cfg.jpeg_quality = 7; cfg.fb_count = 2;
    cfg.fb_location = CAMERA_FB_IN_PSRAM;
  } else {
    cfg.jpeg_quality = 12; cfg.fb_count = 1;
    cfg.fb_location = CAMERA_FB_IN_DRAM;
  }

  cfg.xclk_freq_hz = 20000000;
  if (esp_camera_init(&cfg) != ESP_OK) return false;

  sensor_t *s = esp_camera_sensor_get();
  if (!s) return false;
  detectedSensor = s->id.PID;

  if (detectedSensor == PID_OV3660) {
    esp_camera_deinit(); delay(100);
    cfg.xclk_freq_hz = 24000000;
    if (esp_camera_init(&cfg) != ESP_OK) {
      cfg.xclk_freq_hz = 20000000;
      if (esp_camera_init(&cfg) != ESP_OK) return false;
    }
    s = esp_camera_sensor_get();
    if (!s) return false;
  }

  switch (detectedSensor) {
    case PID_GC2145: strncpy(sensorName, "GC2145", sizeof(sensorName)); break;
    case PID_OV3660: strncpy(sensorName, "OV3660", sizeof(sensorName)); break;
    default: snprintf(sensorName, sizeof(sensorName), "0x%04X", detectedSensor); break;
  }

  applySensorSettings(s, detectedSensor);
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Setup
// ─────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Sanzxcam v4.3 ===");

  pinMode(LED_PIN, OUTPUT); digitalWrite(LED_PIN, LOW);
  pinMode(BOOT_BTN_PIN, INPUT_PULLUP);
  pinMode(REC_BTN_PIN, INPUT_PULLUP);
  setCpuFrequencyMhz(240);

  lcd.init();
  lcd.setRotation(3);
  lcd.fillScreen(COL_BLACK);

  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tjpgdecOutput);

  sdReady = mountSDFull();
  if (sdReady) { scanPhotoCount(); scanVideoCount(); }

  msc.vendorID("ESP32S3"); msc.productID("SD Card"); msc.productRevision("1.0");
  msc.onRead(onRead); msc.onWrite(onWrite);
  msc.begin(sdTotalSectors > 0 ? sdTotalSectors : 0, 512);
  msc.mediaPresent(false);
  USB.begin();

  bool camOK = initCamera();
  bool pidOK = (detectedSensor == PID_GC2145 || detectedSensor == PID_OV3660);
  uint32_t xclkHz = (detectedSensor == PID_OV3660) ? 24000000 : 20000000;
  runBootSequence(sdReady, sdSizeMB, pidOK, detectedSensor, camOK, xclkHz);

  if (!camOK) {
    lcd.fillScreen(COL_BLACK);
    lcd.setFont(&fonts::Font0);
    lcd.setTextColor(COL_GRAY_5);
    int ew = lcd.textWidth("camera init failed");
    lcd.drawString("camera init failed", (DISP_W - ew) / 2, 110);
    while (true) { blinkLED(3, 100, 100); delay(1000); }
  }

  lcd.fillScreen(COL_BLACK);
  fpsLastTime = millis(); fpsFrameCount = 0;
  blinkLED(3, 120, 120);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Loop
// ─────────────────────────────────────────────────────────────────────────────
void loop() {

  // ── Tombol REC (pin 41) ───────────────────────────────────────────────────
  if (appMode == MODE_VIEWFINDER && digitalRead(REC_BTN_PIN) == LOW) {
    delay(50);
    if (digitalRead(REC_BTN_PIN) == LOW) {
      unsigned long recPressStart = millis();
      while (digitalRead(REC_BTN_PIN) == LOW) {
        delay(10); esp_task_wdt_reset();
        if (!recActive) {
          unsigned long held = millis() - recPressStart;
          if (held > 500) {
            int barW = constrain(
              (int)map((long)held, 500L, (long)REC_LONG_PRESS_MS, 0L, (long)DISP_W),
              0, DISP_W);
            lcd.fillRect(0, 0, barW, 3, COL_GRAY_5);
            lcd.fillRect(barW, 0, DISP_W - barW, 3, COL_BLACK);
          }
        }
      }
      unsigned long recDuration = millis() - recPressStart;
      lcd.fillRect(0, 0, DISP_W, 3, COL_BLACK);
      if (recActive)                        stopRecording();
      else if (recDuration >= REC_LONG_PRESS_MS) toggleFaceDetect();
      else                                   startRecording();
    }
  }

  if (recActive) { recordFrame(); return; }

  // ── Tombol BOOT ───────────────────────────────────────────────────────────
  if (digitalRead(BOOT_BTN_PIN) == LOW) {
    unsigned long pressStart = millis();
    while (digitalRead(BOOT_BTN_PIN) == LOW) {
      delay(10); esp_task_wdt_reset();
      if (!usbModeActive && appMode == MODE_VIEWFINDER) {
        unsigned long held = millis() - pressStart;
        if (held > 1000) {
          int barW = constrain(
            (int)map((long)held, 1000L, (long)LONG_PRESS_MS, 0L, (long)DISP_W), 0, DISP_W);
          lcd.fillRect(0, DISP_H - 3, barW, 3, COL_GRAY_7);
          lcd.fillRect(barW, DISP_H - 3, DISP_W - barW, 3, COL_BLACK);
        }
      }
    }
    unsigned long duration = millis() - pressStart;
    if (duration < 50) return;
    unsigned long now = millis();

    if (usbModeActive) {
      exitUSBMode();
    } else if (appMode == MODE_VIEWFINDER && duration >= LONG_PRESS_MS) {
      lcd.fillRect(0, DISP_H - 3, DISP_W, 3, COL_BLACK);
      if (now - lastBtnPress > DEBOUNCE_MS) {
        lastBtnPress = now;
        if (sdReady) enterUSBMode();
      }
    } else if (appMode == MODE_VIEWFINDER) {
      if (now - lastBtnPress > DEBOUNCE_MS) {
        lastBtnPress = now;
        captureAndPreview();
      }
    } else if (appMode == MODE_GALLERY) {
      appMode = MODE_VIEWFINDER;
      lcd.fillScreen(COL_BLACK);
      fpsLastTime = millis(); fpsFrameCount = 0;
    } else if (appMode == MODE_PHOTO_VIEW) {
      appMode = MODE_GALLERY;
      drawGallery();
    }
    return;
  }

  // ── Touch handling ────────────────────────────────────────────────────────
  int32_t tx, ty;
  if (lcd.getTouch(&tx, &ty)) {
    unsigned long touchStart = millis();

    // Tunggu lepas, hitung durasi & deteksi swipe
    int32_t tx2 = tx, ty2 = ty;
    while (lcd.getTouch(&tx2, &ty2)) {
      delay(10); esp_task_wdt_reset();
    }
    unsigned long touchDur = millis() - touchStart;
    int32_t swipeX = tx2 - tx;
    int32_t swipeY = ty2 - ty;

    // ── MODE VIEWFINDER ───────────────────────────────────────────────────
    if (appMode == MODE_VIEWFINDER && !usbModeActive) {

      // Long tap → USB mode
      if (touchDur >= LONG_TAP_MS) {
        if (sdReady) {
          lcd.setFont(&fonts::Font0);
          int tw = lcd.textWidth("masuk USB mode...");
          int pw = tw + 14;
          int px = (DISP_W - pw) / 2;
          lcd.fillRoundRect(px, DISP_H / 2 - 8, pw, 15, 7, COL_GRAY_3);
          lcd.setTextColor(COL_GRAY_5);
          lcd.drawString("masuk USB mode...", px + 7, DISP_H / 2 - 5);
          delay(500);
          enterUSBMode();
        }
        return;
      }

      unsigned long now = millis();

      // [FIXED] Cek pojok kiri atas DULU sebelum hal lain → Gallery
      if (inZoneTopLeft(tx, ty)) {
        if (sdReady) {
          scanGalleryFiles();
          appMode = MODE_GALLERY;
          drawGallery();
        } else {
          lcd.setFont(&fonts::Font0);
          int tw = lcd.textWidth("SD tidak siap");
          lcd.fillRoundRect((DISP_W - tw - 14) / 2, DISP_H/2 - 8, tw + 14, 15, 7, COL_GRAY_3);
          lcd.setTextColor(COL_GRAY_5);
          lcd.drawString("SD tidak siap", (DISP_W - tw) / 2, DISP_H/2 - 5);
          delay(1000);
        }
        return;
      }

      // [FIXED] Cek pojok kanan atas DULU → toggle face detect
      if (inZoneTopRight(tx, ty)) {
        toggleFaceDetect();
        return;
      }

      // [FIXED] Capture HANYA jika tap di 1/4 kanan layar (x > 240)
      // Area kiri & tengah layar tidak trigger capture sama sekali
      if (inZoneCapture(tx, ty)) {
        if (now - lastBtnPress > DEBOUNCE_MS) {
          lastBtnPress = now;
          captureAndPreview();
        }
      }
      // Tap di area lain (tengah/kiri, bukan pojok) → tidak melakukan apa-apa
    }

    // ── MODE GALLERY ──────────────────────────────────────────────────────
    else if (appMode == MODE_GALLERY) {

      // Pojok kanan (back)
      if (inZoneTopRight(tx, ty)) {
        appMode = MODE_VIEWFINDER;
        lcd.fillScreen(COL_BLACK);
        fpsLastTime = millis(); fpsFrameCount = 0;
        return;
      }

      // Swipe vertikal → scroll
      if (abs(swipeY) > 20 && abs(swipeY) > abs(swipeX)) {
        if (swipeY < 0) {
          galleryScroll = min(galleryScroll + 1, max(0, galleryCount - GALLERY_ITEMS_PAGE));
        } else {
          galleryScroll = max(galleryScroll - 1, 0);
        }
        drawGallery();
        return;
      }

      // Tap item → buka foto
      if (ty > 20 && galleryCount > 0) {
        int idx = galleryScroll + (ty - 24) / GALLERY_ITEM_H;
        if (idx >= 0 && idx < galleryCount) {
          appMode = MODE_PHOTO_VIEW;
          showPhotoView(galleryFiles[idx]);
        }
      }
    }

    // ── MODE PHOTO VIEW ───────────────────────────────────────────────────
    else if (appMode == MODE_PHOTO_VIEW) {
      if (inZoneTopRight(tx, ty)) {
        appMode = MODE_GALLERY;
        drawGallery();
      }
    }
  }

  // ── Render mode aktif ─────────────────────────────────────────────────────
  if (appMode == MODE_VIEWFINDER && !usbModeActive) {
    renderViewfinder();
  } else if (usbModeActive) {
    delay(50); esp_task_wdt_reset();
  }
}
