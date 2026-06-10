import sys

with open('camera_test/camera_test.ino', 'r') as f:
    lines = f.readlines()

new_lines = []
in_viewfinder = False
in_mpu_indicator = False

for line in lines:
    if 'void renderViewfinder(){' in line:
        in_viewfinder = True
    if in_viewfinder and 'if (hudEnabled) {' in line:
        new_lines.append(line)
        new_lines.append('        int lw = lcd.width(), lh = lcd.height();\n')
        continue

    if in_viewfinder:
        line = line.replace('drawPill(DISP_W-42,10', 'drawPill(lw-42,10')
        line = line.replace('drawPill(DISP_W/2,10', 'drawPill(lw/2,10')
        line = line.replace('38,DISP_H-10', '38,lh-10')
        line = line.replace('drawPill(DISP_W-36,DISP_H-10', 'drawPill(lw-36,lh-10')
        line = line.replace('70, DISP_H-10', '70, lh-10')
        line = line.replace('170, DISP_H-10', '170, lh-10')
        line = line.replace('drawPill(DISP_W/2, 35', 'drawPill(lw/2, 35')
        line = line.replace('drawPill(DISP_W/2 -', 'drawPill(lw/2 -')

    if 'void mpuDrawIndicator() {' in line:
        in_mpu_indicator = True
    if in_mpu_indicator:
        line = line.replace('drawPill(DISP_W - 36, DISP_H - 22', 'drawPill(lcd.width() - 36, lcd.height() - 22')

    if in_viewfinder and 'updateFPS();' in line:
        in_viewfinder = False
    if in_mpu_indicator and line.strip() == '}':
        in_mpu_indicator = False

    new_lines.append(line)

with open('camera_test/camera_test.ino', 'w') as f:
    f.writelines(new_lines)
