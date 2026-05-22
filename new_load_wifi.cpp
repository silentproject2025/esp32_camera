bool loadWifiConfig() {
  FILE* f = fopen(WIFI_INI_PATH, "r");
  if (!f) {
    createFileTemplate(WIFI_INI_PATH,
      "# Konfigurasi WiFi Sanzxcam\n"
      "# Isi ssid dan pass lalu restart kamera\n"
      "ssid=NamaWiFiKamu\n"
      "pass=PasswordWiFiKamu\n");
    return false;
  }
  char line[128]; bool gotSSID=false;
  while (fgets(line, sizeof(line), f)) {
    char* l = line;
    while(isspace((unsigned char)*l)) l++;
    if (*l == '#' || *l == '\0') continue;
    char* eq = strchr(l, '=');
    if (eq) {
      *eq = '\0';
      char* key = l;
      char* val = eq + 1;
      char* k_end = key + strlen(key) - 1;
      while(k_end > key && isspace((unsigned char)*k_end)) { *k_end = '\0'; k_end--; }
      while(isspace((unsigned char)*val)) val++;
      char* v_end = val + strlen(val) - 1;
      while(v_end >= val && isspace((unsigned char)*v_end)) { *v_end = '\0'; v_end--; }
      if (strcasecmp(key, "ssid") == 0) {
        strncpy(wifiSSID, val, 63); wifiSSID[63] = '\0';
        gotSSID = true;
      } else if (strcasecmp(key, "pass") == 0) {
        strncpy(wifiPass, val, 63); wifiPass[63] = '\0';
      }
    }
  }
  fclose(f);
  if (gotSSID && strcmp(wifiSSID, "NamaWiFiKamu") == 0) return false;
  if (gotSSID) Serial.printf("[WIFI] ssid=%s\n", wifiSSID);
  return gotSSID;
}
