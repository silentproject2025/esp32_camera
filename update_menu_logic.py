import sys

with open('camera_test/camera_test.ino', 'r') as f:
    content = f.read()

# Update handleModeFeatures navigation modulo
content = content.replace("menuFeatSel = (menuFeatSel + 1) % 9;", "menuFeatSel = (menuFeatSel + 1) % 11;")
content = content.replace("menuFeatSel = (menuFeatSel + 8) % 9;", "menuFeatSel = (menuFeatSel + 10) % 11;")

# Update handleModeFeatures interaction logic
# We need to insert handlers for index 8 and 9, and update DLPF to 10.
old_dlpf_logic = """    } else if (menuFeatSel == 8) {
      mpuDlpfIndex = (mpuDlpfIndex + 1) % 7;
      applyDLPF();
      char buf[32]; snprintf(buf, sizeof(buf), "DLPF: %s", DLPF_LABELS[mpuDlpfIndex]);
      islandPush(NOTIF_INFO, buf); drawFeaturesMenu(menuFeatSel);
    }"""

new_interaction_extra = """    } else if (menuFeatSel == 8) {
      if      (mpuKalmanRmeas < 0.04f) mpuKalmanRmeas = 0.05f;
      else if (mpuKalmanRmeas < 0.06f) mpuKalmanRmeas = 0.10f;
      else if (mpuKalmanRmeas < 0.15f) mpuKalmanRmeas = 0.20f;
      else if (mpuKalmanRmeas < 0.40f) mpuKalmanRmeas = 0.50f;
      else                             mpuKalmanRmeas = 0.03f;
      char buf[32]; snprintf(buf, sizeof(buf), "KALMAN-R: %.2f", mpuKalmanRmeas);
      islandPush(NOTIF_INFO, buf); drawFeaturesMenu(menuFeatSel);
    } else if (menuFeatSel == 9) {
      if      (mpuTiltDeadzone < 0.5f) mpuTiltDeadzone = 1.0f;
      else if (mpuTiltDeadzone < 1.5f) mpuTiltDeadzone = 2.0f;
      else if (mpuTiltDeadzone < 2.5f) mpuTiltDeadzone = 3.0f;
      else if (mpuTiltDeadzone < 4.5f) mpuTiltDeadzone = 5.0f;
      else                             mpuTiltDeadzone = 0.0f;
      char buf[32]; snprintf(buf, sizeof(buf), "TILT-DZ: %.1f deg", mpuTiltDeadzone);
      islandPush(NOTIF_INFO, buf); drawFeaturesMenu(menuFeatSel);
    } else if (menuFeatSel == 10) {
      mpuDlpfIndex = (mpuDlpfIndex + 1) % 7;
      applyDLPF();
      char buf[32]; snprintf(buf, sizeof(buf), "DLPF: %s", DLPF_LABELS[mpuDlpfIndex]);
      islandPush(NOTIF_INFO, buf); drawFeaturesMenu(menuFeatSel);
    }"""

content = content.replace(old_dlpf_logic, new_interaction_extra)

with open('camera_test/camera_test.ino', 'w') as f:
    f.write(content)
