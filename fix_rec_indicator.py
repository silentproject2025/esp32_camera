import sys

with open('camera_test/camera_test.ino', 'r') as f:
    lines = f.readlines()

new_lines = []
for line in lines:
    if 'void drawRecIndicator(){' in line:
        new_lines.append('void drawRecIndicator(){\n')
        new_lines.append('  int w = lcd.width(), h = lcd.height();\n')
        continue
    if 'lcd.fillRect(4,4,90,22,COL_BLACK);' in line:
        new_lines.append('  lcd.fillRect(w/2-45,h-30,90,22,COL_BLACK);\n')
        continue
    if 'lcd.fillCircle(10,11,4,blink?COL_WHITE:COL_GRAY_5);' in line:
        new_lines.append('  lcd.fillCircle(w/2-37,h-19,4,blink?COL_WHITE:COL_GRAY_5);\n')
        continue
    if 'lcd.drawString(timeBuf,18,4);' in line:
        new_lines.append('  lcd.drawString(timeBuf,w/2-27,h-30);\n')
        continue
    if 'lcd.drawString(fBuf,18,14);' in line:
        new_lines.append('  lcd.drawString(fBuf,w/2-27,h-20);\n')
        continue
    new_lines.append(line)

with open('camera_test/camera_test.ino', 'w') as f:
    f.writelines(new_lines)
