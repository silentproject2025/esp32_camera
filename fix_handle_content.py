import sys

def fix():
    with open('camera_test/camera_test.ino', 'r') as f:
        content = f.read()

    # Check if handleModeFeatures is empty
    empty_tag = "void handleModeFeatures(ButtonEvent evt) {\\n\\n// ─────────────────────────────────────────────────────────────────────────────"
    # Actually let's just find the original handleModeFeatures content
    original_start = content.find("void handleModeFeatures(ButtonEvent evt) {")
    original_end = content.find("// ─────────────────────────────────────────────────────────────────────────────\\n//  App Mode")

    # This is currently empty or just whitespace.
    # Let's restore the logic from a backup or re-write it.

    restored_logic = """void handleModeFeatures(ButtonEvent evt) {
  if (!evt.valid) return;
  bool* const feats[5] = {&eisEnabled, &hdrEnabled, &autoRotateEnabled, &mpuLogEnabled, &hudEnabled};
  static const char* const labs[5] = {"EIS", "HDR", "AUTO-ROTATE", "MPU LOG", "HUD"};

  if (evt.pin == BTN_B && evt.isShort) {
    saveSettings();
    appMode = MODE_VIEWFINDER;
    islandNoClear = true;
    lcd.fillScreen(COL_BLACK);
    resetAllButtons();
    return;
  }

  if (evt.pin == BTN_D && evt.isShort) { menuFeatSel = (menuFeatSel + 1) % 13; drawFeaturesMenu(menuFeatSel); }
  else if (evt.pin == BTN_C && evt.isShort) { menuFeatSel = (menuFeatSel + 12) % 13; drawFeaturesMenu(menuFeatSel); }
  else if (evt.pin == BTN_BOOT && evt.isShort) {
    if (menuFeatSel < 5) {
      *feats[menuFeatSel] = !(*feats[menuFeatSel]);
      char buf[32]; snprintf(buf, sizeof(buf), "%s %s", labs[menuFeatSel], *feats[menuFeatSel] ? "ON" : "OFF");
      if (menuFeatSel == 4) { // HUD toggle
        saveSettings();
        islandPush(NOTIF_INFO, hudEnabled ? "HUD ON" : "HUD OFF");
      } else {
        islandPush(NOTIF_INFO, buf);
      }
      drawFeaturesMenu(menuFeatSel);
    } else if (menuFeatSel == 5) {
      runMPUCalibration(); drawFeaturesMenu(menuFeatSel);
    } else if (menuFeatSel == 6) {
      hdCaptureEnabled = !hdCaptureEnabled;
      islandPush(NOTIF_INFO, hdCaptureEnabled ? "HD CAPTURE ON" : "HD CAPTURE OFF"); drawFeaturesMenu(menuFeatSel);
    } else if (menuFeatSel == 7) {
      hdCaptureQuality = (hdCaptureQuality == 4) ? 6 : 4;
      char qBuf[32]; snprintf(qBuf, sizeof(qBuf), "HD QUALITY: %d", hdCaptureQuality);
      islandPush(NOTIF_INFO, qBuf); drawFeaturesMenu(menuFeatSel);
    } else if (menuFeatSel == 8) {
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
    } else if (menuFeatSel == 11) {
      DIR* d = opendir("/sdcard/.cache");
      if (d) {
        struct dirent* e; int count = 0;
        while ((e = readdir(d)) != nullptr) {
          String n = e->d_name;
          if (n.endsWith(".bin")) {
            char dp[64]; snprintf(dp, sizeof(dp), "/sdcard/.cache/%s", e->d_name);
            remove(dp); count++; esp_task_wdt_reset();
          }
        }
        closedir(d);
        char msg[32]; snprintf(msg, sizeof(msg), "Cache: %d file dihapus", count);
        islandPush(NOTIF_OK, msg);
      } else {
        islandPush(NOTIF_WARN, "Cache dir tidak ada");
      }
      drawFeaturesMenu(menuFeatSel);
    } else if (menuFeatSel == 12) {
      btStartScan();
    }
  }
}
"""
    new_content = content[:original_start] + restored_logic + content[original_end:]
    with open('camera_test/camera_test.ino', 'w') as f:
        f.write(new_content)

if __name__ == "__main__":
    fix()
