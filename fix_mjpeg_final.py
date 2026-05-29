import sys

file_path = 'camera_test/camera_test.ino'
with open(file_path, 'r') as f:
    content = f.read()

# Fix openMjpegPlayer again (it seems my previous replace failed or was overwritten)
open_mjpeg_old = """void openMjpegPlayer(const char* filename){
  mjpegLoop=false;mjpegSpeedIdx=1;
  if(!mjpegOpen(filename)){
    lcd.fillScreen(COL_BLACK);lcd.setFont(&fonts::Font0);lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("gagal buka file",20,DISP_H/2);
    delay(1500);resetAllButtons();drawGallery();appMode=MODE_GALLERY;return;
  }
  resetAllButtons();appMode=MODE_MJPEG_PLAYER;
}"""

open_mjpeg_new = """void openMjpegPlayer(const char* filename){
  mjpegLoop=false;mjpegSpeedIdx=1;
  recActualFps = 15.0f;
  const char* p = filename;
  while ((p = strstr(p, "_")) != nullptr) {
    int parsed = 0;
    if (sscanf(p, "_%dfps.mjpeg", &parsed) == 1 && parsed >= 5) {
      recActualFps = constrain((float)parsed, 5.0f, 30.0f);
      break;
    }
    p++;
  }
  if(!mjpegOpen(filename)){
    lcd.fillScreen(COL_BLACK);lcd.setFont(&fonts::Font0);lcd.setTextColor(COL_GRAY_5);
    lcd.drawString("gagal buka file",20,DISP_H/2);
    delay(1500);resetAllButtons();drawGallery();appMode=MODE_GALLERY;return;
  }
  resetAllButtons();appMode=MODE_MJPEG_PLAYER;
}"""

# Ensure recActualFps is used in loopMjpegPlayer
loop_mjpeg_old = 'int64_t targetUs=(int64_t)(1000000.0f/(MJPEG_FRAME_RATE*speed));'
loop_mjpeg_new = 'int64_t targetUs=(int64_t)(1000000.0f/(recActualFps*speed));'

content = content.replace(open_mjpeg_old, open_mjpeg_new)
content = content.replace(loop_mjpeg_old, loop_mjpeg_new)

with open(file_path, 'w') as f:
    f.write(content)
