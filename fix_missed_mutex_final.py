import sys

file_path = 'camera_test/camera_test.ino'
with open(file_path, 'r') as f:
    content = f.read()

# I will use re.sub for safety
import re

old_breath = r'if \(!recActive && !g_tilted && !g_shake\) neoBreath\(0, 0, 80\);'
new_breath = """bool tlt=false, shk=false;
        if (xSemaphoreTake(mpuMutex, pdMS_TO_TICKS(5)) == pdTRUE) { tlt=g_tilted; shk=g_shake; xSemaphoreGive(mpuMutex); }
        if (!recActive && !tlt && !shk) neoBreath(0, 0, 80);"""

content = re.sub(old_breath, new_breath, content)

with open(file_path, 'w') as f:
    f.write(content)
