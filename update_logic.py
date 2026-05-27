import sys

with open('camera_test/camera_test.ino', 'r') as f:
    content = f.read()

# Update mpuDrawIndicator
old_indicator = """void mpuDrawIndicator() {
  if (!g_mpuOk) return;
  char buf[16]; uint16_t col;
  if (g_shake) { strncpy(buf, "SHAKE!", sizeof(buf)); col=0xFD20; } // COL_WARN
  else if (g_tilted) {"""

new_indicator = """void mpuDrawIndicator() {
  if (!g_mpuOk) return;
  char buf[16]; uint16_t col;
  bool showTilt = g_tilted && (fabsf(g_tiltX) > mpuTiltDeadzone || fabsf(g_tiltY) > mpuTiltDeadzone);
  if (g_shake) { strncpy(buf, "SHAKE!", sizeof(buf)); col=0xFD20; } // COL_WARN
  else if (showTilt) {"""

content = content.replace(old_indicator, new_indicator)

# Update saveSettings
old_save = '  fprintf(f, "dlpf=%d\\n", (int)mpuDlpfIndex);'
new_save = old_save + '\n  fprintf(f, "kalman_r=%.3f\\n", mpuKalmanRmeas);\n  fprintf(f, "tilt_dz=%.1f\\n",  mpuTiltDeadzone);'
content = content.replace(old_save, new_save)

# Update loadSettings
old_load = '    else if (sscanf(line, "dlpf=%d", &v) == 1) { mpuDlpfIndex = (uint8_t)constrain(v, 0, 6); }'
new_load = old_load + '\n    float fv = 0;\n    if      (sscanf(line, "kalman_r=%f", &fv) == 1) { mpuKalmanRmeas = constrain(fv, 0.01f, 1.0f); }\n    else if (sscanf(line, "tilt_dz=%f", &fv) == 1) { mpuTiltDeadzone = constrain(fv, 0.0f, 10.0f); }'
content = content.replace(old_load, new_load)

with open('camera_test/camera_test.ino', 'w') as f:
    f.write(content)
