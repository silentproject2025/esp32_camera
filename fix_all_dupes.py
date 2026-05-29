import sys
import re

file_path = 'camera_test/camera_test.ino'
with open(file_path, 'r') as f:
    content = f.read()

# 1. Update startRecording (ensure no duplicates)
start_rec_new = """void startRecording() {
  if (!sdReady || recActive) return;
  neoSolid(180, 0, 0);
  recVideoCount++;
  char path[48]; snprintf(path, sizeof(path), "/sdcard/video_%04d_tmp.mjpeg", recVideoCount);
  recFile = fopen(path, "wb");
  if (!recFile) { recVideoCount--; return; }

  if (!recEisBuf) recEisBuf = (uint16_t*)ps_malloc(DISP_W * DISP_H * 2);
  recFrameQueue = xQueueCreate(3, sizeof(RecFrame));

  if (xSemaphoreTake(mpuMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    g_eisOffX = EIS_CROP_X; g_eisOffY = EIS_CROP_Y;
    g_eisBiasX = 0; g_eisBiasY = 0;
    xSemaphoreGive(mpuMutex);
  }
  recFrameCount = 0; recStartMs = millis(); recActive = true;
  char buf[20]; snprintf(buf, sizeof(buf), "REC #%04d", recVideoCount);
  islandPush(NOTIF_REC, buf);
}"""

# 2. Update stopRecording
stop_rec_new = """void stopRecording(){
  if(!recActive||!recFile) return;

  RecFrame rf;
  while (recFrameQueue && xQueueReceive(recFrameQueue, &rf, pdMS_TO_TICKS(200)) == pdTRUE) {
    if (recFile && rf.jpg && rf.len > 0) fwrite(rf.jpg, 1, rf.len, recFile);
    if (rf.jpg) free(rf.jpg);
  }
  if (recFrameQueue) { vQueueDelete(recFrameQueue); recFrameQueue = nullptr; }

  fclose(recFile); recFile = nullptr; recActive = false;

  unsigned long dur = millis() - recStartMs;
  float actualFps = (dur > 0) ? (recFrameCount * 1000.0f / dur) : 15.0f;
  actualFps = constrain(actualFps, 5.0f, 30.0f);
  int fpsInt = (int)roundf(actualFps);
  recActualFps = actualFps;

  char oldPath[52], newPath[52];
  snprintf(oldPath, sizeof(oldPath), "/sdcard/video_%04d_tmp.mjpeg", recVideoCount);
  snprintf(newPath, sizeof(newPath), "/sdcard/video_%04d_%dfps.mjpeg", recVideoCount, fpsInt);
  rename(oldPath, newPath);

  char buf[28]; snprintf(buf, sizeof(buf), "%df %dfps", recFrameCount, fpsInt);
  islandPush(NOTIF_OK, buf);
  neoBurst(0, 180, 0, 3); neoOff();
  fpsLastTime = millis(); fpsFrameCount = 0;
}"""

# 3. recordFrame
record_frame_new = """void recordFrame() {
  if (!recActive || !recFile || !recFrameQueue) return;

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) { esp_task_wdt_reset(); return; }

  uint8_t* jpg = nullptr;
  size_t   jLen = 0;
  bool     ok   = false;
  int recQ = hdCaptureEnabled ? map(hdCaptureQuality, 10, 1, 50, 90) : 80;

  if (eisEnabled && fb->format == PIXFORMAT_RGB565 && recEisBuf) {
    uint16_t* src = (uint16_t*)fb->buf;
    int sx = 0, sy = 0;
    if (xSemaphoreTake(mpuMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
      sx = constrain((int)g_eisOffX, 0, DISP_W - EIS_VP_W);
      sy = constrain((int)g_eisOffY, 0, DISP_H - EIS_VP_H);
      xSemaphoreGive(mpuMutex);
    } else {
      sx = EIS_CROP_X; sy = EIS_CROP_Y;
    }
    for (int y = 0; y < DISP_H; y++) {
      int srY = sy + (y * EIS_VP_H / DISP_H);
      for (int x = 0; x < DISP_W; x++)
        recEisBuf[y * DISP_W + x] =
          src[srY * DISP_W + sx + (x * EIS_VP_W / DISP_W)];
    }
    camera_fb_t fk = *fb;
    fk.buf = (uint8_t*)recEisBuf;
    fk.width = DISP_W; fk.height = DISP_H;
    ok = frame2jpg(&fk, recQ, &jpg, &jLen);

    if ((recFrameCount % 4) == 0)
      lcd.pushImage(0, 0, DISP_W, DISP_H, recEisBuf);

  } else if (fb->format == PIXFORMAT_RGB565) {
    ok = frame2jpg(fb, recQ, &jpg, &jLen);
    if ((recFrameCount % 4) == 0)
      lcd.pushImage(0, 0, DISP_W, DISP_H, (uint16_t*)fb->buf);
  } else if (fb->format == PIXFORMAT_JPEG) {
    jpg = (uint8_t*)malloc(fb->len);
    if (jpg) { memcpy(jpg, fb->buf, fb->len); jLen = fb->len; ok = true; }
  }

  esp_camera_fb_return(fb);

  if (ok && jpg && jLen > 0) {
    RecFrame rf = { jpg, jLen };
    if (xQueueSend(recFrameQueue, &rf, 0) != pdTRUE) {
      free(jpg);
    }
  } else {
    if (jpg) free(jpg);
  }

  esp_task_wdt_reset();
}"""

content = re.sub(r'void startRecording\(\) \{(?:.|\n)*?\}', start_rec_new, content)
content = re.sub(r'void stopRecording\(\)\{(?:.|\n)*?\}', stop_rec_new, content)
content = re.sub(r'void recordFrame\(\) \{(?:.|\n)*?\}', record_frame_new, content)

with open(file_path, 'w') as f:
    f.write(content)
