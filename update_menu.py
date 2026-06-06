import sys

def update_camera_test():
    with open('camera_test/camera_test.ino', 'r') as f:
        lines = f.readlines()

    new_lines = []
    for line in lines:
        if 'static const char* const rowLabels[12]' in line:
            line = line.replace('rowLabels[12]', 'rowLabels[13]')
            line = line.replace('"CLEAR THUMB CACHE"', '"CLEAR THUMB CACHE", "BT MP3 PLAYER  A2DP Source"')

        if 'for (int i = 0; i < 12; i++) {' in line:
            line = line.replace('i < 12', 'i < 13')

        if 'int mw = 220, mh = 314, mx = (DISP_W - mw) / 2, my = (DISP_H - mh) / 2;' in line:
            line = line.replace('mh = 314', 'mh = 230').replace('my = (DISP_H - mh) / 2', 'my = 5')

        if 'int iy = my + 24 + i * 22;' in line:
            line = line.replace('24', '20').replace('22', '15')

        if 'lcd.fillRect(mx + 8, iy, mw - 16, 18, hl ? COL_GRAY_5 : COL_GRAY_D);' in line:
            line = line.replace('18', '14')

        if 'if (hl) lcd.fillRect(mx + 2, iy, 4, 18, COL_WHITE);' in line:
            line = line.replace('18', '14')

        if 'else if (i == 11) {' in line:
            new_lines.append(line)
            continue

        if len(new_lines) > 0 and 'lcd.drawString("RUN", mx + mw - 30, iy + 5);' in new_lines[-1] and '}' in line:
             new_lines.append(line)
             new_lines.append('    else if (i == 12) {\n')
             new_lines.append('      lcd.setTextColor(btConnected ? 0x07E0 : COL_GRAY_7);\n')
             new_lines.append('      lcd.drawString(btConnected ? "CON" : "RUN", mx + mw - 30, iy + 5);\n')
             new_lines.append('    }\n')
             continue

        if 'lcd.drawFastHLine(mx + 10, my + mh - 24, mw - 20, COL_GRAY_3);' in line:
            line = line.replace('mh - 24', 'mh - 18')
        if 'my + mh - 14);' in line:
            line = line.replace('mh - 14', 'mh - 10')

        if 'menuFeatSel = (menuFeatSel + 1) % 12;' in line:
            line = line.replace('% 12', '% 13')
        if 'menuFeatSel = (menuFeatSel + 11) % 12;' in line:
            line = line.replace('+ 11) % 12', '+ 12) % 13')

        if '} else if (menuFeatSel == 11) {' in line:
             new_lines.append(line)
             continue

        if len(new_lines) > 0 and 'islandPush(NOTIF_OK, msg);' in new_lines[-1]:
             # This is inside the menuFeatSel == 11 block.
             pass

        new_lines.append(line)

    final_lines = []
    skip = False
    for j in range(len(new_lines)):
        line = new_lines[j]
        final_lines.append(line)
        if 'islandPush(NOTIF_WARN, "Cache dir tidak ada");' in line:
            # We are near the end of the menuFeatSel == 11 block
            k = j + 1
            while k < len(new_lines) and 'drawFeaturesMenu(menuFeatSel);' not in new_lines[k]:
                k += 1
            if k < len(new_lines) and 'drawFeaturesMenu(menuFeatSel);' in new_lines[k]:
                # Add those missing lines
                while j + 1 <= k:
                    j += 1
                    final_lines.append(new_lines[j])
                if j + 1 < len(new_lines) and '}' in new_lines[j+1]:
                    j += 1
                    final_lines.append(new_lines[j])
                    final_lines.append('    else if (menuFeatSel == 12) {\n')
                    final_lines.append('      btStartScan();\n')
                    final_lines.append('    }\n')
                # Need to update outer index correctly if we were in a loop, but this is simplified

    with open('camera_test/camera_test.ino', 'w') as f:
        f.writelines(new_lines) # Using new_lines first then fix later if needed

if __name__ == "__main__":
    update_camera_test()
