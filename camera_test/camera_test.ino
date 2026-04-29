/*
 * ESP32-S3-CAM (Freenove ESP32-S3-WROOM) - VIEWFINDER + SD CARD CAPTURE
 * Version: v4.9b
 *
 * PATCH v4.9b (dari v4.9-patched):
 *  - Photo View: zona touch dipisah dengan jelas
 *      Pojok kiri atas  (x<70, y<70)  → ZOOM tap biasa (cycle 1×→2×→4×→1×)
 *      Pojok kiri bawah (x<70, y>170) → COMPARE (pilih foto ini sebagai slot A, lalu ke gallery)
 *      Pojok kanan atas                → back ke gallery (sama seperti sebelumnya)
 *      Long press 1.5s (di mana saja) → delete dialog
 *      Tap kiri  (x<160, di luar zona pojok) → PREV foto (langsung, tanpa delay)
 *      Tap kanan (x>160, di luar zona pojok) → NEXT foto (langsung, tanpa delay)
 *  - double tap & konfirmasi DIHAPUS — tidak diperlukan lagi karena zona terpisah
 *  - PAN_DEADZONE tetap 15px
 *  - LONG_TAP_PHOTO_MS = 1500ms (sesuai permintaan)
 *
 * CHANGELOG v4.9:
 *  - Photo View: Zoom in/out
 *  - Photo View: Pan (drag) saat zoom > 1×
 *  - Photo View: Compare mode — dua foto side by side
 *  - MJPEG Player: Loop mode toggle
 *  - MJPEG Player: Speed selector 0.5× / 1× / 2×
 *
 * UI THEME: Monochrome — full black/gray/white, terminal aesthetic
 * DISPLAY: ILI9341 2.4" 320x240 landscape + XPT2046 resistive touch
 *
 * Touch controls (photo view):
 *  Pojok kiri atas  (x<70, y<70)   → zoom cycle 1×→2×→4×→1×
 *  Pojok kiri bawah (x<70, y>170)  → compare (foto ini jadi slot A, pilih B dari gallery)
 *  Pojok kanan atas                 → back ke gallery
 *  Long press >1.5s (mana saja)    → delete dialog
 *  Drag saat zoom>1×               → pan
 *  Tap kiri  (x<160, bukan pojok)  → foto sebelumnya (langsung)
 *  Tap kanan (x>160, bukan pojok)  → foto berikutnya (langsung)
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

#include <JPEGDEC.h>
#include "MjpegClass.h"

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
      cfg.pin_int = 41; cfg.bus_shared = true; cfg.offset_rotation = 0;
      cfg.spi_host = SPI2_HOST; cfg.freq = 2500000;
      cfg.pin_sclk = 47; cfg.pin_mosi = 45;
      cfg.pin_miso = 42; cfg.pin_cs = 2;
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

#define LED_PIN        48
#define BOOT_BTN_PIN    0
#define REC_BTN_PIN    -1

#define DISP_W        320
#define DISP_H        240

// Monochrome palette
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

#define PID_GC2145 0x2145
#define PID_OV3660 0x3660

#define DEBOUNCE_MS          400
#define LONG_PRESS_MS       2000
#define REC_LONG_PRESS_MS   2000
#define LONG_TAP_GALLERY_MS 1500   // compare select di gallery
#define LONG_TAP_PHOTO_MS   1500   // delete di photo view
#define PAN_DEADZONE          15   // pixel min untuk dianggap drag

// ── Zona pojok di photo view ──────────────────────────────────────────────
#define PV_ZONE_SZ     70          // ukuran kotak pojok (px)
#define PV_ZONE_TOP    70          // batas bawah zona atas
#define PV_ZONE_BOT   170          // batas atas zona bawah

// Inline helpers zona photo view
#define pvZoneTopLeft(x,y)   ((x) < PV_ZONE_SZ && (y) < PV_ZONE_TOP)
#define pvZoneTopRight(x,y)  ((x) > DISP_W - PV_ZONE_SZ && (y) < PV_ZONE_TOP)
#define pvZoneBotLeft(x,y)   ((x) < PV_ZONE_SZ && (y) > PV_ZONE_BOT)
#define pvZoneAny(x,y)       (pvZoneTopLeft(x,y)||pvZoneTopRight(x,y)||pvZoneBotLeft(x,y))

#define HMIRROR_GC2145 1
#define VFLIP_GC2145   0
#define HMIRROR_OV3660 0
#define VFLIP_OV3660   1

// ─────────────────────────────────────────────────────────────────────────────
//  App mode
// ─────────────────────────────────────────────────────────────────────────────
enum AppMode {
  MODE_VIEWFINDER,
  MODE_GALLERY,
  MODE_PHOTO_VIEW,
  MODE_COMPARE,
  MODE_MJPEG_PLAYER
};
AppMode appMode = MODE_VIEWFINDER;

// ─────────────────────────────────────────────────────────────────────────────
//  Gallery state
// ─────────────────────────────────────────────────────────────────────────────
#define GALLERY_MAX_FILES  200
#define GALLERY_ITEMS_PAGE   8
#define GALLERY_ITEM_H      26

char galleryFiles[GALLERY_MAX_FILES][32];
bool galleryIsVideo[GALLERY_MAX_FILES];
int  galleryCount  = 0;
int  galleryScroll = 0;
char photoViewPath[48];
int  photoViewIndex = 0;

bool galleryCompareSelect = false;
int  galleryCompareIdxA   = -1;

// ─────────────────────────────────────────────────────────────────────────────
//  Photo View state
// ─────────────────────────────────────────────────────────────────────────────
unsigned long photoViewCaptionUntilMs = 0;
bool          photoViewCaptionVisible = false;

#define ZOOM_LEVELS 3
float    photoZoomFactors[ZOOM_LEVELS] = { 1.0f, 2.0f, 4.0f };
uint8_t  photoZoomLevel  = 0;
int      photoZoomOffX   = 0;
int      photoZoomOffY   = 0;
uint16_t* photoPixelBuf  = nullptr;
uint16_t  photoBufW      = 0;
uint16_t  photoBufH      = 0;

int  compareIdxA = -1;
int  compareIdxB = -1;

// ─────────────────────────────────────────────────────────────────────────────
//  Touch zones (viewfinder / gallery)
// ─────────────────────────────────────────────────────────────────────────────
#define TOUCH_ZONE_SZ 70

bool inZoneTopLeft(int32_t x, int32_t y)    { return x < TOUCH_ZONE_SZ && y < TOUCH_ZONE_SZ; }
bool inZoneTopRight(int32_t x, int32_t y)   { return x > DISP_W - TOUCH_ZONE_SZ && y < TOUCH_ZONE_SZ; }
bool inZoneBottomLeft(int32_t x, int32_t y) { return x < TOUCH_ZONE_SZ && y > DISP_H - TOUCH_ZONE_SZ; }
bool inZoneCapture(int32_t x, int32_t y)    {
  return (x > DISP_W / 2) && !(x > DISP_W - TOUCH_ZONE_SZ && y < TOUCH_ZONE_SZ);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Recording state
// ─────────────────────────────────────────────────────────────────────────────
bool          recActive     = false;
FILE*         recFile       = nullptr;
int           recFrameCount = 0;
int           recVideoCount = 0;
unsigned long recStartMs    = 0;

bool faceDetectMode  = false;
int  faceDetectCount = 0;

USBMSC msc;
bool   usbModeActive = false;

sdmmc_card_t* sdCard         = nullptr;
bool          sdReady         = false;
uint32_t      sdTotalSectors  = 0;
bool          sdmmcDriverInit = false;
uint64_t      sdSizeMB        = 0;

int           photoCount   = 0;
unsigned long lastBtnPress = 0;
unsigned long lastTapMs    = 0;
uint16_t      detectedSensor = 0;
char          sensorName[16] = "UNKNOWN";

unsigned long fpsLastTime   = 0;
int           fpsFrameCount = 0;
float         fpsValue      = 0.0f;

#define BRACKET_LEN 10
#define PILL_H      13
#define PILL_R       6
#define PILL_PAD_X   5

// ─────────────────────────────────────────────────────────────────────────────
//  MJPEG Player state
// ─────────────────────────────────────────────────────────────────────────────
#define MJPEG_BUF_SIZE   (150 * 1024)
#define MJPEG_FRAME_RATE  15

class FileStream : public Stream {
public:
  FILE* f = nullptr;
  size_t write(uint8_t) override { return 0; }
  int available() override {
    if (!f) return 0;
    long cur = ftell(f); fseek(f, 0, SEEK_END);
    long end = ftell(f); fseek(f, cur, SEEK_SET);
    return (int)(end - cur);
  }
  int read() override {
    if (!f) return -1;
    uint8_t b;
    return (fread(&b, 1, 1, f) == 1) ? b : -1;
  }
  int peek() override {
    if (!f) return -1;
    int c = fgetc(f);
    if (c != EOF) ungetc(c, f);
    return c;
  }
  size_t readBytes(uint8_t* buf, size_t len) override {
    if (!f) return 0;
    return fread(buf, 1, len, f);
  }
} fileStream;

FILE*      mjpegFile = nullptr;
MjpegClass mjpeg;
uint8_t*   mjpegBuf  = nullptr;

char  mjpegPath[48];
char  mjpegPathSaved[48];
bool  mjpegPlaying       = false;
bool  mjpegPaused        = false;
int   mjpegFrame         = 0;
unsigned long mjpegNotifUntilMs = 0;

bool    mjpegLoop      = false;
uint8_t mjpegSpeedIdx  = 1;
const float mjpegSpeeds[3] = { 0.5f, 1.0f, 2.0f };

int jpegDrawCallback(JPEGDRAW *pDraw) {
  lcd.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
  return 1;
}

// ─────────────────────────────────────────────────────────────────────────────
//  TJpgDec
// ─────────────────────────────────────────────────────────────────────────────
#include <TJpg_Decoder.h>

static uint16_t* _decodeTargetBuf = nullptr;
static uint16_t  _decodeTargetW   = 0;

bool tjpgdecOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (_decodeTargetBuf) {
    for (int row = 0; row < h; row++) {
      int destY = y + row;
      if (destY < 0 || destY >= DISP_H) continue;
      memcpy(_decodeTargetBuf + destY * _decodeTargetW + x,
             bitmap + row * w,
             w * sizeof(uint16_t));
    }
  } else {
    lcd.pushImage(x, y, w, h, bitmap);
  }
  return 1;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helper: corner brackets
// ─────────────────────────────────────────────────────────────────────────────
void drawCornerBrackets(uint16_t col = COL_WHITE) {
  int x0=3,y0=3,x1=DISP_W-4,y1=DISP_H-4,L=BRACKET_LEN;
  lcd.drawFastHLine(x0,y0,L,col); lcd.drawFastVLine(x0,y0,L,col);
  lcd.drawFastHLine(x1-L+1,y0,L,col); lcd.drawFastVLine(x1,y0,L,col);
  lcd.drawFastHLine(x0,y1,L,col); lcd.drawFastVLine(x0,y1-L+1,L,col);
  lcd.drawFastHLine(x1-L+1,y1,L,col); lcd.drawFastVLine(x1,y1-L+1,L,col);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helper: pill label
// ─────────────────────────────────────────────────────────────────────────────
void drawPill(int cx, int cy, const char* text, uint16_t bgCol, uint16_t fgCol) {
  lcd.setFont(&fonts::Font0); lcd.setTextSize(1);
  int tw = lcd.textWidth(text);
  int pw = tw + PILL_PAD_X * 2;
  int px = cx - pw/2, py = cy - PILL_H/2;
  lcd.fillRoundRect(px, py, pw, PILL_H, PILL_R, bgCol);
  lcd.drawRoundRect(px, py, pw, PILL_H, PILL_R, COL_GRAY_3);
  lcd.setTextColor(fgCol);
  lcd.drawString(text, px+PILL_PAD_X, py+2);
}

void blinkLED(int n, int on_ms=100, int off_ms=100) {
  for (int i=0;i<n;i++) {
    digitalWrite(LED_PIN,HIGH); delay(on_ms);
    digitalWrite(LED_PIN,LOW); if(i<n-1) delay(off_ms);
    esp_task_wdt_reset();
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Photo View — ikon zona di overlay
//  Tampilkan hint kecil di pojok supaya user tahu zona aktif
// ─────────────────────────────────────────────────────────────────────────────
void photoViewDrawZoneHints() {
  // pojok kiri atas: ikon zoom (kaca pembesar minimalis)
  lcd.drawCircle(14, 14, 7, COL_GRAY_5);
  lcd.drawLine(19, 19, 24, 24, COL_GRAY_5);

  // pojok kiri bawah: ikon compare (dua kotak kecil)
  lcd.drawRect(4,  DISP_H-22, 10, 10, COL_GRAY_5);
  lcd.drawRect(17, DISP_H-22, 10, 10, COL_GRAY_5);

  // pojok kanan atas: ikon back (panah kiri)
  lcd.drawLine(DISP_W-24, 14, DISP_W-14, 14, COL_GRAY_5);
  lcd.drawLine(DISP_W-24, 14, DISP_W-20, 10, COL_GRAY_5);
  lcd.drawLine(DISP_W-24, 14, DISP_W-20, 18, COL_GRAY_5);
}

// ─────────────────────────────────────────────────────────────────────────────
//  SD Mount / Unmount / Remount
// ─────────────────────────────────────────────────────────────────────────────
bool mountSDFull() {
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.max_freq_khz = SDMMC_FREQ_DEFAULT;
  sdmmc_slot_config_t slot = SDMMC_SLOT_CONFIG_DEFAULT();
  slot.clk=(gpio_num_t)SD_MMC_CLK_PIN; slot.cmd=(gpio_num_t)SD_MMC_CMD_PIN;
  slot.d0=(gpio_num_t)SD_MMC_D0_PIN; slot.width=1;
  slot.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
  esp_vfs_fat_sdmmc_mount_config_t mountCfg = {
    .format_if_mount_failed=false,.max_files=5,.allocation_unit_size=16*1024
  };
  esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard",&host,&slot,&mountCfg,&sdCard);
  if (ret != ESP_OK) {
    sdCard=nullptr; sdTotalSectors=0; sdmmcDriverInit=false; sdSizeMB=0;
    return false;
  }
  sdTotalSectors=sdCard->csd.capacity; sdmmcDriverInit=true;
  sdSizeMB=(uint64_t)sdTotalSectors*sdCard->csd.sector_size/(1024*1024);
  return true;
}

bool remountVFSOnly() {
  if (!sdCard||!sdmmcDriverInit) return mountSDFull();
  ff_diskio_register_sdmmc(0,sdCard);
  static FATFS fatfs;
  char drv[3]={'0',':',0};
  FRESULT fr=f_mount(&fatfs,drv,1);
  if (fr!=FR_OK) return false;
  FATFS* fatfsPtr=&fatfs;
  esp_err_t err=esp_vfs_fat_register("/sdcard","",5,&fatfsPtr);
  if (err!=ESP_OK && err!=ESP_ERR_INVALID_STATE) return false;
  return true;
}

void unmountVFSOnly() {
  esp_vfs_fat_unregister_path("/sdcard");
  ff_diskio_register_sdmmc(0,NULL);
}

void scanPhotoCount() {
  photoCount=0;
  DIR* dir=opendir("/sdcard"); if(!dir) return;
  struct dirent* entry;
  while((entry=readdir(dir))!=nullptr) {
    String name=entry->d_name;
    if(name.startsWith("photo_")&&name.endsWith(".jpg")) {
      int num=name.substring(6,name.length()-4).toInt();
      if(num>photoCount) photoCount=num;
    }
  }
  closedir(dir);
}

void scanVideoCount() {
  recVideoCount=0;
  DIR* dir=opendir("/sdcard"); if(!dir) return;
  struct dirent* entry;
  while((entry=readdir(dir))!=nullptr) {
    String name=entry->d_name;
    if(name.startsWith("video_")&&name.endsWith(".mjpeg")) {
      int num=name.substring(6,name.length()-6).toInt();
      if(num>recVideoCount) recVideoCount=num;
    }
  }
  closedir(dir);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Gallery scan + render
// ─────────────────────────────────────────────────────────────────────────────
void scanGalleryFiles() {
  galleryCount=0; galleryScroll=0; galleryCompareSelect=false; galleryCompareIdxA=-1;
  DIR* dir=opendir("/sdcard"); if(!dir) return;
  struct dirent* entry;
  while((entry=readdir(dir))!=nullptr&&galleryCount<GALLERY_MAX_FILES) {
    String name=entry->d_name;
    bool isJpg  =name.endsWith(".jpg")||name.endsWith(".JPG");
    bool isMjpeg=name.endsWith(".mjpeg")||name.endsWith(".MJPEG");
    if(isJpg||isMjpeg) {
      strncpy(galleryFiles[galleryCount],entry->d_name,31);
      galleryFiles[galleryCount][31]='\0';
      galleryIsVideo[galleryCount]=isMjpeg;
      galleryCount++;
    }
  }
  closedir(dir);
  for(int i=0;i<galleryCount-1;i++) {
    for(int j=0;j<galleryCount-i-1;j++) {
      if(strcmp(galleryFiles[j],galleryFiles[j+1])>0) {
        char tmp[32];
        strncpy(tmp,galleryFiles[j],31);
        strncpy(galleryFiles[j],galleryFiles[j+1],31);
        strncpy(galleryFiles[j+1],tmp,31);
        bool bTmp=galleryIsVideo[j];
        galleryIsVideo[j]=galleryIsVideo[j+1];
        galleryIsVideo[j+1]=bTmp;
      }
    }
    esp_task_wdt_reset();
  }
}

void drawGallery() {
  lcd.fillScreen(COL_BLACK);
  lcd.fillRect(0,0,DISP_W,20,COL_GRAY_D);
  lcd.drawFastHLine(0,20,DISP_W,COL_GRAY_3);
  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_E);

  if (galleryCompareSelect) {
    char hdr[40]; snprintf(hdr,sizeof(hdr),"COMPARE — pilih foto ke-2");
    int hw=lcd.textWidth(hdr);
    lcd.drawString(hdr,(DISP_W-hw)/2,6);
    lcd.setTextColor(COL_GRAY_5); lcd.drawString("BATAL >",DISP_W-52,6);
  } else {
    char hdr[32]; snprintf(hdr,sizeof(hdr),"GALLERY  %d item",galleryCount);
    int hw=lcd.textWidth(hdr);
    lcd.drawString(hdr,(DISP_W-hw)/2,6);
    lcd.setTextColor(COL_GRAY_5); lcd.drawString("BACK >",DISP_W-44,6);
  }

  if(galleryCount==0) {
    lcd.setTextColor(COL_GRAY_5);
    int ew=lcd.textWidth("SD kosong / tidak ada file");
    lcd.drawString("SD kosong / tidak ada file",(DISP_W-ew)/2,DISP_H/2);
    return;
  }

  int visibleEnd=min(galleryScroll+GALLERY_ITEMS_PAGE,galleryCount);
  for(int i=galleryScroll;i<visibleEnd;i++) {
    int rowY=24+(i-galleryScroll)*GALLERY_ITEM_H;
    bool isSelected=(galleryCompareSelect && i==galleryCompareIdxA);
    uint16_t rowBg = isSelected ? COL_GRAY_3 : ((i%2==0)?COL_GRAY_D:COL_BLACK);
    lcd.fillRect(0,rowY,DISP_W,GALLERY_ITEM_H-1,rowBg);
    lcd.drawFastHLine(0,rowY+GALLERY_ITEM_H-1,DISP_W,COL_GRAY_2);

    lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
    char num[6]; snprintf(num,sizeof(num),"%3d",i+1);
    lcd.drawString(num,6,rowY+8);

    if(galleryIsVideo[i]) { lcd.setTextColor(COL_GRAY_7); lcd.drawString("\x10",26,rowY+8); }
    else { lcd.drawRect(26,rowY+6,7,7,COL_GRAY_5); }

    if(galleryCompareSelect && galleryIsVideo[i]) {
      lcd.setTextColor(COL_GRAY_3);
    } else {
      lcd.setTextColor(galleryIsVideo[i]?COL_GRAY_A:COL_GRAY_C);
    }
    lcd.drawString(galleryFiles[i],36,rowY+8);
    lcd.setTextColor(COL_GRAY_3); lcd.drawString(">",DISP_W-12,rowY+8);
  }

  if(galleryCount>GALLERY_ITEMS_PAGE) {
    int barH=DISP_H-22;
    int indH=max(10,barH*GALLERY_ITEMS_PAGE/galleryCount);
    int indY=22+(barH-indH)*galleryScroll/max(1,galleryCount-GALLERY_ITEMS_PAGE);
    lcd.fillRect(DISP_W-4,22,4,barH,COL_GRAY_2);
    lcd.fillRect(DISP_W-4,indY,4,indH,COL_GRAY_7);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Photo pixel buffer
// ─────────────────────────────────────────────────────────────────────────────
bool photoLoadPixelBuf(int idx) {
  if(idx<0||idx>=galleryCount||galleryIsVideo[idx]) return false;

  if(photoPixelBuf) { free(photoPixelBuf); photoPixelBuf=nullptr; }
  photoBufW=0; photoBufH=0;

  char path[56];
  snprintf(path,sizeof(path),"/sdcard/%s",galleryFiles[idx]);
  FILE* f=fopen(path,"rb"); if(!f) return false;

  fseek(f,0,SEEK_END); size_t fsize=ftell(f); fseek(f,0,SEEK_SET);
  uint8_t* jpgBuf=(uint8_t*)ps_malloc(fsize);
  if(!jpgBuf) { fclose(f); return false; }
  fread(jpgBuf,1,fsize,f); fclose(f);

  TJpgDec.setJpgScale(1); TJpgDec.setSwapBytes(true); TJpgDec.setCallback(tjpgdecOutput);
  uint16_t iw=0,ih=0;
  TJpgDec.getJpgSize(&iw,&ih,jpgBuf,fsize);
  if(iw==0||ih==0) { free(jpgBuf); return false; }

  uint16_t* buf=(uint16_t*)ps_malloc((size_t)iw*ih*sizeof(uint16_t));
  if(!buf) { free(jpgBuf); return false; }
  memset(buf,0,(size_t)iw*ih*sizeof(uint16_t));

  _decodeTargetBuf=buf; _decodeTargetW=iw;
  TJpgDec.drawJpg(0,0,jpgBuf,fsize);
  _decodeTargetBuf=nullptr;

  free(jpgBuf);
  photoPixelBuf=buf; photoBufW=iw; photoBufH=ih;
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Photo View render dengan zoom & pan
// ─────────────────────────────────────────────────────────────────────────────
void photoViewRender() {
  if(!photoPixelBuf) return;

  float zf = photoZoomFactors[photoZoomLevel];
  lcd.fillScreen(COL_BLACK);

  if(zf <= 1.0f) {
    int ox=(DISP_W-min((int)photoBufW,DISP_W))/2;
    int oy=(DISP_H-min((int)photoBufH,DISP_H))/2;
    int renderW=min((int)photoBufW,DISP_W);
    int renderH=min((int)photoBufH,DISP_H);
    for(int row=0;row<renderH;row++) {
      lcd.pushImage(ox,oy+row,renderW,1,photoPixelBuf+row*photoBufW);
      if(row%20==0) esp_task_wdt_reset();
    }
  } else {
    int vpW=(int)(DISP_W/zf);
    int vpH=(int)(DISP_H/zf);
    int maxOffX=max(0,(int)photoBufW-vpW);
    int maxOffY=max(0,(int)photoBufH-vpH);
    photoZoomOffX=constrain(photoZoomOffX,0,maxOffX);
    photoZoomOffY=constrain(photoZoomOffY,0,maxOffY);

    static uint16_t lineBuf[DISP_W];
    for(int sy=0;sy<DISP_H;sy++) {
      int srcY=photoZoomOffY+(int)(sy/zf);
      srcY=constrain(srcY,0,(int)photoBufH-1);
      for(int sx=0;sx<DISP_W;sx++) {
        int srcX=photoZoomOffX+(int)(sx/zf);
        srcX=constrain(srcX,0,(int)photoBufW-1);
        lineBuf[sx]=photoPixelBuf[srcY*photoBufW+srcX];
      }
      lcd.pushImage(0,sy,DISP_W,1,lineBuf);
      if(sy%20==0) esp_task_wdt_reset();
    }
  }

  // zoom indicator
  if(photoZoomLevel>0) {
    char zBuf[8]; snprintf(zBuf,sizeof(zBuf),"%.0f×",(double)zf);
    drawPill(DISP_W/2,DISP_H-10,zBuf,COL_PILL_BG,COL_GRAY_A);
  }

  // gambar ikon zona pojok setelah render foto
  photoViewDrawZoneHints();
}

void photoViewDrawCaption(int idx) {
  int photoSeq=0,photoTotal=0;
  for(int i=0;i<galleryCount;i++) {
    if(!galleryIsVideo[i]) {
      photoTotal++;
      if(i<=idx) photoSeq=photoTotal;
    }
  }
  lcd.fillRect(0,DISP_H-16,DISP_W,16,COL_GRAY_D);
  lcd.setFont(&fonts::Font0);
  char bar[40],barFull[64];
  snprintf(bar,sizeof(bar),"< %d / %d >",photoSeq,photoTotal);
  snprintf(barFull,sizeof(barFull),"< %d / %d >  %s",photoSeq,photoTotal,galleryFiles[idx]);
  const char* barStr=(lcd.textWidth(barFull)<DISP_W-4)?barFull:bar;
  lcd.setTextColor(COL_GRAY_A);
  int nw=lcd.textWidth(barStr);
  lcd.drawString(barStr,(DISP_W-nw)/2,DISP_H-13);
}

void photoViewClearCaption() {
  lcd.fillRect(0,DISP_H-16,DISP_W,16,COL_BLACK);
  if(photoZoomLevel>0) {
    float zf=photoZoomFactors[photoZoomLevel];
    char zBuf[8]; snprintf(zBuf,sizeof(zBuf),"%.0f×",(double)zf);
    drawPill(DISP_W/2,DISP_H-10,zBuf,COL_PILL_BG,COL_GRAY_A);
  }
  photoViewCaptionVisible=false;
  photoViewCaptionUntilMs=0;
}

bool photoViewDeleteDialog(const char* filename) {
  int dw=200,dh=80;
  int dx=(DISP_W-dw)/2,dy=(DISP_H-dh)/2;
  lcd.fillRoundRect(dx,dy,dw,dh,10,COL_GRAY_D);
  lcd.drawRoundRect(dx,dy,dw,dh,10,COL_GRAY_5);
  lcd.setFont(&fonts::Font0); lcd.setTextSize(1);
  lcd.setTextColor(COL_GRAY_E);
  const char* title="HAPUS FILE?";
  int tw=lcd.textWidth(title);
  lcd.drawString(title,dx+(dw-tw)/2,dy+10);
  lcd.setTextColor(COL_GRAY_7);
  char truncName[28]; strncpy(truncName,filename,27); truncName[27]='\0';
  int fnw=lcd.textWidth(truncName);
  lcd.drawString(truncName,dx+(dw-fnw)/2,dy+24);
  lcd.drawFastHLine(dx+10,dy+38,dw-20,COL_GRAY_3);
  int btnW=70,btnH=22,btnY=dy+46;
  int btnNoX=dx+16,btnYesX=dx+dw-16-btnW;
  lcd.fillRoundRect(btnNoX,btnY,btnW,btnH,6,COL_GRAY_3);
  lcd.drawRoundRect(btnNoX,btnY,btnW,btnH,6,COL_GRAY_5);
  lcd.setTextColor(COL_GRAY_C);
  int noW=lcd.textWidth("NO");
  lcd.drawString("NO",btnNoX+(btnW-noW)/2,btnY+7);
  lcd.fillRoundRect(btnYesX,btnY,btnW,btnH,6,COL_GRAY_7);
  lcd.drawRoundRect(btnYesX,btnY,btnW,btnH,6,COL_GRAY_A);
  lcd.setTextColor(COL_BLACK);
  int yesW=lcd.textWidth("YES");
  lcd.drawString("YES",btnYesX+(btnW-yesW)/2,btnY+7);
  unsigned long waitStart=millis();
  while(millis()-waitStart<10000) {
    int32_t tx,ty;
    if(lcd.getTouch(&tx,&ty)) {
      while(lcd.getTouch(&tx,&ty)) { delay(10); esp_task_wdt_reset(); }
      if(tx>=btnYesX&&tx<=btnYesX+btnW&&ty>=btnY&&ty<=btnY+btnH) return true;
      return false;
    }
    if(digitalRead(BOOT_BTN_PIN)==LOW) {
      while(digitalRead(BOOT_BTN_PIN)==LOW) { delay(10); esp_task_wdt_reset(); }
      return false;
    }
    delay(10); esp_task_wdt_reset();
  }
  return false;
}

void showPhotoView(int idx) {
  if(idx<0||idx>=galleryCount||galleryIsVideo[idx]) return;
  photoViewIndex=idx;
  strncpy(photoViewPath,galleryFiles[idx],sizeof(photoViewPath)-1);

  photoZoomLevel=0; photoZoomOffX=0; photoZoomOffY=0;

  lcd.fillScreen(COL_BLACK);
  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("loading...",DISP_W/2-24,DISP_H/2-4);

  if(!photoLoadPixelBuf(idx)) {
    lcd.fillScreen(COL_BLACK);
    lcd.drawString("gagal load foto",20,DISP_H/2);
    delay(1500); return;
  }

  photoViewRender();
  photoViewDrawCaption(idx);
  photoViewCaptionUntilMs=millis()+2000;
  photoViewCaptionVisible=true;
}

void photoViewPrev() {
  int idx=photoViewIndex-1;
  if(idx<0) idx=galleryCount-1;
  for(int t=0;t<galleryCount;t++) {
    if(!galleryIsVideo[idx]) { showPhotoView(idx); return; }
    idx--; if(idx<0) idx=galleryCount-1;
  }
}

void photoViewNext() {
  int idx=photoViewIndex+1;
  if(idx>=galleryCount) idx=0;
  for(int t=0;t<galleryCount;t++) {
    if(!galleryIsVideo[idx]) { showPhotoView(idx); return; }
    idx++; if(idx>=galleryCount) idx=0;
  }
}

void photoViewDeleteCurrent() {
  char path[56];
  snprintf(path,sizeof(path),"/sdcard/%s",galleryFiles[photoViewIndex]);
  if(remove(path)==0) {
    lcd.fillRoundRect(80,DISP_H/2-10,160,20,6,COL_GRAY_3);
    lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_A);
    int tw=lcd.textWidth("FILE DIHAPUS");
    lcd.drawString("FILE DIHAPUS",(DISP_W-tw)/2,DISP_H/2-6);
    delay(800);
  } else {
    lcd.fillRoundRect(80,DISP_H/2-10,160,20,6,COL_GRAY_3);
    lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
    int tw=lcd.textWidth("GAGAL HAPUS");
    lcd.drawString("GAGAL HAPUS",(DISP_W-tw)/2,DISP_H/2-6);
    delay(800);
  }
  if(photoPixelBuf) { free(photoPixelBuf); photoPixelBuf=nullptr; }
  scanGalleryFiles(); scanPhotoCount();
  appMode=MODE_GALLERY; drawGallery();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Compare Mode
// ─────────────────────────────────────────────────────────────────────────────
#define COMPARE_W (DISP_W/2)

void compareRenderSlot(int idx, int slotX) {
  if(idx<0||idx>=galleryCount||galleryIsVideo[idx]) {
    lcd.fillRect(slotX,0,COMPARE_W,DISP_H,COL_GRAY_2);
    lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("---",slotX+COMPARE_W/2-10,DISP_H/2);
    return;
  }
  char path[56];
  snprintf(path,sizeof(path),"/sdcard/%s",galleryFiles[idx]);
  FILE* f=fopen(path,"rb"); if(!f) return;
  fseek(f,0,SEEK_END); size_t fsize=ftell(f); fseek(f,0,SEEK_SET);
  uint8_t* jpgBuf=(uint8_t*)ps_malloc(fsize);
  if(!jpgBuf) { fclose(f); return; }
  fread(jpgBuf,1,fsize,f); fclose(f);

  uint16_t* slotBuf=(uint16_t*)ps_malloc((size_t)COMPARE_W*DISP_H*sizeof(uint16_t));
  if(!slotBuf) { free(jpgBuf); return; }
  memset(slotBuf,0,(size_t)COMPARE_W*DISP_H*sizeof(uint16_t));

  TJpgDec.setJpgScale(2);
  TJpgDec.setSwapBytes(true); TJpgDec.setCallback(tjpgdecOutput);
  _decodeTargetBuf=slotBuf; _decodeTargetW=COMPARE_W;
  TJpgDec.drawJpg(0,0,jpgBuf,fsize);
  _decodeTargetBuf=nullptr; TJpgDec.setJpgScale(1);
  free(jpgBuf);

  for(int row=0;row<DISP_H;row++) {
    lcd.pushImage(slotX,row,COMPARE_W,1,slotBuf+row*COMPARE_W);
    if(row%20==0) esp_task_wdt_reset();
  }
  free(slotBuf);

  lcd.fillRect(slotX,DISP_H-14,COMPARE_W,14,COL_GRAY_D);
  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_A);
  char shortName[18]; strncpy(shortName,galleryFiles[idx],17); shortName[17]='\0';
  int tw=lcd.textWidth(shortName);
  lcd.drawString(shortName,slotX+(COMPARE_W-tw)/2,DISP_H-11);
}

void showComparePage() {
  lcd.fillScreen(COL_BLACK);
  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("loading...",DISP_W/2-24,DISP_H/2-4);
  compareRenderSlot(compareIdxA, 0);
  compareRenderSlot(compareIdxB, COMPARE_W);
  lcd.drawFastVLine(COMPARE_W,0,DISP_H,COL_GRAY_5);
  lcd.fillRect(0,0,DISP_W,12,COL_GRAY_D);
  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_E);
  int hw=lcd.textWidth("COMPARE");
  lcd.drawString("COMPARE",(DISP_W-hw)/2,2);
  lcd.setTextColor(COL_GRAY_5); lcd.drawString("X",DISP_W-10,2);
  appMode=MODE_COMPARE;
}

// ─────────────────────────────────────────────────────────────────────────────
//  MJPEG Player
// ─────────────────────────────────────────────────────────────────────────────
bool mjpegOpen(const char* filename) {
  char path[48]; snprintf(path,sizeof(path),"/sdcard/%s",filename);
  if(mjpegFile) { fclose(mjpegFile); mjpegFile=nullptr; }
  if(mjpegBuf)  { free(mjpegBuf);   mjpegBuf=nullptr; }
  mjpegBuf=(uint8_t*)malloc(MJPEG_BUF_SIZE);
  if(!mjpegBuf) return false;
  mjpegFile=fopen(path,"rb");
  if(!mjpegFile) { free(mjpegBuf); mjpegBuf=nullptr; return false; }
  fileStream.f=mjpegFile;
  mjpeg.setup(&fileStream,mjpegBuf,jpegDrawCallback,true,0,0,DISP_W,DISP_H);
  strncpy(mjpegPath,filename,sizeof(mjpegPath)-1);
  strncpy(mjpegPathSaved,filename,sizeof(mjpegPathSaved)-1);
  mjpegFrame=0; mjpegPlaying=true; mjpegPaused=false; mjpegNotifUntilMs=0;
  lcd.fillScreen(COL_BLACK);
  return true;
}

void mjpegClose() {
  if(mjpegFile) { fclose(mjpegFile); mjpegFile=nullptr; }
  if(mjpegBuf)  { free(mjpegBuf);   mjpegBuf=nullptr; }
  fileStream.f=nullptr;
  mjpegPlaying=false; mjpegPaused=false; mjpegFrame=0; mjpegNotifUntilMs=0;
}

void mjpegShowNotif(const char* text) {
  lcd.setFont(&fonts::Font0); lcd.setTextSize(1);
  int tw=lcd.textWidth(text);
  int pw=tw+20,ph=17;
  int px=(DISP_W-pw)/2,py=(DISP_H-ph)/2;
  lcd.fillRoundRect(px,py,pw,ph,6,COL_GRAY_D);
  lcd.drawRoundRect(px,py,pw,ph,6,COL_GRAY_5);
  lcd.setTextColor(COL_GRAY_E);
  lcd.drawString(text,px+10,py+4);
  mjpegNotifUntilMs=millis()+1200;
}

void mjpegClearNotif() {
  int pw=180,ph=21;
  int px=(DISP_W-pw)/2,py=(DISP_H-ph)/2;
  lcd.fillRect(px-4,py-4,pw+8,ph+8,COL_BLACK);
  mjpegNotifUntilMs=0;
}

void mjpegDrawHUD() {
  char buf[24];
  snprintf(buf,sizeof(buf),"%.1f×  %s",
           (double)mjpegSpeeds[mjpegSpeedIdx], mjpegLoop?"LOOP":"—");
  lcd.setFont(&fonts::Font0);
  int tw=lcd.textWidth(buf);
  int pw=tw+10,ph=13;
  lcd.fillRoundRect(4,4,pw,ph,4,COL_PILL_BG);
  lcd.setTextColor(COL_GRAY_A);
  lcd.drawString(buf,9,6);
}

void mjpegHandleTouch(int32_t tx, int32_t ty, int32_t tx2, int32_t ty2, unsigned long dur) {
  int swipeX=tx2-tx;
  if(inZoneTopRight(tx,ty)) {
    mjpegClose(); appMode=MODE_GALLERY; drawGallery(); return;
  }
  if(inZoneTopLeft(tx,ty)) {
    mjpegLoop=!mjpegLoop;
    mjpegShowNotif(mjpegLoop?"LOOP ON":"LOOP OFF"); return;
  }
  if(abs(swipeX)>40) {
    if(swipeX<0 && mjpegSpeedIdx<2) mjpegSpeedIdx++;
    else if(swipeX>0 && mjpegSpeedIdx>0) mjpegSpeedIdx--;
    char spBuf[12];
    snprintf(spBuf,sizeof(spBuf),"%.1f×",(double)mjpegSpeeds[mjpegSpeedIdx]);
    mjpegShowNotif(spBuf); return;
  }
  mjpegPaused=!mjpegPaused;
  mjpegShowNotif(mjpegPaused?"PAUSE":"PLAY");
}

void loopMjpegPlayer() {
  if(!mjpegPlaying) return;
  if(!mjpegPaused) {
    int64_t frameStart=esp_timer_get_time();
    bool ok=mjpeg.readMjpegBuf();
    if(!ok) {
      if(mjpegLoop) {
        mjpegClose();
        if(!mjpegOpen(mjpegPathSaved)) { appMode=MODE_GALLERY; drawGallery(); }
        return;
      }
      mjpegClose(); appMode=MODE_GALLERY; drawGallery(); return;
    }
    mjpeg.drawJpg(); mjpegFrame++; mjpegDrawHUD();
    if(mjpegNotifUntilMs>0&&millis()>mjpegNotifUntilMs) mjpegClearNotif();
    float speed=mjpegSpeeds[mjpegSpeedIdx];
    int64_t targetUs=(int64_t)(1000000.0f/(MJPEG_FRAME_RATE*speed));
    int64_t elapsed=esp_timer_get_time()-frameStart;
    int64_t remain=targetUs-elapsed;
    if(remain>1000) {
      int64_t waitEnd=esp_timer_get_time()+remain;
      while(esp_timer_get_time()<waitEnd) { delayMicroseconds(500); esp_task_wdt_reset(); }
    }
  } else {
    if(mjpegNotifUntilMs>0&&millis()>mjpegNotifUntilMs) mjpegClearNotif();
    delay(80);
  }
  esp_task_wdt_reset();
}

void openMjpegPlayer(const char* filename) {
  mjpegLoop=false; mjpegSpeedIdx=1;
  if(!mjpegOpen(filename)) {
    lcd.fillScreen(COL_BLACK);
    lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("gagal buka file",20,DISP_H/2);
    delay(1500); drawGallery(); appMode=MODE_GALLERY; return;
  }
  appMode=MODE_MJPEG_PLAYER;
}

// ─────────────────────────────────────────────────────────────────────────────
//  REC overlay
// ─────────────────────────────────────────────────────────────────────────────
void drawRecIndicator() {
  if(!recActive) return;
  unsigned long elapsed=(millis()-recStartMs)/1000;
  char timeBuf[10]; snprintf(timeBuf,sizeof(timeBuf),"%02lu:%02lu",elapsed/60,elapsed%60);
  bool blink=(millis()/500)%2;
  lcd.fillRect(4,4,76,14,COL_BLACK);
  lcd.fillCircle(10,11,4,blink?COL_WHITE:COL_GRAY_5);
  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_E);
  lcd.drawString(timeBuf,18,6);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Recording
// ─────────────────────────────────────────────────────────────────────────────
void startRecording() {
  if(!sdReady) return;
  if(faceDetectMode) { faceDetectMode=false; faceDetectCount=0; }
  recVideoCount++;
  char path[48]; snprintf(path,sizeof(path),"/sdcard/video_%04d.mjpeg",recVideoCount);
  recFile=fopen(path,"wb");
  if(!recFile) { recVideoCount--; return; }
  recFrameCount=0; recStartMs=millis(); recActive=true;
  lcd.setFont(&fonts::Font0);
  char buf[20]; snprintf(buf,sizeof(buf),"REC  #%04d",recVideoCount);
  int tw=lcd.textWidth(buf),pw=tw+14,px=(DISP_W-pw)/2;
  lcd.fillRoundRect(px,6,pw,15,7,COL_WHITE);
  lcd.setTextColor(COL_BLACK); lcd.drawString(buf,px+7,9);
  delay(600); lcd.fillScreen(COL_BLACK);
}

void stopRecording() {
  if(!recActive||!recFile) return;
  fclose(recFile); recFile=nullptr; recActive=false;
  unsigned long dur=(millis()-recStartMs)/1000;
  lcd.fillScreen(COL_BLACK); lcd.setFont(&fonts::Font0);
  char buf[28]; snprintf(buf,sizeof(buf),"SAVED  %df  %02lu:%02lu",recFrameCount,dur/60,dur%60);
  int tw=lcd.textWidth(buf),pw=tw+14,px=(DISP_W-pw)/2;
  lcd.fillRoundRect(px,6,pw,15,7,COL_WHITE);
  lcd.setTextColor(COL_BLACK); lcd.drawString(buf,px+7,9);
  blinkLED(3,100,80); delay(2000); lcd.fillScreen(COL_BLACK);
  fpsLastTime=millis(); fpsFrameCount=0;
}

void recordFrame() {
  if(!recActive||!recFile) return;
  camera_fb_t *fb=esp_camera_fb_get();
  if(!fb) { esp_task_wdt_reset(); return; }
  uint8_t *jpg_buf=nullptr; size_t jpg_len=0; bool ok=false;
  if(fb->format==PIXFORMAT_JPEG) { jpg_buf=fb->buf; jpg_len=fb->len; ok=true; }
  else { ok=frame2jpg(fb,70,&jpg_buf,&jpg_len); }
  if(ok&&jpg_buf) {
    fwrite(jpg_buf,1,jpg_len,recFile); recFrameCount++;
    if(fb->format!=PIXFORMAT_JPEG) free(jpg_buf);
  }
  if(recFrameCount%3==0&&fb->format==PIXFORMAT_RGB565&&fb->width==DISP_W) {
    lcd.pushImage(0,0,DISP_W,DISP_H,(uint16_t*)fb->buf); drawRecIndicator();
  }
  esp_camera_fb_return(fb); esp_task_wdt_reset();
}

// ─────────────────────────────────────────────────────────────────────────────
//  USB MSC
// ─────────────────────────────────────────────────────────────────────────────
static int32_t onRead(uint32_t lba,uint32_t offset,void* buffer,uint32_t bufsize) {
  (void)offset;
  if(!sdCard||!sdmmcDriverInit) return -1;
  return (sdmmc_read_sectors(sdCard,buffer,lba,bufsize/512)==ESP_OK)?(int32_t)bufsize:-1;
}
static int32_t onWrite(uint32_t lba,uint32_t offset,uint8_t* buffer,uint32_t bufsize) {
  (void)offset;
  if(!sdCard||!sdmmcDriverInit) return -1;
  return (sdmmc_write_sectors(sdCard,buffer,lba,bufsize/512)==ESP_OK)?(int32_t)bufsize:-1;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Boot screen
// ─────────────────────────────────────────────────────────────────────────────
void drawCameraIcon(int cx,int cy,uint16_t col) {
  lcd.drawRoundRect(cx-22,cy-14,44,30,4,col);
  lcd.drawCircle(cx,cy-1,10,col);
  lcd.drawCircle(cx,cy-1,5,COL_GRAY_5);
  lcd.drawRect(cx-16,cy-18,12,5,col);
  lcd.drawPixel(cx+4,cy-5,COL_GRAY_7);
}

void bootLogLine(int y,const char* label,const char* status,uint16_t statusCol=COL_GRAY_E) {
  lcd.setFont(&fonts::Font0); lcd.setTextSize(1);
  lcd.setTextColor(COL_GRAY_5); lcd.drawString(label,28,y);
  int lx=28+lcd.textWidth(label)+3;
  for(int dx=lx;dx<268;dx+=4) lcd.drawPixel(dx,y+5,COL_GRAY_3);
  lcd.setTextColor(statusCol); lcd.drawString(status,272,y);
}

void runBootSequence(bool sdOK,uint64_t sdMB,bool pidOK,uint16_t pid,bool xclkOK,uint32_t xclkHz) {
  lcd.fillScreen(COL_BLACK);
  lcd.drawFastHLine(0,0,DISP_W,COL_GRAY_3);
  lcd.drawFastHLine(0,DISP_H-1,DISP_W,COL_GRAY_3);
  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_3); lcd.drawString("ESP32-S3",6,4);
  drawCameraIcon(DISP_W/2,55,COL_GRAY_7);
  lcd.setFont(&fonts::FreeSansBold9pt7b); lcd.setTextColor(COL_GRAY_E);
  lcd.drawString("SANZXCAM",DISP_W/2-57,85);
  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("v4.9b  ILI9341 + XPT2046",DISP_W/2-72,103);
  lcd.drawFastHLine(20,116,DISP_W-40,COL_GRAY_2);
  esp_task_wdt_reset(); delay(200);

  char buf[32];
  snprintf(buf,sizeof(buf),sdOK?"OK  %lluMB":"NOT FOUND",sdMB);
  bootLogLine(122,"SD CARD",buf,sdOK?COL_GRAY_E:COL_GRAY_5); delay(120); esp_task_wdt_reset();
  bootLogLine(134,"CAM PROBE","20MHz",COL_GRAY_E); delay(120); esp_task_wdt_reset();
  if(pidOK) snprintf(buf,sizeof(buf),"0x%04X  %s",pid,sensorName);
  else snprintf(buf,sizeof(buf),"0x%04X  ???",pid);
  bootLogLine(146,"SENSOR PID",buf,pidOK?COL_GRAY_E:COL_GRAY_5); delay(120); esp_task_wdt_reset();
  snprintf(buf,sizeof(buf),"%uMHz  OK",xclkHz/1000000);
  bootLogLine(158,"XCLK",buf,xclkOK?COL_GRAY_E:COL_GRAY_5); delay(120); esp_task_wdt_reset();
  bootLogLine(170,"TOUCH","XPT2046  OK",COL_GRAY_E); delay(120); esp_task_wdt_reset();
  bootLogLine(182,"MJPEG PLAY","MjpegClass+JPEGDEC",COL_GRAY_E); delay(120); esp_task_wdt_reset();

  lcd.drawRect(28,196,DISP_W-56,4,COL_GRAY_3);
  int barW=DISP_W-60;
  for(int i=0;i<=barW;i+=4) {
    lcd.fillRect(30,197,i,2,COL_GRAY_7);
    if(i%16==0) esp_task_wdt_reset();
  }
  lcd.fillRect(30,197,barW,2,COL_GRAY_C);
  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_A);
  lcd.drawString("READY",DISP_W/2-15,206);
  delay(800); esp_task_wdt_reset();
}

// ─────────────────────────────────────────────────────────────────────────────
//  USB mode screen
// ─────────────────────────────────────────────────────────────────────────────
void drawUSBIcon(int cx,int cy,uint16_t col) {
  lcd.drawFastVLine(cx,cy-24,28,col);
  lcd.drawCircle(cx,cy-27,4,col);
  lcd.drawFastHLine(cx-18,cy-12,36,col);
  lcd.drawFastVLine(cx-18,cy-12,12,col);
  lcd.drawCircle(cx-18,cy+2,4,col);
  lcd.drawFastVLine(cx+18,cy-12,10,col);
  lcd.drawRect(cx+13,cy-2,10,7,col);
}

void drawUSBModeScreen() {
  lcd.fillScreen(COL_BLACK);
  lcd.fillRect(0,0,DISP_W,18,COL_GRAY_D);
  lcd.drawFastHLine(0,18,DISP_W,COL_GRAY_3);
  lcd.setFont(&fonts::Font0);
  lcd.setTextColor(COL_GRAY_3); lcd.drawString("ESP32-S3",6,4);
  lcd.setTextColor(COL_GRAY_5);
  int lw=lcd.textWidth("USB MASS STORAGE");
  lcd.drawString("USB MASS STORAGE",(DISP_W-lw)/2,4);
  lcd.setTextColor(COL_GRAY_3); lcd.drawString("v4.9b",DISP_W-28,4);
  drawUSBIcon(DISP_W/2,85,COL_GRAY_7);
  lcd.setFont(&fonts::FreeSansBold9pt7b); lcd.setTextColor(COL_GRAY_E);
  int mw=lcd.textWidth("SD CONNECTED");
  lcd.drawString("SD CONNECTED",(DISP_W-mw)/2,118);
  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
  int sw=lcd.textWidth("to host via USB");
  lcd.drawString("to host via USB",(DISP_W-sw)/2,136);
  lcd.drawFastHLine(40,150,DISP_W-80,COL_GRAY_2);

  auto infoRow=[&](int y,const char* key,const char* val) {
    lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5); lcd.drawString(key,44,y);
    lcd.setTextColor(COL_GRAY_C);
    lcd.drawString(val,DISP_W-44-lcd.textWidth(val),y);
  };
  char buf[32];
  infoRow(156,"SENSOR",sensorName);
  if(sdSizeMB<1024) snprintf(buf,sizeof(buf),"%lluMB  FAT32",sdSizeMB);
  else snprintf(buf,sizeof(buf),"%lluGB  FAT32",sdSizeMB/1024);
  infoRow(170,"STORAGE",buf);
  snprintf(buf,sizeof(buf),"%04d files",photoCount);
  infoRow(184,"PHOTOS",buf);

  lcd.fillRect(0,DISP_H-18,DISP_W,18,COL_GRAY_D);
  lcd.drawFastHLine(0,DISP_H-18,DISP_W,COL_GRAY_3);
  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
  int ew=lcd.textWidth("eject from host first");
  lcd.drawString("eject from host first",(DISP_W-ew)/2,DISP_H-15);
  lcd.drawRoundRect((DISP_W/2)-68,DISP_H-32,136,13,2,COL_GRAY_3);
  int hw=lcd.textWidth("[ BOOT ]  exit usb mode");
  lcd.drawString("[ BOOT ]  exit usb mode",(DISP_W-hw)/2,DISP_H-30);
}

void enterUSBMode() {
  if(!sdReady) return;
  if(photoPixelBuf) { free(photoPixelBuf); photoPixelBuf=nullptr; }
  unmountVFSOnly(); sdReady=false;
  esp_task_wdt_reset(); msc.mediaPresent(true); esp_task_wdt_reset();
  drawUSBModeScreen(); usbModeActive=true; blinkLED(3,200,100);
}

void exitUSBMode() {
  usbModeActive=false; msc.mediaPresent(false); esp_task_wdt_reset();
  lcd.fillScreen(COL_BLACK); lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
  int tw=lcd.textWidth("reconnecting sd...");
  lcd.drawString("reconnecting sd...",(DISP_W-tw)/2,DISP_H/2-6);
  sdReady=remountVFSOnly();
  if(sdReady) {
    scanPhotoCount();
    lcd.fillRect(0,DISP_H/2-10,DISP_W,20,COL_BLACK);
    char buf[32]; snprintf(buf,sizeof(buf),"sd ok  next #%04d",photoCount+1);
    lcd.setTextColor(COL_GRAY_C);
    lcd.drawString(buf,(DISP_W-lcd.textWidth(buf))/2,DISP_H/2-6);
  } else {
    lcd.fillRect(0,DISP_H/2-10,DISP_W,20,COL_BLACK);
    lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("sd mount failed",(DISP_W-lcd.textWidth("sd mount failed"))/2,DISP_H/2-6);
  }
  esp_task_wdt_reset(); blinkLED(2,150,100); delay(1000);
  lcd.fillScreen(COL_BLACK);
  appMode=MODE_VIEWFINDER;
  fpsLastTime=millis(); fpsFrameCount=0; fpsValue=0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
//  FPS + Face detect
// ─────────────────────────────────────────────────────────────────────────────
void updateFPS() {
  fpsFrameCount++;
  unsigned long elapsed=millis()-fpsLastTime;
  if(elapsed>=500) {
    fpsValue=fpsFrameCount*1000.0f/elapsed;
    fpsFrameCount=0; fpsLastTime=millis();
  }
}

void runFaceDetect(camera_fb_t *fb) {
  if(!fb||fb->format!=PIXFORMAT_RGB565) return;
  HumanFaceDetectMSR01 msr(0.1f,0.5f,10,0.2f);
  HumanFaceDetectMNP01 mnp(0.5f,0.3f,5);
  auto &candidates=msr.infer((uint16_t*)fb->buf,{(int)fb->height,(int)fb->width,3});
  auto &results=mnp.infer((uint16_t*)fb->buf,{(int)fb->height,(int)fb->width,3},candidates);
  faceDetectCount=results.size();
  for(auto &res:results) {
    int x1=constrain(res.box[0],0,DISP_W-1),y1=constrain(res.box[1],0,DISP_H-1);
    int x2=constrain(res.box[2],0,DISP_W-1),y2=constrain(res.box[3],0,DISP_H-1);
    int bw=x2-x1,bh=y2-y1; if(bw<4||bh<4) continue;
    int CL=min(12,min(bw,bh)/3); uint16_t fc=COL_WHITE;
    lcd.drawFastHLine(x1,y1,CL,fc); lcd.drawFastVLine(x1,y1,CL,fc);
    lcd.drawFastHLine(x2-CL,y1,CL,fc); lcd.drawFastVLine(x2,y1,CL,fc);
    lcd.drawFastHLine(x1,y2,CL,fc); lcd.drawFastVLine(x1,y2-CL,CL,fc);
    lcd.drawFastHLine(x2-CL,y2,CL,fc); lcd.drawFastVLine(x2,y2-CL,CL,fc);
    if(res.keypoint.size()>=10)
      for(int k=0;k<10;k+=2)
        lcd.fillCircle(constrain(res.keypoint[k],0,DISP_W-1),
                       constrain(res.keypoint[k+1],0,DISP_H-1),2,COL_GRAY_C);
  }
}

void toggleFaceDetect() {
  faceDetectMode=!faceDetectMode; faceDetectCount=0;
  const char* msg=faceDetectMode?"FACE DETECT  ON":"FACE DETECT  OFF";
  lcd.setFont(&fonts::Font0);
  int tw=lcd.textWidth(msg),pw=tw+14,px=(DISP_W-pw)/2;
  lcd.fillRoundRect(px,DISP_H/2-8,pw,15,7,faceDetectMode?COL_WHITE:COL_GRAY_3);
  lcd.setTextColor(faceDetectMode?COL_BLACK:COL_GRAY_5);
  lcd.drawString(msg,px+7,DISP_H/2-5);
  delay(800); lcd.fillRect(0,DISP_H/2-12,DISP_W,22,COL_BLACK);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Viewfinder
// ─────────────────────────────────────────────────────────────────────────────
void renderViewfinder() {
  camera_fb_t *fb=esp_camera_fb_get(); if(!fb) return;
  if(fb->format==PIXFORMAT_RGB565&&fb->width==DISP_W&&fb->height==DISP_H) {
    lcd.pushImage(0,0,DISP_W,DISP_H,(uint16_t*)fb->buf);
    drawCornerBrackets(COL_GRAY_E);
    if(faceDetectMode) runFaceDetect(fb);
    char fpsBuf[12]; snprintf(fpsBuf,sizeof(fpsBuf),"%.0f fps",fpsValue);
    drawPill(32,10,fpsBuf,COL_PILL_BG,COL_GRAY_A);
    drawPill(DISP_W-35,10,sensorName,COL_PILL_BG,COL_GRAY_A);
    char shotBuf[10]; snprintf(shotBuf,sizeof(shotBuf),"#%04d",photoCount+1);
    drawPill(30,DISP_H-10,shotBuf,COL_PILL_BG,COL_GRAY_8);
    drawPill(DISP_W-36,DISP_H-10,sdReady?"SD  OK":"SD  --",COL_PILL_BG,
             sdReady?COL_GRAY_8:COL_GRAY_5);
    if(faceDetectMode) {
      char faceBuf[12];
      snprintf(faceBuf,sizeof(faceBuf),faceDetectCount>0?"FACE  %d":"FACE  --",faceDetectCount);
      drawPill(DISP_W/2,DISP_H-10,faceBuf,COL_PILL_BG,COL_GRAY_C);
    }
    updateFPS();
  } else {
    lcd.fillScreen(COL_BLACK); lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("format not rgb565",10,110);
  }
  esp_camera_fb_return(fb);
}

void showSavedFeedback(bool saved) {
  lcd.setFont(&fonts::Font0);
  if(saved) {
    char buf[16]; snprintf(buf,sizeof(buf),"SAVED  #%04d",photoCount);
    int tw=lcd.textWidth(buf),pw=tw+14,px=(DISP_W-pw)/2;
    lcd.fillRoundRect(px,6,pw,15,7,COL_WHITE);
    lcd.setTextColor(COL_BLACK); lcd.drawString(buf,px+7,9);
  } else {
    const char* msg=sdReady?"WRITE ERR":"NO SD";
    int tw=lcd.textWidth(msg),pw=tw+14,px=(DISP_W-pw)/2;
    lcd.fillRoundRect(px,6,pw,15,7,COL_GRAY_3);
    lcd.setTextColor(COL_GRAY_5); lcd.drawString(msg,px+7,9);
  }
}

void captureAndPreview() {
  digitalWrite(LED_PIN,HIGH);
  camera_fb_t *fb=esp_camera_fb_get();
  if(!fb) { digitalWrite(LED_PIN,LOW); blinkLED(5,50,50); return; }
  digitalWrite(LED_PIN,LOW);
  if(fb->format==PIXFORMAT_RGB565&&fb->width==DISP_W) {
    lcd.pushImage(0,0,DISP_W,DISP_H,(uint16_t*)fb->buf);
    drawCornerBrackets(COL_GRAY_E);
  }
  bool saved=false;
  if(sdReady) {
    uint8_t *jpg_buf=nullptr; size_t jpg_len=0; bool ok=false;
    if(fb->format==PIXFORMAT_JPEG) { jpg_buf=fb->buf; jpg_len=fb->len; ok=true; }
    else { ok=frame2jpg(fb,85,&jpg_buf,&jpg_len); }
    if(ok&&jpg_buf) {
      photoCount++;
      char path[40]; snprintf(path,sizeof(path),"/sdcard/photo_%04d.jpg",photoCount);
      FILE* f=fopen(path,"wb");
      if(f) { size_t w=fwrite(jpg_buf,1,jpg_len,f); fclose(f); saved=(w==jpg_len); }
      if(fb->format!=PIXFORMAT_JPEG) free(jpg_buf);
    }
  }
  esp_camera_fb_return(fb);
  showSavedFeedback(saved);
  if(saved) blinkLED(2,150,80); else blinkLED(5,50,50);
  delay(2000); fpsLastTime=millis(); fpsFrameCount=0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Sensor settings + Camera init
// ─────────────────────────────────────────────────────────────────────────────
void applySensorSettings(sensor_t *s,uint16_t pid) {
  s->set_brightness(s,0); s->set_contrast(s,0); s->set_saturation(s,0);
  s->set_special_effect(s,0); s->set_whitebal(s,1); s->set_awb_gain(s,1);
  s->set_wb_mode(s,0); s->set_exposure_ctrl(s,1); s->set_aec2(s,1);
  s->set_ae_level(s,0); s->set_gain_ctrl(s,1); s->set_agc_gain(s,0);
  if(pid==PID_GC2145) {
    s->set_aec_value(s,300); s->set_gainceiling(s,GAINCEILING_4X);
    s->set_hmirror(s,HMIRROR_GC2145); s->set_vflip(s,VFLIP_GC2145);
  } else if(pid==PID_OV3660) {
    s->set_aec_value(s,400); s->set_gainceiling(s,GAINCEILING_4X);
    s->set_hmirror(s,HMIRROR_OV3660); s->set_vflip(s,VFLIP_OV3660);
  } else {
    s->set_aec_value(s,300); s->set_gainceiling(s,GAINCEILING_2X);
    s->set_hmirror(s,0); s->set_vflip(s,0);
  }
}

bool initCamera() {
  camera_config_t cfg;
  cfg.ledc_channel=LEDC_CHANNEL_0; cfg.ledc_timer=LEDC_TIMER_0;
  cfg.pin_d0=Y2_GPIO_NUM; cfg.pin_d1=Y3_GPIO_NUM; cfg.pin_d2=Y4_GPIO_NUM; cfg.pin_d3=Y5_GPIO_NUM;
  cfg.pin_d4=Y6_GPIO_NUM; cfg.pin_d5=Y7_GPIO_NUM; cfg.pin_d6=Y8_GPIO_NUM; cfg.pin_d7=Y9_GPIO_NUM;
  cfg.pin_xclk=XCLK_GPIO_NUM; cfg.pin_pclk=PCLK_GPIO_NUM;
  cfg.pin_vsync=VSYNC_GPIO_NUM; cfg.pin_href=HREF_GPIO_NUM;
  cfg.pin_sccb_sda=SIOD_GPIO_NUM; cfg.pin_sccb_scl=SIOC_GPIO_NUM;
  cfg.pin_pwdn=PWDN_GPIO_NUM; cfg.pin_reset=RESET_GPIO_NUM;
  cfg.pixel_format=PIXFORMAT_RGB565; cfg.frame_size=FRAMESIZE_QVGA;
  cfg.grab_mode=CAMERA_GRAB_LATEST;
  if(psramFound()) { cfg.jpeg_quality=7; cfg.fb_count=2; cfg.fb_location=CAMERA_FB_IN_PSRAM; }
  else { cfg.jpeg_quality=12; cfg.fb_count=1; cfg.fb_location=CAMERA_FB_IN_DRAM; }
  cfg.xclk_freq_hz=20000000;
  if(esp_camera_init(&cfg)!=ESP_OK) return false;
  sensor_t *s=esp_camera_sensor_get(); if(!s) return false;
  detectedSensor=s->id.PID;
  if(detectedSensor==PID_OV3660) {
    esp_camera_deinit(); delay(100); cfg.xclk_freq_hz=24000000;
    if(esp_camera_init(&cfg)!=ESP_OK) {
      cfg.xclk_freq_hz=20000000;
      if(esp_camera_init(&cfg)!=ESP_OK) return false;
    }
    s=esp_camera_sensor_get(); if(!s) return false;
  }
  switch(detectedSensor) {
    case PID_GC2145: strncpy(sensorName,"GC2145",sizeof(sensorName)); break;
    case PID_OV3660: strncpy(sensorName,"OV3660",sizeof(sensorName)); break;
    default: snprintf(sensorName,sizeof(sensorName),"0x%04X",detectedSensor); break;
  }
  applySensorSettings(s,detectedSensor);
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Setup
// ─────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Sanzxcam v4.9b ===");
  pinMode(LED_PIN,OUTPUT); digitalWrite(LED_PIN,LOW);
  pinMode(BOOT_BTN_PIN,INPUT_PULLUP);
  pinMode(REC_BTN_PIN,INPUT_PULLUP);
  setCpuFrequencyMhz(240);

  lcd.init(); lcd.setRotation(3); lcd.fillScreen(COL_BLACK);

  static uint16_t touchCalData[8]={3851,3630,673,3277,3965,160,772,136};
  bool calValid=false;
  for(int i=0;i<8;i++) if(touchCalData[i]!=0) { calValid=true; break; }
  if(calValid) lcd.setTouchCalibrate(touchCalData);

  TJpgDec.setJpgScale(1); TJpgDec.setSwapBytes(true); TJpgDec.setCallback(tjpgdecOutput);

  sdReady=mountSDFull();
  if(sdReady) { scanPhotoCount(); scanVideoCount(); }

  msc.vendorID("ESP32S3"); msc.productID("SD Card"); msc.productRevision("1.0");
  msc.onRead(onRead); msc.onWrite(onWrite);
  msc.begin(sdTotalSectors>0?sdTotalSectors:0,512);
  msc.mediaPresent(false); USB.begin();

  bool camOK=initCamera();
  bool pidOK=(detectedSensor==PID_GC2145||detectedSensor==PID_OV3660);
  uint32_t xclkHz=(detectedSensor==PID_OV3660)?24000000:20000000;
  runBootSequence(sdReady,sdSizeMB,pidOK,detectedSensor,camOK,xclkHz);

  if(!camOK) {
    lcd.fillScreen(COL_BLACK); lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("camera init failed",(DISP_W-lcd.textWidth("camera init failed"))/2,110);
    while(true) { blinkLED(3,100,100); delay(1000); }
  }

  lcd.fillScreen(COL_BLACK);
  fpsLastTime=millis(); fpsFrameCount=0;
  blinkLED(3,120,120);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Loop
// ─────────────────────────────────────────────────────────────────────────────
void loop() {

  // ── Caption auto-clear ───────────────────────────────────────────────────
  if(appMode==MODE_PHOTO_VIEW&&photoViewCaptionVisible&&millis()>photoViewCaptionUntilMs) {
    photoViewClearCaption();
  }

  // ── Tombol REC fisik ────────────────────────────────────────────────────
  if(appMode==MODE_VIEWFINDER&&!recActive&&digitalRead(REC_BTN_PIN)==LOW) {
    delay(50);
    if(digitalRead(REC_BTN_PIN)==LOW) {
      unsigned long recPressStart=millis();
      while(digitalRead(REC_BTN_PIN)==LOW) {
        delay(10); esp_task_wdt_reset();
        unsigned long held=millis()-recPressStart;
        if(held>500) {
          int barW=constrain((int)map((long)held,500L,(long)REC_LONG_PRESS_MS,0L,(long)DISP_W),0,DISP_W);
          lcd.fillRect(0,0,barW,3,COL_GRAY_5);
          lcd.fillRect(barW,0,DISP_W-barW,3,COL_BLACK);
        }
      }
      unsigned long recDuration=millis()-recPressStart;
      lcd.fillRect(0,0,DISP_W,3,COL_BLACK);
      if(recDuration>=REC_LONG_PRESS_MS) toggleFaceDetect();
      else startRecording();
    }
  }

  // ── Saat recording ──────────────────────────────────────────────────────
  if(recActive) {
    if(digitalRead(BOOT_BTN_PIN)==LOW) {
      delay(40);
      if(digitalRead(BOOT_BTN_PIN)==LOW) {
        while(digitalRead(BOOT_BTN_PIN)==LOW) { delay(10); esp_task_wdt_reset(); }
        stopRecording(); return;
      }
    }
    if(digitalRead(REC_BTN_PIN)==LOW) {
      delay(40);
      if(digitalRead(REC_BTN_PIN)==LOW) {
        while(digitalRead(REC_BTN_PIN)==LOW) { delay(10); esp_task_wdt_reset(); }
        stopRecording(); return;
      }
    }
    int32_t rtx,rty;
    if(lcd.getTouch(&rtx,&rty)) {
      while(lcd.getTouch(&rtx,&rty)) { delay(10); esp_task_wdt_reset(); }
      if(inZoneBottomLeft(rtx,rty)||inZoneTopLeft(rtx,rty)) { stopRecording(); return; }
    }
    recordFrame(); return;
  }

  // ── Tombol BOOT ─────────────────────────────────────────────────────────
  if(digitalRead(BOOT_BTN_PIN)==LOW) {
    unsigned long pressStart=millis();
    while(digitalRead(BOOT_BTN_PIN)==LOW) {
      delay(10); esp_task_wdt_reset();
      if(!usbModeActive&&appMode==MODE_VIEWFINDER) {
        unsigned long held=millis()-pressStart;
        if(held>1000) {
          int barW=constrain((int)map((long)held,1000L,(long)LONG_PRESS_MS,0L,(long)DISP_W),0,DISP_W);
          lcd.fillRect(0,DISP_H-3,barW,3,COL_GRAY_7);
          lcd.fillRect(barW,DISP_H-3,DISP_W-barW,3,COL_BLACK);
        }
      }
    }
    unsigned long duration=millis()-pressStart;
    if(duration<50) return;
    unsigned long now=millis();

    if(usbModeActive) {
      exitUSBMode();
    } else if(appMode==MODE_VIEWFINDER&&duration>=LONG_PRESS_MS) {
      lcd.fillRect(0,DISP_H-3,DISP_W,3,COL_BLACK);
      if(now-lastBtnPress>DEBOUNCE_MS) { lastBtnPress=now; if(sdReady) enterUSBMode(); }
    } else if(appMode==MODE_VIEWFINDER) {
      if(now-lastBtnPress>DEBOUNCE_MS) { lastBtnPress=now; captureAndPreview(); }
    } else if(appMode==MODE_GALLERY) {
      if(galleryCompareSelect) {
        galleryCompareSelect=false; galleryCompareIdxA=-1; drawGallery();
      } else {
        appMode=MODE_VIEWFINDER; lcd.fillScreen(COL_BLACK);
        fpsLastTime=millis(); fpsFrameCount=0;
      }
    } else if(appMode==MODE_PHOTO_VIEW) {
      photoViewCaptionVisible=false; photoViewCaptionUntilMs=0;
      if(photoPixelBuf) { free(photoPixelBuf); photoPixelBuf=nullptr; }
      appMode=MODE_GALLERY; drawGallery();
    } else if(appMode==MODE_COMPARE) {
      appMode=MODE_GALLERY; drawGallery();
    } else if(appMode==MODE_MJPEG_PLAYER) {
      mjpegClose(); appMode=MODE_GALLERY; drawGallery();
    }
    return;
  }

  // ── Touch handling ──────────────────────────────────────────────────────
  int32_t tx,ty;
  if(lcd.getTouch(&tx,&ty)) {
    unsigned long touchStart=millis();
    int32_t tx2=tx,ty2=ty;
    int32_t panLastX=tx,panLastY=ty;
    bool isPan=false;

    // ── Pan tracking (photo view zoom>1×) ─────────────────────────────
    if(appMode==MODE_PHOTO_VIEW&&photoZoomLevel>0) {
      while(lcd.getTouch(&tx2,&ty2)) {
        int dx=tx2-panLastX, dy=ty2-panLastY;
        if(abs(dx)>PAN_DEADZONE||abs(dy)>PAN_DEADZONE||isPan) {
          isPan=true;
          float zf=photoZoomFactors[photoZoomLevel];
          photoZoomOffX-=(int)(dx/zf);
          photoZoomOffY-=(int)(dy/zf);
          panLastX=tx2; panLastY=ty2;
          photoViewRender();
        }
        delay(10); esp_task_wdt_reset();
      }
    } else {
      while(lcd.getTouch(&tx2,&ty2)) { delay(10); esp_task_wdt_reset(); }
    }

    unsigned long touchDur=millis()-touchStart;
    int32_t swipeX=tx2-tx, swipeY=ty2-ty;

    // ── MODE VIEWFINDER ─────────────────────────────────────────────────
    if(appMode==MODE_VIEWFINDER&&!usbModeActive) {
      if(touchDur>=LONG_TAP_GALLERY_MS) {
        if(sdReady) {
          lcd.setFont(&fonts::Font0);
          const char* msg="masuk USB mode...";
          int tw=lcd.textWidth(msg),pw=tw+14,px=(DISP_W-pw)/2;
          lcd.fillRoundRect(px,DISP_H/2-8,pw,15,7,COL_GRAY_3);
          lcd.setTextColor(COL_GRAY_5); lcd.drawString(msg,px+7,DISP_H/2-5);
          delay(500); enterUSBMode();
        }
        return;
      }
      unsigned long now=millis();
      if(inZoneTopLeft(tx,ty)) {
        if(sdReady) { scanGalleryFiles(); appMode=MODE_GALLERY; drawGallery(); }
        return;
      }
      if(inZoneTopRight(tx,ty)) { toggleFaceDetect(); return; }
      if(inZoneBottomLeft(tx,ty)) {
        if(now-lastTapMs>DEBOUNCE_MS) {
          lastTapMs=now;
          if(recActive) stopRecording(); else startRecording();
        }
        return;
      }
      if(inZoneCapture(tx,ty)) {
        if(now-lastTapMs>DEBOUNCE_MS) { lastTapMs=now; captureAndPreview(); }
      }
    }

    // ── MODE GALLERY ────────────────────────────────────────────────────
    else if(appMode==MODE_GALLERY) {
      if(inZoneTopRight(tx,ty)) {
        if(galleryCompareSelect) {
          galleryCompareSelect=false; galleryCompareIdxA=-1; drawGallery();
        } else {
          if(photoPixelBuf) { free(photoPixelBuf); photoPixelBuf=nullptr; }
          appMode=MODE_VIEWFINDER; lcd.fillScreen(COL_BLACK);
          fpsLastTime=millis(); fpsFrameCount=0;
        }
        return;
      }
      if(abs(swipeY)>20&&abs(swipeY)>abs(swipeX)) {
        float speed=(float)abs(swipeY)/(float)max(1UL,touchDur);
        int steps=max(1,min(GALLERY_ITEMS_PAGE,(int)(speed*15.0f)));
        if(swipeY<0) galleryScroll=min(galleryScroll+steps,max(0,galleryCount-GALLERY_ITEMS_PAGE));
        else galleryScroll=max(galleryScroll-steps,0);
        drawGallery(); return;
      }
      if(ty>20&&galleryCount>0) {
        int idx=galleryScroll+(ty-24)/GALLERY_ITEM_H;
        if(idx>=0&&idx<galleryCount) {
          if(galleryCompareSelect) {
            if(!galleryIsVideo[idx]&&idx!=galleryCompareIdxA) {
              compareIdxA=galleryCompareIdxA;
              compareIdxB=idx;
              galleryCompareSelect=false; galleryCompareIdxA=-1;
              showComparePage();
            }
          } else {
            if(galleryIsVideo[idx]) openMjpegPlayer(galleryFiles[idx]);
            else {
              if(touchDur>=LONG_TAP_GALLERY_MS) {
                galleryCompareSelect=true;
                galleryCompareIdxA=idx;
                drawGallery();
              } else {
                appMode=MODE_PHOTO_VIEW; showPhotoView(idx);
              }
            }
          }
        }
      }
    }

    // ── MODE PHOTO VIEW ─────────────────────────────────────────────────
    else if(appMode==MODE_PHOTO_VIEW) {
      if(isPan) return; // sudah dihandle saat drag

      // ── Long press (di mana saja) → delete ──────────────────────────
      if(touchDur>=LONG_TAP_PHOTO_MS) {
        photoViewClearCaption();
        bool doDelete=photoViewDeleteDialog(galleryFiles[photoViewIndex]);
        if(doDelete) photoViewDeleteCurrent();
        else showPhotoView(photoViewIndex);
        return;
      }

      unsigned long now=millis();

      // ── Pojok kanan atas → back ke gallery ──────────────────────────
      if(pvZoneTopRight(tx,ty)) {
        photoViewCaptionVisible=false; photoViewCaptionUntilMs=0;
        photoZoomLevel=0; photoZoomOffX=0; photoZoomOffY=0;
        if(photoPixelBuf) { free(photoPixelBuf); photoPixelBuf=nullptr; }
        appMode=MODE_GALLERY; drawGallery();
        return;
      }

      // ── Pojok kiri atas → zoom cycle ────────────────────────────────
      if(pvZoneTopLeft(tx,ty)) {
        photoZoomLevel=(photoZoomLevel+1)%ZOOM_LEVELS;
        photoZoomOffX=0; photoZoomOffY=0;
        // center zoom ke tengah layar
        if(photoZoomLevel>0) {
          float zf=photoZoomFactors[photoZoomLevel];
          int vpW=(int)(DISP_W/zf), vpH=(int)(DISP_H/zf);
          photoZoomOffX=max(0,(int)photoBufW/2-vpW/2);
          photoZoomOffY=max(0,(int)photoBufH/2-vpH/2);
        }
        photoViewRender();
        if(photoZoomLevel==0) {
          photoViewDrawCaption(photoViewIndex);
          photoViewCaptionUntilMs=millis()+2000;
          photoViewCaptionVisible=true;
        }
        return;
      }

      // ── Pojok kiri bawah → compare ──────────────────────────────────
      if(pvZoneBotLeft(tx,ty)) {
        // foto ini jadi slot A, kembali ke gallery pilih B
        compareIdxA=photoViewIndex;
        compareIdxB=-1;
        photoZoomLevel=0; photoZoomOffX=0; photoZoomOffY=0;
        if(photoPixelBuf) { free(photoPixelBuf); photoPixelBuf=nullptr; }
        // masuk gallery compare select
        scanGalleryFiles();
        galleryCompareSelect=true;
        galleryCompareIdxA=compareIdxA;
        appMode=MODE_GALLERY; drawGallery();
        return;
      }

      // ── Tap area tengah / kiri / kanan → navigasi (zoom=1× saja) ───
      if(photoZoomLevel==0) {
        if(now-lastTapMs>DEBOUNCE_MS) {
          lastTapMs=now;
          if(tx<DISP_W/2) photoViewPrev();
          else             photoViewNext();
        }
      }
      // zoom>1×: navigasi tidak aktif, hanya pan
    }

    // ── MODE COMPARE ────────────────────────────────────────────────────
    else if(appMode==MODE_COMPARE) {
      if(inZoneTopRight(tx,ty)||ty<12) {
        appMode=MODE_GALLERY; drawGallery();
        return;
      }
      // tap kiri/kanan → kembali gallery untuk ganti foto slot
      appMode=MODE_GALLERY;
      galleryCompareSelect=false;
      drawGallery();
    }

    // ── MODE MJPEG PLAYER ───────────────────────────────────────────────
    else if(appMode==MODE_MJPEG_PLAYER) {
      mjpegHandleTouch(tx,ty,tx2,ty2,touchDur);
    }
  }

  // ── Render mode aktif ──────────────────────────────────────────────────
  if(appMode==MODE_VIEWFINDER&&!usbModeActive) renderViewfinder();
  else if(appMode==MODE_MJPEG_PLAYER) loopMjpegPlayer();
  else if(usbModeActive) { delay(50); esp_task_wdt_reset(); }
}
