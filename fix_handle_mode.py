import sys

def fix_handle():
    with open('camera_test/camera_test.ino', 'r') as f:
        lines = f.readlines()

    final_lines = []
    i = 0
    while i < len(lines):
        line = lines[i]
        final_lines.append(line)
        if 'islandPush(NOTIF_WARN, "Cache dir tidak ada");' in line:
            while i + 1 < len(lines) and 'drawFeaturesMenu(menuFeatSel);' not in lines[i+1]:
                i += 1
                final_lines.append(lines[i])
            if i + 1 < len(lines) and 'drawFeaturesMenu(menuFeatSel);' in lines[i+1]:
                i += 1
                final_lines.append(lines[i])
            if i + 1 < len(lines) and '}' in lines[i+1]:
                i += 1
                final_lines.append(lines[i])
                final_lines.append('    else if (menuFeatSel == 12) {\n')
                final_lines.append('      btStartScan();\n')
                final_lines.append('    }\n')
        i += 1

    with open('camera_test/camera_test.ino', 'w') as f:
        f.writelines(final_lines)

if __name__ == "__main__":
    fix_handle()
