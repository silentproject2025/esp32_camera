/*
 * ESP32-S3-CAM (Freenove ESP32-S3-WROOM)
 * Version: v5.9-fix5
 *
 * ═══════════════════════════════════════════════════════════════
 *  CHANGELOG v5.9-fix5 (di atas v5.9-fix4):
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
 *  TETAP dari v5.9-fix4:
 *  [MULTI-KEY] Support hingga 5 Gemini API key sekaligus
 *  [KEY-MANAGER] Menu manajemen API key dari dalam kamera
 *  [AI-DESCRIBE] Deskripsi foto via Google Gemini Vision API
 *  [WIFI-SD]     WiFi config dari SD card
 *  [SETTINGS]    Simpan/load preferensi ke /sdcard/settings.ini
 *  [JUMP]        Jump-to-number di Gallery (BOOT long-press)
 *  [STEGO]       Steganografi JPEG & BMP
 *  [EXIF]        Inject EXIF ke JPEG
 * ═══════════════════════════════════════════════════════════════
 *
 * TOMBOL LAYOUT (final v5.9-fix5):
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

// ─────────────────────────────────────────────────────────────────────────────
//  App Mode
// ─────────────────────────────────────────────────────────────────────────────
enum AppMode {
  MODE_VIEWFINDER,
  MODE_GALLERY,
  MODE_PHOTO_VIEW,
  MODE_MJPEG_PLAYER,
  MODE_MENU_LED,
  MODE_MENU_EXP,
  MODE_MENU_EXP_ADJ,
  MODE_MENU_FORMAT,
  MODE_DIALOG_DELETE,
  MODE_JUMP_INPUT,
  MODE_AI_FEATURE_MENU,
  MODE_AI_DESCRIBE,
  MODE_AI_NO_CONFIG,
  MODE_KEY_MANAGER,
  MODE_FEATURES,
};
AppMode appMode  = MODE_VIEWFINDER;
Adafruit_NeoPixel strip(NEO_NUM, NEO_PIN, NEO_GRB + NEO_KHZ800);
enum NeoMode { NEO_MODE_OFF, NEO_MODE_SOLID, NEO_MODE_BREATH, NEO_MODE_PULSE, NEO_MODE_SPIN };
NeoMode g_neoMode = NEO_MODE_OFF;
uint8_t g_neoR=0, g_neoG=0, g_neoB=0;
uint32_t g_neoLastMs=0;
bool g_neoState=false;
AppMode prevMode = MODE_VIEWFINDER;

// ─────────────────────────────────────────────────────────────────────────────
//  AI FEATURE DEFINITIONS
// ─────────────────────────────────────────────────────────────────────────────
enum AIFeature : uint8_t {
  AI_FEAT_DESCRIBE    = 0,
  AI_FEAT_SCAVENGER   = 1,
  AI_FEAT_MOOD        = 2,
  AI_FEAT_ANPR        = 3,
  AI_FEAT_SKY         = 4,
  AI_FEAT_PEST        = 5,
  AI_FEAT_PRODUCE     = 6,
  AI_FEAT_COUNT       = 7,
};

struct AIFeatureDef {
  const char* label;
  const char* icon;
  uint16_t    iconColor;
  const char* prompt;
};

static const AIFeatureDef AI_FEATURES[AI_FEAT_COUNT] = {
  {
    "Describe",
    "?",
    COL_AI_ACCENT,
    "Deskripsikan isi gambar ini dalam Bahasa Indonesia. "
    "Jelaskan objek utama, warna dominan, suasana, dan konteks gambar. "
    "Maksimal 4 kalimat, gunakan bahasa sederhana."
  },
  {
    "Scavenger Hunt",
    "S",
    0xFFE0,
    "Kamu adalah game master Scavenger Hunt. "
    "Analisis gambar ini dan tentukan apakah gambar tersebut memenuhi tantangan yang diberikan. "
    "Pertama, sebutkan tantangan yang kamu ciptakan berdasarkan objek di gambar. "
    "Lalu jawab apakah gambar memenuhi tantangan tersebut, berikan poin 0-10, dan tantangan baru. "
    "Format: TANTANGAN: [kalimat] | HASIL: [lulus/gagal] | POIN: [angka] | TANTANGAN BARU: [kalimat]. "
    "Gunakan Bahasa Indonesia."
  },
  {
    "Mood Reader",
    "M",
    0xF81F,
    "Kamu adalah psikolog ekspresi wajah. "
    "Analisis wajah atau suasana keseluruhan dalam gambar ini. "
    "Jika ada wajah manusia: identifikasi emosi utama, perkiraan intensitas 0-100%, dan penjelasan singkat. "
    "Jika tidak ada wajah: analisis mood keseluruhan gambar. "
    "Gunakan Bahasa Indonesia. Maksimal 5 kalimat."
  },
  {
    "ANPR",
    "P",
    0xFC00,
    "Kamu adalah sistem ANPR (Automatic Number Plate Recognition). "
    "Cari dan baca semua teks pelat nomor kendaraan dalam gambar ini. "
    "Untuk setiap pelat: teks pelat lengkap, posisi di gambar, perkiraan keterbacaan. "
    "Jika tidak ada pelat: nyatakan 'TIDAK ADA PELAT'. "
    "Format: PELAT: [teks] | POSISI: [lokasi] | KUALITAS: [tingkat]. "
    "Gunakan Bahasa Indonesia."
  },
  {
    "Sky Watch",
    "W",
    0x841F,
    "Kamu adalah ahli meteorologi dan astronomi. "
    "Analisis langit, awan, atau fenomena atmosfer dalam gambar ini. "
    "Identifikasi: jenis awan, kondisi cuaca, perkiraan waktu, dan fenomena langit menarik. "
    "Jika langit tidak terlihat: analisis kondisi pencahayaan yang ada. "
    "Gunakan Bahasa Indonesia. Maksimal 5 kalimat."
  },
  {
    "Pest Count",
    "#",
    0xF800,
    "Kamu adalah entomolog dan ahli hama pertanian. "
    "Analisis gambar ini dan hitung semua serangga, hama, atau organisme kecil yang terlihat. "
    "Untuk setiap jenis: identifikasi nama, hitung jumlah, dan estimasi total. "
    "Format: SPESIES: [nama] | TERLIHAT: [angka] | ESTIMASI TOTAL: [angka]. "
    "Beri penilaian tingkat infestasi dan rekomendasi penanganan. Gunakan Bahasa Indonesia."
  },
  {
    "Produce ID",
    "F",
    0x07E0,
    "Kamu adalah ahli botani dan hortikultura. "
    "Identifikasi semua buah, sayuran, tanaman, atau produk pertanian dalam gambar ini. "
    "Untuk setiap item: nama umum, nama ilmiah, perkiraan jumlah, kondisi, dan nilai gizi singkat. "
    "Format: ITEM: [nama] | JUMLAH: [angka] | KONDISI: [status] | INFO: [kalimat singkat]. "
    "Di bagian akhir, estimasi total nilai gizi koleksi ini. Gunakan Bahasa Indonesia."
  },
};

static AIFeature aiSelectedFeature = AI_FEAT_DESCRIBE;
static bool aiFromViewfinder = false;

// ─────────────────────────────────────────────────────────────────────────────
//  GC2145 Capture Format
// ─────────────────────────────────────────────────────────────────────────────
enum GC2145Format : uint8_t {
  GFMT_BMP = 0,
  GFMT_JPG = 1,
};

// ─────────────────────────────────────────────────────────────────────────────
//  WiFi & Gemini AI config
// ─────────────────────────────────────────────────────────────────────────────
#define WIFI_INI_PATH    "/sdcard/wifi.ini"
#define GEMINI_INI_PATH  "/sdcard/gemini.ini"
#define GEMINI_URL_BASE  "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key="
#define AI_RESULT_MAX    800
#define AI_LINE_MAX       40
#define AI_LINE_W         56

#define GEMINI_KEY_MAX    5

static char wifiSSID[64]                      = "";
static char wifiPass[64]                      = "";
static char geminiApiKeys[GEMINI_KEY_MAX][80] = {};
static int  geminiKeyCount                    = 0;
static int  geminiKeyActive                   = 0;
static char aiResult[AI_RESULT_MAX+1]         = "";
static bool aiDescribeBusy                    = false;
static char aiLines[AI_LINE_MAX][AI_LINE_W]   = {};
static int  aiTotalLines                      = 0;
static int  aiScrollLine                      = 0;

// ─────────────────────────────────────────────────────────────────────────────
//  PHOTO VIEW VARIABLES
// ─────────────────────────────────────────────────────────────────────────────
char  photoViewPath[48]               = {};
int   photoViewIndex                  = 0;
unsigned long photoViewCaptionUntilMs = 0;
bool          photoViewCaptionVisible = false;

// [PORTED v6.1] MPU6050 & EIS & HDR globals
#define PIN_MPU_SDA 43
#define PIN_MPU_SCL 44
#define MPU_I2C_ADDR 0x68
#define MPU_READ_MS 80
#define MPU_TILT_DEG 15.0f
#define MPU_SHAKE_THRESH 3.5f
#define MPU_LOG_MS 5000

#define EIS_CROP_PCT 0.12f
#define EIS_ALPHA 0.05f
#define EIS_GYRO_SCALE 8.0f
#define EIS_CROP_X ((int)(DISP_W * EIS_CROP_PCT / 2))
#define EIS_CROP_Y ((int)(DISP_H * EIS_CROP_PCT / 2))
#define EIS_VP_W (DISP_W - EIS_CROP_X * 2)
#define EIS_VP_H (DISP_H - EIS_CROP_Y * 2)

#define HDR_DOUBLE_TAP_MS 400
#define HDR_STABLE_THRESH 0.08f
#define HDR_WAIT_MS 3000

static Adafruit_MPU6050 mpu;
static bool g_mpuOk = false;
static uint8_t mpuDlpfIndex = 4; // default 21Hz
static const char* DLPF_LABELS[7] = {"260Hz","184Hz","94Hz","44Hz","21Hz","10Hz","5Hz"};
static const mpu6050_bandwidth_t DLPF_VALUES[7] = {
  MPU6050_BAND_260_HZ, MPU6050_BAND_184_HZ, MPU6050_BAND_94_HZ,
  MPU6050_BAND_44_HZ,  MPU6050_BAND_21_HZ,  MPU6050_BAND_10_HZ,
  MPU6050_BAND_5_HZ
};

void applyDLPF() {
  if (g_mpuOk) {
    mpu.setFilterBandwidth(DLPF_VALUES[mpuDlpfIndex]);
  }
}
static float g_accX=0, g_accY=0, g_accZ=0;
static float g_gyroX=0, g_gyroY=0, g_gyroZ=0;
static float g_gyroCalX = 0.0f, g_gyroCalY = 0.0f, g_gyroCalZ = 0.0f;
static bool g_mpuCalLoaded = false;
static float g_accOffX = 0.0f;
static float g_accOffY = 0.0f;
static float g_accOffZ = 0.0f;
static float g_tiltX=0.0f, g_tiltY=0.0f;
static float mpuKalmanRmeas = 0.10f;
static float mpuTiltDeadzone = 2.0f;
static bool g_shake=false, g_tilted=false;
static uint8_t g_orientation=0;
static uint32_t g_mpuLastMs=0, g_mpuLogMs=0;

static float g_eisOffX=0.0f, g_eisOffY=0.0f;
static float g_eisBiasX=0.0f, g_eisBiasY=0.0f;

static bool g_hdrWaiting=false;
static uint32_t g_hdrFirstMs=0;

static bool eisEnabled = true;
static bool hdrEnabled = true;
static bool autoRotateEnabled = true;
static bool mpuLogEnabled = false;
static bool hudEnabled = true;
static bool galleryGridMode = true;
static bool galleryGridActive = false;
static int menuFeatSel = 0;
static bool hdCaptureEnabled = false;
static uint8_t hdCaptureQuality = 4; // 4 = best, 6 = good

// ─────────────────────────────────────────────────────────────────────────────
//  KEY MANAGER STATE
// ─────────────────────────────────────────────────────────────────────────────
static int  kmSelIdx  = 0;
static bool kmDirty   = false;

// ─────────────────────────────────────────────────────────────────────────────
//  BUTTON MANAGER
// ─────────────────────────────────────────────────────────────────────────────
class ButtonManager {
public:
  enum class State { IDLE, PRESSED };
  uint8_t       pin;
  State         state         = State::IDLE;
  unsigned long pressTime     = 0;
  unsigned long lastEventTime = 0;
  ButtonEvent   pendingEvt    = {};

  ButtonManager() {}
  explicit ButtonManager(uint8_t p) : pin(p) {}

  void tick() {
    bool low = (digitalRead(pin) == LOW);
    switch (state) {
      case State::IDLE:
        if (low) { pressTime = millis(); state = State::PRESSED; }
        break;
      case State::PRESSED:
        if (!low) {
          uint32_t dur = millis() - pressTime;
          if (dur >= DEBOUNCE_MS && (millis() - lastEventTime) >= 100) {
            pendingEvt = { pin, dur, (dur >= LONG_PRESS_MS), (dur < LONG_PRESS_MS), true };
            lastEventTime = millis();
          }
          state = State::IDLE;
        }
        break;
    }
  }

  bool pollEvent(ButtonEvent& evt) {
    if (!pendingEvt.valid) return false;
    evt = pendingEvt;
    pendingEvt.valid = false;
    return true;
  }

  bool     isHeld()        const { return state == State::PRESSED; }
  uint32_t heldDuration()  const { return (state != State::PRESSED) ? 0 : millis() - pressTime; }
  void     reset()               { state = State::IDLE; pendingEvt.valid = false; pressTime = 0; }
};

ButtonManager btnBoot(BTN_BOOT);
ButtonManager btnB   (BTN_B);
ButtonManager btnC   (BTN_C);
ButtonManager btnD   (BTN_D);

inline void tickAllButtons()  { btnBoot.tick();  btnB.tick();  btnC.tick();  btnD.tick(); }
inline void resetAllButtons() { btnBoot.reset(); btnB.reset(); btnC.reset(); btnD.reset(); }

// ─────────────────────────────────────────────────────────────────────────────
//  DYNAMIC ISLAND
// ─────────────────────────────────────────────────────────────────────────────
#define ISLAND_SHOW_MS      1000
#define ISLAND_ANIM_MS       150
#define ISLAND_MAX_STACK       3
#define ISLAND_W_SINGLE      188
#define ISLAND_W_STACK       216
#define ISLAND_H_ROW          18
#define ISLAND_PAD_V           5
#define ISLAND_PAD_H          10
#define ISLAND_ICON_SZ        10
#define ISLAND_RADIUS         10
#define ISLAND_CX       (DISP_W / 2)

#define ISLAND_FREEZE_MS  (ISLAND_ANIM_MS + ISLAND_SHOW_MS + ISLAND_ANIM_MS + 200)

enum IslandState { ISLAND_HIDDEN, ISLAND_SLIDING_IN, ISLAND_VISIBLE, ISLAND_SLIDING_OUT };

struct NotifEntry {
  NotifType type;
  char      text[28];
  bool      valid;
};

static NotifEntry    islandStack[ISLAND_MAX_STACK];
static int           islandCount     = 0;
static IslandState   islandState     = ISLAND_HIDDEN;
static unsigned long islandShowAt    = 0;
static unsigned long islandHideAt    = 0;
static unsigned long islandAnimStart = 0;
static int           islandLastX     = 0;
static int           islandLastY     = 0;
static int           islandLastW     = 0;
static int           islandLastH     = 0;
static bool          islandDrawnOnce = false;
static unsigned long islandFreezeUntilMs = 0;
static bool          islandNoClear   = false;

static void islandCalcDims(int n, int& iW, int& iH, int& iX) {
  iW = (n > 1) ? ISLAND_W_STACK : ISLAND_W_SINGLE;
  iH = ISLAND_PAD_V * 2 + n * ISLAND_H_ROW + (n - 1) * 2;
  iX = ISLAND_CX - iW / 2;
}

static void islandDrawRow(int idx, int x, int y, int w, bool isFresh) {
  if (idx >= islandCount) return;
  NotifEntry& n        = islandStack[idx];
  const NotifStyle& s  = NOTIF_STYLES[(int)n.type];
  uint16_t rowBg = isFresh ? COL_GRAY_D : COL_BLACK;
  lcd.fillRect(x - 2, y, w + 4, ISLAND_H_ROW - 1, rowBg);
  int iconY = y + (ISLAND_H_ROW - ISLAND_ICON_SZ) / 2;
  lcd.fillRect(x, iconY, ISLAND_ICON_SZ, ISLAND_ICON_SZ, s.iconBg);
  lcd.setFont(&fonts::Font0); lcd.setTextSize(1);
  lcd.setTextColor(s.iconFg);
  lcd.drawString(s.sym, x + 2, iconY + 1);
  lcd.setTextColor(isFresh ? COL_GRAY_E : COL_GRAY_7);
  char buf[28];
  strncpy(buf, n.text, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';
  int maxW = w - ISLAND_ICON_SZ - 8;
  while (strlen(buf) > 4 && lcd.textWidth(buf) > maxW) {
    esp_task_wdt_reset();
    esp_task_wdt_reset();
    int l = strlen(buf);
    buf[l-1] = '\0';
    if (strlen(buf) > 3) { buf[strlen(buf)-1] = '.'; buf[strlen(buf)-2] = '.'; }
  }
  lcd.drawString(buf, x + ISLAND_ICON_SZ + 4, y + 4);
  if (n.type == NOTIF_REC && isFresh) {
    int dotX = x + w - 4;
    int dotY = y + ISLAND_H_ROW / 2;
    bool blink = (millis() / 400) % 2;
    lcd.fillCircle(dotX, dotY, 2, blink ? 0xF800 : COL_GRAY_3);
  }
}

static void islandErase() {
  esp_task_wdt_reset();
  esp_task_wdt_reset();
  if (islandNoClear)    return;
  if (!islandDrawnOnce) return;
  if (islandLastW <= 0 || islandLastH <= 0) return;
  int eraseX = islandLastX - 3;
  int eraseY = islandLastY;
  int eraseW = islandLastW + 6;
  int eraseH = islandLastH + 4;
  if (eraseY < 0) { eraseH += eraseY; eraseY = 0; }
  if (eraseH <= 0) return;
  if (eraseX < 0) { eraseW += eraseX; eraseX = 0; }
  if (eraseW <= 0) return;
  lcd.fillRect(eraseX, eraseY, eraseW, eraseH, COL_BLACK);
}

static void islandDraw(int offsetY = 0) {
  esp_task_wdt_reset();
  esp_task_wdt_reset();
  int n = min(islandCount, ISLAND_MAX_STACK);
  if (n == 0) return;
  int iW, iH, iX;
  islandCalcDims(n, iW, iH, iX);
  int iY = max(-100, offsetY);
  islandLastX = iX; islandLastY = iY; islandLastW = iW; islandLastH = iH;
  if (iY + iH <= 0) return;
  lcd.fillRoundRect(iX - 1, iY + 1, iW + 2, iH + 2, ISLAND_RADIUS + 1, COL_GRAY_2);
  lcd.fillRoundRect(iX, iY, iW, iH, ISLAND_RADIUS, COL_GRAY_D);
  lcd.drawRoundRect(iX, iY, iW, iH, ISLAND_RADIUS, COL_GRAY_5);
  if (iH > 4 && iW > 4)
    lcd.drawRoundRect(iX + 1, iY + 1, iW - 2, iH - 2, ISLAND_RADIUS - 1, COL_GRAY_3);
  int rowY = iY + ISLAND_PAD_V;
  for (int i = 0; i < n; i++) {
    islandDrawRow(i, iX + ISLAND_PAD_H, rowY, iW - ISLAND_PAD_H * 2, (i == 0));
    if (i < n - 1)
      lcd.drawFastHLine(iX + ISLAND_PAD_H, rowY + ISLAND_H_ROW + 1,
                        iW - ISLAND_PAD_H * 2, COL_GRAY_3);
    rowY += ISLAND_H_ROW + 2;
  }
  islandDrawnOnce = true;
}

static void islandHide() {
  islandState = ISLAND_HIDDEN; islandCount = 0;
  islandLastW = 0; islandLastH = 0; islandLastY = 0; islandDrawnOnce = false;
}

void islandPush(NotifType type, const char* text) {
  for (int i = ISLAND_MAX_STACK - 1; i > 0; i--) islandStack[i] = islandStack[i-1];
  islandStack[0].type  = type;
  islandStack[0].valid = true;
  strncpy(islandStack[0].text, text, sizeof(islandStack[0].text) - 1);
  islandStack[0].text[sizeof(islandStack[0].text) - 1] = '\0';
  if (islandCount < ISLAND_MAX_STACK) islandCount++;
  islandShowAt    = millis();
  int showMs = (type == NOTIF_WARN) ? 2000 : (type == NOTIF_INFO) ? 800 : 1000;
  islandHideAt    = millis() + showMs;
  islandAnimStart = millis();
  islandState     = ISLAND_SLIDING_IN;
  islandFreezeUntilMs = millis() + ISLAND_FREEZE_MS;
  if (type == NOTIF_WARN) neoBurst(200, 80, 0, 2);
}

void islandTick() {
  esp_task_wdt_reset();
  esp_task_wdt_reset();
  unsigned long now = millis();
  switch (islandState) {
    case ISLAND_HIDDEN: break;
    case ISLAND_SLIDING_IN: {
      unsigned long elapsed = now - islandAnimStart;
      int nItems = min(islandCount, ISLAND_MAX_STACK);
      int iW, iH, iX;
      islandCalcDims(nItems, iW, iH, iX);
      if (elapsed >= ISLAND_ANIM_MS) {
        islandDraw(0); islandState = ISLAND_VISIBLE;
      } else {
        float progress = (float)elapsed / ISLAND_ANIM_MS;
        progress = 1.0f - pow(1.0f - progress, 3);
        int offsetY = (int)(-iH * (1.0f - progress));
        islandDraw(offsetY);
      }
      break;
    }
    case ISLAND_VISIBLE:
      if (now >= islandHideAt) {
        islandAnimStart = now; islandState = ISLAND_SLIDING_OUT;
      } else {
        if (islandCount > 0 && islandStack[0].type == NOTIF_REC) {
          islandErase(); islandDraw(0);
        }
      }
      break;
    case ISLAND_SLIDING_OUT: {
      unsigned long elapsed = now - islandAnimStart;
      int nItems = min(islandCount, ISLAND_MAX_STACK);
      int iW, iH, iX;
      islandCalcDims(nItems, iW, iH, iX);
      if (elapsed >= ISLAND_ANIM_MS) {
        islandHide();
      } else {
        float progress = (float)elapsed / ISLAND_ANIM_MS;
        progress = pow(progress, 3);
        int offsetY = (int)(-iH * progress);
        islandDraw(offsetY);
      }
      break;
    }
  }
}

void islandForceHide() {
  islandNoClear = false;
  if (islandState != ISLAND_HIDDEN && islandDrawnOnce) islandErase();
  islandHide();
  islandFreezeUntilMs = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  VARIABEL GLOBAL UTAMA
// ─────────────────────────────────────────────────────────────────────────────
sdmmc_card_t* sdCard         = nullptr;
bool          sdReady         = false;
uint32_t      sdTotalSectors  = 0;
bool          sdmmcDriverInit = false;
uint64_t      sdSizeMB        = 0;

bool ledFlashEnabled = false;
static bool neoFlashEnabled = false;

uint8_t expPreset    = 0;
int     expManualVal = 300;
int     expManualGain= 0;

GC2145Format gc2145CaptureFormat = GFMT_BMP;

uint16_t detectedSensor= 0;
char     sensorName[16]= "UNKNOWN";

int      photoCount    = 0;

// ─────────────────────────────────────────────────────────────────────────────
//  SETTINGS PERSISTENCE
// ─────────────────────────────────────────────────────────────────────────────
#define SETTINGS_PATH "/sdcard/settings.ini"

void saveSettings() {
  if (sdReady) neoBurst(0, 180, 0, 2);
  if (!sdReady) return;
  FILE* f = fopen(SETTINGS_PATH, "w");
  if (!f) return;
  fprintf(f, "# Sanzxcam v5.9-fix5 settings\n");
  fprintf(f, "flash=%d\n",      (int)ledFlashEnabled);
  fprintf(f, "neoflash=%d\n",   (int)neoFlashEnabled);
  fprintf(f, "exp_preset=%d\n", (int)expPreset);
  fprintf(f, "exp_val=%d\n",    expManualVal);
  fprintf(f, "exp_gain=%d\n",   expManualGain);
  fprintf(f, "gc2145_fmt=%d\n", (int)gc2145CaptureFormat);
  fprintf(f, "eis=%d\n", (int)eisEnabled);
  fprintf(f, "hdr=%d\n", (int)hdrEnabled);
  fprintf(f, "autorotate=%d\n", (int)autoRotateEnabled);
  fprintf(f, "mpulog=%d\n", (int)mpuLogEnabled);
  fprintf(f, "hud=%d\n", (int)hudEnabled);
  fprintf(f, "hd_capture=%d\n", (int)hdCaptureEnabled);
  fprintf(f, "hd_quality=%d\n", (int)hdCaptureQuality);
  fprintf(f, "dlpf=%d\n", (int)mpuDlpfIndex);
  fprintf(f, "kalman_r=%.3f\n", mpuKalmanRmeas);
  fprintf(f, "tilt_dz=%.1f\n",  mpuTiltDeadzone);
  fclose(f);
}

void loadSettings() {
  FILE* f = fopen(SETTINGS_PATH, "r");
  if (!f) return;
  char line[64];
  while (fgets(line, sizeof(line), f)) {
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
    int v = 0;
    if      (sscanf(line, "flash=%d",      &v) == 1) { ledFlashEnabled    = (bool)v; }
    else if (sscanf(line, "neoflash=%d",   &v) == 1) { neoFlashEnabled    = (bool)v; }
    else if (sscanf(line, "exp_preset=%d", &v) == 1) { expPreset          = (uint8_t)constrain(v,0,5); }
    else if (sscanf(line, "exp_val=%d",    &v) == 1) { expManualVal       = constrain(v,0,1200); }
    else if (sscanf(line, "exp_gain=%d",   &v) == 1) { expManualGain      = constrain(v,0,30); }
    else if (sscanf(line, "gc2145_fmt=%d", &v) == 1) { gc2145CaptureFormat= (v==1)?GFMT_JPG:GFMT_BMP; }
    else if (sscanf(line, "eis=%d", &v) == 1) { eisEnabled = (bool)v; }
    else if (sscanf(line, "hdr=%d", &v) == 1) { hdrEnabled = (bool)v; }
    else if (sscanf(line, "autorotate=%d", &v) == 1) { autoRotateEnabled = (bool)v; }
    else if (sscanf(line, "mpulog=%d", &v) == 1) { mpuLogEnabled = (bool)v; }
    else if (sscanf(line, "hud=%d", &v) == 1) { hudEnabled = (bool)v; }
    else if (sscanf(line, "hd_capture=%d", &v) == 1) { hdCaptureEnabled = (bool)v; }
    else if (sscanf(line, "hd_quality=%d", &v) == 1) { hdCaptureQuality = (uint8_t)constrain(v, 1, 10); }
    else if (sscanf(line, "dlpf=%d", &v) == 1) { mpuDlpfIndex = (uint8_t)constrain(v, 0, 6); }
    float fv = 0;
    if      (sscanf(line, "kalman_r=%f", &fv) == 1) { mpuKalmanRmeas = constrain(fv, 0.01f, 1.0f); }
    else if (sscanf(line, "tilt_dz=%f", &fv) == 1) { mpuTiltDeadzone = constrain(fv, 0.0f, 10.0f); }
  }
  fclose(f);
  if (g_mpuOk) applyDLPF();
}

void loadMPUCalibration() {
  FILE* f = fopen("/sdcard/mpu_cal.ini", "r");
  if (!f) {
    g_mpuCalLoaded = false;
    return;
  }
  char line[64];
  while (fgets(line, sizeof(line), f)) {
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
    float v = 0;
    if (sscanf(line, "calX=%f", &v) == 1)      { g_gyroCalX = v; }
    else if (sscanf(line, "calY=%f", &v) == 1) { g_gyroCalY = v; }
    else if (sscanf(line, "calZ=%f", &v) == 1) { g_gyroCalZ = v; }
  }
  fclose(f);
  g_mpuCalLoaded = true;
  Serial.printf("[MPU] Cal loaded: X=%.5f Y=%.5f Z=%.5f\n", g_gyroCalX, g_gyroCalY, g_gyroCalZ);
}

void runMPUCalibration() {
  lcd.fillScreen(COL_BLACK);
  neoRainbow();
  lcd.setFont(&fonts::Font0); lcd.setTextSize(1); lcd.setTextColor(COL_GRAY_7);
  const char* msg = "letakkan kamera datar & diam";
  lcd.drawString(msg, (DISP_W - lcd.textWidth(msg)) / 2, 80);

  for (int i = 5; i >= 1; i--) {
    lcd.fillRect(0, 110, DISP_W, 40, COL_BLACK);
    lcd.setFont(&fonts::FreeSansBold9pt7b); lcd.setTextSize(2); lcd.setTextColor(COL_WHITE);
    char buf[8]; snprintf(buf, sizeof(buf), "%d", i);
    lcd.drawString(buf, (DISP_W - lcd.textWidth(buf)) / 2, 110);
    for (int j = 0; j < 10; j++) { delay(100); esp_task_wdt_reset(); }
  }

  lcd.fillRect(0, 80, DISP_W, 80, COL_BLACK);
  lcd.setFont(&fonts::Font0); lcd.setTextSize(1); lcd.setTextColor(COL_GRAY_5);
  const char* calMsg = "mengkalibrasi...";
  lcd.drawString(calMsg, (DISP_W - lcd.textWidth(calMsg)) / 2, 110);

  float sx = 0, sy = 0, sz = 0;
  for (int i = 0; i < 500; i++) {
    sensors_event_t a, g, t; mpu.getEvent(&a, &g, &t);
    sx += g.gyro.x; sy += g.gyro.y; sz += g.gyro.z;
    if (i % 50 == 0) esp_task_wdt_reset();
    delay(5);
  }
  g_gyroCalX = sx / 500.0f; g_gyroCalY = sy / 500.0f; g_gyroCalZ = sz / 500.0f;

  lcd.fillRect(0, 110, DISP_W, 40, COL_BLACK);
  lcd.setTextColor(COL_GRAY_C);
  char res[64]; snprintf(res, sizeof(res), "X:%.5f Y:%.5f Z:%.5f", g_gyroCalX, g_gyroCalY, g_gyroCalZ);
  lcd.drawString(res, (DISP_W - lcd.textWidth(res)) / 2, 110);
  for (int j = 0; j < 20; j++) { delay(100); esp_task_wdt_reset(); }

  if (sdReady) {
    FILE* f = fopen("/sdcard/mpu_cal.ini", "w");
    if (f) {
      fprintf(f, "# MPU6050 Calibration - Sanzxcam\n");
      fprintf(f, "calX=%.5f\ncalY=%.5f\ncalZ=%.5f\n", g_gyroCalX, g_gyroCalY, g_gyroCalZ);
      fclose(f);
      g_mpuCalLoaded = true;
      islandPush(NOTIF_OK, "Kalibrasi tersimpan");
    }
  } else {
    islandPush(NOTIF_WARN, "Tersimpan di RAM saja");
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  WIFI & GEMINI CONFIG LOADER
// ─────────────────────────────────────────────────────────────────────────────
static void createFileTemplate(const char* path, const char* content) {
  FILE* f = fopen(path, "w");
  if (!f) return;
  fputs(content, f);
  fclose(f);
  Serial.printf("[CFG] template dibuat: %s\n", path);
}

bool loadWifiConfig() {
  FILE* f = fopen(WIFI_INI_PATH, "r");
  if (!f) {
    createFileTemplate(WIFI_INI_PATH,
      "# Konfigurasi WiFi Sanzxcam\n"
      "# Isi ssid dan pass lalu restart kamera\n"
      "ssid=NamaWiFiKamu\n"
      "pass=PasswordWiFiKamu\n");
    return false;
  }
  char line[128]; bool gotSSID=false, gotPass=false;
  while (fgets(line, sizeof(line), f)) {
    if (line[0]=='#'||line[0]=='\n'||line[0]=='\r') continue;
    char val[64]="";
    if      (sscanf(line,"ssid=%63[^\r\n]",val)==1) { strncpy(wifiSSID,val,63); gotSSID=true; }
    else if (sscanf(line,"pass=%63[^\r\n]",val)==1) { strncpy(wifiPass,val,63); gotPass=true; }
  }
  fclose(f);
  if (strcmp(wifiSSID,"NamaWiFiKamu")==0) return false;
  Serial.printf("[WIFI] ssid=%s\n", wifiSSID);
  return gotSSID && gotPass;
}

bool loadGeminiConfig() {
  FILE* f = fopen(GEMINI_INI_PATH, "r");
  if (!f) {
    createFileTemplate(GEMINI_INI_PATH,
      "# Gemini API Keys untuk Sanzxcam AI\n"
      "# Bisa isi hingga 5 key, auto-fallback jika 403/429\n"
      "# Dapatkan key GRATIS di: aistudio.google.com\n"
      "key1=AIzaXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n"
      "key2=\nkey3=\nkey4=\nkey5=\n");
    return false;
  }
  geminiKeyCount  = 0;
  geminiKeyActive = 0;
  memset(geminiApiKeys, 0, sizeof(geminiApiKeys));
  char line[128];
  while (fgets(line, sizeof(line), f)) {
    if (line[0]=='#'||line[0]=='\n'||line[0]=='\r') continue;
    if (geminiKeyCount >= GEMINI_KEY_MAX) break;
    char val[80] = "";
    for (int ki = 1; ki <= GEMINI_KEY_MAX; ki++) {
      char pattern[16];
      snprintf(pattern, sizeof(pattern), "key%d=%%79[^\r\n]", ki);
      if (sscanf(line, pattern, val) == 1) {
        int vlen = strlen(val);
        while (vlen > 0 && (val[vlen-1]==' '||val[vlen-1]=='\t')) { val[--vlen]='\0'; }
        if (vlen >= 20 && strncmp(val,"AIzaXXXX",8)!=0) {
          strncpy(geminiApiKeys[geminiKeyCount], val, 79);
          geminiApiKeys[geminiKeyCount][79] = '\0';
          geminiKeyCount++;
          Serial.printf("[GEMINI] key%d loaded\n", geminiKeyCount);
        }
        break;
      }
    }
  }
  fclose(f);
  return (geminiKeyCount > 0);
}

bool saveGeminiConfig() {
  if (!sdReady) return false;
  FILE* f = fopen(GEMINI_INI_PATH, "w");
  if (!f) return false;
  fprintf(f, "# Gemini API Keys untuk Sanzxcam AI\n");
  fprintf(f, "# Bisa isi hingga 5 key, auto-fallback jika 403/429\n");
  fprintf(f, "# Dapatkan key GRATIS di: aistudio.google.com\n");
  for (int i = 0; i < GEMINI_KEY_MAX; i++) {
    if (i < geminiKeyCount && strlen(geminiApiKeys[i]) > 0)
      fprintf(f, "key%d=%s\n", i+1, geminiApiKeys[i]);
    else
      fprintf(f, "key%d=\n", i+1);
  }
  fclose(f);
  return true;
}

static void maskApiKey(const char* key, char* out, int outLen) {
  int len = strlen(key);
  if (len == 0) { strncpy(out, "(kosong)", outLen-1); return; }
  if (len <= 12) { snprintf(out, outLen, "%.8s...", key); return; }
  snprintf(out, outLen, "%.8s...%s", key, key + len - 4);
}

// ─────────────────────────────────────────────────────────────────────────────
//  GALLERY — type & globals (declared EARLY so AI menu can reference them)
// ─────────────────────────────────────────────────────────────────────────────
enum GalleryFileType : uint8_t {
  GFILE_JPG   = 0,
  GFILE_BMP   = 1,
  GFILE_VIDEO = 2,
};

#define GALLERY_MAX_FILES  1000
#define GALLERY_ITEMS_PAGE    8
#define GALLERY_ITEM_H       26

char             (*galleryFiles)[32] = nullptr;
GalleryFileType*   galleryFileType   = nullptr;
int   galleryCount  = 0;
int   galleryScroll = 0;
int   gallerySelIdx = 0;

inline bool gIsVideo(int i) { return galleryFileType[i] == GFILE_VIDEO; }
inline bool gIsBmp  (int i) { return galleryFileType[i] == GFILE_BMP;   }
inline bool gIsJpg  (int i) { return galleryFileType[i] == GFILE_JPG;   }
inline bool gIsPhoto(int i) { return galleryFileType[i] != GFILE_VIDEO;  }

// Forward declarations for functions used in AI feature menu
void scanGalleryFiles();
void scanPhotoCount();
void photoViewRender();
void photoViewDrawCaption(int idx);
void openAIDescribeWithFeature(int idx, AIFeature feature);
void drawAINoConfigScreen(bool missingWifi, bool missingGemini);

// ─────────────────────────────────────────────────────────────────────────────
//  AI FEATURE MENU — DRAW
// ─────────────────────────────────────────────────────────────────────────────

// [PORTED v6.1] Features Menu
void drawFeaturesMenu(int sel) {
  int mw = 220, mh = 314, mx = (DISP_W - mw) / 2, my = (DISP_H - mh) / 2;
  lcd.fillScreen(COL_BLACK);
  lcd.fillRoundRect(mx, my, mw, mh, 10, COL_GRAY_D);
  lcd.drawRoundRect(mx, my, mw, mh, 10, COL_GRAY_5);
  lcd.setFont(&fonts::Font0); lcd.setTextSize(1); lcd.setTextColor(COL_GRAY_E);
  const char* title = "--- EXPERIMENTAL FEATURES ---";
  lcd.drawString(title, mx + (mw - lcd.textWidth(title)) / 2, my + 7);
  lcd.drawFastHLine(mx + 10, my + 19, mw - 20, COL_GRAY_3);
  static const char* const rowLabels[12] = {
    "EIS  Electronic Stab", "HDR  Triple Exposure",
    "AUTO-ROTATE  MPU tilt", "MPU LOG  CSV to SD", "HUD  Overlay",
    "CALIBRATE MPU  recalibrate",
    "HD CAPTURE  quality saat foto", "HD QUALITY  4=max / 6=bagus",
    "KALMAN-R  noise filter", "TILT-DZ  deadzone deg",
    "DLPF  filter bandwidth", "CLEAR THUMB CACHE"
  };
  bool* const rowVals[5] = {&eisEnabled, &hdrEnabled, &autoRotateEnabled, &mpuLogEnabled, &hudEnabled};
  for (int i = 0; i < 12; i++) {
    int iy = my + 24 + i * 22; bool hl = (i == sel);
    lcd.fillRect(mx + 8, iy, mw - 16, 18, hl ? COL_GRAY_5 : COL_GRAY_D);
    if (hl) lcd.fillRect(mx + 2, iy, 4, 18, COL_WHITE);
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
  }
  lcd.drawFastHLine(mx + 10, my + mh - 24, mw - 20, COL_GRAY_3);
  lcd.setTextColor(COL_GRAY_A);
  lcd.drawString("C/D=nav  BOOT=toggle/run  B=back", mx + (mw - lcd.textWidth("C/D=nav  BOOT=toggle/run  B=back")) / 2, my + mh - 14);
}

// UI UPDATE v6.0
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

  if (evt.pin == BTN_D && evt.isShort) { menuFeatSel = (menuFeatSel + 1) % 12; drawFeaturesMenu(menuFeatSel); }
  else if (evt.pin == BTN_C && evt.isShort) { menuFeatSel = (menuFeatSel + 11) % 12; drawFeaturesMenu(menuFeatSel); }
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
    }
  }
}
// UI UPDATE v6.0
void drawAIFeatureMenu(int sel, bool fromViewfinder) {
  lcd.fillScreen(COL_BLACK);
  lcd.fillRect(0, 0, DISP_W, 20, COL_GRAY_D);
  lcd.drawFastHLine(0, 20, DISP_W, COL_GRAY_3);
  lcd.setFont(&fonts::Font0); lcd.setTextSize(1);
  lcd.setTextColor(COL_AI_ACCENT);
  const char* title = "AI FEATURE";
  lcd.drawString(title, (DISP_W - lcd.textWidth(title)) / 2, 6);
  lcd.setTextColor(COL_GRAY_5);
  if (fromViewfinder) {
    lcd.drawString("VIEWFINDER", 4, 6);
    lcd.setTextColor(COL_AI_WARN);
    lcd.drawString("capture!", DISP_W - 46, 6);
  } else {
    lcd.drawString("GALLERY", 4, 6);
  }
  const int itemH = 28;
  const int startY = 22;
  for (int i = 0; i < AI_FEAT_COUNT; i++) {
    const AIFeatureDef& feat = AI_FEATURES[i];
    bool isSelected = (i == sel);
    int iy = startY + i * itemH;

    uint16_t rowBg = isSelected ? (((feat.iconColor >> 3) & 0x18C3) | 0x1082) : (i % 2 == 0 ? COL_GRAY_D : COL_BLACK);
    lcd.fillRect(0, iy, DISP_W, itemH - 1, rowBg);
    lcd.drawFastHLine(0, iy + itemH - 1, DISP_W, COL_GRAY_2);
    char numBuf[4]; snprintf(numBuf, sizeof(numBuf), "%d", i + 1);
    lcd.setTextColor(isSelected ? COL_WHITE : COL_GRAY_5);
    lcd.drawString(numBuf, 5, iy + 10);
    int iconX = 18, iconY = iy + 6;

    uint16_t icBg = isSelected ? feat.iconColor : ((feat.iconColor >> 2) & 0x39E7);
    lcd.fillRect(iconX, iconY, 14, 14, icBg);

    lcd.setTextColor(isSelected ? COL_BLACK : COL_GRAY_7);
    int iconTw = lcd.textWidth(feat.icon);
    lcd.drawString(feat.icon, iconX + (14 - iconTw) / 2, iconY + 3);

    lcd.setTextColor(isSelected ? COL_WHITE : COL_GRAY_C);
    lcd.drawString(feat.label, 38, iy + 10);

    if (isSelected) {
      lcd.setTextColor(feat.iconColor);
      lcd.drawString(">", DISP_W - 12, iy + 10);
    }
  }
  int footerY = startY + AI_FEAT_COUNT * itemH;
  if (footerY + 14 < DISP_H - 22) {
    lcd.drawFastHLine(0, footerY, DISP_W, COL_GRAY_2);
    lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_3);
    char preview[50]; strncpy(preview, AI_FEATURES[sel].prompt, 49); preview[49] = '\0';
    int maxW = DISP_W - 8;
    while (strlen(preview) > 6 && lcd.textWidth(preview) > maxW) {
      int l = strlen(preview); preview[l-1] = '\0';
      if (strlen(preview) > 3) {
        preview[strlen(preview)-1] = '.'; preview[strlen(preview)-2] = '.'; preview[strlen(preview)-3] = '.';
      }
    }
    lcd.drawString(preview, 4, footerY + 4);
  }
  lcd.fillRect(0, DISP_H - 18, DISP_W, 18, COL_GRAY_D);
  lcd.drawFastHLine(0, DISP_H - 18, DISP_W, COL_GRAY_3);
  lcd.setFont(&fonts::Font0); lcd.setTextSize(1);
  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("C=naik", 4, DISP_H - 14);
  lcd.drawString("D=turun", 60, DISP_H - 14);
  lcd.drawString("BOOT=pilih", 130, DISP_H - 14);
  lcd.drawString("B=batal", 240, DISP_H - 14);
}

// ─────────────────────────────────────────────────────────────────────────────
//  AI FEATURE MENU — OPEN & HANDLE
// ─────────────────────────────────────────────────────────────────────────────
void openAIFeatureMenu(bool fromViewfinder) {
  aiFromViewfinder = fromViewfinder;
  aiSelectedFeature = AI_FEAT_DESCRIBE;
  drawAIFeatureMenu((int)aiSelectedFeature, fromViewfinder);
  resetAllButtons();
  appMode = MODE_AI_FEATURE_MENU;
  neoBurst(0, 200, 200, 2);
}


bool captureForAI(char* outPath, int outPathLen) {
  snprintf(outPath, outPathLen, "/sdcard/ai_temp.jpg");
  if (ledFlashEnabled) {
    digitalWrite(LED_FLASH, HIGH); if (neoFlashEnabled) neoWhiteFlash(); delay(150);
    for (int i = 0; i < 2; i++) {
      camera_fb_t *tfb = esp_camera_fb_get();
      if (tfb) esp_camera_fb_return(tfb);
      esp_task_wdt_reset();
    }
  }
  camera_fb_t *fb = esp_camera_fb_get();
  if (ledFlashEnabled) digitalWrite(LED_FLASH, LOW);
  if (!fb) return false;
  neoBurst(0, 80, 255, 1);
  uint8_t *jpg_buf = nullptr; size_t jpg_len = 0; bool ok = false;
  if (fb->format == PIXFORMAT_RGB565) {
    if (eisEnabled) {
      uint16_t* src = (uint16_t*)fb->buf;
      uint16_t* tmp = (uint16_t*)ps_malloc(DISP_W * DISP_H * 2);
      if (!tmp) tmp = (uint16_t*)malloc(DISP_W * DISP_H * 2);
      if (tmp) {
        int sx = constrain((int)g_eisOffX, 0, DISP_W - EIS_VP_W); int sy = constrain((int)g_eisOffY, 0, DISP_H - EIS_VP_H);
        for (int y = 0; y < DISP_H; y++) {
          int srY = sy + (y * EIS_VP_H / DISP_H);
          for (int x = 0; x < DISP_W; x++) {
            int srX = sx + (x * EIS_VP_W / DISP_W);
            tmp[y * DISP_W + x] = src[srY * DISP_W + srX];
          }
        }
        camera_fb_t fk = *fb; fk.buf = (uint8_t*)tmp; fk.width = DISP_W; fk.height = DISP_H;
        int captureQ = hdCaptureEnabled ? map(hdCaptureQuality, 10, 1, 50, 95) : 85;
        ok = frame2jpg(&fk, captureQ, &jpg_buf, &jpg_len); free(tmp);
      }
    } else { int captureQ = hdCaptureEnabled ? map(hdCaptureQuality, 10, 1, 50, 95) : 85;
    ok = frame2jpg(fb, captureQ, &jpg_buf, &jpg_len); }
  } else if (fb->format == PIXFORMAT_JPEG) {
    jpg_buf = fb->buf; jpg_len = fb->len; ok = true;
  }
  bool saved = false;
  if (ok && jpg_buf && jpg_len > 0) {
    FILE* f = fopen(outPath, "wb");
    if (f) { size_t w = fwrite(jpg_buf, 1, jpg_len, f); fclose(f); saved = (w == jpg_len); }
    if (fb->format != PIXFORMAT_JPEG) free(jpg_buf);
  }
  esp_camera_fb_return(fb); esp_task_wdt_reset();
  return saved;
}

void handleModeAIFeatureMenu(ButtonEvent evtBoot, ButtonEvent evtB,
                              ButtonEvent evtC, ButtonEvent evtD) {
  bool redraw = false;
  int sel = (int)aiSelectedFeature;

  if (evtC.valid && evtC.isShort) {
    sel = (sel - 1 + AI_FEAT_COUNT) % AI_FEAT_COUNT;
    aiSelectedFeature = (AIFeature)sel;
    redraw = true;
  }
  if (evtD.valid && evtD.isShort) {
    sel = (sel + 1) % AI_FEAT_COUNT;
    aiSelectedFeature = (AIFeature)sel;
    redraw = true;
  }

  if (redraw) {
    drawAIFeatureMenu(sel, aiFromViewfinder);
    return;
  }

  if (evtB.valid && evtB.isShort) {
    resetAllButtons();
    if (aiFromViewfinder) {
      appMode = MODE_VIEWFINDER;
      islandNoClear = true;
      lcd.fillScreen(COL_BLACK);
    } else {
      appMode = MODE_PHOTO_VIEW;
      photoViewRender();
      photoViewDrawCaption(photoViewIndex);
      photoViewCaptionUntilMs = millis() + 2000;
      photoViewCaptionVisible = true;
    }
    return;
  }

  if (evtBoot.valid && evtBoot.isShort) {
    if (aiFromViewfinder) {
      if (!sdReady) {
        islandPush(NOTIF_WARN, "SD tidak tersedia");
        resetAllButtons();
        appMode = MODE_VIEWFINDER;
        islandNoClear = true;
        lcd.fillScreen(COL_BLACK);
        return;
      }

      lcd.fillScreen(COL_BLACK);
      lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
      const char* capMsg = "capturing...";
      lcd.drawString(capMsg, (DISP_W - lcd.textWidth(capMsg)) / 2, DISP_H / 2 - 4);

      char tempPath[48];
      bool capOK = captureForAI(tempPath, sizeof(tempPath));
      if (!capOK) {
        islandPush(NOTIF_WARN, "Gagal capture");
        resetAllButtons();
        appMode = MODE_VIEWFINDER;
        islandNoClear = true;
        lcd.fillScreen(COL_BLACK);
        return;
      }

      scanGalleryFiles();
      scanPhotoCount();

      int tempIdx = -1;
      for (int i = 0; i < galleryCount; i++) {
        if (strcmp(galleryFiles[i], "ai_temp.jpg") == 0) {
          tempIdx = i; break;
        }
      }
      if (tempIdx < 0) {
        islandPush(NOTIF_WARN, "File temp tidak ditemukan");
        resetAllButtons();
        appMode = MODE_VIEWFINDER;
        islandNoClear = true;
        lcd.fillScreen(COL_BLACK);
        return;
      }

      photoViewIndex = tempIdx;
      openAIDescribeWithFeature(tempIdx, aiSelectedFeature);

    } else {
      openAIDescribeWithFeature(photoViewIndex, aiSelectedFeature);
    }
    return;
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  KEY MANAGER — Draw screen
// ─────────────────────────────────────────────────────────────────────────────
void drawKeyManagerScreen() {
  lcd.fillScreen(COL_BLACK);
  lcd.fillRect(0,0,DISP_W,20,COL_GRAY_D);
  lcd.drawFastHLine(0,20,DISP_W,COL_GRAY_3);
  lcd.setFont(&fonts::Font0); lcd.setTextSize(1);
  lcd.setTextColor(COL_AI_ACCENT);
  const char* title = "GEMINI KEY MANAGER";
  lcd.drawString(title,(DISP_W-lcd.textWidth(title))/2,6);
  if (kmDirty) {
    lcd.setTextColor(COL_AI_WARN);
    lcd.drawString("*UNSAVED*", DISP_W-58, 6);
  }
  int y = 24;
  if (geminiKeyCount == 0) {
    lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("Tidak ada key.", 8, y+10); y+=24;
    lcd.setTextColor(COL_GRAY_3);
    lcd.drawString("Edit gemini.ini di PC", 8, y); y+=12;
    lcd.drawString("lalu restart kamera.", 8, y);
  } else {
    for (int i = 0; i < geminiKeyCount; i++) {
      bool isSelected = (i == kmSelIdx);
      bool isActive   = (i == geminiKeyActive);
      uint16_t rowBg = isSelected ? COL_GRAY_5 : (i%2==0 ? COL_GRAY_D : COL_BLACK);
      lcd.fillRect(0, y, DISP_W, 22, rowBg);
      char numBuf[4]; snprintf(numBuf, sizeof(numBuf), "k%d", i+1);
      lcd.setTextColor(isSelected ? COL_WHITE : COL_GRAY_5);
      lcd.drawString(numBuf, 4, y+7);
      char masked[32];
      maskApiKey(geminiApiKeys[i], masked, sizeof(masked));
      lcd.setTextColor(isSelected ? COL_WHITE : COL_GRAY_C);
      lcd.drawString(masked, 28, y+7);
      if (isActive) { lcd.setTextColor(COL_AI_ACCENT); lcd.drawString("*", DISP_W-16, y+7); }
      lcd.drawFastHLine(0, y+22, DISP_W, COL_GRAY_2);
      y += 23;
    }
  }
  y = 24 + geminiKeyCount * 23;
  if (geminiKeyCount < GEMINI_KEY_MAX) {
    lcd.setTextColor(COL_GRAY_3);
    char slotBuf[32];
    snprintf(slotBuf, sizeof(slotBuf), "+ %d slot kosong (edit via PC)", GEMINI_KEY_MAX - geminiKeyCount);
    lcd.drawString(slotBuf, 4, y+4);
  }
  lcd.drawFastHLine(0, DISP_H-40, DISP_W, COL_GRAY_2);
  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("C/D=pilih", 4, DISP_H-35);
  lcd.setTextColor(COL_GRAY_3);
  lcd.drawString("B=set aktif", 80, DISP_H-35);
  lcd.setTextColor(0xF800);
  lcd.drawString("BOOT-long=hapus", 180, DISP_H-35);
  lcd.drawFastHLine(0, DISP_H-22, DISP_W, COL_GRAY_2);
  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("BOOT=simpan&kembali", 4, DISP_H-18);
  lcd.setTextColor(COL_GRAY_3);
  lcd.drawString("D-long=batal", 190, DISP_H-18);
  lcd.setTextColor(COL_GRAY_2);
  lcd.drawString("* = key aktif terakhir", 4, DISP_H-8);
}

void kmDeleteKey(int idx) {
  if (idx < 0 || idx >= geminiKeyCount) return;
  for (int i = idx; i < geminiKeyCount-1; i++)
    strncpy(geminiApiKeys[i], geminiApiKeys[i+1], 79);
  memset(geminiApiKeys[geminiKeyCount-1], 0, 80);
  geminiKeyCount--;
  if (geminiKeyActive >= geminiKeyCount) geminiKeyActive = max(0, geminiKeyCount-1);
  if (kmSelIdx >= geminiKeyCount) kmSelIdx = max(0, geminiKeyCount-1);
  kmDirty = true;
}

void openKeyManager() {
  kmSelIdx = 0; kmDirty = false;
  drawKeyManagerScreen();
  resetAllButtons();
  appMode = MODE_KEY_MANAGER;
}

void handleModeKeyManager(ButtonEvent evtBoot, ButtonEvent evtB,
                           ButtonEvent evtC, ButtonEvent evtD) {
  bool redraw = false;
  if (evtC.valid && evtC.isShort) {
    if (geminiKeyCount > 0) { kmSelIdx = (kmSelIdx - 1 + geminiKeyCount) % geminiKeyCount; redraw = true; }
  }
  if (evtD.valid && evtD.isShort) {
    if (geminiKeyCount > 0) { kmSelIdx = (kmSelIdx + 1) % geminiKeyCount; redraw = true; }
  }
  if (evtB.valid && evtB.isShort) {
    if (geminiKeyCount > 0) {
      geminiKeyActive = kmSelIdx;
      char buf[28]; snprintf(buf, sizeof(buf), "Key %d diset aktif", kmSelIdx+1);
      islandPush(NOTIF_INFO, buf); redraw = true;
    }
  }
  if (evtBoot.valid && evtBoot.isLong) {
    if (geminiKeyCount > 0) {
      char delBuf[28]; snprintf(delBuf, sizeof(delBuf), "Key %d dihapus", kmSelIdx+1);
      kmDeleteKey(kmSelIdx);
      islandPush(NOTIF_WARN, delBuf); redraw = true;
    }
  }
  if (evtBoot.valid && evtBoot.isShort) {
    if (kmDirty) {
      bool ok = saveGeminiConfig();
      islandPush(ok ? NOTIF_OK : NOTIF_WARN, ok ? "Key disimpan ke SD" : "Gagal simpan key");
    }
    kmDirty = false; resetAllButtons();
    appMode = MODE_PHOTO_VIEW;
    photoViewRender();
    photoViewDrawCaption(photoViewIndex);
    photoViewCaptionUntilMs = millis() + 2000;
    photoViewCaptionVisible = true;
    return;
  }
  if (evtD.valid && evtD.isLong) {
    if (kmDirty) { loadGeminiConfig(); islandPush(NOTIF_INFO, "Perubahan dibatalkan"); }
    kmDirty = false; resetAllButtons();
    appMode = MODE_PHOTO_VIEW;
    photoViewRender();
    photoViewDrawCaption(photoViewIndex);
    photoViewCaptionUntilMs = millis() + 2000;
    photoViewCaptionVisible = true;
    return;
  }
  if (redraw) drawKeyManagerScreen();
}

// ─────────────────────────────────────────────────────────────────────────────
//  STEGANOGRAFI JPEG
// ─────────────────────────────────────────────────────────────────────────────
#define STEGO_PAYLOAD_LEN  32
#define STEGO_MAGIC        "SANZXCAM"
#define STEGO_VERSION      "v5.9"

void stegoMakePayload(char* out, int maxLen, int photoNum) {
  snprintf(out, maxLen, "%s|%04d|%s", STEGO_MAGIC, photoNum, STEGO_VERSION);
}

uint8_t* stegoEmbedToJpeg(const uint8_t* jpgIn, size_t jpgLen,
                           const char* payload, int payLen, size_t* outLen) {
  if (!jpgIn || jpgLen < 4 || !outLen) return nullptr;
  if (jpgIn[0] != 0xFF || jpgIn[1] != 0xD8) return nullptr;
  if (!payload || payLen <= 0 || payLen > 500) return nullptr;
  int comTotal = 4 + payLen;
  size_t newLen = jpgLen + (size_t)comTotal;
  uint8_t* out = (uint8_t*)ps_malloc(newLen);
  if (!out) { out = (uint8_t*)malloc(newLen); if (!out) return nullptr; }
  out[0]=0xFF; out[1]=0xD8; out[2]=0xFF; out[3]=0xFE;
  uint16_t comLen = (uint16_t)(2 + payLen);
  out[4]=(uint8_t)((comLen>>8)&0xFF); out[5]=(uint8_t)(comLen&0xFF);
  memcpy(out+6, payload, (size_t)payLen);
  memcpy(out+6+payLen, jpgIn+2, jpgLen-2);
  *outLen = newLen;
  return out;
}

bool stegoExtractFromJpeg(const uint8_t* jpgBuf, size_t jpgLen,
                           char* outPayload, int maxLen) {
  if (!jpgBuf || jpgLen < 6 || !outPayload || maxLen < 2) return false;
  if (jpgBuf[0] != 0xFF || jpgBuf[1] != 0xD8) return false;
  size_t pos = 2;
  while (pos + 3 < jpgLen) {
    if (jpgBuf[pos] != 0xFF) break;
    uint8_t marker = jpgBuf[pos+1];
    if (marker == 0xFF) { pos++; continue; }
    if (marker == 0xD8) { pos += 2; continue; }
    if (marker == 0xD9) break;
    if (marker >= 0xD0 && marker <= 0xD7) { pos += 2; continue; }
    if (pos + 3 >= jpgLen) break;
    uint16_t segLen = ((uint16_t)jpgBuf[pos+2]<<8)|jpgBuf[pos+3];
    if (segLen < 2) break;
    if (marker == 0xFE) {
      int dataLen = (int)segLen - 2;
      if (dataLen <= 0) { pos += 2 + segLen; continue; }
      int readLen = (dataLen < maxLen-1) ? dataLen : maxLen-1;
      memcpy(outPayload, jpgBuf+pos+4, (size_t)readLen);
      outPayload[readLen] = '\0';
      return (strncmp(outPayload, STEGO_MAGIC, strlen(STEGO_MAGIC)) == 0);
    }
    size_t skip = 2 + (size_t)segLen;
    if (pos + skip > jpgLen) break;
    pos += skip;
  }
  return false;
}

// ─────────────────────────────────────────────────────────────────────────────
//  STEGANOGRAFI BMP
// ─────────────────────────────────────────────────────────────────────────────
#define STEGO_BMP_MAX_PAYLOAD 64

bool stegoBmpExtract(const char* path, char* outPayload, int maxLen) {
  if (!path || !outPayload || maxLen < 2) return false;
  FILE* f = fopen(path, "rb"); if (!f) return false;
  uint8_t hdr[54];
  if (fread(hdr,1,54,f)!=54) { fclose(f); return false; }
  if (hdr[0]!='B'||hdr[1]!='M') { fclose(f); return false; }
  uint32_t dataOffset=(uint32_t)hdr[10]|((uint32_t)hdr[11]<<8)|((uint32_t)hdr[12]<<16)|((uint32_t)hdr[13]<<24);
  int32_t bmpW=(int32_t)((uint32_t)hdr[18]|((uint32_t)hdr[19]<<8)|((uint32_t)hdr[20]<<16)|((uint32_t)hdr[21]<<24));
  int32_t bmpH=(int32_t)((uint32_t)hdr[22]|((uint32_t)hdr[23]<<8)|((uint32_t)hdr[24]<<16)|((uint32_t)hdr[25]<<24));
  if (bmpH<0) bmpH=-bmpH;
  if (bmpW<=0||bmpH<=0) { fclose(f); return false; }
  uint16_t bpp=(uint16_t)hdr[28]|((uint16_t)hdr[29]<<8);
  if (bpp!=24) { fclose(f); return false; }
  fseek(f,(long)dataOffset,SEEK_SET);
  int rowSize=((bmpW*3+3)/4)*4;
  int totalPayloadBits=(maxLen-1)*8;
  memset(outPayload,0,maxLen);
  int bitIdx=0,charIdx=0; uint8_t curChar=0; bool found=false;
  uint8_t* rowBuf=(uint8_t*)malloc(rowSize);
  if (!rowBuf) { fclose(f); return false; }
  for (int row=bmpH-1;row>=0&&bitIdx<totalPayloadBits;row--) {
    fseek(f,(long)dataOffset+(long)row*rowSize,SEEK_SET);
    if (fread(rowBuf,1,rowSize,f)!=(size_t)rowSize) break;
    for (int x=0;x<bmpW&&bitIdx<totalPayloadBits;x++) {
      uint8_t bit=rowBuf[x*3+0]&0x01;
      int bitPos=7-(bitIdx%8);
      curChar|=(bit<<bitPos); bitIdx++;
      if (bitIdx%8==0) {
        outPayload[charIdx]=curChar;
        if (curChar=='\0') { found=true; break; }
        charIdx++; curChar=0;
        if (charIdx>=maxLen-1) { found=true; break; }
      }
    }
    if (found) break;
  }
  outPayload[maxLen-1]='\0';
  free(rowBuf); fclose(f);
  return (strncmp(outPayload,STEGO_MAGIC,strlen(STEGO_MAGIC))==0);
}

// ─────────────────────────────────────────────────────────────────────────────
//  EXIF INJECT
// ─────────────────────────────────────────────────────────────────────────────
static inline void exifL16(uint8_t* b,uint16_t v){b[0]=v&0xFF;b[1]=(v>>8)&0xFF;}
static inline void exifL32(uint8_t* b,uint32_t v){b[0]=v&0xFF;b[1]=(v>>8)&0xFF;b[2]=(v>>16)&0xFF;b[3]=(v>>24)&0xFF;}
static void exifEntry(uint8_t* b,uint16_t tag,uint16_t type,uint32_t count,uint32_t valOrOff){
  exifL16(b+0,tag);exifL16(b+2,type);exifL32(b+4,count);exifL32(b+8,valOrOff);}
static void exifMakeTimestamp(char* buf,size_t bufLen){
  unsigned long totalSec=millis()/1000;
  unsigned long sec=totalSec%60,min=(totalSec/60)%60,hour=(totalSec/3600)%24;
  snprintf(buf,bufLen,"2025:01:01 %02lu:%02lu:%02lu",hour,min,sec);}

uint8_t* exifInjectToJpeg(const uint8_t* jpgIn,size_t jpgLen,int photoNum,const char* sensorStr,size_t* outLen){
  if(!jpgIn||jpgLen<4||!outLen) return nullptr;
  if(jpgIn[0]!=0xFF||jpgIn[1]!=0xD8) return nullptr;
  char strDesc[16],strMake[12],strModel[12],strSoftware[12],strDt[24];
  snprintf(strDesc,sizeof(strDesc),"photo_%04d",photoNum);
  snprintf(strMake,sizeof(strMake),"SANZXCAM");
  snprintf(strModel,sizeof(strModel),"%s",sensorStr?sensorStr:"UNKNOWN");
  snprintf(strSoftware,sizeof(strSoftware),"v5.9-fix5");
  exifMakeTimestamp(strDt,sizeof(strDt));
  int lenDesc=strlen(strDesc)+1,lenMake=strlen(strMake)+1;
  int lenModel=strlen(strModel)+1,lenSoftware=strlen(strSoftware)+1,lenDt=20;
  const uint32_t offIFD0=8,nIFD0=5;
  const uint32_t offExifIFD=offIFD0+2+nIFD0*12+4,nExifIFD=2;
  const uint32_t offDataArea=offExifIFD+2+nExifIFD*12+4;
  uint32_t offDesc=offDataArea,offMake=offDesc+lenDesc;
  uint32_t offModel=offMake+lenMake,offSoftware=offModel+lenModel;
  uint32_t offDt=offSoftware+lenSoftware,tiffSize=offDt+lenDt;
  uint32_t app1DataSize=6+tiffSize;
  uint16_t app1Len=(uint16_t)(2+app1DataSize);
  size_t totalSize=2+2+2+app1DataSize+(jpgLen-2);
  uint8_t* out=(uint8_t*)ps_malloc(totalSize);
  if(!out){out=(uint8_t*)malloc(totalSize);if(!out)return nullptr;}
  size_t pos=0;
  out[pos++]=0xFF;out[pos++]=0xD8;out[pos++]=0xFF;out[pos++]=0xE1;
  out[pos++]=(app1Len>>8)&0xFF;out[pos++]=app1Len&0xFF;
  out[pos++]='E';out[pos++]='x';out[pos++]='i';out[pos++]='f';
  out[pos++]=0x00;out[pos++]=0x00;
  uint8_t* tiff=out+pos;size_t tp=0;
  tiff[tp++]='I';tiff[tp++]='I';tiff[tp++]=0x2A;tiff[tp++]=0x00;
  exifL32(tiff+tp,offIFD0);tp+=4;
  exifL16(tiff+tp,(uint16_t)nIFD0);tp+=2;
  exifEntry(tiff+tp,0x010E,2,(uint32_t)lenDesc,offDesc);tp+=12;
  exifEntry(tiff+tp,0x010F,2,(uint32_t)lenMake,offMake);tp+=12;
  exifEntry(tiff+tp,0x0110,2,(uint32_t)lenModel,offModel);tp+=12;
  exifEntry(tiff+tp,0x0131,2,(uint32_t)lenSoftware,offSoftware);tp+=12;
  exifEntry(tiff+tp,0x8769,4,1,offExifIFD);tp+=12;
  exifL32(tiff+tp,0);tp+=4;
  exifL16(tiff+tp,(uint16_t)nExifIFD);tp+=2;
  exifEntry(tiff+tp,0x9000,7,4,0x30323230);tp+=12;
  exifEntry(tiff+tp,0x9003,2,(uint32_t)lenDt,offDt);tp+=12;
  exifL32(tiff+tp,0);tp+=4;
  memcpy(tiff+tp,strDesc,(size_t)lenDesc);tp+=lenDesc;
  memcpy(tiff+tp,strMake,(size_t)lenMake);tp+=lenMake;
  memcpy(tiff+tp,strModel,(size_t)lenModel);tp+=lenModel;
  memcpy(tiff+tp,strSoftware,(size_t)lenSoftware);tp+=lenSoftware;
  memset(tiff+tp,0,(size_t)lenDt);
  memcpy(tiff+tp,strDt,min((int)strlen(strDt),lenDt-1));tp+=lenDt;
  pos+=tp;
  memcpy(out+pos,jpgIn+2,jpgLen-2);pos+=jpgLen-2;
  *outLen=pos;
  return out;
}

// ─────────────────────────────────────────────────────────────────────────────
//  BMP SAVE
// ─────────────────────────────────────────────────────────────────────────────
bool saveBMP(const uint8_t* rgb565Buf,int w,int h,const char* path,
             const char* stegoPayload=nullptr,int stegoPayLen=0){
  FILE* f=fopen(path,"wb"); if(!f) return false;
  int rowSize=((w*3+3)/4)*4,dataSize=rowSize*h,fileSize=54+dataSize;
  uint8_t hdr[54]; memset(hdr,0,sizeof(hdr));
  hdr[0]='B';hdr[1]='M';
  hdr[2]=fileSize&0xFF;hdr[3]=(fileSize>>8)&0xFF;hdr[4]=(fileSize>>16)&0xFF;hdr[5]=(fileSize>>24)&0xFF;
  hdr[10]=54;hdr[14]=40;
  hdr[18]=w&0xFF;hdr[19]=(w>>8)&0xFF;hdr[20]=(w>>16)&0xFF;hdr[21]=(w>>24)&0xFF;
  hdr[22]=h&0xFF;hdr[23]=(h>>8)&0xFF;hdr[24]=(h>>16)&0xFF;hdr[25]=(h>>24)&0xFF;
  hdr[26]=1;hdr[28]=24;
  hdr[34]=dataSize&0xFF;hdr[35]=(dataSize>>8)&0xFF;hdr[36]=(dataSize>>16)&0xFF;hdr[37]=(dataSize>>24)&0xFF;
  hdr[38]=0x13;hdr[39]=0x0B;hdr[42]=0x13;hdr[43]=0x0B;
  fwrite(hdr,1,54,f);
  int totalStegoBits=(stegoPayload&&stegoPayLen>0)?stegoPayLen*8:0,stegoPixelIdx=0;
  static uint8_t rowBuf[320*3+4];
  for(int y=h-1;y>=0;y--){
    memset(rowBuf,0,rowSize);
    const uint16_t* srcRow=(const uint16_t*)rgb565Buf+(size_t)y*w;
    for(int x=0;x<w;x++){
      uint16_t raw=srcRow[x],px=(raw<<8)|(raw>>8);
      uint8_t r=((px>>11)&0x1F)<<3,g=((px>>5)&0x3F)<<2,b=(px&0x1F)<<3;
      if(stegoPixelIdx<totalStegoBits){
        int charIdx=stegoPixelIdx/8,bitPos=7-(stegoPixelIdx%8);
        uint8_t bit=((uint8_t)stegoPayload[charIdx]>>bitPos)&0x01;
        b=(b&0xFE)|bit; stegoPixelIdx++;
      }
      rowBuf[x*3+0]=b;rowBuf[x*3+1]=g;rowBuf[x*3+2]=r;
    }
    fwrite(rowBuf,1,rowSize,f);
    if(y%30==0) esp_task_wdt_reset();
  }
  fclose(f); return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  BMP LOAD
// ─────────────────────────────────────────────────────────────────────────────
uint16_t* loadBMP(const char* path,uint16_t* outW,uint16_t* outH){
  FILE* f=fopen(path,"rb"); if(!f) return nullptr;
  uint8_t hdr[54];
  if(fread(hdr,1,54,f)!=54){fclose(f);return nullptr;}
  if(hdr[0]!='B'||hdr[1]!='M'){fclose(f);return nullptr;}
  uint32_t dataOffset=(uint32_t)hdr[10]|((uint32_t)hdr[11]<<8)|((uint32_t)hdr[12]<<16)|((uint32_t)hdr[13]<<24);
  int32_t bmpW=(int32_t)((uint32_t)hdr[18]|((uint32_t)hdr[19]<<8)|((uint32_t)hdr[20]<<16)|((uint32_t)hdr[21]<<24));
  int32_t bmpH=(int32_t)((uint32_t)hdr[22]|((uint32_t)hdr[23]<<8)|((uint32_t)hdr[24]<<16)|((uint32_t)hdr[25]<<24));
  bool topDown=false; if(bmpH<0){topDown=true;bmpH=-bmpH;}
  uint16_t bpp=(uint16_t)hdr[28]|((uint16_t)hdr[29]<<8);
  if(bpp!=24){fclose(f);return nullptr;}
  if(bmpW<=0||bmpH<=0||bmpW>4096||bmpH>4096){fclose(f);return nullptr;}
  size_t pixCount=(size_t)bmpW*(size_t)bmpH;
  uint16_t* buf=(uint16_t*)ps_malloc(pixCount*sizeof(uint16_t));
  if(!buf) buf=(uint16_t*)malloc(pixCount*sizeof(uint16_t));
  if(!buf){fclose(f);return nullptr;}
  int rowSize=((bmpW*3+3)/4)*4;
  uint8_t* rowBuf=(uint8_t*)malloc(rowSize);
  if(!rowBuf){free(buf);fclose(f);return nullptr;}
  fseek(f,(long)dataOffset,SEEK_SET);
  for(int row=0;row<bmpH;row++){
    if(fread(rowBuf,1,rowSize,f)!=(size_t)rowSize) memset(rowBuf,0,rowSize);
    int destRow=topDown?row:(bmpH-1-row);
    uint16_t* destLine=buf+(size_t)destRow*bmpW;
    for(int x=0;x<bmpW;x++){
      uint8_t b=rowBuf[x*3+0],g=rowBuf[x*3+1],r=rowBuf[x*3+2];
      uint16_t px_le=((uint16_t)(r&0xF8)<<8)|((uint16_t)(g&0xFC)<<3)|((uint16_t)(b>>3));
      destLine[x]=(px_le<<8)|(px_le>>8);
    }
    if(row%30==0) esp_task_wdt_reset();
  }
  free(rowBuf);fclose(f);
  *outW=(uint16_t)bmpW;*outH=(uint16_t)bmpH;
  return buf;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Gallery remaining vars
// ─────────────────────────────────────────────────────────────────────────────
unsigned long galleryHoldStart = 0;
unsigned long galleryLastStep  = 0;
int           galleryHoldDir   = 0;

#define ZOOM_LEVELS 3
float    photoZoomFactors[ZOOM_LEVELS] = { 1.0f, 2.0f, 4.0f };
uint8_t  photoZoomLevel = 0;
int      photoZoomOffX  = 0;
int      photoZoomOffY  = 0;
uint16_t* photoPixelBuf = nullptr;
uint16_t  photoBufW     = 0;
uint16_t  photoBufH     = 0;

bool          recActive     = false;

FILE*         recFile       = nullptr;
int           recFrameCount = 0;
int           recVideoCount = 0;
unsigned long recStartMs    = 0;


const char* expPresetNames[6] = { "AUTO", "GRAY", "MOON", "NIGHT", "NIGHT-BW", "MANUAL" };

struct ExpPresetCfg {
  bool aec_on; bool aec2_on; int aec_val;
  bool agc_on; int agc_gain; int gainceiling; int ae_level;
  int special_effect;
};
const ExpPresetCfg expPresets[6] = {
  { true,  true,  300,  true,  0, 2,  0, 0 },
  { true,  true,  300,  true,  0, 2,  0, 2 },
  { false, false,  20,  false, 0, 0, -2, 0 },
  { false, true,  1200, false, 5, 6,  2, 0 },
  { false, true,  1200, false, 5, 6,  2, 2 },
  { false, false, 300,  false, 0, 0,  0, 0 },
};
int menuLedSel    = 0;
int menuExpSel    = 0;
int menuFormatSel = 0;

unsigned long deleteDialogOpenMs = 0;
#define DELETE_TIMEOUT_MS 8000

USBMSC msc;
bool   usbModeActive = false;

unsigned long fpsLastTime   = 0;
int           fpsFrameCount = 0;
float         fpsValue      = 0.0f;

#define BRACKET_LEN 10
#define PILL_H      13
#define PILL_R       6
#define PILL_PAD_X   5

// ─────────────────────────────────────────────────────────────────────────────
//  JUMP-TO-NUMBER STATE
// ─────────────────────────────────────────────────────────────────────────────
#define JUMP_TIMEOUT_MS  10000
#define JUMP_DIGIT_COUNT 4

static bool          jumpActive       = false;
static int           jumpDigits[JUMP_DIGIT_COUNT] = {0,0,0,0};
static int           jumpCursorPos    = 0;
static unsigned long jumpOpenMs       = 0;

static int jumpGetValue() {
  return jumpDigits[0]*1000+jumpDigits[1]*100+jumpDigits[2]*10+jumpDigits[3];
}
static void jumpResetToCurrentIndex() {
  int v=constrain(gallerySelIdx+1,0,9999);
  jumpDigits[0]=(v/1000)%10; jumpDigits[1]=(v/100)%10;
  jumpDigits[2]=(v/10)%10;   jumpDigits[3]=v%10;
  jumpCursorPos=3;
}

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
    if(!f) return 0;
    long cur=ftell(f); fseek(f,0,SEEK_END);
    long end=ftell(f); fseek(f,cur,SEEK_SET);
    return (int)(end-cur);
  }
  int read() override { if(!f) return -1; uint8_t b; return(fread(&b,1,1,f)==1)?b:-1; }
  int peek() override { if(!f) return -1; int c=fgetc(f); if(c!=EOF) ungetc(c,f); return c; }
  size_t readBytes(uint8_t* buf,size_t len) override { if(!f) return 0; return fread(buf,1,len,f); }
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

bool    mjpegLoop     = false;
float    mjpegActualFps = 15.0f;
uint8_t mjpegSpeedIdx = 1;
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

bool tjpgdecOutput(int16_t x,int16_t y,uint16_t w,uint16_t h,uint16_t* bitmap){
  if(_decodeTargetBuf){
    for(int row=0;row<h;row++){
      int destY=y+row; if(destY<0||destY>=DISP_H) continue;
      memcpy(_decodeTargetBuf+destY*_decodeTargetW+x,bitmap+row*w,w*sizeof(uint16_t));
    }
  } else { lcd.pushImage(x,y,w,h,bitmap); }
  return 1;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helper
// ─────────────────────────────────────────────────────────────────────────────
static void blockingWaitAllRelease(uint32_t timeoutMs=500){
  unsigned long t0=millis();
  while(millis()-t0<timeoutMs){
    bool anyLow=(digitalRead(BTN_BOOT)==LOW||digitalRead(BTN_B)==LOW||
                 digitalRead(BTN_C)==LOW||digitalRead(BTN_D)==LOW);
    if(!anyLow) break;
    delay(10); esp_task_wdt_reset();
  }
  delay(50);
}

void drawCornerBrackets(uint16_t col=COL_WHITE){
  int x0=3,y0=3,x1=DISP_W-4,y1=DISP_H-4,L=BRACKET_LEN;
  lcd.drawFastHLine(x0,y0,L,col);lcd.drawFastVLine(x0,y0,L,col);
  lcd.drawFastHLine(x1-L+1,y0,L,col);lcd.drawFastVLine(x1,y0,L,col);
  lcd.drawFastHLine(x0,y1,L,col);lcd.drawFastVLine(x0,y1-L+1,L,col);
  lcd.drawFastHLine(x1-L+1,y1,L,col);lcd.drawFastVLine(x1,y1-L+1,L,col);
}

void drawPill(int cx,int cy,const char* text,uint16_t bgCol,uint16_t fgCol){
  lcd.setFont(&fonts::Font0);lcd.setTextSize(1);
  int tw=lcd.textWidth(text),pw=tw+PILL_PAD_X*2;
  int px=cx-pw/2,py=cy-PILL_H/2;
  lcd.fillRoundRect(px,py,pw,PILL_H,PILL_R,bgCol);
  lcd.drawRoundRect(px,py,pw,PILL_H,PILL_R,COL_GRAY_3);
  lcd.setTextColor(fgCol); lcd.drawString(text,px+PILL_PAD_X,py+2);
}

// ─────────────────────────────────────────────────────────────────────────────
//  SD Mount / Unmount
// ─────────────────────────────────────────────────────────────────────────────
bool mountSDFull(){
  sdmmc_host_t host=SDMMC_HOST_DEFAULT();
  host.max_freq_khz=SDMMC_FREQ_DEFAULT;
  sdmmc_slot_config_t slot=SDMMC_SLOT_CONFIG_DEFAULT();
  slot.clk=(gpio_num_t)SD_MMC_CLK_PIN;slot.cmd=(gpio_num_t)SD_MMC_CMD_PIN;
  slot.d0=(gpio_num_t)SD_MMC_D0_PIN;slot.width=1;
  slot.flags|=SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
  esp_vfs_fat_sdmmc_mount_config_t mountCfg={
    .format_if_mount_failed=false,.max_files=5,.allocation_unit_size=16*1024};
  esp_err_t ret=esp_vfs_fat_sdmmc_mount("/sdcard",&host,&slot,&mountCfg,&sdCard);
  if(ret!=ESP_OK){sdCard=nullptr;sdTotalSectors=0;sdmmcDriverInit=false;sdSizeMB=0;return false;}
  sdTotalSectors=sdCard->csd.capacity;sdmmcDriverInit=true;
  sdSizeMB=(uint64_t)sdTotalSectors*sdCard->csd.sector_size/(1024*1024);
  return true;
}

bool remountVFSOnly(){
  if(!sdCard||!sdmmcDriverInit) return mountSDFull();
  ff_diskio_register_sdmmc(0,sdCard);
  static FATFS fatfs; char drv[3]={'0',':',0};
  FRESULT fr=f_mount(&fatfs,drv,1); if(fr!=FR_OK) return false;
  FATFS* fatfsPtr=&fatfs;
  esp_err_t err=esp_vfs_fat_register("/sdcard","",5,&fatfsPtr);
  if(err!=ESP_OK&&err!=ESP_ERR_INVALID_STATE) return false;
  return true;
}

void unmountVFSOnly(){
  esp_vfs_fat_unregister_path("/sdcard");
  ff_diskio_register_sdmmc(0,NULL);
}

// ─────────────────────────────────────────────────────────────────────────────
//  scanPhotoCount / scanVideoCount / scanGalleryFiles
// ─────────────────────────────────────────────────────────────────────────────
void scanPhotoCount(){
  photoCount=0;
  DIR* dir=opendir("/sdcard"); if(!dir) return;
  struct dirent* entry;
  while((entry=readdir(dir))!=nullptr){
    String name=entry->d_name;
    bool isJpg=name.startsWith("photo_")&&name.endsWith(".jpg");
    bool isBmp=name.startsWith("photo_")&&name.endsWith(".bmp");
    if(isJpg||isBmp){
      int num=name.substring(6,name.length()-4).toInt();
      if(num>photoCount) photoCount=num;
    }
  }
  closedir(dir);
}

void scanVideoCount(){
  recVideoCount=0;
  DIR* dir=opendir("/sdcard"); if(!dir) return;
  struct dirent* entry;
  while((entry=readdir(dir))!=nullptr){
    String name=entry->d_name;
    if(name.startsWith("video_")&&name.endsWith(".mjpeg")){
      int num=name.substring(6,name.length()-6).toInt();
      if(num>recVideoCount) recVideoCount=num;
    }
  }
  closedir(dir);
}

void scanGalleryFiles(){
  galleryCount=0;galleryScroll=0;
  DIR* dir=opendir("/sdcard"); if(!dir) return;
  struct dirent* entry;
  while((entry=readdir(dir))!=nullptr&&galleryCount<GALLERY_MAX_FILES){
    if (strncmp(entry->d_name, ".", 1) == 0) continue;
    String name=entry->d_name;
    bool isJpg=name.endsWith(".jpg")||name.endsWith(".JPG");
    bool isBmp=name.endsWith(".bmp")||name.endsWith(".BMP");
    bool isMjpeg=name.endsWith(".mjpeg")||name.endsWith(".MJPEG");
    if(isJpg||isBmp||isMjpeg){
      strncpy(galleryFiles[galleryCount],entry->d_name,31);
      galleryFiles[galleryCount][31]='\0';
      if(isMjpeg)    galleryFileType[galleryCount]=GFILE_VIDEO;
      else if(isBmp) galleryFileType[galleryCount]=GFILE_BMP;
      else           galleryFileType[galleryCount]=GFILE_JPG;
      galleryCount++;
    }
  }
  if (sdReady) thumbCacheEnsureDir();
  closedir(dir);
  for(int i=1;i<galleryCount;i++){
    char tmpN[32]; strncpy(tmpN,galleryFiles[i],31); tmpN[31]='\0';
    GalleryFileType tmpT=galleryFileType[i];
    int j=i-1;
    while(j>=0&&strcmp(galleryFiles[j],tmpN)>0){
      strncpy(galleryFiles[j+1],galleryFiles[j],31);
      galleryFileType[j+1]=galleryFileType[j]; j--;
      if(j%20==0) esp_task_wdt_reset();
    }
    strncpy(galleryFiles[j+1],tmpN,31); galleryFileType[j+1]=tmpT;
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// UI UPDATE v6.0


void thumbCacheEnsureDir() {
  struct stat st;
  if (stat("/sdcard/.cache", &st) != 0) {
    mkdir("/sdcard/.cache", 0755);
  }
}

void thumbCachePath(const char* galleryFilename, char* outPath, int outLen) {
  char base[32];
  strncpy(base, galleryFilename, sizeof(base)-1);
  base[sizeof(base)-1] = '\0';
  char* dot = strrchr(base, '.');
  if (dot) *dot = '\0';
  snprintf(outPath, outLen, "/sdcard/.cache/%s.bin", base);
}

bool thumbCacheLoad(const char* galleryFilename, uint16_t* outBuf) {
  char path[64];
  thumbCachePath(galleryFilename, path, sizeof(path));
  FILE* f = fopen(path, "rb");
  if (!f) return false;
  size_t bytesRead = fread(outBuf, 1, 72*42*2, f);
  fclose(f);
  return (bytesRead == 72*42*2);
}

void thumbCacheSave(const char* galleryFilename, const uint16_t* buf) {
  char path[64];
  thumbCachePath(galleryFilename, path, sizeof(path));
  FILE* f = fopen(path, "wb");
  if (!f) return;
  fwrite(buf, 1, 72*42*2, f);
  fclose(f);
}

bool thumbDecodeJpeg(const char* galleryFilename, uint16_t* outBuf) {
  char path[64];
  snprintf(path, sizeof(path), "/sdcard/%s", galleryFilename);
  FILE* f = fopen(path, "rb");
  if (!f) return false;
  fseek(f, 0, SEEK_END);
  size_t fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (fsize == 0 || fsize > 250000) { fclose(f); return false; }
  uint8_t* buf = (uint8_t*)ps_malloc(fsize);
  if (!buf) buf = (uint8_t*)malloc(fsize);
  if (!buf) { fclose(f); return false; }
  if (fread(buf, 1, fsize, f) != fsize) { free(buf); fclose(f); return false; }
  fclose(f);

  uint16_t* tmpBuf = (uint16_t*)ps_malloc(80*60*2);
  if (!tmpBuf) tmpBuf = (uint16_t*)malloc(80*60*2);
  if (!tmpBuf) { free(buf); return false; }
  memset(tmpBuf, 0, 80*60*2);

  TJpgDec.setJpgScale(4);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tjpgdecOutput);
  _decodeTargetBuf = tmpBuf;
  _decodeTargetW   = 80;
  TJpgDec.drawJpg(0, 0, buf, fsize);
  _decodeTargetBuf = nullptr;
  free(buf);

  for (int dy = 0; dy < 42; dy++) {
    int sy = dy * 60 / 42;
    for (int dx = 0; dx < 72; dx++) {
      int sx = dx * 80 / 72;
      outBuf[dy * 72 + dx] = tmpBuf[sy * 80 + sx];
    }
  }
  free(tmpBuf);
  esp_task_wdt_reset();
  return true;
}

void drawGalleryGrid() {
  lcd.fillScreen(COL_BLACK);
  lcd.fillRect(0, 0, DISP_W, 20, COL_GRAY_D);
  lcd.drawFastHLine(0, 20, DISP_W, COL_GRAY_3);
  lcd.setFont(&fonts::Font0);
  lcd.setTextColor(COL_GRAY_E);

  int currentPage = galleryScroll / 16 + 1;
  int totalPage = max(1, (galleryCount + 15) / 16);
  char hdr[32];
  snprintf(hdr, sizeof(hdr), "GALLERY  %d item", galleryCount);
  lcd.drawString(hdr, (DISP_W - lcd.textWidth(hdr)) / 2, 6);
  lcd.setTextColor(COL_GRAY_5); lcd.drawString("B=BACK", DISP_W - 46, 6);
  char pageInfo[8]; snprintf(pageInfo, sizeof(pageInfo), "%d/%d", currentPage, totalPage);
  lcd.setTextColor(COL_GRAY_3); lcd.drawString(pageInfo, 4, 6);

  if (galleryCount == 0) {
    lcd.setTextColor(COL_GRAY_5);
    const char* msg = "SD kosong / tidak ada file";
    lcd.drawString(msg, (DISP_W - lcd.textWidth(msg)) / 2, DISP_H / 2); return;
  }

  static uint16_t thumbBuf[72 * 42];
  int startIdx = (galleryScroll / 16) * 16;
  for (int i = 0; i < 16; i++) {
    int idx = startIdx + i;
    if (idx >= galleryCount) break;
    int row = i / 4, col = i % 4;
    int x = 4 + col * (76 + 2);
    int y = 22 + row * (56 + 2);

    bool isSelected = (idx == gallerySelIdx);
    lcd.fillRect(x, y, 76, 56, isSelected ? COL_GRAY_5 : COL_GRAY_D);
    lcd.drawRect(x, y, 76, 56, isSelected ? COL_WHITE : COL_GRAY_3);

    GalleryFileType ft = galleryFileType[idx];
    if (ft == GFILE_JPG) {
      bool ok = false;
      if (thumbCacheLoad(galleryFiles[idx], thumbBuf)) {
        lcd.setClipRect(x + 2, y + 2, 72, 42);
        lcd.pushImage(x + 2, y + 2, 72, 42, thumbBuf);
        lcd.clearClipRect();
        ok = true;
      } else {
        if (thumbDecodeJpeg(galleryFiles[idx], thumbBuf)) {
          thumbCacheSave(galleryFiles[idx], thumbBuf);
          lcd.setClipRect(x + 2, y + 2, 72, 42);
          lcd.pushImage(x + 2, y + 2, 72, 42, thumbBuf);
          lcd.clearClipRect();
          ok = true;
        }
      }
      if (!ok) {
        lcd.setTextColor(COL_GRAY_7);
        lcd.drawString("JPG", x + 28, y + 20);
      }
    } else if (ft == GFILE_BMP) {
      for (int r = 0; r < 42; r++) {
        uint16_t gCol = lcd.color565(0, 0, 20 + r / 2);
        lcd.drawFastHLine(x + 2, y + 2 + r, 72, gCol);
      }
      lcd.setTextColor(COL_BMP_ACCENT);
      lcd.drawString("BMP", x + (76 - lcd.textWidth("BMP")) / 2, y + 18);
    } else if (ft == GFILE_VIDEO) {
      lcd.fillRect(x + 2, y + 2, 72, 42, COL_GRAY_2);
      int cx = x + 38, cy = y + 21;
      lcd.fillTriangle(cx - 8, cy - 8, cx - 8, cy + 8, cx + 10, cy, COL_GRAY_7);
      char vidNum[12] = ""; int vn = 0;
      for (int j = 0; galleryFiles[idx][j] && vn < 11; j++)
        if (isdigit(galleryFiles[idx][j])) vidNum[vn++] = galleryFiles[idx][j];
      vidNum[vn] = '\0';
      lcd.setTextColor(COL_VID_ACCENT);
      lcd.drawString(vidNum, x + (76 - lcd.textWidth(vidNum)) / 2, y + 34);
    }
    lcd.setTextColor(COL_GRAY_3);
    char idxBuf[8]; snprintf(idxBuf, sizeof(idxBuf), "%d", idx + 1);
    lcd.drawString(idxBuf, x + 4, y + 46);
  }

  TJpgDec.setJpgScale(1);
  if (galleryCount > 16) {
    int barH = DISP_H - 22, indH = max(10, barH * 16 / galleryCount);
    int indY = 22 + (barH - indH) * (galleryScroll / 16) / max(1, (galleryCount - 1) / 16);
    lcd.fillRect(DISP_W - 4, 22, 4, barH, COL_GRAY_2);
    lcd.fillRect(DISP_W - 4, indY, 4, indH, COL_GRAY_7);
  }

  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_2);
  const char* footer = "B-long=list  BOOT=buka  D-long=AI";
  lcd.drawString(footer, (DISP_W - lcd.textWidth(footer)) / 2, DISP_H - 10);
}

//  Gallery draw functions
// ─────────────────────────────────────────────────────────────────────────────
// UI UPDATE v6.0
void drawGallery(){
  lcd.fillScreen(COL_BLACK);
  lcd.fillRect(0,0,DISP_W,20,COL_GRAY_D);
  lcd.drawFastHLine(0,20,DISP_W,COL_GRAY_3);
  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_E);
  int currentPage=galleryScroll/GALLERY_ITEMS_PAGE+1;
  int totalPage=max(1,(galleryCount+GALLERY_ITEMS_PAGE-1)/GALLERY_ITEMS_PAGE);
  char hdr[32]; snprintf(hdr,sizeof(hdr),"GALLERY  %d item",galleryCount);
  lcd.drawString(hdr,(DISP_W-lcd.textWidth(hdr))/2,6);
  lcd.setTextColor(COL_GRAY_5); lcd.drawString("B=BACK",DISP_W-46,6);
  char pageInfo[8]; snprintf(pageInfo,sizeof(pageInfo),"%d/%d",currentPage,totalPage);
  lcd.setTextColor(COL_GRAY_3); lcd.drawString(pageInfo,4,6);
  if(galleryCount==0){
    lcd.setTextColor(COL_GRAY_5);
    const char* msg="SD kosong / tidak ada file";
    lcd.drawString(msg,(DISP_W-lcd.textWidth(msg))/2,DISP_H/2); return;
  }
  int visibleEnd=min(galleryScroll+GALLERY_ITEMS_PAGE,galleryCount);
  for(int i=galleryScroll;i<visibleEnd;i++){
    int rowY=24+(i-galleryScroll)*GALLERY_ITEM_H;
    uint16_t rowBg=(i%2==0)?COL_GRAY_D:COL_BLACK;
    lcd.fillRect(0,rowY,DISP_W,GALLERY_ITEM_H-1,rowBg);
    lcd.drawFastHLine(0,rowY+GALLERY_ITEM_H-1,DISP_W,COL_GRAY_2);
    lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
    char num[6]; snprintf(num,sizeof(num),"%3d",i+1); lcd.drawString(num,6,rowY+8);
    GalleryFileType ft=galleryFileType[i];
    if(ft==GFILE_VIDEO){ lcd.setTextColor(COL_VID_ACCENT); lcd.drawString("\x10",26,rowY+8); }
    else if(ft==GFILE_BMP){ lcd.drawRect(26,rowY+5,9,9,COL_BMP_ACCENT); lcd.setTextColor(COL_BMP_ACCENT); lcd.drawString("B",28,rowY+6); }
    else { lcd.drawRect(26,rowY+6,7,7,COL_GRAY_5); }
    uint16_t nameCol=(ft==GFILE_VIDEO)?COL_GRAY_A:COL_GRAY_C;
    lcd.setTextColor(nameCol); lcd.drawString(galleryFiles[i],38,rowY+8);
    const char* typeLabel=(ft==GFILE_VIDEO)?"VID":(ft==GFILE_BMP)?"BMP":"JPG";
    uint16_t typeCol=(ft==GFILE_VIDEO)?COL_VID_ACCENT:(ft==GFILE_BMP)?COL_BMP_ACCENT:COL_JPG_ACCENT;
    lcd.setTextColor(typeCol); lcd.drawString(typeLabel,DISP_W-28,rowY+8);
  }
  int selY=24+(gallerySelIdx-galleryScroll)*GALLERY_ITEM_H;
  lcd.drawRect(0,selY,DISP_W-4,GALLERY_ITEM_H-1,COL_GRAY_5);
  if(galleryCount>GALLERY_ITEMS_PAGE){
    int barH=DISP_H-22,indH=max(10,barH*GALLERY_ITEMS_PAGE/galleryCount);
    int indY=22+(barH-indH)*galleryScroll/max(1,galleryCount-GALLERY_ITEMS_PAGE);
    lcd.fillRect(DISP_W-4,22,4,barH,COL_GRAY_2); lcd.fillRect(DISP_W-4,indY,4,indH,COL_GRAY_7);
  }
  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_2);
  lcd.drawString("B-long=grid  D-long=AI",(DISP_W-lcd.textWidth("B-long=grid  D-long=AI"))/2,DISP_H-10);
}

void galleryUpdateHighlight(int oldSel,int newSel){
  int oldRow=oldSel-galleryScroll;
  if(oldRow>=0&&oldRow<GALLERY_ITEMS_PAGE){
    int y=24+oldRow*GALLERY_ITEM_H;
    lcd.drawRect(0,y,DISP_W-4,GALLERY_ITEM_H-1,(oldSel%2==0)?COL_GRAY_D:COL_BLACK);
  }
  int newRow=newSel-galleryScroll;
  if(newRow>=0&&newRow<GALLERY_ITEMS_PAGE){
    int y=24+newRow*GALLERY_ITEM_H;
    lcd.drawRect(0,y,DISP_W-4,GALLERY_ITEM_H-1,COL_GRAY_5);
  }
}

void galleryGridUpdateHighlight(int oldSel, int newSel) {
  int startIdx = (galleryScroll / 16) * 16;

  // erase old highlight
  int oldI = oldSel - startIdx;
  if (oldI >= 0 && oldI < 16) {
    int row = oldI / 4, col = oldI % 4;
    int x = 4 + col * 78;
    int y = 22 + row * 58;
    lcd.drawRect(x, y, 76, 56, COL_GRAY_3);
  }

  // draw new highlight
  int newI = newSel - startIdx;
  if (newI >= 0 && newI < 16) {
    int row = newI / 4, col = newI % 4;
    int x = 4 + col * 78;
    int y = 22 + row * 58;
    lcd.drawRect(x, y, 76, 56, COL_WHITE);
  }
}

void galleryStep(int delta){
  if(galleryCount<=0) return;
  int oldSel=gallerySelIdx;
  gallerySelIdx=(gallerySelIdx+delta%galleryCount+galleryCount)%galleryCount;
  if(gallerySelIdx==oldSel) return;

  int pageSize = galleryGridMode ? 16 : GALLERY_ITEMS_PAGE;
  bool needRedraw = false;

  if(gallerySelIdx < galleryScroll) {
    galleryScroll = (gallerySelIdx / pageSize) * pageSize;
    needRedraw = true;
  } else if(gallerySelIdx >= galleryScroll + pageSize) {
    galleryScroll = (gallerySelIdx / pageSize) * pageSize;
    needRedraw = true;
  }

  if(needRedraw) {
    if(galleryGridMode) drawGalleryGrid();
    else drawGallery();
  } else {
    if(galleryGridMode) galleryGridUpdateHighlight(oldSel, gallerySelIdx);
    else galleryUpdateHighlight(oldSel, gallerySelIdx);
  }
}

bool photoLoadPixelBuf(int idx){
  if(idx<0||idx>=galleryCount||gIsVideo(idx)) return false;
  if(photoPixelBuf){free(photoPixelBuf);photoPixelBuf=nullptr;}
  photoBufW=0;photoBufH=0;
  char path[56]; snprintf(path,sizeof(path),"/sdcard/%s",galleryFiles[idx]);
  if(gIsBmp(idx)){
    uint16_t bw=0,bh=0; uint16_t* buf=loadBMP(path,&bw,&bh);
    if(!buf||bw==0||bh==0) return false;
    photoPixelBuf=buf;photoBufW=bw;photoBufH=bh; return true;
  } else {
    FILE* f=fopen(path,"rb"); if(!f) return false;
    fseek(f,0,SEEK_END);size_t fsize=ftell(f);fseek(f,0,SEEK_SET);
    uint8_t* jpgBuf=(uint8_t*)ps_malloc(fsize);
    if(!jpgBuf){fclose(f);return false;}
    fread(jpgBuf,1,fsize,f);fclose(f);
    TJpgDec.setJpgScale(1);TJpgDec.setSwapBytes(true);TJpgDec.setCallback(tjpgdecOutput);
    uint16_t iw=0,ih=0; TJpgDec.getJpgSize(&iw,&ih,jpgBuf,fsize);
    if(iw==0||ih==0){free(jpgBuf);return false;}
    uint16_t* buf=(uint16_t*)ps_malloc((size_t)iw*ih*sizeof(uint16_t));
    if(!buf){free(jpgBuf);return false;}
    memset(buf,0,(size_t)iw*ih*sizeof(uint16_t));
    _decodeTargetBuf=buf;_decodeTargetW=iw;
    TJpgDec.drawJpg(0,0,jpgBuf,fsize);
    _decodeTargetBuf=nullptr; free(jpgBuf);
    photoPixelBuf=buf;photoBufW=iw;photoBufH=ih; return true;
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Photo View
// ─────────────────────────────────────────────────────────────────────────────
void photoViewRender(){
  if(!photoPixelBuf) return;
  float zf=photoZoomFactors[photoZoomLevel];
  lcd.fillScreen(COL_BLACK);
  if(zf<=1.0f){
    int ox=(DISP_W-min((int)photoBufW,DISP_W))/2;
    int oy=(DISP_H-min((int)photoBufH,DISP_H))/2;
    int renderW=min((int)photoBufW,DISP_W),renderH=min((int)photoBufH,DISP_H);
    for(int row=0;row<renderH;row++){
      lcd.pushImage(ox,oy+row,renderW,1,photoPixelBuf+row*photoBufW);
      if(row%20==0) esp_task_wdt_reset();
    }
  } else {
    int vpW=(int)(DISP_W/zf),vpH=(int)(DISP_H/zf);
    int maxOffX=max(0,(int)photoBufW-vpW),maxOffY=max(0,(int)photoBufH-vpH);
    photoZoomOffX=constrain(photoZoomOffX,0,maxOffX);
    photoZoomOffY=constrain(photoZoomOffY,0,maxOffY);
    static uint16_t lineBuf[DISP_W];
    for(int sy=0;sy<DISP_H;sy++){
      int srcY=constrain(photoZoomOffY+(int)(sy/zf),0,(int)photoBufH-1);
      for(int sx=0;sx<DISP_W;sx++){
        int srcX=constrain(photoZoomOffX+(int)(sx/zf),0,(int)photoBufW-1);
        lineBuf[sx]=photoPixelBuf[srcY*photoBufW+srcX];
      }
      lcd.pushImage(0,sy,DISP_W,1,lineBuf);
      if(sy%20==0) esp_task_wdt_reset();
    }
  }
  if(photoZoomLevel>0){
    char zBuf[8]; snprintf(zBuf,sizeof(zBuf),"%.0f\xd7",(double)zf);
    drawPill(DISP_W/2,DISP_H-10,zBuf,COL_PILL_BG,COL_GRAY_A);
  }
  lcd.setFont(&fonts::Font0);lcd.setTextSize(1);lcd.setTextColor(COL_GRAY_3);
  lcd.drawString("B=zoom",2,2);
  lcd.drawString("BOOT=back",DISP_W-58,2);
  if(photoZoomLevel==0){
    lcd.drawString("C=prev",2,DISP_H-10);
    lcd.setTextColor(COL_AI_ACCENT);
    lcd.drawString("Dlong=AI",DISP_W-52,DISP_H-10);
  } else {
    lcd.drawString("C/D=pan",2,DISP_H-10);
    lcd.drawString("Blong=del",DISP_W-58,DISP_H-10);
  }
}

void photoViewDrawCaption(int idx){
  char stegoInfo[48]="";
  char path[56]; snprintf(path,sizeof(path),"/sdcard/%s",galleryFiles[idx]);
  if(gIsJpg(idx)){
    FILE* f=fopen(path,"rb");
    if(f){
      fseek(f,0,SEEK_END);long fsize_l=ftell(f);fseek(f,0,SEEK_SET);
      if(fsize_l>0){
        size_t fsize=(size_t)fsize_l,readSize=(fsize<8192)?fsize:8192;
        uint8_t* buf=(uint8_t*)ps_malloc(readSize);
        if(!buf) buf=(uint8_t*)malloc(readSize);
        if(buf){
          size_t readN=fread(buf,1,readSize,f);
          char payload[STEGO_PAYLOAD_LEN+1];
          if(stegoExtractFromJpeg(buf,readN,payload,sizeof(payload))){
            char* p1=strchr(payload,'|'),*p2=p1?strchr(p1+1,'|'):nullptr;
            if(p1&&p2){
              char num[8],ver[12]; int numLen=p2-p1-1;
              strncpy(num,p1+1,numLen);num[numLen]='\0';
              strncpy(ver,p2+1,sizeof(ver)-1);ver[sizeof(ver)-1]='\0';
              snprintf(stegoInfo,sizeof(stegoInfo)," [#%s %s]",num,ver);
            }
          }
          free(buf);
        }
      }
      fclose(f);
    }
  } else if(gIsBmp(idx)){
    char payload[STEGO_BMP_MAX_PAYLOAD+1];
    if(stegoBmpExtract(path,payload,sizeof(payload))){
      char* p1=strchr(payload,'|'),*p2=p1?strchr(p1+1,'|'):nullptr;
      if(p1&&p2){
        char num[8],ver[12]; int numLen=p2-p1-1;
        strncpy(num,p1+1,numLen);num[numLen]='\0';
        strncpy(ver,p2+1,sizeof(ver)-1);ver[sizeof(ver)-1]='\0';
        snprintf(stegoInfo,sizeof(stegoInfo)," [#%s %s]",num,ver);
      }
    }
  }
  int photoSeq=0,photoTotal=0;
  for(int i=0;i<galleryCount;i++){
    if(gIsPhoto(i)){photoTotal++;if(i<=idx)photoSeq=photoTotal;}
  }
  const char* typeTag=gIsBmp(idx)?" [BMP]":"";
  lcd.fillRect(0,DISP_H-16,DISP_W,16,COL_GRAY_D);
  lcd.setFont(&fonts::Font0);
  char bar[56],barFull[88];
  snprintf(bar,sizeof(bar),"< %d / %d >%s%s",photoSeq,photoTotal,typeTag,stegoInfo);
  snprintf(barFull,sizeof(barFull),"< %d / %d >  %s%s%s",photoSeq,photoTotal,galleryFiles[idx],typeTag,stegoInfo);
  const char* barStr=(lcd.textWidth(barFull)<DISP_W-4)?barFull:bar;
  lcd.setTextColor(COL_GRAY_A);
  lcd.drawString(barStr,(DISP_W-lcd.textWidth(barStr))/2,DISP_H-13);
}

void photoViewClearCaption(){
  lcd.fillRect(0,DISP_H-16,DISP_W,16,COL_BLACK);
  if(photoZoomLevel>0){
    float zf=photoZoomFactors[photoZoomLevel];
    char zBuf[8]; snprintf(zBuf,sizeof(zBuf),"%.0f\xd7",(double)zf);
    drawPill(DISP_W/2,DISP_H-10,zBuf,COL_PILL_BG,COL_GRAY_A);
  }
  photoViewCaptionVisible=false;photoViewCaptionUntilMs=0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Delete dialog
// ─────────────────────────────────────────────────────────────────────────────
// UI UPDATE v6.0
void drawDeleteDialog(const char* filename){
  int dw=200, dh=88, dx=(DISP_W-dw)/2, dy=(DISP_H-dh)/2;
  lcd.fillRoundRect(dx, dy, dw, dh, 4, COL_BLACK);
  lcd.drawRoundRect(dx, dy, dw, dh, 4, 0xF800);

  lcd.setFont(&fonts::Font0); lcd.setTextSize(1);
  lcd.setTextColor(0xF800);
  const char* title = "HAPUS FILE?";
  lcd.drawString(title, dx+(dw-lcd.textWidth(title))/2, dy+8);

  lcd.setTextColor(COL_GRAY_5);
  char truncName[28]; strncpy(truncName, filename, 27); truncName[27]=0;
  lcd.drawString(truncName, dx+(dw-lcd.textWidth(truncName))/2, dy+22);

  // Timer bar background only — actual fill drawn in handleModeDialogDelete
  lcd.fillRect(dx+10, dy+36, dw-20, 4, COL_GRAY_2);

  // Button row
  int by = dy+50;
  int b1w = lcd.textWidth("BOOT = HAPUS")+16, b1h=14;
  int b2w = lcd.textWidth("B = batal")+16,    b2h=14;
  int gap2 = 6;
  int totalW = b1w+b2w+gap2;
  int bx = dx+(dw-totalW)/2;

  lcd.drawRect(bx, by, b1w, b1h, 0xA000);
  lcd.setTextColor(0xF800);
  lcd.drawString("BOOT = HAPUS", bx+8, by+3);

  int bx2 = bx+b1w+gap2;
  lcd.drawRect(bx2, by, b2w, b2h, COL_GRAY_3);
  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("B = batal", bx2+8, by+3);
}

void openDeleteDialog(){
  drawDeleteDialog(galleryFiles[photoViewIndex]);
  deleteDialogOpenMs = millis();
  blockingWaitAllRelease(600);  // TAMBAH INI sebelum resetAllButtons
  resetAllButtons();
  appMode = MODE_DIALOG_DELETE;
  neoSolid(120, 0, 0);
}

void photoViewDeleteCurrent(){
  char path[56]; snprintf(path,sizeof(path),"/sdcard/%s",galleryFiles[photoViewIndex]);
  bool ok=(remove(path)==0);
  if (ok && gIsJpg(photoViewIndex)) {
    char cachePath[64];
    thumbCachePath(galleryFiles[photoViewIndex], cachePath, sizeof(cachePath));
    remove(cachePath);
  }
  islandPush(ok?NOTIF_OK:NOTIF_WARN,ok?"FILE DIHAPUS":"GAGAL HAPUS");
  if(photoPixelBuf){free(photoPixelBuf);photoPixelBuf=nullptr;}
  scanGalleryFiles();scanPhotoCount();
  gallerySelIdx=0;
  resetAllButtons(); appMode=MODE_GALLERY; if(galleryGridMode) drawGalleryGrid(); else drawGallery();
  neoBurst(120, 0, 180, 1); neoOff();
}

// ─────────────────────────────────────────────────────────────────────────────
//  showPhotoView
// ─────────────────────────────────────────────────────────────────────────────
void showPhotoView(int idx){
  if(idx<0||idx>=galleryCount||gIsVideo(idx)) return;
  photoViewIndex=idx;
  strncpy(photoViewPath,galleryFiles[idx],sizeof(photoViewPath)-1);
  photoZoomLevel=0;photoZoomOffX=0;photoZoomOffY=0;
  lcd.fillScreen(COL_BLACK);
  lcd.setFont(&fonts::Font0);lcd.setTextColor(COL_GRAY_5);
  const char* loadMsg=gIsBmp(idx)?"loading BMP...":"loading...";
  lcd.drawString(loadMsg,DISP_W/2-30,DISP_H/2-4);
  if(!photoLoadPixelBuf(idx)){
    lcd.fillScreen(COL_BLACK);
    lcd.drawString("gagal load foto",20,DISP_H/2);
    delay(1500);lcd.setRotation(3); resetAllButtons();appMode=MODE_GALLERY; if(galleryGridMode) drawGalleryGrid(); else drawGallery();return;
  }
  photoViewRender();
  photoViewDrawCaption(idx);
  photoViewCaptionUntilMs=millis()+2000;photoViewCaptionVisible=true;
  resetAllButtons();appMode=MODE_PHOTO_VIEW;
}

void photoViewPrev(){
  int idx=photoViewIndex-1; if(idx<0) idx=galleryCount-1;
  for(int t=0;t<galleryCount;t++){
    if(gIsPhoto(idx)){showPhotoView(idx);return;}
    idx--;if(idx<0) idx=galleryCount-1;
  }
}

void photoViewNext(){
  int idx=photoViewIndex+1; if(idx>=galleryCount) idx=0;
  for(int t=0;t<galleryCount;t++){
    if(gIsPhoto(idx)){showPhotoView(idx);return;}
    idx++;if(idx>=galleryCount) idx=0;
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Menu LED
// ─────────────────────────────────────────────────────────────────────────────
void drawLedMenu(int sel){
  int mw=220,mh=114,mx=(DISP_W-mw)/2,my=(DISP_H-mh)/2;
  lcd.fillRoundRect(mx,my,mw,mh,10,COL_GRAY_D);
  lcd.drawRoundRect(mx,my,mw,mh,10,COL_GRAY_5);
  lcd.setFont(&fonts::Font0);lcd.setTextSize(1);
  lcd.setTextColor(COL_GRAY_E);
  const char* title="LED FLASH";
  lcd.drawString(title,mx+(mw-lcd.textWidth(title))/2,my+7);
  lcd.drawFastHLine(mx+10,my+19,mw-20,COL_GRAY_3);
  for(int i=0;i<3;i++){
    int iy=my+24+i*24;
    bool isChecked=false;
    if(i==0) isChecked = (!ledFlashEnabled && !neoFlashEnabled);
    else if(i==1) isChecked = (ledFlashEnabled && !neoFlashEnabled);
    else if(i==2) isChecked = (ledFlashEnabled && neoFlashEnabled);
    bool isHighlight=(i==sel);
    lcd.fillRect(mx+8,iy,mw-16,18,isHighlight?COL_GRAY_5:COL_GRAY_D);
    lcd.setTextColor(isHighlight?COL_WHITE:(isChecked?COL_GRAY_E:COL_GRAY_7));
    lcd.drawRect(mx+12,iy+5,8,8,isHighlight?COL_WHITE:COL_GRAY_5);
    if(isChecked){lcd.drawFastHLine(mx+14,iy+9,4,isHighlight?COL_WHITE:COL_GRAY_E);lcd.drawFastVLine(mx+14,iy+7,4,isHighlight?COL_WHITE:COL_GRAY_E);}
    if(i==0)      lcd.drawString("LED OFF   - tanpa flash",mx+24,iy+5);
    else if(i==1) lcd.drawString("LED ON    - flash LED saja",mx+24,iy+5);
    else if(i==2) lcd.drawString("LED+NEO   - flash LED & NeoPixel putih",mx+24,iy+5);
  }
  lcd.setTextColor(COL_GRAY_3);
  const char* hint="C/D=pilih  BOOT=ok  B=batal";
  lcd.drawString(hint,mx+(mw-lcd.textWidth(hint))/2,my+mh-13);
}

void openLedMenu(){
  lcd.setRotation(3);
  lcd.setRotation(3);
  if (neoFlashEnabled) menuLedSel = 2;
  else if (ledFlashEnabled) menuLedSel = 1;
  else menuLedSel = 0;
  drawLedMenu(menuLedSel);
  resetAllButtons();prevMode=appMode;appMode=MODE_MENU_LED;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Menu Format
// ─────────────────────────────────────────────────────────────────────────────
void drawFormatMenu(int sel){
  int mw=220,mh=120,mx=(DISP_W-mw)/2,my=(DISP_H-mh)/2;
  lcd.fillRoundRect(mx,my,mw,mh,10,COL_GRAY_D);
  lcd.drawRoundRect(mx,my,mw,mh,10,COL_GRAY_5);
  lcd.setFont(&fonts::Font0);lcd.setTextSize(1);
  lcd.setTextColor(COL_GRAY_E);
  const char* title="FORMAT FOTO  GC2145";
  lcd.drawString(title,mx+(mw-lcd.textWidth(title))/2,my+7);
  lcd.drawFastHLine(mx+10,my+19,mw-20,COL_GRAY_3);
  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("pilih format simpan foto:",mx+10,my+24);
  const char* labels[2]={
    "BMP  - raw RGB, warna asli, stego",
    "JPG  - kompresi, stego+EXIF"
  };
  for(int i=0;i<2;i++){
    int iy=my+38+i*26;
    bool isSelected=(i==(int)gc2145CaptureFormat),isHighlighted=(i==sel);
    lcd.fillRect(mx+8,iy,mw-16,20,isHighlighted?COL_GRAY_5:COL_GRAY_D);
    int cbX=mx+14,cbY=iy+6;
    lcd.drawRect(cbX,cbY,10,10,isHighlighted?COL_WHITE:COL_GRAY_7);
    if(isSelected){
      lcd.drawFastHLine(cbX+2,cbY+5,6,isHighlighted?COL_WHITE:COL_GRAY_E);
      lcd.drawFastVLine(cbX+2,cbY+3,4,isHighlighted?COL_WHITE:COL_GRAY_E);
      lcd.drawPixel(cbX+4,cbY+7,isHighlighted?COL_WHITE:COL_GRAY_E);
    }
    lcd.setTextColor(isHighlighted?COL_WHITE:(isSelected?COL_GRAY_E:COL_GRAY_7));
    lcd.drawString(labels[i],cbX+14,iy+6);
  }
  lcd.setTextColor(COL_GRAY_3);
  const char* hint="C/D=pilih  BOOT=ok  B=batal";
  lcd.drawString(hint,mx+(mw-lcd.textWidth(hint))/2,my+mh-13);
}

void openFormatMenu(){
  menuFormatSel=(int)gc2145CaptureFormat;
  drawFormatMenu(menuFormatSel);
  resetAllButtons();prevMode=appMode;appMode=MODE_MENU_FORMAT;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Menu Exposure
// ─────────────────────────────────────────────────────────────────────────────
// UI UPDATE v6.0
void drawExpMenu(int sel){
  lcd.fillScreen(COL_BLACK);

  // Arc meter — centered at x=160, y=80
  lcd.fillArc(160, 80, 55, 49, 180, 180 + (sel * 36), COL_GRAY_7);
  lcd.fillArc(160, 80, 55, 49, 180 + (sel * 36), 360, COL_GRAY_2);

  // Preset name center of arc
  lcd.setFont(&fonts::FreeSansBold9pt7b);
  lcd.setTextColor(COL_WHITE);
  lcd.drawString(expPresetNames[sel], 160 - lcd.textWidth(expPresetNames[sel])/2, 65);

  // Index below name
  lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
  char idxBuf[8]; snprintf(idxBuf, sizeof(idxBuf), "%d/5", sel);
  lcd.drawString(idxBuf, 160 - lcd.textWidth(idxBuf)/2, 82);

  // Chip row — 6 chips, centered, 46x20px each, 4px gap
  const char* shorts[6] = {"AUTO","GRAY","MOON","NIGHT","N-BW","MAN"};
  int cw=46, ch=20, gap=4;
  int tw = 6*cw + 5*gap;
  int sx = (DISP_W - tw) / 2;
  for(int i=0; i<6; i++){
    int ix = sx + i*(cw+gap), iy = 100;
    lcd.fillRoundRect(ix, iy, cw, ch, 3, i==sel ? COL_WHITE : COL_GRAY_D);
    if(i==sel) lcd.drawRoundRect(ix, iy, cw, ch, 3, COL_WHITE);
    lcd.setFont(&fonts::Font0);
    lcd.setTextColor(i==sel ? COL_BLACK : COL_GRAY_5);
    lcd.drawString(shorts[i], ix+(cw-lcd.textWidth(shorts[i]))/2, iy+6);
  }

  // Bottom section
  int by = 140;
  if(sel == 5){
    lcd.setFont(&fonts::FreeSansBold9pt7b); lcd.setTextSize(2);
    lcd.setTextColor(COL_GRAY_C);
    char valBuf[16]; snprintf(valBuf, sizeof(valBuf), "%d", expManualVal);
    lcd.drawString(valBuf, (DISP_W-lcd.textWidth(valBuf))/2, by+10);
    lcd.setTextSize(1); lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_3);
    lcd.drawString("C/D = adjust  B = gain", (DISP_W-lcd.textWidth("C/D = adjust  B = gain"))/2, by+50);
  } else {
    const char* descs[5] = {
      "kamera atur otomatis", "grayscale effect",
      "objek terang / bulan", "malam gelap", "malam grayscale"
    };
    lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_5);
    lcd.drawString(descs[sel], (DISP_W-lcd.textWidth(descs[sel]))/2, by+20);
  }

  // Footer
  lcd.setTextColor(COL_GRAY_3);
  lcd.drawString("C/D=pilih  BOOT=ok  B=batal", (DISP_W-lcd.textWidth("C/D=pilih  BOOT=ok  B=batal"))/2, DISP_H-30);

  // GC2145 format line (keep as-is from original)
  if(detectedSensor==PID_GC2145){
    lcd.setTextColor(COL_GRAY_5);
    char fmtBuf[32];
    snprintf(fmtBuf,sizeof(fmtBuf),"Format: %s  [D-long=ganti]",
             gc2145CaptureFormat==GFMT_BMP?"BMP":"JPG");
    lcd.drawString(fmtBuf,(DISP_W-lcd.textWidth(fmtBuf))/2, DISP_H-14);
  }
}
void drawExpAdjOverlay(){
  int oh=38,oy=DISP_H-oh;
  lcd.fillRect(0,oy,DISP_W,oh,COL_GRAY_D);
  lcd.drawFastHLine(0,oy,DISP_W,COL_GRAY_5);
  lcd.setFont(&fonts::Font0);lcd.setTextSize(1);lcd.setTextColor(COL_GRAY_E);
  char buf[32];
  snprintf(buf,sizeof(buf),"EXP  %4d",expManualVal);lcd.drawString(buf,8,oy+4);
  snprintf(buf,sizeof(buf),"GAIN %2d",expManualGain);lcd.drawString(buf,DISP_W-56,oy+4);
  int barX=8,barY=oy+18,barW=DISP_W-70,barH=7;
  lcd.fillRect(barX,barY,barW,barH,COL_GRAY_3);
  lcd.fillRect(barX,barY,map(expManualVal,0,1200,0,barW),barH,COL_GRAY_C);
  lcd.drawRect(barX,barY,barW,barH,COL_GRAY_5);
  int gbarX=DISP_W-56,gbarY=oy+18,gbarW=48,gbarH=7;
  lcd.fillRect(gbarX,gbarY,gbarW,gbarH,COL_GRAY_3);
  lcd.fillRect(gbarX,gbarY,map(expManualGain,0,30,0,gbarW),gbarH,COL_GRAY_7);
  lcd.drawRect(gbarX,gbarY,gbarW,gbarH,COL_GRAY_5);
  lcd.setTextColor(COL_GRAY_5);
  const char* hint="C=exp-  D=exp+  B=gain+  BOOT=ok";
  lcd.drawString(hint,(DISP_W-lcd.textWidth(hint))/2,oy+28);
}

void openExpMenu(){
  lcd.setRotation(3);
  lcd.setRotation(3);
  menuExpSel=(int)expPreset;
  drawExpMenu(menuExpSel);
  resetAllButtons();prevMode=appMode;appMode=MODE_MENU_EXP;
}

// ─────────────────────────────────────────────────────────────────────────────
//  MJPEG Player
// ─────────────────────────────────────────────────────────────────────────────
bool mjpegOpen(const char* filename){
  char path[48]; snprintf(path,sizeof(path),"/sdcard/%s",filename);
  if(mjpegFile){fclose(mjpegFile);mjpegFile=nullptr;}
  if(mjpegBuf){free(mjpegBuf);mjpegBuf=nullptr;}
  mjpegBuf=(uint8_t*)malloc(MJPEG_BUF_SIZE);
  if(!mjpegBuf) return false;
  mjpegFile=fopen(path,"rb");
  if(!mjpegFile){free(mjpegBuf);mjpegBuf=nullptr;return false;}

  mjpegActualFps = 15.0f;
  char iniPath[48]; strncpy(iniPath, path, sizeof(iniPath)-1);
  char* dot = strrchr(iniPath, '.');
  if(dot) strcpy(dot, ".ini");
  FILE* fIni = fopen(iniPath, "r");
  if(fIni){
    char line[48];
    while(fgets(line, sizeof(line), fIni)){
      sscanf(line, "fps=%f", &mjpegActualFps);
    }
    fclose(fIni);
  }

  fileStream.f=mjpegFile;
  mjpeg.setup(&fileStream,mjpegBuf,jpegDrawCallback,true,0,0,DISP_W,DISP_H);
  strncpy(mjpegPath,filename,sizeof(mjpegPath)-1);
  strncpy(mjpegPathSaved,filename,sizeof(mjpegPathSaved)-1);
  mjpegFrame=0;mjpegPlaying=true;mjpegPaused=false;mjpegNotifUntilMs=0;
  lcd.fillScreen(COL_BLACK); return true;
}
void mjpegClose(){
  if(mjpegFile){fclose(mjpegFile);mjpegFile=nullptr;}
  if(mjpegBuf){free(mjpegBuf);mjpegBuf=nullptr;}
  fileStream.f=nullptr;
  mjpegPlaying=false;mjpegPaused=false;mjpegFrame=0;mjpegNotifUntilMs=0;
}

void mjpegShowNotif(const char* text){
  lcd.setFont(&fonts::Font0);lcd.setTextSize(1);
  int tw=lcd.textWidth(text),pw=tw+20,ph=17;
  int px=(DISP_W-pw)/2,py=(DISP_H-ph)/2;
  lcd.fillRoundRect(px,py,pw,ph,6,COL_GRAY_D);
  lcd.drawRoundRect(px,py,pw,ph,6,COL_GRAY_5);
  lcd.setTextColor(COL_GRAY_E);lcd.drawString(text,px+10,py+4);
  mjpegNotifUntilMs=millis()+1200;
}

void mjpegClearNotif(){
  lcd.fillRect((DISP_W-188)/2-4,(DISP_H-25)/2,196,25,COL_BLACK);
  mjpegNotifUntilMs=0;
}

void mjpegDrawHUD(){
  char buf[32];
  snprintf(buf, sizeof(buf), "%.1f\xd7 %.0ffps %s", (double)mjpegSpeeds[mjpegSpeedIdx], (double)mjpegActualFps, mjpegLoop ? "LOOP" : "-");
  lcd.setFont(&fonts::Font0);
  int tw=lcd.textWidth(buf),pw=tw+10,ph=13;
  lcd.fillRoundRect(4,4,pw,ph,4,COL_PILL_BG);
  lcd.setTextColor(COL_GRAY_A);lcd.drawString(buf,9,6);
}



void loopMjpegPlayer(){
  if(!mjpegPlaying) return;
  if(!mjpegPaused){
    bool ok=mjpeg.readMjpegBuf();
    if(!ok){
      if(mjpegLoop){
        mjpegClose();
        if(!mjpegOpen(mjpegPathSaved)){lcd.setRotation(3); resetAllButtons();appMode=MODE_GALLERY; if(galleryGridMode) drawGalleryGrid(); else drawGallery();}
        return;
      }
      mjpegClose();lcd.setRotation(3); resetAllButtons();appMode=MODE_GALLERY; if(galleryGridMode) drawGalleryGrid(); else drawGallery();return;
    }
    mjpeg.drawJpg();mjpegFrame++;mjpegDrawHUD();
    if(mjpegNotifUntilMs>0&&millis()>mjpegNotifUntilMs) mjpegClearNotif();
    float speed = mjpegSpeeds[mjpegSpeedIdx];
    uint32_t frameIntervalMs = (uint32_t)(1000.0f / (mjpegActualFps * speed));
    frameIntervalMs = constrain(frameIntervalMs, 16, 500); // clamp between 2fps and 62fps
    vTaskDelay(pdMS_TO_TICKS(frameIntervalMs));
  } else {
    if(mjpegNotifUntilMs>0&&millis()>mjpegNotifUntilMs) mjpegClearNotif();
    delay(16);
  }
  esp_task_wdt_reset();
}
void openMjpegPlayer(const char* filename){
  mjpegLoop=false;mjpegSpeedIdx=1;
  if(!mjpegOpen(filename)){
    lcd.fillScreen(COL_BLACK);lcd.setFont(&fonts::Font0);lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("gagal buka file",20,DISP_H/2);
    delay(1500);resetAllButtons();drawGallery();appMode=MODE_GALLERY;return;
  }
  resetAllButtons();appMode=MODE_MJPEG_PLAYER;
}

// ─────────────────────────────────────────────────────────────────────────────
//  REC indicator & Recording
// ─────────────────────────────────────────────────────────────────────────────
void drawRecIndicator(){
  if(!recActive) return;
  unsigned long elapsed=(millis()-recStartMs)/1000;
  char timeBuf[10]; snprintf(timeBuf,sizeof(timeBuf),"%02lu:%02lu",elapsed/60,elapsed%60);
  bool blink=(millis()/500)%2;
  lcd.fillRect(4,4,90,22,COL_BLACK);
  lcd.fillCircle(10,11,4,blink?COL_WHITE:COL_GRAY_5);
  lcd.setFont(&fonts::Font0);lcd.setTextColor(COL_GRAY_E);
  lcd.drawString(timeBuf,18,4);
  char fBuf[10]; snprintf(fBuf,sizeof(fBuf),"%df",recFrameCount);
  lcd.setTextColor(COL_GRAY_5); lcd.drawString(fBuf,18,14);
}


void startRecording() {
  if (!sdReady || recActive) return;
  neoSolid(180, 0, 0);
  if (!sdReady || recActive) return;
  recVideoCount++;
  char path[48]; snprintf(path, sizeof(path), "/sdcard/video_%04d.mjpeg", recVideoCount);
  recFile = fopen(path, "wb");
  if (!recFile) { recVideoCount--; return; }
  g_eisOffX = EIS_CROP_X; g_eisOffY = EIS_CROP_Y;
  g_eisBiasX = 0; g_eisBiasY = 0;
  recFrameCount = 0; recStartMs = millis(); recActive = true;
  char buf[20]; snprintf(buf, sizeof(buf), "REC #%04d", recVideoCount);
  islandPush(NOTIF_REC, buf);
}

void stopRecording(){
  if(!recActive||!recFile) return;
  fclose(recFile);recFile=nullptr;recActive=false;
  float actualFps = (float)recFrameCount / ((millis() - recStartMs) / 1000.0f);
  char iniPath[48]; snprintf(iniPath, sizeof(iniPath), "/sdcard/video_%04d.ini", recVideoCount);
  FILE* fIni = fopen(iniPath, "w");
  if(fIni){
    fprintf(fIni, "fps=%.2f\n", actualFps);
    fprintf(fIni, "frames=%d\n", recFrameCount);
    fclose(fIni);
  }
  unsigned long dur=(millis()-recStartMs)/1000;
  char buf[28]; snprintf(buf,sizeof(buf),"%df  %02lu:%02lu",recFrameCount,dur/60,dur%60);
  islandPush(NOTIF_OK,buf);
  neoBurst(0, 180, 0, 3); neoOff();
  fpsLastTime=millis();fpsFrameCount=0;
}
void recordFrame() {
  if (!recActive || !recFile) return;
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) { esp_task_wdt_reset(); return; }
  if (eisEnabled && fb->format == PIXFORMAT_RGB565) {
    uint16_t* src = (uint16_t*)fb->buf;
    uint16_t* tmp = (uint16_t*)ps_malloc(DISP_W * DISP_H * 2);
    if (!tmp) tmp = (uint16_t*)malloc(DISP_W * DISP_H * 2);
    if (tmp) {
      int sx = constrain((int)g_eisOffX, 0, DISP_W - EIS_VP_W); int sy = constrain((int)g_eisOffY, 0, DISP_H - EIS_VP_H);
      for (int y = 0; y < DISP_H; y++) {
        int srY = sy + (y * EIS_VP_H / DISP_H);
        for (int x = 0; x < DISP_W; x++) {
          int srX = sx + (x * EIS_VP_W / DISP_W);
          tmp[y * DISP_W + x] = src[srY * DISP_W + srX];
        }
      }
      uint8_t* out = nullptr; size_t oLen = 0;
      camera_fb_t fk = *fb; fk.buf = (uint8_t*)tmp; fk.width = DISP_W; fk.height = DISP_H;
      int recQ = hdCaptureEnabled ? map(hdCaptureQuality, 10, 1, 50, 95) : 85;
      if (frame2jpg(&fk, recQ, &out, &oLen)) { fwrite(out, 1, oLen, recFile); recFrameCount++; free(out); }
      if (recFrameCount % 3 == 0) {
        int dw = min((int)DISP_W, (int)lcd.width());
        int dh = min((int)DISP_H, (int)lcd.height());
        lcd.pushImage(0, 0, dw, dh, tmp);
      }
      free(tmp);
    }
    esp_camera_fb_return(fb); return;
  }
  uint8_t* jpg = nullptr; size_t jLen = 0; bool ok = false;
  if (fb->format == PIXFORMAT_JPEG) { jpg = fb->buf; jLen = fb->len; ok = true; }
  else { int recQ = hdCaptureEnabled ? map(hdCaptureQuality, 10, 1, 50, 95) : 85;
  ok = frame2jpg(fb, recQ, &jpg, &jLen); }
  if (ok && jpg) {
    fwrite(jpg, 1, jLen, recFile); recFrameCount++;
    if (fb->format != PIXFORMAT_JPEG) free(jpg);
  }
  if (recFrameCount % 3 == 0 && fb->format == PIXFORMAT_RGB565 && fb->width == DISP_W) {
    int dw = min((int)fb->width, (int)lcd.width());
    int dh = min((int)fb->height, (int)lcd.height());
    lcd.pushImage(0, 0, dw, dh, (uint16_t*)fb->buf);
  }
  esp_camera_fb_return(fb); esp_task_wdt_reset();
}

// ─────────────────────────────────────────────────────────────────────────────
//  USB MSC
// ─────────────────────────────────────────────────────────────────────────────
static int32_t onRead(uint32_t lba,uint32_t offset,void* buffer,uint32_t bufsize){
  (void)offset;
  if(!sdCard||!sdmmcDriverInit) return -1;
  return(sdmmc_read_sectors(sdCard,buffer,lba,bufsize/512)==ESP_OK)?(int32_t)bufsize:-1;
}
static int32_t onWrite(uint32_t lba,uint32_t offset,uint8_t* buffer,uint32_t bufsize){
  (void)offset;
  if(!sdCard||!sdmmcDriverInit) return -1;
  return(sdmmc_write_sectors(sdCard,buffer,lba,bufsize/512)==ESP_OK)?(int32_t)bufsize:-1;
}

// ─────────────────────────────────────────────────────────────────────────────
//  FPS
// ─────────────────────────────────────────────────────────────────────────────
void updateFPS(){
  fpsFrameCount++;
  unsigned long elapsed=millis()-fpsLastTime;
  if(elapsed>=500){
    float newFps=fpsFrameCount*1000.0f/elapsed;
    fpsValue=fpsValue*0.7f+newFps*0.3f;
    fpsFrameCount=0;fpsLastTime=millis();
  }
}


// ─────────────────────────────────────────────────────────────────────────────
//  Exposure
// ─────────────────────────────────────────────────────────────────────────────

static uint8_t mpuCalcOrientation(float pitch, float roll) {
  if (fabsf(roll) < 45.0f && fabsf(pitch) < 45.0f) return 0;
  if (roll > 45.0f) return 1;
  if (roll < -45.0f) return 3;
  if (pitch > 45.0f) return 2;
  return 0;
}

struct KalmanAngle {
  float angle   = 0.0f;
  float bias    = 0.0f;
  float P[2][2] = {{1,0},{0,1}};
  float Q_angle = 0.001f;
  float Q_bias  = 0.003f;

  float update(float newAngle, float newRate, float dt) {
    float rate = newRate - bias;
    angle += dt * rate;
    P[0][0] += dt * (dt * P[1][1] - P[0][1] - P[1][0] + Q_angle);
    P[0][1] -= dt * P[1][1];
    P[1][0] -= dt * P[1][1];
    P[1][1] += Q_bias * dt;
    float S = P[0][0] + mpuKalmanRmeas;
    float K[2] = { P[0][0] / S, P[1][0] / S };
    float y = newAngle - angle;
    angle += K[0] * y;
    bias  += K[1] * y;
    P[0][0] -= K[0] * P[0][0];
    P[0][1] -= K[0] * P[0][1];
    P[1][0] -= K[1] * P[0][0];
    P[1][1] -= K[1] * P[0][1];
    return angle;
  }
};

static KalmanAngle kalmanX, kalmanY;
static uint32_t kalmanLastMs = 0;

void mpuTick() {
  esp_task_wdt_reset();
  if (!g_mpuOk) return;
  uint32_t now = millis();
  if (now - g_mpuLastMs < MPU_READ_MS) return;
  g_mpuLastMs = now;

  sensors_event_t a, g_ev, t;
  mpu.getEvent(&a, &g_ev, &t);

  g_accX = a.acceleration.x;
  g_accY = a.acceleration.y;
  g_accZ = a.acceleration.z;

  // Pure accelerometer + offset correction
  float ax = a.acceleration.x - g_accOffX;
  float ay = a.acceleration.y - g_accOffY;
  float az = a.acceleration.z - g_accOffZ;

  float rawTiltX = atan2f(ay, az) * RAD_TO_DEG;
  float rawTiltY = atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD_TO_DEG;

  float dt = (kalmanLastMs == 0) ? 0.08f : (now - kalmanLastMs) / 1000.0f;
  kalmanLastMs = now;
  dt = constrain(dt, 0.01f, 0.2f);

  g_tiltX = kalmanX.update(rawTiltX, g_gyroX * RAD_TO_DEG, dt);
  g_tiltY = kalmanY.update(rawTiltY, g_gyroY * RAD_TO_DEG, dt);

  // Keep gyro lines unchanged (radians for EIS consistency)
  g_gyroX = g_ev.gyro.x - g_gyroCalX;
  g_gyroY = g_ev.gyro.y - g_gyroCalY;
  g_gyroZ = g_ev.gyro.z - g_gyroCalZ;

  g_tilted = (fabsf(g_tiltX) > MPU_TILT_DEG || fabsf(g_tiltY) > MPU_TILT_DEG);

  float mag = sqrtf(ax * ax + ay * ay + az * az);
  static float magFiltered = 9.8f;
  static int shakeCount = 0;
  magFiltered = magFiltered * 0.92f + mag * 0.08f;
  bool rawShake = (fabsf(magFiltered - 9.8f) > 3.0f);
  if (rawShake) shakeCount = min(shakeCount + 1, 5);
  else          shakeCount = max(shakeCount - 1, 0);
  g_shake = (shakeCount >= 3);
  if (g_shake) neoSpin(200, 60, 0);

  g_orientation = mpuCalcOrientation(g_tiltY, g_tiltX);

  if (autoRotateEnabled && appMode == MODE_VIEWFINDER) {
    uint8_t r = 3;
    if (g_tiltX > 45.0f) r = 2;
    else if (g_tiltX < -45.0f) r = 0;
    if (lcd.getRotation() != r) lcd.setRotation(r);
  }

  if (eisEnabled) {
    float gmag = sqrtf(g_gyroX * g_gyroX + g_gyroY * g_gyroY);
    if (gmag < 0.05f) {
      g_eisBiasX = g_eisBiasX * (1.0f - EIS_ALPHA) + g_gyroX * EIS_ALPHA;
      g_eisBiasY = g_eisBiasY * (1.0f - EIS_ALPHA) + g_gyroY * EIS_ALPHA;
    }
    float gdx = g_gyroX - g_eisBiasX, gdy = g_gyroY - g_eisBiasY;
    float tx = g_eisOffX + (gdx * 0.08f * 306.0f);
    float ty = g_eisOffY + (gdy * 0.08f * 306.0f);
    tx = tx * (1.0f - EIS_ALPHA) + (float)EIS_CROP_X * EIS_ALPHA;
    ty = ty * (1.0f - EIS_ALPHA) + (float)EIS_CROP_Y * EIS_ALPHA;
    tx = constrain(tx, 0.0f, (float)EIS_CROP_X * 2.0f);
    ty = constrain(ty, 0.0f, (float)EIS_CROP_Y * 2.0f);
    g_eisOffX = g_eisOffX * 0.85f + tx * 0.15f;
    g_eisOffY = g_eisOffY * 0.85f + ty * 0.15f;
  }

  if (sdReady && (now - g_mpuLogMs) >= MPU_LOG_MS && mpuLogEnabled) {
    g_mpuLogMs = now;
    FILE* lf = fopen("/sdcard/mpu_log.csv", "a");
    if (lf) {
      if (ftell(lf) == 0) fprintf(lf, "millis,tiltX,tiltY,gyroX,gyroY,gyroZ,orient\n");
      fprintf(lf, "%lu,%.2f,%.2f,%.3f,%.3f,%.3f,%d\n",
              (unsigned long)now, g_tiltX, g_tiltY, g_gyroX, g_gyroY, g_gyroZ, (int)g_orientation);
      fclose(lf);
    }
    esp_task_wdt_reset();
  }
}

void mpuDrawIndicator() {
  if (!g_mpuOk) return;
  char buf[16]; uint16_t col;
  bool showTilt = g_tilted && (fabsf(g_tiltX) > mpuTiltDeadzone || fabsf(g_tiltY) > mpuTiltDeadzone);
  if (g_shake) { strncpy(buf, "SHAKE!", sizeof(buf)); col=0xFD20; neoSpin(200, 60, 0); } // COL_WARN
  else if (showTilt) {
    if (fabsf(g_tiltX) >= fabsf(g_tiltY))
      snprintf(buf, sizeof(buf), "%.0f\xb0 %s", fabsf(g_tiltX), g_tiltX > 0 ? "R" : "L");
    else
      snprintf(buf, sizeof(buf), "%.0f\xb0 %s", fabsf(g_tiltY), g_tiltY > 0 ? "F" : "B");
    col=0xFD20; // COL_WARN
    float tilt = sqrtf(g_tiltX * g_tiltX + g_tiltY * g_tiltY);
    if (tilt > 30.0f) neoSolid(200, 80, 0); else neoSolid(180, 150, 0);
  } else { strncpy(buf, "LEVEL", sizeof(buf)); col=0x7BCF; neoSolid(0, 40, 0); } // COL_GRAY_7
  drawPill(DISP_W - 36, DISP_H - 22, buf, 0x18C3, col); // COL_PILL_BG = 0x18C3
}


// Write one register to GC2145 via SCCB
static void gc2145WriteReg(uint8_t reg, uint8_t val) {
  sensor_t* s = esp_camera_sensor_get();
  if (!s) return;
  SCCB_Write(s->slv_addr, reg, val);
}

// Read one register from GC2145 via SCCB
static uint8_t gc2145ReadReg(uint8_t reg) {
  sensor_t* s = esp_camera_sensor_get();
  if (!s) return 0;
  return (uint8_t)SCCB_Read(s->slv_addr, reg);
}

// Enable or disable AEC (auto exposure control)
// reg 0xb6 bit[0]: 1=AEC on, 0=manual
static void gc2145SetAEC(bool enable) {
  gc2145WriteReg(0xfe, 0x00); // select page 0
  uint8_t val = gc2145ReadReg(0xb6);
  if (enable) val |=  0x01;
  else        val &= ~0x01;
  gc2145WriteReg(0xb6, val);
}

// Set manual shutter value (0–1200 scale mapped to 13-bit GC2145 register)
// GC2145 exposure = 13-bit: reg 0x03[4:0] high, reg 0x04[7:0] low
static void gc2145SetExposure(uint16_t expVal) {
  uint16_t mapped = (uint16_t)((uint32_t)expVal * 0x1FFF / 1200);
  mapped = constrain(mapped, 0, 0x1FFF);
  gc2145WriteReg(0xfe, 0x00); // page 0
  gc2145WriteReg(0x03, (mapped >> 8) & 0x1F);
  gc2145WriteReg(0x04, mapped & 0xFF);
}

// Set gain ceiling via page 1 reg 0x08 (0x10=lowest, 0xFF=highest)
static void gc2145SetGain(uint8_t gain) {
  gc2145WriteReg(0xfe, 0x01); // page 1
  gc2145WriteReg(0x08, gain);
  gc2145WriteReg(0xfe, 0x00); // back to page 0
}

void applyExpPreset(uint8_t preset){
  sensor_t *s=esp_camera_sensor_get(); if(!s) return;
  expPreset=preset;
  const ExpPresetCfg& c=expPresets[preset];
  if (detectedSensor == PID_GC2145) {
    // All set_* functions are set_dummy on GC2145 — use direct register access
    int aecVal  = (preset == 5) ? expManualVal  : c.aec_val;
    int gainVal = (preset == 5) ? expManualGain : c.agc_gain;
    gc2145SetAEC(c.aec_on);
    if (!c.aec_on) {
      gc2145SetExposure((uint16_t)constrain(aecVal, 0, 1200));
      // map agc_gain 0-30 to gain register 0x10-0xFF
      uint8_t gainReg = (uint8_t)map(gainVal, 0, 30, 0x10, 0xFF);
      gc2145SetGain(gainReg);
    }
    // special_effect is the ONE set_* that actually works on GC2145
    s->set_special_effect(s, c.special_effect);
    s->set_hmirror(s, HMIRROR_GC2145);
    s->set_vflip(s, VFLIP_GC2145);
    delay(150); esp_task_wdt_reset();
    for (int i = 0; i < 2; i++) {
      camera_fb_t *fb = esp_camera_fb_get();
      if (fb) esp_camera_fb_return(fb);
      esp_task_wdt_reset();
    }
    return; // skip the OV3660/generic set_* calls below
  }
  int aecVal=(preset==5)?expManualVal:c.aec_val;
  int gainVal=(preset==5)?expManualGain:c.agc_gain;
  s->set_exposure_ctrl(s,c.aec_on?1:0);
  s->set_aec2(s,c.aec2_on?1:0);
  if(!c.aec_on) s->set_aec_value(s,aecVal);
  s->set_gain_ctrl(s,c.agc_on?1:0);
  if(!c.agc_on) s->set_agc_gain(s,gainVal);
  s->set_gainceiling(s,(gainceiling_t)c.gainceiling);
  s->set_ae_level(s,c.ae_level);
  s->set_special_effect(s,c.special_effect);
  if(detectedSensor==PID_GC2145){s->set_hmirror(s,HMIRROR_GC2145);s->set_vflip(s,VFLIP_GC2145);}
  else if(detectedSensor==PID_OV3660){s->set_hmirror(s,HMIRROR_OV3660);s->set_vflip(s,VFLIP_OV3660);}
  delay(150);esp_task_wdt_reset();
  for(int i=0;i<2;i++){
    camera_fb_t *fb=esp_camera_fb_get();
    if(fb) esp_camera_fb_return(fb);
    esp_task_wdt_reset();
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  AI — BASE64 ENCODER
// ─────────────────────────────────────────────────────────────────────────────
static void base64Encode(const uint8_t* in,size_t inLen,String& out){
  static const char b64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  out=""; out.reserve(((inLen+2)/3)*4+4);
  uint32_t val=0; int bits=-6;
  for(size_t i=0;i<inLen;i++){
    val=(val<<8)+in[i];bits+=8;
    while(bits>=0){out+=b64[(val>>bits)&0x3F];bits-=6;}
  }
  if(bits>-6) out+=b64[((val<<8)>>(bits+8))&0x3F];
  while(out.length()%4) out+='=';
}

// ─────────────────────────────────────────────────────────────────────────────
//  AI — WORD WRAP
// ─────────────────────────────────────────────────────────────────────────────
void aiWrapText(const char* text){
  aiTotalLines=0;aiScrollLine=0;
  memset(aiLines,0,sizeof(aiLines));
  const int maxW=DISP_W-12;
  lcd.setFont(&fonts::Font0);lcd.setTextSize(1);
  char word[48];int wi=0;
  char lineBuf[AI_LINE_W];lineBuf[0]='\0';

  auto flushLine=[&](){
    if(aiTotalLines<AI_LINE_MAX&&lineBuf[0]){
      strncpy(aiLines[aiTotalLines],lineBuf,AI_LINE_W-1);
      aiLines[aiTotalLines][AI_LINE_W-1]='\0';
      aiTotalLines++;lineBuf[0]='\0';
    }
  };

  const char* p=text;
  while(*p){
    if(*p=='\n'){
      if(wi>0){word[wi]='\0';wi=0;
        char test[AI_LINE_W];
        snprintf(test,sizeof(test),"%s%s%s",lineBuf,lineBuf[0]?" ":"",word);
        if(lcd.textWidth(test)>maxW){flushLine();strncpy(lineBuf,word,AI_LINE_W-1);}
        else strncpy(lineBuf,test,AI_LINE_W-1);
      }
      flushLine();p++;continue;
    }
    if(*p==' '||wi>=47){
      word[wi]='\0';wi=0;
      if(word[0]){
        char test[AI_LINE_W];
        snprintf(test,sizeof(test),"%s%s%s",lineBuf,lineBuf[0]?" ":"",word);
        if(lcd.textWidth(test)>maxW){flushLine();strncpy(lineBuf,word,AI_LINE_W-1);}
        else strncpy(lineBuf,test,AI_LINE_W-1);
      }
      if(*p==' ') p++;continue;
    }
    word[wi++]=*p++;
  }
  if(wi>0){
    word[wi]='\0';
    char test[AI_LINE_W];
    snprintf(test,sizeof(test),"%s%s%s",lineBuf,lineBuf[0]?" ":"",word);
    if(lcd.textWidth(test)>maxW){flushLine();strncpy(lineBuf,word,AI_LINE_W-1);}
    else strncpy(lineBuf,test,AI_LINE_W-1);
  }
  flushLine();
}

// ─────────────────────────────────────────────────────────────────────────────
//  AI — RENDER layar hasil
// ─────────────────────────────────────────────────────────────────────────────
void drawAIDescribeScreen(){
  lcd.fillScreen(COL_BLACK);
  lcd.fillRect(0,0,DISP_W,20,COL_GRAY_D);
  lcd.drawFastHLine(0,20,DISP_W,COL_GRAY_3);
  lcd.setFont(&fonts::Font0);lcd.setTextSize(1);
  lcd.setTextColor(COL_AI_ACCENT);

  const char* title = AI_FEATURES[(int)aiSelectedFeature].label;
  lcd.drawString(title,(DISP_W-lcd.textWidth(title))/2,6);

  char keyBadge[8]; snprintf(keyBadge,sizeof(keyBadge),"k%d/%d",geminiKeyActive+1,geminiKeyCount);
  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString(keyBadge,DISP_W-30,6);

  lcd.setTextColor(AI_FEATURES[(int)aiSelectedFeature].iconColor);
  lcd.drawString(AI_FEATURES[(int)aiSelectedFeature].icon,4,6);

  lcd.setTextColor(COL_GRAY_5);
  char fname[36]; snprintf(fname,sizeof(fname),"[%s]",
    (photoViewIndex>=0&&photoViewIndex<galleryCount)?galleryFiles[photoViewIndex]:"capture");
  lcd.drawString(fname,(DISP_W-lcd.textWidth(fname))/2,22);

  const int lineH=13,textStartY=34;
  const int visLines=(DISP_H-textStartY-14)/lineH;
  lcd.fillRect(0,textStartY,DISP_W,DISP_H-textStartY-14,COL_BLACK);

  for(int i=0;i<visLines;i++){
    int lineIdx=aiScrollLine+i;
    if(lineIdx>=aiTotalLines) break;
    lcd.setTextColor(COL_GRAY_C);
    lcd.drawString(aiLines[lineIdx],4,textStartY+i*lineH);
  }

  if(aiTotalLines>visLines){
    int barH=DISP_H-textStartY-14;
    int indH=max(8,barH*visLines/aiTotalLines);
    int indY=textStartY+(barH-indH)*aiScrollLine/max(1,aiTotalLines-visLines);
    lcd.fillRect(DISP_W-3,textStartY,3,barH,COL_GRAY_2);
    lcd.fillRect(DISP_W-3,indY,3,indH,COL_GRAY_7);
  }

  lcd.fillRect(0,DISP_H-12,DISP_W,12,COL_GRAY_D);
  lcd.drawFastHLine(0,DISP_H-12,DISP_W,COL_GRAY_3);
  lcd.setTextColor(COL_GRAY_5);
  char footBuf[52];
  snprintf(footBuf,sizeof(footBuf),"C=^ D=v   B=kembali   %d/%d baris",
           aiScrollLine+1,aiTotalLines);
  lcd.drawString(footBuf,(DISP_W-lcd.textWidth(footBuf))/2,DISP_H-10);
}

// ─────────────────────────────────────────────────────────────────────────────
//  AI — STATUS overlay
// ─────────────────────────────────────────────────────────────────────────────
void drawAIStatus(const char* line1,const char* line2=""){
  int dw=260,dh=50,dx=(DISP_W-dw)/2,dy=(DISP_H-dh)/2;
  lcd.fillRoundRect(dx,dy,dw,dh,8,COL_GRAY_D);
  lcd.drawRoundRect(dx,dy,dw,dh,8,COL_GRAY_5);
  lcd.setFont(&fonts::Font0);lcd.setTextSize(1);
  lcd.setTextColor(COL_AI_ACCENT);
  lcd.drawString(line1,dx+(dw-lcd.textWidth(line1))/2,dy+10);
  if(line2[0]){
    lcd.setTextColor(COL_GRAY_7);
    lcd.drawString(line2,dx+(dw-lcd.textWidth(line2))/2,dy+26);
  }
  esp_task_wdt_reset();
}

// ─────────────────────────────────────────────────────────────────────────────
//  AI — drawAINoConfigScreen
// ─────────────────────────────────────────────────────────────────────────────
void drawAINoConfigScreen(bool missingWifi,bool missingGemini){
  lcd.fillScreen(COL_BLACK);
  lcd.fillRect(0,0,DISP_W,20,COL_GRAY_D);
  lcd.drawFastHLine(0,20,DISP_W,COL_GRAY_3);
  lcd.setFont(&fonts::Font0);lcd.setTextSize(1);
  lcd.setTextColor(COL_GRAY_E);
  const char* title="SETUP AI";
  lcd.drawString(title,(DISP_W-lcd.textWidth(title))/2,6);
  int y=26;
  lcd.setTextColor(COL_GRAY_7);
  lcd.drawString("File config sudah dibuat di SD card.",4,y);y+=12;
  lcd.drawString("Cabut SD, isi file berikut di PC:",4,y);y+=14;
  if(missingWifi){
    lcd.setTextColor(COL_AI_ACCENT);
    lcd.drawString("/sdcard/wifi.ini",4,y);y+=11;
    lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("  ssid=NamaWiFiKamu",4,y);y+=11;
    lcd.drawString("  pass=PasswordKamu",4,y);y+=13;
  }
  if(missingGemini){
    lcd.setTextColor(COL_AI_ACCENT);
    lcd.drawString("/sdcard/gemini.ini",4,y);y+=11;
    lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("  key1=AIzaXXXXXXXXXXXX",4,y);y+=11;
    lcd.drawString("  key2=AIzaXXXX  (opsional)",4,y);y+=11;
    lcd.setTextColor(COL_GRAY_3);
    lcd.drawString("  (dari aistudio.google.com)",4,y);y+=13;
  }
  lcd.drawFastHLine(0,y,DISP_W,COL_GRAY_2);y+=4;
  lcd.setTextColor(COL_GRAY_3);
  lcd.drawString("Setelah diisi, pasang SD & restart.",4,y);y+=12;
  if(missingGemini&&sdReady){
    lcd.setTextColor(COL_AI_ACCENT);
    lcd.drawString("D-long = kelola key di kamera",4,y);
  }
  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("B=kembali",DISP_W-52,DISP_H-10);
}

// ─────────────────────────────────────────────────────────────────────────────
//  AI — Baca foto dari SD sebagai JPEG
// ─────────────────────────────────────────────────────────────────────────────
static uint8_t* aiLoadPhotoAsJpeg(int idx,size_t* outLen){
  char path[56]; snprintf(path,sizeof(path),"/sdcard/%s",galleryFiles[idx]);
  *outLen=0;
  if(gIsJpg(idx)){
    FILE* f=fopen(path,"rb"); if(!f) return nullptr;
    fseek(f,0,SEEK_END);size_t sz=ftell(f);fseek(f,0,SEEK_SET);
    if(sz==0||sz>300000){fclose(f);return nullptr;}
    uint8_t* buf=(uint8_t*)ps_malloc(sz);
    if(!buf) buf=(uint8_t*)malloc(sz);
    if(!buf){fclose(f);return nullptr;}
    fread(buf,1,sz,f);fclose(f);*outLen=sz; return buf;
  }
  if(gIsBmp(idx)){
    uint16_t bw=0,bh=0;
    uint16_t* pixels=loadBMP(path,&bw,&bh);
    if(!pixels||bw==0||bh==0) return nullptr;
    uint8_t* jpgOut=nullptr;size_t jpgLen=0;
    bool ok=fmt2jpg((uint8_t*)pixels,(size_t)bw*bh*2,bw,bh,PIXFORMAT_RGB565,80,&jpgOut,&jpgLen);
    free(pixels);
    if(!ok||!jpgOut) return nullptr;
    *outLen=jpgLen; return jpgOut;
  }
  return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
//  AI — Core HTTP call dengan multi-key fallback
// ─────────────────────────────────────────────────────────────────────────────
bool doAICall(int idx, const char* customPrompt) {
  bool wifiOK   = (wifiSSID[0]!='\0'&&strcmp(wifiSSID,"NamaWiFiKamu")!=0);
  bool geminiOK = (geminiKeyCount>0);
  if(!wifiOK)   wifiOK   = loadWifiConfig();
  if(!geminiOK) geminiOK = loadGeminiConfig();
  if(!wifiOK||!geminiOK){
    resetAllButtons();
    drawAINoConfigScreen(!wifiOK,!geminiOK);
    appMode=MODE_AI_NO_CONFIG;
    return false;
  }

  lcd.fillScreen(COL_BLACK);
  drawAIStatus("CONNECTING","menghubungkan WiFi...");

  if(WiFi.status()!=WL_CONNECTED){
    WiFi.begin(wifiSSID,wifiPass);
    neoSpin(0, 80, 255);
    unsigned long t0=millis();
    while(WiFi.status()!=WL_CONNECTED&&millis()-t0<15000){delay(300);esp_task_wdt_reset();}
    if(WiFi.status()!=WL_CONNECTED){
      drawAIStatus("WiFi gagal","periksa wifi.ini");
      neoBurst(180, 0, 0, 5);
      delay(2500);return false;
    }
    neoOff();
  }

  char featLabel[24];
  strncpy(featLabel,AI_FEATURES[(int)aiSelectedFeature].label,sizeof(featLabel)-1);
  char statusHdr[32]; snprintf(statusHdr,sizeof(statusHdr),"AI: %s",featLabel);

  drawAIStatus(statusHdr,"membaca foto...");
  esp_task_wdt_reset();

  size_t jpgLen=0;
  uint8_t* jpgBuf=aiLoadPhotoAsJpeg(idx,&jpgLen);
  if(!jpgBuf||jpgLen==0){drawAIStatus("gagal baca foto","");delay(2000);return false;}

  drawAIStatus(statusHdr,"encode base64...");
  esp_task_wdt_reset();

  String b64;
  base64Encode(jpgBuf,jpgLen,b64);
  free(jpgBuf);

  String body;
  body.reserve(b64.length()+512);
  body  = "{\"contents\":[{\"parts\":[";
  body += "{\"inline_data\":{\"mime_type\":\"image/jpeg\",\"data\":\"";
  body += b64;
  body += "\"}},";
  body += "{\"text\":\"";
  for(const char* p=customPrompt;*p;p++){
    if(*p=='"') body+="\\\"";
    else if(*p=='\n') body+="\\n";
    else body+=*p;
  }
  body += "\"}";
  body += "]}]}";
  b64="";
  esp_task_wdt_reset();

  int httpCode=0;
  String resp="";

  for(int ki=0;ki<geminiKeyCount;ki++){
    int keyIdx=(geminiKeyActive+ki)%geminiKeyCount;
    char statusMsg[48];
    if(geminiKeyCount>1) snprintf(statusMsg,sizeof(statusMsg),"kirim... (key %d/%d)",keyIdx+1,geminiKeyCount);
    else strncpy(statusMsg,"mengirim ke Gemini...",sizeof(statusMsg)-1);
    drawAIStatus(statusHdr,statusMsg);
    neoBreath(0, 200, 200);
    esp_task_wdt_reset();

    String url=String(GEMINI_URL_BASE)+String(geminiApiKeys[keyIdx]);
    WiFiClientSecure client; client.setInsecure();
    HTTPClient http; http.begin(client,url);
    http.addHeader("Content-Type","application/json");
    http.setTimeout(30000);
    esp_task_wdt_reset();
    httpCode=http.POST(body);
    esp_task_wdt_reset();

    if(httpCode==200){
      neoBurst(0, 220, 0, 1); neoFade(0, 220, 0, 0, 0, 0, 500);
      resp=http.getString(); http.end();
      geminiKeyActive=keyIdx;
      Serial.printf("[GEMINI] key%d berhasil (HTTP 200) feat=%s\n",keyIdx+1,featLabel);
      break;
    }
    if(httpCode==403||httpCode==429){
      http.end();
      Serial.printf("[GEMINI] key%d HTTP %d → fallback\n",keyIdx+1,httpCode);
      if(ki<geminiKeyCount-1){
        neoBurst(200, 180, 0, 2);
        char fbMsg[48];
        snprintf(fbMsg,sizeof(fbMsg),"key%d %s → coba key%d...",
                 keyIdx+1,httpCode==403?"quota":"rate limit",
                 ((geminiKeyActive+ki+1)%geminiKeyCount)+1);
        drawAIStatus("FALLBACK",fbMsg); delay(800); esp_task_wdt_reset();
      }
      continue;
    }
    Serial.printf("[GEMINI] key%d HTTP %d\n",keyIdx+1,httpCode);
    http.end();
    char errBuf[32]; snprintf(errBuf,sizeof(errBuf),"HTTP error %d",httpCode);
    drawAIStatus(errBuf,"cek koneksi internet");
    body=""; delay(3000);return false;
  }

  body="";
  if(httpCode!=200){
    for(int i=0; i<3; i++) { neoBurst(180,0,0,1); neoBurst(180,150,0,1); }
    Serial.printf("[GEMINI] semua %d key gagal\n",geminiKeyCount);
    char failMsg[40]; snprintf(failMsg,sizeof(failMsg),"semua %d key gagal (quota?)",geminiKeyCount);
    drawAIStatus("GAGAL",failMsg); delay(3000);return false;
  }

  drawAIStatus(statusHdr,"parsing respons...");
  esp_task_wdt_reset();

  DynamicJsonDocument doc(8192);
  DeserializationError err=deserializeJson(doc,resp);
  if(err){
    neoBurst(180, 0, 120, 3);
    Serial.printf("[GEMINI] JSON parse error: %s\n",err.c_str());
    drawAIStatus("parse error","respons tidak valid");
    delay(2500);return false;
  }

  const char* text=doc["candidates"][0]["content"]["parts"][0]["text"]|"";
  if(!text||strlen(text)==0){drawAIStatus("respons kosong","");delay(2000);return false;}

  strncpy(aiResult,text,AI_RESULT_MAX);
  aiResult[AI_RESULT_MAX]='\0';
  Serial.printf("[GEMINI] feat=%s hasil: %.80s...\n",featLabel,aiResult);
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  AI — Entry point utama
// ─────────────────────────────────────────────────────────────────────────────
void openAIDescribeWithFeature(int idx, AIFeature feature) {
  aiSelectedFeature = feature;
  aiDescribeBusy = true;
  aiResult[0] = '\0'; aiTotalLines = 0; aiScrollLine = 0;

  const char* prompt = AI_FEATURES[(int)feature].prompt;
  bool ok = doAICall(idx, prompt);
  aiDescribeBusy = false;

  if(ok){
    aiWrapText(aiResult);
    drawAIDescribeScreen();
    resetAllButtons();
    appMode = MODE_AI_DESCRIBE;
    char notifBuf[32];
    snprintf(notifBuf,sizeof(notifBuf),"%s selesai",AI_FEATURES[(int)feature].label);
    islandPush(NOTIF_INFO,notifBuf);
  } else {
    if(appMode==MODE_AI_NO_CONFIG) return;
    resetAllButtons();
    appMode = MODE_PHOTO_VIEW;
    photoViewRender();
    photoViewDrawCaption(photoViewIndex);
    char warnBuf[32];
    snprintf(warnBuf,sizeof(warnBuf),"%s gagal",AI_FEATURES[(int)feature].label);
    islandPush(NOTIF_WARN,warnBuf);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  AI — Mode handler untuk MODE_AI_DESCRIBE
// ─────────────────────────────────────────────────────────────────────────────
void handleModeAIDescribe(ButtonEvent evtB,ButtonEvent evtC,ButtonEvent evtD){
  if(aiDescribeBusy) return;
  bool redraw=false;
  const int visLines=(DISP_H-34-14)/13;
  if(evtC.valid&&evtC.isShort){ if(aiScrollLine>0){aiScrollLine--;redraw=true;} }
  if(evtD.valid&&evtD.isShort){ if(aiScrollLine<aiTotalLines-visLines){aiScrollLine++;redraw=true;} }
  if(evtC.valid&&evtC.isLong){ aiScrollLine=max(0,aiScrollLine-5);redraw=true; }
  if(evtD.valid&&evtD.isLong){ aiScrollLine=min(max(0,aiTotalLines-visLines),aiScrollLine+5);redraw=true; }
  if(redraw) drawAIDescribeScreen();
  if(evtB.valid&&evtB.isShort){
    resetAllButtons();
    appMode=MODE_PHOTO_VIEW;
    photoViewRender();
    photoViewDrawCaption(photoViewIndex);
    photoViewCaptionUntilMs=millis()+2000;
    photoViewCaptionVisible=true;
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  AI — Mode handler untuk MODE_AI_NO_CONFIG
// ─────────────────────────────────────────────────────────────────────────────
void handleModeAINoConfig(ButtonEvent evtB,ButtonEvent evtD){
  if(evtB.valid&&evtB.isShort){
    resetAllButtons();
    appMode=MODE_PHOTO_VIEW;
    photoViewRender();
    photoViewDrawCaption(photoViewIndex);
    photoViewCaptionUntilMs=millis()+2000;
    photoViewCaptionVisible=true;
  }
  if(evtD.valid&&evtD.isLong&&sdReady){
    loadGeminiConfig();
    openKeyManager();
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  VIEWFINDER
// ─────────────────────────────────────────────────────────────────────────────
// UI UPDATE v6.0
void renderViewfinder(){
  bool frozen=(millis()<islandFreezeUntilMs);
  islandNoClear=true;
  camera_fb_t *fb=nullptr;
  if(!frozen){fb=esp_camera_fb_get();if(!fb) return;}
  if(!frozen){
    if(fb->format==PIXFORMAT_RGB565&&fb->width==DISP_W&&fb->height==DISP_H){
      int dw = min((int)fb->width, (int)lcd.width()); int dh = min((int)fb->height, (int)lcd.height()); lcd.pushImage(0, 0, dw, dh, (uint16_t*)fb->buf);
      uint16_t bktCol=recActive?0xF800:(COL_GRAY_E);
      drawCornerBrackets(bktCol);

      if (recActive) {
        lcd.fillCircle(10, 10, 4, (millis()/500%2) ? 0xF800 : 0x4000);
      }

      if (hudEnabled) {
        char fpsBuf[12]; snprintf(fpsBuf,sizeof(fpsBuf),"%.0f fps",fpsValue);
        drawPill(32,10,fpsBuf,COL_PILL_BG,COL_GRAY_A);
        if (!recActive && !g_tilted && !g_shake) neoBreath(0, 0, 80);
        char sensorPill[20];
        snprintf(sensorPill,sizeof(sensorPill),"%s%s",sensorName,ledFlashEnabled?" *":"");
        drawPill(DISP_W-42,10,sensorPill,COL_PILL_BG,COL_GRAY_A);
        if(expPreset>0){
          char expBuf[12];
          if(expPreset==5) snprintf(expBuf,sizeof(expBuf),"M %d",expManualVal);
          else             snprintf(expBuf,sizeof(expBuf),"%s",expPresetNames[expPreset]);
          drawPill(DISP_W/2,10,expBuf,COL_PILL_BG,COL_GRAY_E);
        }
        const char* fmtTag;
        if(detectedSensor==PID_GC2145) fmtTag=(gc2145CaptureFormat==GFMT_BMP)?"BMP":"JPG";
        else fmtTag="JPG";
        char shotBuf[12]; snprintf(shotBuf,sizeof(shotBuf),"#%04d %s",photoCount+1,fmtTag);
        drawPill(38,DISP_H-10,shotBuf,COL_PILL_BG,COL_GRAY_8);
        drawPill(DISP_W-36,DISP_H-10,sdReady?"SD  OK":"SD  --",
                 COL_PILL_BG,sdReady?COL_GRAY_8:COL_GRAY_5);
        lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_3);
        lcd.drawString("Clong=AI", 70, DISP_H-10); lcd.drawString("Dshort=FEAT", 170, DISP_H-10);
        if(recActive) drawRecIndicator();
        if(hdrEnabled) drawPill(DISP_W/2, 35, "HDR", COL_PILL_BG, 0x07E0);
        if(eisEnabled) { char eb[12]; snprintf(eb, sizeof(eb), recActive ? "EIS●" : "EIS"); drawPill(DISP_W/2 - (hdrEnabled ? 45 : 0), 35, eb, COL_PILL_BG, 0xCE59); }
        mpuDrawIndicator();
      }
      updateFPS();
    } else {
      lcd.fillScreen(COL_BLACK);
      lcd.setFont(&fonts::Font0);lcd.setTextColor(COL_GRAY_5);
      lcd.drawString("format not rgb565",10,110);
    }
    esp_camera_fb_return(fb);
  }
  islandTick();
}

// ─────────────────────────────────────────────────────────────────────────────
//  captureAndPreview
// ─────────────────────────────────────────────────────────────────────────────

// [PORTED v6.1] HDR Triple Capture
bool captureHDRFrame(const char* path, int aecVal) {
  sensor_t* s = esp_camera_sensor_get(); if (!s) return false;
  if (detectedSensor == PID_GC2145) {
    gc2145SetAEC(false);
    gc2145SetExposure((uint16_t)constrain(aecVal, 0, 1200));
  } else {
    s->set_exposure_ctrl(s, 0); s->set_aec_value(s, aecVal);
  }
  delay(250); esp_task_wdt_reset();
  for (int i = 0; i < 3; i++) { camera_fb_t* t = esp_camera_fb_get(); if (t) esp_camera_fb_return(t); esp_task_wdt_reset(); }
  camera_fb_t* fb = esp_camera_fb_get(); if (!fb) return false;
  uint8_t* jpg = nullptr; size_t jLen = 0; bool ok = false;
  if (fb->format == PIXFORMAT_RGB565) {
    int captureQ = hdCaptureEnabled ? map(hdCaptureQuality, 10, 1, 50, 95) : 85;
    ok = frame2jpg(fb, captureQ, &jpg, &jLen);
  }
  else if (fb->format == PIXFORMAT_JPEG) { jpg = fb->buf; jLen = fb->len; ok = true; }
  if (ok && jpg) {
    char payload[32]; stegoMakePayload(payload, sizeof(payload), photoCount);
    size_t sLen = 0;
    uint8_t* sBuf = stegoEmbedToJpeg(jpg, jLen, payload, (int)strlen(payload), &sLen);
    if (!sBuf) { sBuf = jpg; sLen = jLen; }
    size_t fLen = 0;
    uint8_t* fBuf = exifInjectToJpeg(sBuf, sLen, photoCount, sensorName, &fLen);
    if (!fBuf) { fBuf = sBuf; fLen = sLen; }
    FILE* f = fopen(path, "wb");
    if (f) { fwrite(fBuf, 1, fLen, f); fclose(f); }
    if (fBuf != sBuf) free(fBuf);
    if (sBuf != jpg) free(sBuf);
    if (fb->format != PIXFORMAT_JPEG) free(jpg);
  }
  esp_camera_fb_return(fb); return ok;
}

void runHDRFlow() {
  lcd.fillScreen(COL_BLACK);
  int cx = DISP_W / 2, cy = DISP_H / 2;
  uint32_t start = millis(); bool failed = false;
  int origVal = expManualVal;
  neoFade(180,0,0, 0,180,0, 3000);
  while (millis() - start < HDR_WAIT_MS) {
    esp_task_wdt_reset(); // [PORTED v6.1] MPU tick
  mpuTick();
    float gmag = sqrtf(g_gyroX * g_gyroX + g_gyroY * g_gyroY + g_gyroZ * g_gyroZ);
    if (gmag > HDR_STABLE_THRESH) { failed = true; break; }
    lcd.setFont(&fonts::FreeSansBold9pt7b); lcd.setTextColor(0x07E0); // COL_ACCENT
    lcd.drawString("HDR CAPTURE", cx - lcd.textWidth("HDR CAPTURE") / 2, cy - 40);
    lcd.setFont(&fonts::Font0); lcd.setTextColor(0x528A); // COL_GRAY_5
    lcd.drawString("tahan kamera diam...", cx - lcd.textWidth("tahan kamera diam...") / 2, cy - 15);
    int barW = 200;
    lcd.drawRect(cx - barW / 2, cy + 25, barW, 8, 0x3186); // COL_GRAY_3
    int prog = (int)(barW * (millis() - start) / (float)HDR_WAIT_MS);
    lcd.fillRect(cx - barW / 2, cy + 25, prog, 8, 0x07E0); // COL_ACCENT
    delay(30);
  }
  if (failed) {
    lcd.setFont(&fonts::FreeSansBold9pt7b); lcd.setTextColor(0xFD20); // COL_WARN
    lcd.drawString("GAGAL - tekan lagi", cx - lcd.textWidth("GAGAL - tekan lagi") / 2, cy);
    delay(1500);
  } else {
    photoCount++;
    char p0[48], p1[48], p2[48];
    snprintf(p0, sizeof(p0), "/sdcard/photo_%04d_hdr0.jpg", photoCount);
    snprintf(p1, sizeof(p1), "/sdcard/photo_%04d_hdr1.jpg", photoCount);
    snprintf(p2, sizeof(p2), "/sdcard/photo_%04d_hdr2.jpg", photoCount);
    captureHDRFrame(p0, 50); captureHDRFrame(p1, origVal); captureHDRFrame(p2, 800);
    applyExpPreset(expPreset);
    char msg[32]; snprintf(msg, sizeof(msg), "HDR OK #%04d x3", photoCount);
    islandPush(NOTIF_OK, msg);
    neoBurst(200, 150, 0, 3);
  }
  lcd.fillScreen(COL_BLACK);
  blockingWaitAllRelease(300);
}

void captureAndPreview() {
  if (ledFlashEnabled) {
    digitalWrite(LED_FLASH, HIGH); if (neoFlashEnabled) neoWhiteFlash(); delay(150);
    for (int i = 0; i < 2; i++) {
      camera_fb_t *tfb = esp_camera_fb_get();
      if (tfb) esp_camera_fb_return(tfb); esp_task_wdt_reset();
    }
  }
  camera_fb_t* fb = esp_camera_fb_get();
  if (ledFlashEnabled) digitalWrite(LED_FLASH, LOW);
  if (!fb) { neoBurst(180, 0, 0, 5); return; }
  neoBurst(0, 80, 255, 1);
  if (fb->format == PIXFORMAT_RGB565 && fb->width == DISP_W) {
    int dw = min((int)fb->width, (int)lcd.width());
    int dh = min((int)fb->height, (int)lcd.height());
    lcd.pushImage(0, 0, dw, dh, (uint16_t*)fb->buf);
  }
  drawCornerBrackets(COL_GRAY_E);
  bool saved = false;
  if (sdReady) {
    photoCount++;
    bool isGCrgb = (detectedSensor == PID_GC2145 && fb->format == PIXFORMAT_RGB565 && fb->width == DISP_W && fb->height == DISP_H);
    if (isGCrgb && gc2145CaptureFormat == GFMT_BMP) {
      char path[48]; snprintf(path, sizeof(path), "/sdcard/photo_%04d.bmp", photoCount);
      char payload[32]; stegoMakePayload(payload, sizeof(payload), photoCount);
      saved = saveBMP(fb->buf, fb->width, fb->height, path, payload, (int)strlen(payload));
    } else {
      uint8_t* jpg = nullptr; size_t jLen = 0; bool ok = false;
      if (fb->format == PIXFORMAT_RGB565) {
        if (eisEnabled) {
          uint16_t* src = (uint16_t*)fb->buf;
          uint16_t* tmp = (uint16_t*)ps_malloc(DISP_W * DISP_H * 2);
          if (!tmp) tmp = (uint16_t*)malloc(DISP_W * DISP_H * 2);
          if (tmp) {
            int sx = constrain((int)g_eisOffX, 0, DISP_W - EIS_VP_W); int sy = constrain((int)g_eisOffY, 0, DISP_H - EIS_VP_H);
            for (int y = 0; y < DISP_H; y++) {
              int srY = sy + (y * EIS_VP_H / DISP_H);
              for (int x = 0; x < DISP_W; x++) {
                int srX = sx + (x * EIS_VP_W / DISP_W);
                tmp[y * DISP_W + x] = src[srY * DISP_W + srX];
              }
            }
            camera_fb_t fk = *fb; fk.buf = (uint8_t*)tmp; fk.width = DISP_W; fk.height = DISP_H;
            int captureQ = hdCaptureEnabled ? map(hdCaptureQuality, 10, 1, 50, 95) : 85;
            ok = frame2jpg(&fk, captureQ, &jpg, &jLen); free(tmp);
          }
        } else { int captureQ = hdCaptureEnabled ? map(hdCaptureQuality, 10, 1, 50, 95) : 85;
        ok = frame2jpg(fb, captureQ, &jpg, &jLen); }
      } else if (fb->format == PIXFORMAT_JPEG) { jpg = fb->buf; jLen = fb->len; ok = true; }
      if (ok && jpg && jLen > 0) {
        char payload[32]; stegoMakePayload(payload, sizeof(payload), photoCount);
        size_t sLen = 0;
        uint8_t* sBuf = stegoEmbedToJpeg(jpg, jLen, payload, (int)strlen(payload), &sLen);
        if (!sBuf) { sBuf = jpg; sLen = jLen; }
        size_t fLen = 0;
        uint8_t* fBuf = exifInjectToJpeg(sBuf, sLen, photoCount, sensorName, &fLen);
        if (!fBuf) { fBuf = sBuf; fLen = sLen; }
        char path[48]; snprintf(path, sizeof(path), "/sdcard/photo_%04d.jpg", photoCount);
        FILE* f = fopen(path, "wb");
        if (f) { size_t w = fwrite(fBuf, 1, fLen, f); fclose(f); saved = (w == fLen); }
        if (fBuf != sBuf) free(fBuf);
        if (sBuf != jpg) free(sBuf);
        if (fb->format != PIXFORMAT_JPEG) free(jpg);
      }
    }
    if (!saved) photoCount--;
  }
  esp_camera_fb_return(fb);
  if (saved) {
    char msg[28];
    bool isBmp = (detectedSensor == PID_GC2145 && gc2145CaptureFormat == GFMT_BMP);
    snprintf(msg, sizeof(msg), "SAVED %s #%04d", isBmp ? "BMP" : "JPG", photoCount);
    islandPush(NOTIF_OK, msg);
    neoBurst(200, 150, 0, 3); if (isBmp) neoBurst(0, 80, 200, 2); else neoBurst(0, 180, 0, 2);
  } else {
    islandPush(NOTIF_WARN, sdReady ? "WRITE ERR" : "NO SD CARD"); neoBurst(180, 0, 0, 5);
  }
  fpsLastTime = millis(); fpsFrameCount = 0;
  resetAllButtons();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Sensor settings + Camera init
// ─────────────────────────────────────────────────────────────────────────────
void applySensorSettings(sensor_t *s,uint16_t pid){
  s->set_brightness(s,0);s->set_contrast(s,0);s->set_saturation(s,0);
  s->set_special_effect(s,0);s->set_whitebal(s,1);s->set_awb_gain(s,1);
  s->set_wb_mode(s,0);s->set_exposure_ctrl(s,1);s->set_aec2(s,1);
  s->set_ae_level(s,0);s->set_gain_ctrl(s,1);s->set_agc_gain(s,0);
  if(pid==PID_GC2145){
    s->set_aec_value(s,300);s->set_gainceiling(s,GAINCEILING_4X);
    s->set_hmirror(s,HMIRROR_GC2145);s->set_vflip(s,VFLIP_GC2145);
  } else if(pid==PID_OV3660){
    s->set_aec_value(s,400);s->set_gainceiling(s,GAINCEILING_4X);
    s->set_hmirror(s,HMIRROR_OV3660);s->set_vflip(s,VFLIP_OV3660);
  } else {
    s->set_aec_value(s,300);s->set_gainceiling(s,GAINCEILING_2X);
    s->set_hmirror(s,0);s->set_vflip(s,0);
  }
}

bool initCamera(){
  camera_config_t cfg;
  cfg.ledc_channel=LEDC_CHANNEL_0;cfg.ledc_timer=LEDC_TIMER_0;
  cfg.pin_d0=Y2_GPIO_NUM;cfg.pin_d1=Y3_GPIO_NUM;cfg.pin_d2=Y4_GPIO_NUM;cfg.pin_d3=Y5_GPIO_NUM;
  cfg.pin_d4=Y6_GPIO_NUM;cfg.pin_d5=Y7_GPIO_NUM;cfg.pin_d6=Y8_GPIO_NUM;cfg.pin_d7=Y9_GPIO_NUM;
  cfg.pin_xclk=XCLK_GPIO_NUM;cfg.pin_pclk=PCLK_GPIO_NUM;
  cfg.pin_vsync=VSYNC_GPIO_NUM;cfg.pin_href=HREF_GPIO_NUM;
  cfg.pin_sccb_sda=SIOD_GPIO_NUM;cfg.pin_sccb_scl=SIOC_GPIO_NUM;
  cfg.pin_pwdn=PWDN_GPIO_NUM;cfg.pin_reset=RESET_GPIO_NUM;
  cfg.pixel_format=PIXFORMAT_RGB565;cfg.frame_size=FRAMESIZE_QVGA;
  cfg.grab_mode=CAMERA_GRAB_LATEST;
  if(psramFound()){cfg.jpeg_quality=7;cfg.fb_count=2;cfg.fb_location=CAMERA_FB_IN_PSRAM;}
  else{cfg.jpeg_quality=12;cfg.fb_count=1;cfg.fb_location=CAMERA_FB_IN_DRAM;}
  cfg.xclk_freq_hz=20000000;
  if(esp_camera_init(&cfg)!=ESP_OK) return false;
  sensor_t *s=esp_camera_sensor_get();if(!s) return false;
  detectedSensor=s->id.PID;
  if(detectedSensor==PID_OV3660){
    esp_camera_deinit();delay(100);cfg.xclk_freq_hz=24000000;
    if(esp_camera_init(&cfg)!=ESP_OK){
      cfg.xclk_freq_hz=20000000;
      if(esp_camera_init(&cfg)!=ESP_OK) return false;
    }
    s=esp_camera_sensor_get();if(!s) return false;
  }
  switch(detectedSensor){
    case PID_GC2145: strncpy(sensorName,"GC2145",sizeof(sensorName));break;
    case PID_OV3660: strncpy(sensorName,"OV3660",sizeof(sensorName));break;
    default: snprintf(sensorName,sizeof(sensorName),"0x%04X",detectedSensor);break;
  }
  applySensorSettings(s,detectedSensor);
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  JUMP INPUT DIALOG
// ─────────────────────────────────────────────────────────────────────────────
void drawJumpDialog(){
  const int dw=240,dh=100,dx=(DISP_W-dw)/2,dy=(DISP_H-dh)/2;
  lcd.fillRoundRect(dx,dy,dw,dh,10,COL_GRAY_D);
  lcd.drawRoundRect(dx,dy,dw,dh,10,COL_GRAY_5);
  lcd.drawRoundRect(dx+1,dy+1,dw-2,dh-2,9,COL_GRAY_3);
  lcd.setFont(&fonts::Font0);lcd.setTextSize(1);
  lcd.setTextColor(COL_GRAY_E);
  const char* title="GO TO INDEX";
  lcd.drawString(title,dx+(dw-lcd.textWidth(title))/2,dy+7);
  lcd.drawFastHLine(dx+10,dy+19,dw-20,COL_GRAY_3);
  const int boxW=26,boxH=28,boxGap=6;
  const int totalBoxW=JUMP_DIGIT_COUNT*boxW+(JUMP_DIGIT_COUNT-1)*boxGap;
  int boxStartX=dx+(dw-totalBoxW)/2;
  const int boxY=dy+24;
  for(int i=0;i<JUMP_DIGIT_COUNT;i++){
    int bx=boxStartX+i*(boxW+boxGap);
    bool isCursor=(i==jumpCursorPos);
    lcd.fillRoundRect(bx,boxY,boxW,boxH,4,isCursor?COL_GRAY_5:COL_GRAY_2);
    lcd.drawRoundRect(bx,boxY,boxW,boxH,4,isCursor?COL_WHITE:COL_GRAY_3);
    char dBuf[2]; snprintf(dBuf,sizeof(dBuf),"%d",jumpDigits[i]);
    lcd.setFont(&fonts::FreeSansBold9pt7b);
    int tw=lcd.textWidth(dBuf);
    lcd.setTextColor(isCursor?COL_WHITE:COL_GRAY_A);
    lcd.drawString(dBuf,bx+(boxW-tw)/2,boxY+6);
    if(isCursor){
      int cx2=bx+boxW/2,cy2=boxY+boxH+3;
      lcd.fillTriangle(cx2-4,cy2,cx2+4,cy2,cx2,cy2+5,COL_GRAY_A);
    }
  }
  lcd.setFont(&fonts::Font0);lcd.setTextSize(1);
  lcd.setTextColor(COL_GRAY_5);
  char totalBuf[12]; snprintf(totalBuf,sizeof(totalBuf),"/ %d",galleryCount);
  lcd.drawString(totalBuf,boxStartX+totalBoxW+8,boxY+10);
  lcd.drawFastHLine(dx+10,dy+dh-22,dw-20,COL_GRAY_2);
  lcd.setTextColor(COL_GRAY_5);
  const char* hint="C=- D=+  B=kursor  BOOT=ok";
  lcd.drawString(hint,dx+(dw-lcd.textWidth(hint))/2,dy+dh-18);
  unsigned long elapsed=millis()-jumpOpenMs;
  int barW=dw-20,prog=barW-(int)((long)barW*elapsed/JUMP_TIMEOUT_MS);
  prog=constrain(prog,0,barW);
  lcd.fillRect(dx+10,dy+dh-6,barW,3,COL_GRAY_2);
  lcd.fillRect(dx+10,dy+dh-6,prog,3,COL_GRAY_5);
}

void openJumpDialog(){
  jumpResetToCurrentIndex();jumpOpenMs=millis();jumpActive=true;
  drawJumpDialog();resetAllButtons();appMode=MODE_JUMP_INPUT;
}

void handleModeJumpInput(ButtonEvent evtBoot,ButtonEvent evtB,
                          ButtonEvent evtC,ButtonEvent evtD){
  if(millis()-jumpOpenMs>=JUMP_TIMEOUT_MS){
    jumpActive=false;lcd.setRotation(3); resetAllButtons();appMode=MODE_GALLERY; if(galleryGridMode) drawGalleryGrid(); else drawGallery();
    islandPush(NOTIF_INFO,"JUMP BATAL (timeout)");return;
  }
  bool redraw=false;
  if(evtD.valid&&evtD.isShort){jumpDigits[jumpCursorPos]=(jumpDigits[jumpCursorPos]+1)%10;redraw=true;}
  if(evtC.valid&&evtC.isShort){jumpDigits[jumpCursorPos]=(jumpDigits[jumpCursorPos]+9)%10;redraw=true;}
  if(evtB.valid&&evtB.isShort){jumpCursorPos=(jumpCursorPos+1)%JUMP_DIGIT_COUNT;redraw=true;}
  if(evtB.valid&&evtB.isLong){jumpCursorPos=(jumpCursorPos+JUMP_DIGIT_COUNT-1)%JUMP_DIGIT_COUNT;redraw=true;}
  if(evtBoot.valid&&evtBoot.isShort){
    int target=jumpGetValue();jumpActive=false;
    if(target<1||target>galleryCount){
      lcd.setRotation(3); resetAllButtons();appMode=MODE_GALLERY; if(galleryGridMode) drawGalleryGrid(); else drawGallery();
      char warnBuf[32]; snprintf(warnBuf,sizeof(warnBuf),"INDEX %d diluar range",target);
      islandPush(NOTIF_WARN,warnBuf);
    } else {
      int newIdx=constrain(target-1,0,galleryCount-1);
      gallerySelIdx=newIdx;
      galleryScroll=(newIdx/GALLERY_ITEMS_PAGE)*GALLERY_ITEMS_PAGE;
      galleryHoldDir=0;
      lcd.setRotation(3); resetAllButtons();appMode=MODE_GALLERY; if(galleryGridMode) drawGalleryGrid(); else drawGallery();
      char infoBuf[24]; snprintf(infoBuf,sizeof(infoBuf),"JUMP #%d",target);
      islandPush(NOTIF_INFO,infoBuf);
    }
    return;
  }
  if(evtBoot.valid&&evtBoot.isLong){
    jumpActive=false;lcd.setRotation(3); resetAllButtons();appMode=MODE_GALLERY; if(galleryGridMode) drawGalleryGrid(); else drawGallery();
    islandPush(NOTIF_INFO,"JUMP BATAL");return;
  }
  if(redraw) drawJumpDialog();
  unsigned long elapsed=millis()-jumpOpenMs;
  int barW=240-20,prog=barW-(int)((long)barW*elapsed/JUMP_TIMEOUT_MS);
  prog=constrain(prog,0,barW);
  int dx=(DISP_W-240)/2,dy=(DISP_H-100)/2;
  lcd.fillRect(dx+10,dy+100-6,barW,3,COL_GRAY_2);
  lcd.fillRect(dx+10,dy+100-6,prog,3,COL_GRAY_5);
  esp_task_wdt_reset();
}

// ─────────────────────────────────────────────────────────────────────────────
//  BOOT ANIMATION
// ─────────────────────────────────────────────────────────────────────────────
static const char GLITCH_CHARS[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#@$%&*!?";
static const int  GLITCH_CHAR_COUNT=sizeof(GLITCH_CHARS)-1;
char glitchRandChar(){return GLITCH_CHARS[random(GLITCH_CHAR_COUNT)];}

void glitchScrambleText(const char* target,int cx,int cy,uint16_t col,int steps,int stepDelayMs){
  int len=strlen(target);
  char buf[16];memset(buf,0,sizeof(buf));
  for(int step=0;step<=len;step++){
    for(int i=0;i<len;i++) buf[i]=(i<step)?target[i]:glitchRandChar();
    buf[len]='\0';
    lcd.fillRect(cx-70,cy-8,140,16,COL_BLACK);
    lcd.setFont(&fonts::FreeSansBold9pt7b);
    int tw=lcd.textWidth(buf);
    lcd.setTextColor(col); lcd.drawString(buf,cx-tw/2,cy-7);
    for(int r=0;r<stepDelayMs/10;r++){delay(10);esp_task_wdt_reset();}
  }
}

void glitchBarsFlash(int count,int delayMs){
  for(int i=0;i<count;i++){
    int barY=random(DISP_H),barX=random(DISP_W-60);
    int barW=random(30,120),barH=(random(10)>7)?5:2;
    uint16_t barCol=(random(3)==0)?COL_GRAY_5:COL_GRAY_3;
    lcd.fillRect(barX,barY,barW,barH,barCol);
    delay(delayMs);esp_task_wdt_reset();
    lcd.fillRect(barX,barY,barW,barH,COL_BLACK);
  }
}

void glitchChromatic(const char* text,int cx,int cy,int offsetX,uint16_t colR,uint16_t colB){
  lcd.setFont(&fonts::FreeSansBold9pt7b);
  int tw=lcd.textWidth(text);
  lcd.setTextColor(colR);lcd.drawString(text,cx-tw/2-offsetX,cy-7);
  lcd.setTextColor(colB);lcd.drawString(text,cx-tw/2+offsetX,cy-7);
}


void runBootSequence(bool sdOK,uint64_t sdMB,bool pidOK,uint16_t pid,
                     bool xclkOK,uint32_t xclkHz){
  lcd.fillScreen(COL_BLACK);
  int cx=DISP_W/2,cy=DISP_H/2-10;
  lcd.setFont(&fonts::FreeSansBold9pt7b);
  char scramBuf[10];
  for(int i=0;i<8;i++) scramBuf[i]=glitchRandChar();
  scramBuf[8]='\0';
  int tw0=lcd.textWidth(scramBuf);
  lcd.setTextColor(COL_GRAY_5);lcd.drawString(scramBuf,cx-tw0/2,cy-7);
  delay(80);esp_task_wdt_reset();
  glitchBarsFlash(6,15);
  glitchScrambleText("SANZXCAM",cx,cy,COL_GRAY_C,8,50);
  glitchChromatic("SANZXCAM",cx,cy,3,0xF000,0x001F);
  delay(60);esp_task_wdt_reset();
  lcd.fillRect(cx-72,cy-10,144,20,COL_BLACK);
  lcd.setFont(&fonts::FreeSansBold9pt7b);lcd.setTextColor(COL_GRAY_E);
  int tw1=lcd.textWidth("SANZXCAM");
  lcd.drawString("SANZXCAM",cx-tw1/2,cy-7);
  delay(400);esp_task_wdt_reset();
  glitchBarsFlash(4,20);
  lcd.setFont(&fonts::Font0);lcd.setTextSize(1);lcd.setTextColor(COL_GRAY_5);
  const char* sub="ESP32-S3  CAMERA SYSTEM";
  lcd.drawString(sub,cx-lcd.textWidth(sub)/2,cy+10);
  lcd.setTextColor(COL_GRAY_3);
  const char* ver="v5.9-fix5";
  lcd.drawString(ver,cx-lcd.textWidth(ver)/2,cy+22);
  lcd.drawFastHLine(cx-80,cy+32,160,COL_GRAY_2);
  delay(600);esp_task_wdt_reset();
  glitchBarsFlash(3,25);
  glitchChromatic("SANZXCAM",cx,cy,2,0xA000,0x000F);
  delay(40);esp_task_wdt_reset();
  lcd.fillRect(cx-72,cy-10,144,20,COL_BLACK);
  lcd.setFont(&fonts::FreeSansBold9pt7b);lcd.setTextColor(COL_GRAY_E);
  lcd.drawString("SANZXCAM",cx-tw1/2,cy-7);
  delay(800);esp_task_wdt_reset();
  for(int stripe=0;stripe<DISP_H;stripe+=8){
    lcd.fillRect(0,stripe,DISP_W,8,COL_BLACK);delay(10);esp_task_wdt_reset();
  }

}

// ─────────────────────────────────────────────────────────────────────────────
//  USB mode screen
// ─────────────────────────────────────────────────────────────────────────────
void drawUSBIcon(int cx,int cy,uint16_t col){
  lcd.drawFastVLine(cx,cy-24,28,col);lcd.drawCircle(cx,cy-27,4,col);
  lcd.drawFastHLine(cx-18,cy-12,36,col);lcd.drawFastVLine(cx-18,cy-12,12,col);
  lcd.drawCircle(cx-18,cy+2,4,col);lcd.drawFastVLine(cx+18,cy-12,10,col);
  lcd.drawRect(cx+13,cy-2,10,7,col);
}

void drawUSBModeScreen(){
  lcd.fillScreen(COL_BLACK);
  lcd.fillRect(0,0,DISP_W,18,COL_GRAY_D);
  lcd.drawFastHLine(0,18,DISP_W,COL_GRAY_3);
  lcd.setFont(&fonts::Font0);
  lcd.setTextColor(COL_GRAY_3);lcd.drawString("ESP32-S3",6,4);
  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("USB MASS STORAGE",(DISP_W-lcd.textWidth("USB MASS STORAGE"))/2,4);
  lcd.setTextColor(COL_GRAY_3);lcd.drawString("v5.9-fix5",DISP_W-52,4);
  drawUSBIcon(DISP_W/2,85,COL_GRAY_7);
  lcd.setFont(&fonts::FreeSansBold9pt7b);lcd.setTextColor(COL_GRAY_E);
  lcd.drawString("SD CONNECTED",(DISP_W-lcd.textWidth("SD CONNECTED"))/2,118);
  lcd.setFont(&fonts::Font0);lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("to host via USB",(DISP_W-lcd.textWidth("to host via USB"))/2,136);
  lcd.drawFastHLine(40,150,DISP_W-80,COL_GRAY_2);
  auto infoRow=[&](int y,const char* key,const char* val){
    lcd.setFont(&fonts::Font0);lcd.setTextColor(COL_GRAY_5);lcd.drawString(key,44,y);
    lcd.setTextColor(COL_GRAY_C);lcd.drawString(val,DISP_W-44-lcd.textWidth(val),y);
  };
  char buf[32];
  infoRow(156,"SENSOR",sensorName);
  if(sdSizeMB<1024) snprintf(buf,sizeof(buf),"%lluMB  FAT32",sdSizeMB);
  else              snprintf(buf,sizeof(buf),"%lluGB  FAT32",sdSizeMB/1024);
  infoRow(170,"STORAGE",buf);
  snprintf(buf,sizeof(buf),"%04d files",photoCount);
  infoRow(184,"PHOTOS",buf);
  lcd.fillRect(0,DISP_H-18,DISP_W,18,COL_GRAY_D);
  lcd.drawFastHLine(0,DISP_H-18,DISP_W,COL_GRAY_3);
  lcd.setFont(&fonts::Font0);lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("eject from host first",(DISP_W-lcd.textWidth("eject from host first"))/2,DISP_H-15);
  lcd.drawRoundRect((DISP_W/2)-68,DISP_H-32,136,13,2,COL_GRAY_3);
  lcd.drawString("[ BOOT ]  exit usb mode",(DISP_W-lcd.textWidth("[ BOOT ]  exit usb mode"))/2,DISP_H-30);
}

void enterUSBMode(){
  neoBreath(0, 80, 255);
  if(!sdReady) return;
  if(photoPixelBuf){free(photoPixelBuf);photoPixelBuf=nullptr;}
  islandForceHide();
  unmountVFSOnly();sdReady=false;
  WiFi.disconnect(true);
  esp_task_wdt_reset();msc.mediaPresent(true);esp_task_wdt_reset();
  drawUSBModeScreen();usbModeActive=true;
  resetAllButtons();
}

void exitUSBMode(){
  usbModeActive=false;msc.mediaPresent(false);esp_task_wdt_reset();
  lcd.fillScreen(COL_BLACK);lcd.setFont(&fonts::Font0);lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("reconnecting sd...",(DISP_W-lcd.textWidth("reconnecting sd..."))/2,DISP_H/2-6);
  sdReady=remountVFSOnly();
  if(sdReady){
    scanPhotoCount();loadSettings();loadGeminiConfig();applyExpPreset(expPreset);
    lcd.fillRect(0,DISP_H/2-10,DISP_W,20,COL_BLACK);
    char buf2[32]; snprintf(buf2,sizeof(buf2),"sd ok  next #%04d",photoCount+1);
    lcd.setTextColor(COL_GRAY_C);lcd.drawString(buf2,(DISP_W-lcd.textWidth(buf2))/2,DISP_H/2-6);
  } else {
    lcd.fillRect(0,DISP_H/2-10,DISP_W,20,COL_BLACK);
    lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("sd mount failed",(DISP_W-lcd.textWidth("sd mount failed"))/2,DISP_H/2-6);
  }
  neoOff();
  lcd.fillScreen(COL_BLACK);resetAllButtons();
  appMode=MODE_VIEWFINDER;
  fpsLastTime=millis();fpsFrameCount=0;fpsValue=0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Setup
// ─────────────────────────────────────────────────────────────────────────────


void neoSetup() {
  strip.begin();
  strip.setBrightness(80);
  for(int i=0; i<NEO_NUM; i++) strip.setPixelColor(i, 0, 0, 0);
  strip.show();
}
void neoSolid(uint8_t r, uint8_t g, uint8_t b) {
  g_neoMode = NEO_MODE_SOLID; g_neoR = r; g_neoG = g; g_neoB = b;
  strip.setPixelColor(0, r, g, b); strip.show();
}
void neoOff() {
  g_neoMode = NEO_MODE_OFF; strip.setPixelColor(0, 0, 0, 0); strip.show();
}
void neoBurst(uint8_t r, uint8_t g, uint8_t b, int times) {
  g_neoMode = NEO_MODE_OFF;
  for (int i = 0; i < times; i++) {
    strip.setPixelColor(0, r, g, b); strip.show(); delay(150);
    strip.setPixelColor(0, 0, 0, 0); strip.show(); if(i<times-1) delay(100);
    esp_task_wdt_reset();
  }
}

void neoWhiteFlash() {
  strip.setPixelColor(0, 255, 255, 255); strip.show();
  delay(150);
  strip.setPixelColor(0, 0, 0, 0); strip.show();
  g_neoMode = NEO_MODE_OFF;
}
void neoPulse(uint8_t r, uint8_t g, uint8_t b) {
  g_neoMode = NEO_MODE_PULSE; g_neoR = r; g_neoG = g; g_neoB = b;
}
void neoBreath(uint8_t r, uint8_t g, uint8_t b) {
  g_neoMode = NEO_MODE_BREATH; g_neoR = r; g_neoG = g; g_neoB = b;
}
void neoSpin(uint8_t r, uint8_t g, uint8_t b) {
  g_neoMode = NEO_MODE_SPIN; g_neoR = r; g_neoG = g; g_neoB = b;
}
void neoFade(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, uint32_t durationMs) {
  g_neoMode = NEO_MODE_OFF;
  uint32_t start = millis();
  while (millis() - start < durationMs) {
    float p = (float)(millis() - start) / (float)durationMs;
    strip.setPixelColor(0, r1+(r2-r1)*p, g1+(g2-g1)*p, b1+(b2-b1)*p);
    strip.show(); esp_task_wdt_reset(); delay(10);
  }
  strip.setPixelColor(0, r2, g2, b2); strip.show();
}
void neoRainbow() {
  g_neoMode = NEO_MODE_OFF;
  for (int i=0; i<255; i++) {
    uint32_t c = strip.ColorHSV((uint16_t)i * 257, 255, 255);
    strip.setPixelColor(0, strip.gamma32(c));
    strip.show(); delay(5); esp_task_wdt_reset();
  }
}
void neoTick() {
  uint32_t now = millis();
  if (recActive) {
    if ((now / 1000) % 2 == 0) { strip.setPixelColor(0, 180, 0, 0); }
    else { strip.setPixelColor(0, 60, 0, 0); }
    strip.show(); return;
  }
  if (g_neoMode == NEO_MODE_OFF || g_neoMode == NEO_MODE_SOLID) return;
  if (g_neoMode == NEO_MODE_BREATH) {
    float v = (exp(sin(now/1500.0*PI)) - 0.36787944) * 0.42545906;
    strip.setPixelColor(0, g_neoR*v, g_neoG*v, g_neoB*v); strip.show();
  } else if (g_neoMode == NEO_MODE_PULSE) {
    float v = (sin(now/1000.0*PI) + 1.0) / 2.0;
    strip.setPixelColor(0, g_neoR*v, g_neoG*v, g_neoB*v); strip.show();
  } else if (g_neoMode == NEO_MODE_SPIN) {
    if (now - g_neoLastMs > 100) {
      g_neoLastMs = now; g_neoState = !g_neoState;
      if (g_neoState) strip.setPixelColor(0, g_neoR, g_neoG, g_neoB);
      else strip.setPixelColor(0, 0, 0, 0);
      strip.show();
    }
  }
}

void setup(){
  Serial.begin(115200);
  Serial.println("\n=== Sanzxcam v5.9-fix5 ===");
  Serial.println("[AI-MENU] 7 fitur: Describe/Scavenger/Mood/ANPR/Sky/Pest/Produce");
  Serial.println("[TRIGGER] Clong=viewfinder  Dlong=gallery  Dlong=photo view");

  galleryFiles   =(char(*)[32])     ps_malloc(GALLERY_MAX_FILES*32);
  galleryFileType=(GalleryFileType*)ps_malloc(GALLERY_MAX_FILES*sizeof(GalleryFileType));
  if(!galleryFiles||!galleryFileType){Serial.println("PSRAM alloc failed!");ESP.restart();}

  neoSetup();
  pinMode(LED_FLASH,OUTPUT);digitalWrite(LED_FLASH,LOW);
  pinMode(BTN_BOOT, INPUT_PULLUP);
  pinMode(BTN_B,    INPUT_PULLUP);
  pinMode(BTN_C,    INPUT_PULLUP);
  pinMode(BTN_D,    INPUT_PULLUP);

  setCpuFrequencyMhz(240);
  lcd.init();lcd.setRotation(3);lcd.fillScreen(COL_BLACK);

  static uint16_t touchCalData[8]={3851,3630,673,3277,3965,160,772,136};
  lcd.setTouchCalibrate(touchCalData);

  TJpgDec.setJpgScale(1);TJpgDec.setSwapBytes(true);TJpgDec.setCallback(tjpgdecOutput);

  sdReady=mountSDFull();
  if(sdReady) { neoSolid(0, 80, 0); delay(3000); neoOff(); }
  else neoPulse(200, 80, 0);
  if(sdReady && sdTotalSectors > 0 && sdTotalSectors < 1000) neoPulse(180, 0, 0);
  if(sdReady){
    scanPhotoCount();
    scanVideoCount();
    loadSettings();
    loadMPUCalibration();
    loadWifiConfig();
    loadGeminiConfig();
  }

  msc.vendorID("ESP32S3");msc.productID("SD Card");msc.productRevision("1.0");
  msc.onRead(onRead);msc.onWrite(onWrite);
  msc.begin(sdTotalSectors>0?sdTotalSectors:0,512);
  msc.mediaPresent(false);USB.begin();

  bool camOK=initCamera();
  // [PORTED v6.1] MPU init
  Wire.begin(PIN_MPU_SDA, PIN_MPU_SCL);
  g_mpuOk = mpu.begin(MPU_I2C_ADDR, &Wire);
  if (g_mpuOk) {
    mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    applyDLPF();

    // Calibrate accelerometer offset
    g_accOffX = 0.0f;
    g_accOffY = 0.0f;
    g_accOffZ = 0.0f;

    if (g_mpuCalLoaded) {
      Serial.printf("[MPU] Skip gyro calibration, using SD: X=%.5f Y=%.5f Z=%.5f\n", g_gyroCalX, g_gyroCalY, g_gyroCalZ);
    } else {
      g_gyroCalX = 0.0f; g_gyroCalY = 0.0f; g_gyroCalZ = 0.0f;
      Serial.println("[MPU] No cal file found. Gyro bias set to 0. Use CALIBRATE menu.");
    }
  } esp_task_wdt_reset();
  Serial.printf("[MPU] %s\n", g_mpuOk ? "OK" : "FAIL");
  g_eisOffX = EIS_CROP_X; g_eisOffY = EIS_CROP_Y;
  bool pidOK=(detectedSensor==PID_GC2145||detectedSensor==PID_OV3660);
  uint32_t xclkHz=(detectedSensor==PID_OV3660)?24000000:20000000;

  if(camOK&&sdReady) applyExpPreset(expPreset);

  runBootSequence(sdReady,sdSizeMB,pidOK,detectedSensor,camOK,xclkHz);

  if(!camOK){
    lcd.fillScreen(COL_BLACK);lcd.setFont(&fonts::Font0);lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("camera init failed",(DISP_W-lcd.textWidth("camera init failed"))/2,110);
    while(true){neoPulse(180, 0, 0); neoTick(); delay(10); esp_task_wdt_reset();}
  }

  lcd.fillScreen(COL_BLACK);
  fpsLastTime=millis();fpsFrameCount=0;
  neoOff();
  blockingWaitAllRelease(600);
  Serial.println("[BOOT] Fixes applied: gyro-cal, no-autokal, hd-quality-map");
  resetAllButtons();
}

// ─────────────────────────────────────────────────────────────────────────────
//  MODE HANDLERS
// ─────────────────────────────────────────────────────────────────────────────
void handleModeViewfinder(ButtonEvent evt){
  galleryGridActive=false;
  if(!evt.valid){renderViewfinder();return;}
  if(evt.pin==BTN_BOOT){
    if(evt.isLong){if(sdReady) enterUSBMode();}
    else {
      if (hdrEnabled) {
        if (!g_hdrWaiting) {
          g_hdrWaiting = true; g_hdrFirstMs = millis();
        } else if (millis() - g_hdrFirstMs < HDR_DOUBLE_TAP_MS) {
          g_hdrWaiting = false; captureAndPreview();
        }
      } else {
        captureAndPreview();
      }
    }
  }
  else if(evt.pin==BTN_B){
    if(evt.isShort){if(recActive) stopRecording();else startRecording();}
    else           {openLedMenu();}
  }
  else if(evt.pin==BTN_C){
    if(evt.isShort){
      if(sdReady){
        scanGalleryFiles();
        gallerySelIdx=0;galleryScroll=0;galleryHoldDir=0;
        islandForceHide();islandNoClear=false;
        lcd.setRotation(3); resetAllButtons();appMode=MODE_GALLERY; if(galleryGridMode) drawGalleryGrid(); else drawGallery();
      }
    }
    else{
      if(!sdReady){islandPush(NOTIF_WARN,"SD tidak tersedia");return;}
      islandForceHide();
      openAIFeatureMenu(true);
    }
  }
  else if(evt.pin==BTN_D){
    if(evt.isShort){ Serial.println("[UI] Opening Features Menu"); lcd.setRotation(3); menuFeatSel=0; drawFeaturesMenu(menuFeatSel); resetAllButtons(); appMode=MODE_FEATURES; }
    else           {openExpMenu();}
  }
}

// UI UPDATE v6.0
void handleModeGallery(ButtonEvent evt){
  bool cHeld=btnC.isHeld(),dHeld=btnD.isHeld();
  if(cHeld&&galleryHoldDir!=-1){
    galleryHoldDir=-1;galleryHoldStart=millis();
    galleryLastStep=millis();galleryStep(-1);return;
  }
  if(dHeld&&galleryHoldDir!=1){
    galleryHoldDir=1;galleryHoldStart=millis();
    galleryLastStep=millis();galleryStep(1);return;
  }
  if(!cHeld&&!dHeld) galleryHoldDir=0;
  if(galleryHoldDir!=0){
    uint32_t held=millis()-galleryHoldStart;
    uint32_t interval=(held<500)?200:(held<1500)?100:50;
    if(millis()-galleryLastStep>=interval){galleryStep(galleryHoldDir);galleryLastStep=millis();}
    return;
  }
  if(!evt.valid) return;
  if(evt.pin==BTN_BOOT){
    if(evt.isLong){
      if(galleryCount>0) openJumpDialog();
      else islandPush(NOTIF_WARN,"GALLERY KOSONG");
    } else if(evt.isShort){
      if(galleryCount>0&&gallerySelIdx>=0&&gallerySelIdx<galleryCount){
        GalleryFileType ft=galleryFileType[gallerySelIdx];
        if(ft==GFILE_VIDEO) openMjpegPlayer(galleryFiles[gallerySelIdx]);
        else                showPhotoView(gallerySelIdx);
      }
    }
  }
  else if(evt.pin==BTN_B){
    if(evt.isLong){
      galleryGridMode=!galleryGridMode;
      galleryScroll=(gallerySelIdx / (galleryGridMode ? 16 : GALLERY_ITEMS_PAGE)) * (galleryGridMode ? 16 : GALLERY_ITEMS_PAGE);
      if(galleryGridMode) drawGalleryGrid(); else drawGallery();
    } else {
      if(photoPixelBuf){free(photoPixelBuf);photoPixelBuf=nullptr;}
      galleryHoldDir=0;islandForceHide();resetAllButtons();
      islandNoClear=true;
      appMode=MODE_VIEWFINDER;lcd.fillScreen(COL_BLACK);
      fpsLastTime=millis();fpsFrameCount=0;
    }
  }
  else if(evt.pin==BTN_D&&evt.isLong){
    if(galleryCount==0){islandPush(NOTIF_WARN,"Gallery kosong");return;}
    GalleryFileType ft=galleryFileType[gallerySelIdx];
    if(ft==GFILE_VIDEO){islandPush(NOTIF_WARN,"Pilih foto, bukan video");return;}
    photoViewIndex=gallerySelIdx;
    strncpy(photoViewPath,galleryFiles[gallerySelIdx],sizeof(photoViewPath)-1);
    openAIFeatureMenu(false);
  }
}

void handleModePhotoView(ButtonEvent evt){
  static unsigned long lastPanTime=0;
  bool cHeld=btnC.isHeld(),dHeld=btnD.isHeld();
  if(photoZoomLevel>0&&(cHeld||dHeld)&&millis()-lastPanTime>120){
    float zf=photoZoomFactors[photoZoomLevel];
    int maxOffX=max(0,(int)photoBufW-(int)(DISP_W/zf));
    int maxOffY=max(0,(int)photoBufH-(int)(DISP_H/zf));
    if(cHeld) photoZoomOffX=constrain(photoZoomOffX-PAN_STEP,0,maxOffX);
    if(dHeld) photoZoomOffX=constrain(photoZoomOffX+PAN_STEP,0,maxOffX);
    photoViewRender();lastPanTime=millis();return;
  }
  if(!evt.valid) return;
  static unsigned long lastActionTime=0;
  if(millis()-lastActionTime<150) return;
  lastActionTime=millis();

  if(evt.pin==BTN_BOOT&&evt.isShort){
    photoViewCaptionVisible=false;photoViewCaptionUntilMs=0;
    photoZoomLevel=0;photoZoomOffX=0;photoZoomOffY=0;
    if(photoPixelBuf){free(photoPixelBuf);photoPixelBuf=nullptr;}
    lcd.setRotation(3); resetAllButtons();appMode=MODE_GALLERY; if(galleryGridMode) drawGalleryGrid(); else drawGallery();return;
  }
  if(evt.pin==BTN_B){
    if(evt.isLong){photoViewClearCaption();openDeleteDialog();}
    else{
      photoZoomLevel=(photoZoomLevel+1)%ZOOM_LEVELS;
      photoZoomOffX=0;photoZoomOffY=0;
      if(photoZoomLevel>0){
        float zf=photoZoomFactors[photoZoomLevel];
        int vpW=(int)(DISP_W/zf),vpH=(int)(DISP_H/zf);
        photoZoomOffX=max(0,(int)photoBufW/2-vpW/2);
        photoZoomOffY=max(0,(int)photoBufH/2-vpH/2);
      }
      photoViewRender();
      if(photoZoomLevel==0){
        photoViewDrawCaption(photoViewIndex);
        photoViewCaptionUntilMs=millis()+2000;photoViewCaptionVisible=true;
      }
    }
    return;
  }

  if(evt.pin==BTN_D&&evt.isLong){
    if(!sdReady){islandPush(NOTIF_WARN,"SD tidak tersedia");return;}
    photoViewClearCaption();
    openAIFeatureMenu(false);
    return;
  }

  if(photoZoomLevel==0){
    if(evt.pin==BTN_C&&evt.isShort) photoViewPrev();
    if(evt.pin==BTN_D&&evt.isShort) photoViewNext();
  } else {
    float zf=photoZoomFactors[photoZoomLevel];
    int maxOffX=max(0,(int)photoBufW-(int)(DISP_W/zf));
    int maxOffY=max(0,(int)photoBufH-(int)(DISP_H/zf));
    bool moved=false;
    if(evt.pin==BTN_C){
      if(evt.isShort){photoZoomOffX=constrain(photoZoomOffX-PAN_STEP,0,maxOffX);moved=true;}
      else           {photoZoomOffY=constrain(photoZoomOffY-PAN_STEP,0,maxOffY);moved=true;}
    }
    if(evt.pin==BTN_D){
      if(evt.isShort){photoZoomOffX=constrain(photoZoomOffX+PAN_STEP,0,maxOffX);moved=true;}
      else           {photoZoomOffY=constrain(photoZoomOffY+PAN_STEP,0,maxOffY);moved=true;}
    }
    if(moved) photoViewRender();
  }
}

void handleModeMjpegPlayer(ButtonEvent evtBoot,ButtonEvent evtB,
                            ButtonEvent evtC,ButtonEvent evtD){
  static unsigned long lastToggleTime=0;
  if(evtBoot.valid){mjpegClose();lcd.setRotation(3); resetAllButtons();appMode=MODE_GALLERY; if(galleryGridMode) drawGalleryGrid(); else drawGallery();return;}
  if(evtB.valid&&(millis()-lastToggleTime)>=200){
    mjpegPaused=!mjpegPaused;mjpegShowNotif(mjpegPaused?"PAUSE":"PLAY");lastToggleTime=millis();
  }
  if(evtC.valid&&(millis()-lastToggleTime)>=200){
    mjpegLoop=!mjpegLoop;mjpegShowNotif(mjpegLoop?"LOOP ON":"LOOP OFF");lastToggleTime=millis();
  }
  if(evtD.valid&&(millis()-lastToggleTime)>=200){
    mjpegSpeedIdx=(mjpegSpeedIdx+1)%3;
    char spBuf[12]; snprintf(spBuf,sizeof(spBuf),"%.1f\xd7",(double)mjpegSpeeds[mjpegSpeedIdx]);
    mjpegShowNotif(spBuf);lastToggleTime=millis();
  }
  loopMjpegPlayer();
}

void handleModeMenuLed(ButtonEvent evt){
  if(!evt.valid) return;
  if(evt.pin==BTN_BOOT){
    if(menuLedSel==0){ ledFlashEnabled=false; neoFlashEnabled=false; islandPush(NOTIF_FLASH,"FLASH OFF"); }
    else if(menuLedSel==1){ ledFlashEnabled=true; neoFlashEnabled=false; islandPush(NOTIF_FLASH,"FLASH ON"); }
    else if(menuLedSel==2){ ledFlashEnabled=true; neoFlashEnabled=true; islandPush(NOTIF_FLASH,"FLASH+NEO ON"); }
    saveSettings();lcd.fillScreen(COL_BLACK);resetAllButtons();
    islandNoClear=true;appMode=MODE_VIEWFINDER;
  }
  else if(evt.pin==BTN_B){lcd.fillScreen(COL_BLACK);resetAllButtons();islandNoClear=true;appMode=MODE_VIEWFINDER;}
  else if(evt.pin==BTN_C||evt.pin==BTN_D){menuLedSel=(menuLedSel+1)%3;drawLedMenu(menuLedSel);}
}

void handleModeMenuFormat(ButtonEvent evt){
  if(!evt.valid) return;
  if(evt.pin==BTN_BOOT){
    gc2145CaptureFormat=(menuFormatSel==0)?GFMT_BMP:GFMT_JPG;
    char fbBuf[28];
    snprintf(fbBuf,sizeof(fbBuf),"FORMAT: %s",gc2145CaptureFormat==GFMT_BMP?"BMP+stego":"JPG+EXIF+stego");
    islandPush(NOTIF_INFO,fbBuf);saveSettings();
    lcd.fillScreen(COL_BLACK);resetAllButtons();
    drawExpMenu(menuExpSel);
    appMode=MODE_MENU_EXP;
  }
  else if(evt.pin==BTN_B){
    drawExpMenu(menuExpSel);resetAllButtons();appMode=MODE_MENU_EXP;
  }
  else if(evt.pin==BTN_C||evt.pin==BTN_D){menuFormatSel=(menuFormatSel==0)?1:0;drawFormatMenu(menuFormatSel);}
}

void handleModeMenuExp(ButtonEvent evt){
  if(!evt.valid) return;
  if(evt.pin==BTN_BOOT){
    applyExpPreset((uint8_t)menuExpSel);
    if(menuExpSel==5){
      lcd.fillScreen(COL_BLACK);resetAllButtons();appMode=MODE_MENU_EXP_ADJ;return;
    }
    char fbBuf[24]; snprintf(fbBuf,sizeof(fbBuf),"MODE: %s",expPresetNames[menuExpSel]);
    islandPush(NOTIF_INFO,fbBuf);saveSettings();
    lcd.fillScreen(COL_BLACK);resetAllButtons();
    islandNoClear=true;appMode=MODE_VIEWFINDER;
  }
  else if(evt.pin==BTN_B){lcd.fillScreen(COL_BLACK);resetAllButtons();islandNoClear=true;appMode=MODE_VIEWFINDER;}
  else if(evt.pin==BTN_C){menuExpSel=(menuExpSel+5)%6;drawExpMenu(menuExpSel);}
  else if(evt.pin==BTN_D){
    if(evt.isShort){menuExpSel=(menuExpSel+1)%6;drawExpMenu(menuExpSel);}
    else if(evt.isLong&&detectedSensor==PID_GC2145){
      openFormatMenu();
    }
  }
}

void handleModeMenuExpAdj(ButtonEvent evt){
  camera_fb_t *fb=esp_camera_fb_get();
  if(fb){
    if(fb->format==PIXFORMAT_RGB565&&fb->width==DISP_W&&fb->height==DISP_H){
      int visH=DISP_H-38;
      for(int row=0;row<visH;row++)
        lcd.pushImage(0,row,DISP_W,1,(uint16_t*)fb->buf+row*DISP_W);
    }
    esp_camera_fb_return(fb);
  }
  drawExpAdjOverlay();esp_task_wdt_reset();
  if(!evt.valid) return;
  if(evt.pin==BTN_BOOT){
    char fbBuf[24]; snprintf(fbBuf,sizeof(fbBuf),"MODE: %s",expPresetNames[5]);
    islandPush(NOTIF_INFO,fbBuf);saveSettings();
    lcd.fillScreen(COL_BLACK);resetAllButtons();
    islandNoClear=true;appMode=MODE_VIEWFINDER;return;
  }
  bool changed=false;
  int step=(expManualVal<100)?10:(expManualVal<400)?25:50;
  if(evt.pin==BTN_C){expManualVal=constrain(expManualVal-step,0,1200);changed=true;}
  if(evt.pin==BTN_D){expManualVal=constrain(expManualVal+step,0,1200);changed=true;}
  if(evt.pin==BTN_B){expManualGain=(expManualGain+1)%31;changed=true;}
  if(changed) applyExpPreset(5);
}

// UI UPDATE v6.0
void handleModeDialogDelete(ButtonEvent evt){
  unsigned long elapsed=millis()-deleteDialogOpenMs;
  if(elapsed<DELETE_TIMEOUT_MS){
    int dw=200, dh=88, dx=(DISP_W-dw)/2, dy=(DISP_H-dh)/2;
    int barW=dw-20;
    int prog = (int)(barW * (1.0 - (double)elapsed/DELETE_TIMEOUT_MS));
    prog = constrain(prog, 0, barW);
    lcd.fillRect(dx+10, dy+36, barW, 4, COL_GRAY_2);
    lcd.fillRect(dx+10, dy+36, prog,  4, 0xF800);
  } else {
    neoOff();
    resetAllButtons();showPhotoView(photoViewIndex);return;
  }
  if(!evt.valid) return;
  if(evt.pin==BTN_BOOT) { neoOff(); photoViewDeleteCurrent(); }
  else { neoOff(); resetAllButtons(); showPhotoView(photoViewIndex); }
}
// ─────────────────────────────────────────────────────────────────────────────
//  LOOP
// ─────────────────────────────────────────────────────────────────────────────
void loop(){
  neoTick();
  esp_task_wdt_reset();

  if (g_hdrWaiting && (millis() - g_hdrFirstMs >= HDR_DOUBLE_TAP_MS)) {
    g_hdrWaiting = false;
    if (appMode == MODE_VIEWFINDER) runHDRFlow();
  }

  tickAllButtons();

  ButtonEvent evtBoot={},evtB={},evtC={},evtD={};
  btnBoot.pollEvent(evtBoot);
  btnB.pollEvent(evtB);
  btnC.pollEvent(evtC);
  btnD.pollEvent(evtD);

  ButtonEvent singleEvt={};
  if     (evtBoot.valid) singleEvt=evtBoot;
  else if(evtB.valid)    singleEvt=evtB;
  else if(evtC.valid)    singleEvt=evtC;
  else if(evtD.valid)    singleEvt=evtD;

  if(appMode==MODE_PHOTO_VIEW&&photoViewCaptionVisible&&millis()>photoViewCaptionUntilMs)
    photoViewClearCaption();

  // [PORTED v6.1] MPU tick
  mpuTick();
  if(usbModeActive){if(evtBoot.valid) exitUSBMode();return;}
  if(recActive){if(evtB.valid) stopRecording();else recordFrame();return;}

  switch(appMode){
    case MODE_VIEWFINDER:      handleModeViewfinder(singleEvt);                break;
    case MODE_GALLERY:         handleModeGallery(singleEvt);                   break;
    case MODE_PHOTO_VIEW:      handleModePhotoView(singleEvt);                 break;
    case MODE_MJPEG_PLAYER:    handleModeMjpegPlayer(evtBoot,evtB,evtC,evtD); break;
    case MODE_MENU_LED:        handleModeMenuLed(singleEvt);                   break;
    case MODE_MENU_EXP:        handleModeMenuExp(singleEvt);                   break;
    case MODE_MENU_EXP_ADJ:    handleModeMenuExpAdj(singleEvt);                break;
    case MODE_DIALOG_DELETE:   handleModeDialogDelete(singleEvt);              break;
    case MODE_MENU_FORMAT:     handleModeMenuFormat(singleEvt);                break;
    case MODE_JUMP_INPUT:      handleModeJumpInput(evtBoot,evtB,evtC,evtD);    break;
    case MODE_FEATURES:        handleModeFeatures(singleEvt);                  break;
    case MODE_AI_FEATURE_MENU: handleModeAIFeatureMenu(evtBoot,evtB,evtC,evtD);break;
    case MODE_AI_DESCRIBE:     handleModeAIDescribe(evtB,evtC,evtD);           break;
    case MODE_AI_NO_CONFIG:    handleModeAINoConfig(evtB,evtD);                break;
    case MODE_KEY_MANAGER:     handleModeKeyManager(evtBoot,evtB,evtC,evtD);  break;
  }
  if (appMode != MODE_VIEWFINDER) {
    bool isMenu = (appMode == MODE_JUMP_INPUT || appMode == MODE_AI_DESCRIBE ||
                   appMode == MODE_AI_NO_CONFIG || appMode == MODE_AI_FEATURE_MENU ||
                   appMode == MODE_KEY_MANAGER || appMode == MODE_FEATURES ||
                   appMode == MODE_MENU_EXP || appMode == MODE_MENU_LED ||
                   appMode == MODE_MENU_FORMAT || appMode == MODE_MENU_EXP_ADJ);
    if (!isMenu) islandNoClear = false;
    islandTick();
  }

}
