import sys

with open('camera_test/camera_test.ino', 'r') as f:
    content = f.read()

# Update drawFeaturesMenu UI structure
old_dims = "int mw = 220, mh = 248, mx = (DISP_W - mw) / 2, my = (DISP_H - mh) / 2;"
new_dims = "int mw = 220, mh = 292, mx = (DISP_W - mw) / 2, my = (DISP_H - mh) / 2;"
content = content.replace(old_dims, new_dims)

old_labels = """  static const char* const rowLabels[9] = {
    "EIS  Electronic Stab", "HDR  Triple Exposure",
    "AUTO-ROTATE  MPU tilt", "MPU LOG  CSV to SD", "HUD  Overlay",
    "CALIBRATE MPU  recalibrate",
    "HD CAPTURE  quality saat foto", "HD QUALITY  4=max / 6=bagus",
    "DLPF  filter bandwidth"
  };"""

new_labels = """  static const char* const rowLabels[11] = {
    "EIS  Electronic Stab", "HDR  Triple Exposure",
    "AUTO-ROTATE  MPU tilt", "MPU LOG  CSV to SD", "HUD  Overlay",
    "CALIBRATE MPU  recalibrate",
    "HD CAPTURE  quality saat foto", "HD QUALITY  4=max / 6=bagus",
    "KALMAN-R  noise filter", "TILT-DZ  deadzone deg",
    "DLPF  filter bandwidth"
  };"""
content = content.replace(old_labels, new_labels)

old_loop = "for (int i = 0; i < 9; i++) {"
new_loop = "for (int i = 0; i < 11; i++) {"
content = content.replace(old_loop, new_loop)

# Update rendering logic for new rows
# We need to insert logic for index 8 and 9, and move DLPF to 10.
old_dlpf_render = """    } else if (i == 8) {
      lcd.setTextColor(COL_WHITE);
      lcd.drawString(DLPF_LABELS[mpuDlpfIndex], mx + mw - 45, iy + 5);
    }"""

new_render_extra = """    } else if (i == 8) {
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
    }"""

content = content.replace(old_dlpf_render, new_render_extra)

with open('camera_test/camera_test.ino', 'w') as f:
    f.write(content)
