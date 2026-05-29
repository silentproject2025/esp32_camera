import sys
import re

file_path = 'camera_test/camera_test.ino'
with open(file_path, 'r') as f:
    content = f.read()

old_sx = r'int sx = constrain\(\(int\)g_eisOffX, 0, DISP_W - EIS_VP_W\); int sy = constrain\(\(int\)g_eisOffY, 0, DISP_H - EIS_VP_H\);'
new_sx = """int sx = 0, sy = 0;
            if (xSemaphoreTake(mpuMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
              sx = constrain((int)g_eisOffX, 0, DISP_W - EIS_VP_W);
              sy = constrain((int)g_eisOffY, 0, DISP_H - EIS_VP_H);
              xSemaphoreGive(mpuMutex);
            } else {
              sx = EIS_CROP_X; sy = EIS_CROP_Y;
            }"""

content = re.sub(old_sx, new_sx, content)

with open(file_path, 'w') as f:
    f.write(content)
