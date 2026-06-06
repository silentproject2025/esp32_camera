import sys

def fix_menu():
    with open('camera_test/camera_test.ino', 'r') as f:
        lines = f.readlines()

    new_lines = []
    i = 0
    while i < len(lines):
        line = lines[i]

        # 1. Update rowLabels size and add BT MP3 PLAYER
        if 'static const char* const rowLabels[12]' in line:
            line = line.replace('rowLabels[12]', 'rowLabels[13]')
            line = line.replace('"CLEAR THUMB CACHE"', '"CLEAR THUMB CACHE", "BT MP3 PLAYER  A2DP Source"')

        # 2. Update loop limit in drawFeaturesMenu
        if 'for (int i = 0; i < 12; i++) {' in line:
            line = line.replace('i < 12', 'i < 13')

        # 3. Add rendering logic for index 12 in drawFeaturesMenu
        if 'else if (i == 11) {' in line:
            new_lines.append(line)
            i += 1
            # Go to the end of the i == 11 block
            while i < len(lines) and '    }' not in lines[i]:
                new_lines.append(lines[i])
                i += 1
            if i < len(lines):
                new_lines.append(lines[i])
                new_lines.append('    else if (i == 12) {\n')
                new_lines.append('      lcd.setTextColor(btConnected ? 0x07E0 : COL_GRAY_7);\n')
                new_lines.append('      lcd.drawString(btConnected ? "CON" : "RUN", mx + mw - 30, iy + 5);\n')
                new_lines.append('    }\n')
                i += 1
                continue

        # 4. Update navigation in handleModeFeatures
        if 'menuFeatSel = (menuFeatSel + 1) % 12;' in line:
            line = line.replace('% 12', '% 13')
        if 'menuFeatSel = (menuFeatSel + 11) % 12;' in line:
            line = line.replace('+ 11) % 12', '+ 12) % 13')

        # 5. Add BT_BOOT trigger logic for index 12 in handleModeFeatures
        if 'else if (menuFeatSel == 11) {' in line:
             new_lines.append(line)
             i += 1
             # Go to the end of the menuFeatSel == 11 block
             while i < len(lines) and '      drawFeaturesMenu(menuFeatSel);\n    }' not in (lines[i-1] + lines[i]):
                 new_lines.append(lines[i])
                 i += 1
             if i < len(lines):
                 new_lines.append(lines[i])
                 new_lines.append('    else if (menuFeatSel == 12) {\n')
                 new_lines.append('      btStartScan();\n')
                 new_lines.append('    }\n')
                 i += 1
                 continue

        new_lines.append(line)
        i += 1

    with open('camera_test/camera_test.ino', 'w') as f:
        f.writelines(new_lines)

if __name__ == "__main__":
    fix_menu()
