import sys
import re

file_path = 'camera_test/camera_test.ino'
with open(file_path, 'r') as f:
    content = f.read()

# I seem to have multiple partial setup/workerTask blocks.
# Let's find the FIRST workerTask and LAST resetAllButtons in setup and clean everything in between.

worker_start = content.find('void workerTask(void* param) {')
setup_end = content.find('void handleModeViewfinder(ButtonEvent evt){')

if worker_start != -1 and setup_end != -1:
    clean_part = """void workerTask(void* param) {
  const TickType_t mpuInterval = pdMS_TO_TICKS(MPU_READ_MS);
  const TickType_t neoInterval = pdMS_TO_TICKS(30);
  TickType_t lastMpu = 0, lastNeo = 0;

  for (;;) {
    TickType_t now = xTaskGetTickCount();

    // MPU tick
    if (now - lastMpu >= mpuInterval) {
      if (xSemaphoreTake(mpuMutex, 0) == pdTRUE) {
        mpuTick();
        xSemaphoreGive(mpuMutex);
      }
      lastMpu = now;
    }

    // Neo tick
    if (now - lastNeo >= neoInterval) {
      neoTick();
      lastNeo = now;
    }

    // SD write from queue (non-blocking check)
    RecFrame rf;
    if (recFrameQueue &&
        xQueueReceive(recFrameQueue, &rf, 0) == pdTRUE) {
      if (recFile && rf.jpg && rf.len > 0) {
        fwrite(rf.jpg, 1, rf.len, recFile);
        recFrameCount++;
      }
      if (rf.jpg) free(rf.jpg);
    }

    vTaskDelay(pdMS_TO_TICKS(5));
    esp_task_wdt_reset();
  }
}

void setup(){
  Serial.begin(115200);
  Serial.println("\\n=== Sanzxcam v5.9-fix5 ===");
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
      Serial.printf("[MPU] Skip gyro calibration, using SD: X=%.5f Y=%.5f Z=%.5f\\n", g_gyroCalX, g_gyroCalY, g_gyroCalZ);
    } else {
      g_gyroCalX = 0.0f; g_gyroCalY = 0.0f; g_gyroCalZ = 0.0f;
      Serial.println("[MPU] No cal file found. Gyro bias set to 0. Use CALIBRATE menu.");
    }
  } esp_task_wdt_reset();
  Serial.printf("[MPU] %s\\n", g_mpuOk ? "OK" : "FAIL");
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
  mpuMutex = xSemaphoreCreateMutex();
  neoMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    workerTask,       // task function
    "worker",         // name
    8192,             // stack size
    nullptr,          // param
    1,                // priority (low)
    &workerTaskHandle,
    1                 // Core 1
  );
  resetAllButtons();
}

"""
    content = content[:worker_start] + clean_part + content[setup_end:]
    with open(file_path, 'w') as f:
        f.write(content)
