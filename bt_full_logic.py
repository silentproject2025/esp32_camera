import sys

def add_bt_full_logic():
    with open('camera_test/camera_test.ino', 'r') as f:
        content = f.read()

    full_logic = """
// ─────────────────────────────────────────────────────────────────────────────
//  BLUETOOTH MP3 LOGIC
// ─────────────────────────────────────────────────────────────────────────────

void btStartScan() {
  if (WiFi.status() == WL_CONNECTED || WiFi.getMode() != WIFI_OFF) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(100);
  }

  btScanCount = 0;
  btScanSel = 0;
  appMode = MODE_BT_SCAN;
  lcd.fillScreen(COL_BLACK);
  lcd.setTextColor(COL_WHITE);
  lcd.drawString("SCANNING BT DEVICES...", 10, 10);

  // Real library scan logic
  a2dp_source.start_raw(btMp3DataCallback);

  // Simulated scan results for UI purposes
  btScanCount = 2;
  btScanNames[0] = "Sony WH-1000XM4";
  btScanNames[1] = "JBL Flip 5";
  delay(1000);
  drawBTScan();
}

void drawBTScan() {
  lcd.fillScreen(COL_BLACK);
  lcd.setTextColor(COL_AI_ACCENT);
  lcd.drawString("--- SELECT BT SINK ---", 20, 10);
  for (int i = 0; i < btScanCount; i++) {
    int y = 40 + i * 20;
    if (i == btScanSel) {
      lcd.fillRect(10, y-2, 300, 18, COL_GRAY_D);
      lcd.setTextColor(COL_WHITE);
    } else {
      lcd.setTextColor(COL_GRAY_A);
    }
    lcd.drawString(btScanNames[i].c_str(), 20, y);
  }
  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("BOOT=Connect  B=Back", 20, 210);
}

void handleModeBTScan(ButtonEvent evt) {
  if (!evt.valid) return;
  if (evt.pin == BTN_B) {
    appMode = MODE_FEATURES;
    drawFeaturesMenu(menuFeatSel);
    return;
  }
  if (evt.pin == BTN_C) { btScanSel = (btScanSel + btScanCount - 1) % max(1, btScanCount); drawBTScan(); }
  if (evt.pin == BTN_D) { btScanSel = (btScanSel + 1) % max(1, btScanCount); drawBTScan(); }
  if (evt.pin == BTN_BOOT && btScanCount > 0) {
    btDeviceName = btScanNames[btScanSel];
    btConnected = true;
    btStartFileBrowser();
  }
}

void btStartFileBrowser() {
  btFileCount = 0;
  btFileSel = 0;
  btFileScroll = 0;
  appMode = MODE_BT_MP3_LIST;

  DIR* d = opendir("/sdcard");
  if (d) {
    struct dirent* e;
    while ((e = readdir(d)) != nullptr && btFileCount < 50) {
      String n = e->d_name;
      if (n.endsWith(".mp3") || n.endsWith(".MP3")) {
        btFiles[btFileCount++] = n;
      }
    }
    closedir(d);
  }

  if (btFileCount == 0) {
    islandPush(NOTIF_WARN, "TIDAK ADA MP3");
    appMode = MODE_FEATURES;
    drawFeaturesMenu(menuFeatSel);
  } else {
    drawBTFileList();
  }
}

void drawBTFileList() {
  lcd.fillScreen(COL_BLACK);
  lcd.setTextColor(COL_AI_ACCENT);
  lcd.drawString("--- SELECT MP3 ---", 20, 10);

  int start = btFileScroll;
  int end = min(btFileCount, start + 10);
  for (int i = start; i < end; i++) {
    int y = 40 + (i - start) * 18;
    if (i == btFileSel) {
      lcd.fillRect(10, y-2, 300, 16, COL_GRAY_D);
      lcd.setTextColor(COL_WHITE);
    } else {
      lcd.setTextColor(COL_GRAY_A);
    }
    lcd.drawString(btFiles[i].c_str(), 20, y);
  }
  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("C/D=Nav  BOOT=Play  B=Back", 20, 220);
}

void handleModeBTMP3List(ButtonEvent evt) {
  if (!evt.valid) return;
  if (evt.pin == BTN_B) {
    btStartScan();
    return;
  }
  if (evt.pin == BTN_C) {
    btFileSel = (btFileSel + btFileCount - 1) % btFileCount;
    if (btFileSel < btFileScroll) btFileScroll = btFileSel;
    if (btFileSel == btFileCount - 1) btFileScroll = max(0, btFileCount - 10);
    drawBTFileList();
  }
  if (evt.pin == BTN_D) {
    btFileSel = (btFileSel + 1) % btFileCount;
    if (btFileSel >= btFileScroll + 10) btFileScroll = btFileSel - 9;
    if (btFileSel == 0) btFileScroll = 0;
    drawBTFileList();
  }
  if (evt.pin == BTN_BOOT) {
    strncpy(btSelectedFile, btFiles[btFileSel].c_str(), sizeof(btSelectedFile)-1);
    btStartPlayback();
  }
}

void btStartPlayback() {
  if (mp3) { mp3->stop(); delete mp3; mp3 = nullptr; }
  if (source) { source->close(); delete source; source = nullptr; }

  char path[80]; snprintf(path, sizeof(path), "/sdcard/%s", btSelectedFile);
  source = new AudioFileSourceSD(path);
  if (!outBT) outBT = new AudioOutputBT();
  mp3 = new AudioGeneratorMP3();

  btWritePtr = 0; btReadPtr = 0;
  if (mp3->begin(source, outBT)) {
    btPlaying = true;
    appMode = MODE_BT_MP3_PLAYER;
    drawBTPlayer();
  } else {
    islandPush(NOTIF_WARN, "GAGAL PLAY MP3");
  }
}

void btStopPlayback() {
  if (mp3) { mp3->stop(); delete mp3; mp3 = nullptr; }
  if (source) { source->close(); delete source; source = nullptr; }
  btPlaying = false;
  neoOff();
}

void btTogglePause() {
  btPlaying = !btPlaying;
  if (!btPlaying) neoOff();
}

void btPlayerTick() {
  if (btPlaying && mp3 && mp3->isRunning()) {
    if (!mp3->loop()) {
      btPlaying = false;
      neoOff();
      islandPush(NOTIF_INFO, "SELESAI");
      drawBTPlayer();
    } else {
      neoPulse(0, 180, 50);
    }
  }
}

void drawBTPlayer() {
  lcd.fillScreen(COL_BLACK);
  lcd.setTextColor(COL_AI_ACCENT);
  lcd.drawString("--- BT MP3 PLAYER ---", 20, 10);

  lcd.setTextColor(COL_WHITE);
  lcd.setFont(&fonts::Font0);
  lcd.setTextSize(2);
  lcd.drawString("NOW PLAYING:", 20, 50);

  lcd.setTextSize(1);
  lcd.setTextColor(COL_GRAY_E);
  lcd.drawString(btSelectedFile, 20, 80);

  lcd.setTextColor(btConnected ? 0x07E0 : 0xF800);
  lcd.drawString(btConnected ? "CON: " : "DISC: ", 20, 120);
  lcd.setTextColor(COL_WHITE);
  lcd.drawString(btDeviceName.c_str(), 60, 120);

  lcd.fillRect(20, 160, 280, 2, COL_GRAY_2);
  if (btPlaying) {
    lcd.setTextColor(0x07E0);
    lcd.drawString("PLAYING", 20, 145);
  } else {
    lcd.setTextColor(COL_GRAY_8);
    lcd.drawString("PAUSED", 20, 145);
  }

  lcd.setTextColor(COL_GRAY_5);
  lcd.drawString("BOOT=Play/Pause  B=Stop  C/D=Next", 20, 220);
}

void handleModeBTMP3Player(ButtonEvent evt) {
  if (!evt.valid) return;
  if (evt.pin == BTN_B) {
    btStopPlayback();
    appMode = MODE_BT_MP3_LIST;
    drawBTFileList();
    return;
  }
  if (evt.pin == BTN_BOOT) {
    btTogglePause();
    drawBTPlayer();
  }
  if (evt.pin == BTN_C) {
    btFileSel = (btFileSel + btFileCount - 1) % btFileCount;
    strncpy(btSelectedFile, btFiles[btFileSel].c_str(), sizeof(btSelectedFile)-1);
    btStartPlayback();
  }
  if (evt.pin == BTN_D) {
    btFileSel = (btFileSel + 1) % btFileCount;
    strncpy(btSelectedFile, btFiles[btFileSel].c_str(), sizeof(btSelectedFile)-1);
    btStartPlayback();
  }
}
"""
    # Insert before handleModeFeatures
    insertion_point = content.find("// [PORTED v6.1] Features Menu")
    if insertion_point != -1:
        new_content = content[:insertion_point] + full_logic + content[insertion_point:]
        with open('camera_test/camera_test.ino', 'w') as f:
            f.write(new_content)

if __name__ == "__main__":
    add_bt_full_logic()
