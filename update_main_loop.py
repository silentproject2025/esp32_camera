import sys

def update_main_loop():
    with open('camera_test/camera_test.ino', 'r') as f:
        lines = f.readlines()

    new_lines = []
    for line in lines:
        if 'tickAllButtons();' in line:
            new_lines.append('  btPlayerTick();\n')
            new_lines.append(line)
            continue

        if 'case MODE_KEY_MANAGER:' in line:
            new_lines.append(line)
            new_lines.append('    case MODE_BT_SCAN:         handleModeBTScan(singleEvt);                    break;\n')
            new_lines.append('    case MODE_BT_MP3_LIST:     handleModeBTMP3List(singleEvt);                break;\n')
            new_lines.append('    case MODE_BT_MP3_PLAYER:   handleModeBTMP3Player(singleEvt);              break;\n')
            continue

        if 'bool isMenu = (' in line:
            line = line.replace('MODE_DIALOG_MULTI_DELETE)', 'MODE_DIALOG_MULTI_DELETE || appMode == MODE_BT_SCAN || appMode == MODE_BT_MP3_LIST || appMode == MODE_BT_MP3_PLAYER)')
            new_lines.append(line)
            continue

        new_lines.append(line)

    with open('camera_test/camera_test.ino', 'w') as f:
        f.writelines(new_lines)

if __name__ == "__main__":
    update_main_loop()
