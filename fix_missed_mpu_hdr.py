import sys
import re

file_path = 'camera_test/camera_test.ino'
with open(file_path, 'r') as f:
    content = f.read()

# Fix HDR gmag
old_hdr_gmag = r'float gmag = sqrtf\(g_gyroX \* g_gyroX \+ g_gyroY \* g_gyroY \+ g_gyroZ \* g_gyroZ\);'
new_hdr_gmag = """float gmag = 0;
    if (xSemaphoreTake(mpuMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
      gmag = sqrtf(g_gyroX * g_gyroX + g_gyroY * g_gyroY + g_gyroZ * g_gyroZ);
      xSemaphoreGive(mpuMutex);
    }"""

content = re.sub(old_hdr_gmag, new_hdr_gmag, content)

with open(file_path, 'w') as f:
    f.write(content)
