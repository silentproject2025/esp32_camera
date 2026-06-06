import sys

def make_compact():
    with open('camera_test/camera_test.ino', 'r') as f:
        lines = f.readlines()

    new_lines = []
    for line in lines:
        if 'int mw = 220, mh = 314, mx = (DISP_W - mw) / 2, my = (DISP_H - mh) / 2;' in line:
            line = line.replace('mh = 314', 'mh = 230')
            line = line.replace('my = (DISP_H - mh) / 2', 'my = 5')
        if 'int iy = my + 24 + i * 22;' in line:
            line = line.replace('24', '20').replace('22', '15')
        if 'lcd.fillRect(mx + 8, iy, mw - 16, 18, hl ? COL_GRAY_5 : COL_GRAY_D);' in line:
            line = line.replace('18', '14')
        if 'if (hl) lcd.fillRect(mx + 2, iy, 4, 18, COL_WHITE);' in line:
            line = line.replace('18', '14')
        if 'lcd.drawFastHLine(mx + 10, my + mh - 24, mw - 20, COL_GRAY_3);' in line:
            line = line.replace('mh - 24', 'mh - 18')
        if 'my + mh - 14);' in line:
            line = line.replace('mh - 14', 'mh - 10')

        new_lines.append(line)

    with open('camera_test/camera_test.ino', 'w') as f:
        f.writelines(new_lines)

if __name__ == "__main__":
    make_compact()
