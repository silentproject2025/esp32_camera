import sys

with open('camera_test/camera_test.ino', 'r') as f:
    lines = f.readlines()

new_lines = []
skip = False
for i, line in enumerate(lines):
    if 'void renderViewfinder(){' in line:
        new_lines.append(line)
        new_lines.append('  bool frozen=(millis()<islandFreezeUntilMs);\n')
        new_lines.append('  islandNoClear=true;\n')
        new_lines.append('  camera_fb_t *fb=nullptr;\n')
        new_lines.append('  if(!frozen){fb=esp_camera_fb_get();if(!fb) return;}\n')
        new_lines.append('  if(!frozen){\n')
        new_lines.append('    if(fb->format==PIXFORMAT_RGB565&&fb->width==DISP_W&&fb->height==DISP_H){\n')
        new_lines.append('      uint16_t* drawBuf = (uint16_t*)fb->buf;\n')
        new_lines.append('      uint16_t* tmp = nullptr;\n')
        new_lines.append('      if (eisEnabled || lcd.getRotation() != 3) {\n')
        new_lines.append('        tmp = (uint16_t*)ps_malloc(DISP_W * DISP_H * 2);\n')
        new_lines.append('        if (tmp) {\n')
        new_lines.append('          if (eisEnabled) applyEIS((uint16_t*)fb->buf, tmp, DISP_W, DISP_H, g_eisOffX, g_eisOffY);\n')
        new_lines.append('          else memcpy(tmp, fb->buf, DISP_W * DISP_H * 2);\n')
        new_lines.append('          drawBuf = tmp;\n')
        new_lines.append('        }\n')
        new_lines.append('      }\n')
        new_lines.append('      int curRot = lcd.getRotation();\n')
        new_lines.append('      if (curRot == 3) {\n')
        new_lines.append('        lcd.pushImage(0, 0, DISP_W, DISP_H, drawBuf);\n')
        new_lines.append('      } else {\n')
        new_lines.append('        uint16_t* rotBuf = (uint16_t*)ps_malloc(DISP_W * DISP_H * 2);\n')
        new_lines.append('        if (rotBuf) {\n')
        new_lines.append('          int targetRot = (curRot == 0) ? 3 : (curRot == 1) ? 2 : (curRot == 2) ? 1 : 0;\n')
        new_lines.append('          rotateBuffer(drawBuf, rotBuf, DISP_W, DISP_H, targetRot);\n')
        new_lines.append('          if (curRot == 0 || curRot == 2) lcd.pushImage(0, 0, DISP_H, DISP_W, rotBuf);\n')
        new_lines.append('          else lcd.pushImage(0, 0, DISP_W, DISP_H, rotBuf);\n')
        new_lines.append('          free(rotBuf);\n')
        new_lines.append('        } else {\n')
        new_lines.append('           lcd.pushImage(0, 0, DISP_W, DISP_H, drawBuf, DISP_W);\n')
        new_lines.append('        }\n')
        new_lines.append('      }\n')
        new_lines.append('      if (tmp) free(tmp);\n')
        skip = True
        continue

    if skip and 'uint16_t bktCol=recActive?0xF800:(COL_GRAY_E);' in line:
        skip = False

    if not skip:
        new_lines.append(line)

with open('camera_test/camera_test.ino', 'w') as f:
    f.writelines(new_lines)
