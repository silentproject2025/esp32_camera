#include <stdio.h>
#include <string.h>
#include <ctype.h>

char wifiSSID[64] = "";
char wifiPass[64] = "";

char* trim(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

bool loadWifiConfigMock(const char* content) {
    char line[128];
    const char* p = content;
    bool gotSSID = false;

    // Simulate line reading
    char buffer[1024];
    strncpy(buffer, content, sizeof(buffer));
    char* l = strtok(buffer, "\n");

    while (l != NULL) {
        char currentLine[128];
        strncpy(currentLine, l, sizeof(currentLine));
        char* trimmedLine = trim(currentLine);

        if (trimmedLine[0] == '#' || trimmedLine[0] == '\0') {
            l = strtok(NULL, "\n");
            continue;
        }

        char* eq = strchr(trimmedLine, '=');
        if (eq) {
            *eq = '\0';
            char* key = trim(trimmedLine);
            char* val = trim(eq + 1);

            if (strcasecmp(key, "ssid") == 0) {
                strncpy(wifiSSID, val, 63);
                wifiSSID[63] = '\0';
                gotSSID = true;
            } else if (strcasecmp(key, "pass") == 0) {
                strncpy(wifiPass, val, 63);
                wifiPass[63] = '\0';
            }
        }
        l = strtok(NULL, "\n");
    }

    if (strcmp(wifiSSID, "NamaWiFiKamu") == 0) return false;
    return gotSSID;
}

int main() {
    const char* test1 = "ssid=MyWiFi \npass= MyPass \n";
    loadWifiConfigMock(test1);
    printf("Test 1: SSID='%s', Pass='%s'\n", wifiSSID, wifiPass);

    const char* test2 = "  ssid = OpenWiFi\n  pass = \n";
    wifiSSID[0] = wifiPass[0] = '\0';
    loadWifiConfigMock(test2);
    printf("Test 2: SSID='%s', Pass='%s'\n", wifiSSID, wifiPass);

    return 0;
}
