import sys

file_path = 'camera_test/camera_test.ino'
with open(file_path, 'r') as f:
    content = f.read()

# Find the double startRecording mess
bad_block = """  recFrameCount = 0; recStartMs = millis(); recActive = true;
  char buf[20]; snprintf(buf, sizeof(buf), "REC #%04d", recVideoCount);
  islandPush(NOTIF_REC, buf);
}
  g_eisOffX = EIS_CROP_X; g_eisOffY = EIS_CROP_Y;
  g_eisBiasX = 0; g_eisBiasY = 0;
  recFrameCount = 0; recStartMs = millis(); recActive = true;
  char buf[20]; snprintf(buf, sizeof(buf), "REC #%04d", recVideoCount);
  islandPush(NOTIF_REC, buf);
}"""

good_block = """  recFrameCount = 0; recStartMs = millis(); recActive = true;
  char buf[20]; snprintf(buf, sizeof(buf), "REC #%04d", recVideoCount);
  islandPush(NOTIF_REC, buf);
}"""

content = content.replace(bad_block, good_block)

with open(file_path, 'w') as f:
    f.write(content)
