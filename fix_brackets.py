import sys

with open('camera_test/camera_test.ino', 'r') as f:
    lines = f.readlines()

new_lines = []
for line in lines:
    if 'void drawCornerBrackets(uint16_t col=COL_WHITE){' in line:
        new_lines.append('void drawCornerBrackets(uint16_t col=COL_WHITE){\n')
        new_lines.append('  int w = lcd.width(), h = lcd.height();\n')
        new_lines.append('  int x0=3,y0=3,x1=w-4,y1=h-4,L=BRACKET_LEN;\n')
        continue
    if 'int x0=3,y0=3,x1=DISP_W-4,y1=DISP_H-4,L=BRACKET_LEN;' in line:
        continue
    new_lines.append(line)

with open('camera_test/camera_test.ino', 'w') as f:
    f.writelines(new_lines)
