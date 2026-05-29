import sys

file_path = 'camera_test/camera_test.ino'
with open(file_path, 'r') as f:
    content = f.read()

old_vfind = """void handleModeViewfinder(ButtonEvent evt){
  galleryGridActive=false;
  if(!evt.valid){renderViewfinder();return;}"""

new_vfind = """void handleModeViewfinder(ButtonEvent evt){
  galleryGridActive=false;
  if (autoRotateEnabled) {
    uint8_t r = 3;
    if (xSemaphoreTake(mpuMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
      if (g_tiltX > 45.0f) r = 2;
      else if (g_tiltX < -45.0f) r = 0;
      xSemaphoreGive(mpuMutex);
    }
    if (lcd.getRotation() != r) lcd.setRotation(r);
  }
  if(!evt.valid){renderViewfinder();return;}"""

content = content.replace(old_vfind, new_vfind)

with open(file_path, 'w') as f:
    f.write(content)
