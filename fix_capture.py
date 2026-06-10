import sys

with open('camera_test/camera_test.ino', 'r') as f:
    lines = f.readlines()

new_lines = []
skip = False
skip_inner = False
skip_old_eis = False
skip_old_jpg = False
skip_bmp = False

for i, line in enumerate(lines):
    if 'void captureAndPreview() {' in line:
        new_lines.append(line)
        skip = True
        continue

    if skip:
        if 'if (fb->format == PIXFORMAT_RGB565 && fb->width == DISP_W) {' in line:
            new_lines.append('  int curRot = lcd.getRotation();\n')
            new_lines.append('  int targetRot = (curRot == 0) ? 3 : (curRot == 1) ? 2 : (curRot == 2) ? 1 : 0;\n')
            new_lines.append('  if (fb->format == PIXFORMAT_RGB565 && fb->width == DISP_W) {\n')
            new_lines.append('    uint16_t* drawBuf = (uint16_t*)fb->buf;\n')
            new_lines.append('    uint16_t* tmpRot = nullptr;\n')
            new_lines.append('    if (curRot != 3) {\n')
            new_lines.append('      tmpRot = (uint16_t*)ps_malloc(DISP_W * DISP_H * 2);\n')
            new_lines.append('      if (tmpRot) {\n')
            new_lines.append('        rotateBuffer((uint16_t*)fb->buf, tmpRot, DISP_W, DISP_H, targetRot);\n')
            new_lines.append('        drawBuf = tmpRot;\n')
            new_lines.append('      }\n')
            new_lines.append('    }\n')
            new_lines.append('    if (curRot == 0 || curRot == 2) lcd.pushImage(0, 0, DISP_H, DISP_W, drawBuf);\n')
            new_lines.append('    else lcd.pushImage(0, 0, DISP_W, DISP_H, drawBuf);\n')
            new_lines.append('    if (tmpRot) free(tmpRot);\n')
            new_lines.append('  }\n')
            skip_inner = True
            continue

        if 'drawCornerBrackets(COL_GRAY_E);' in line:
             new_lines.append(line)
             skip_inner = False
             continue

        if 'bool isGCrgb = (detectedSensor == PID_GC2145 && fb->format == PIXFORMAT_RGB565 && fb->width == DISP_W && fb->height == DISP_H);' in line:
             new_lines.append('    uint16_t* finalBuf = (uint16_t*)fb->buf;\n')
             new_lines.append('    uint16_t* eisBuf = nullptr;\n')
             new_lines.append('    uint16_t* rotBuf = nullptr;\n')
             new_lines.append('    int finalW = DISP_W, finalH = DISP_H;\n')
             new_lines.append('    if (fb->format == PIXFORMAT_RGB565) {\n')
             new_lines.append('      if (eisEnabled) {\n')
             new_lines.append('        eisBuf = (uint16_t*)ps_malloc(DISP_W * DISP_H * 2);\n')
             new_lines.append('        if (eisBuf) {\n')
             new_lines.append('          applyEIS((uint16_t*)fb->buf, eisBuf, DISP_W, DISP_H, g_eisOffX, g_eisOffY);\n')
             new_lines.append('          finalBuf = eisBuf;\n')
             new_lines.append('        }\n')
             new_lines.append('      }\n')
             new_lines.append('      if (curRot != 3) {\n')
             new_lines.append('        rotBuf = (uint16_t*)ps_malloc(DISP_W * DISP_H * 2);\n')
             new_lines.append('        if (rotBuf) {\n')
             new_lines.append('          rotateBuffer(finalBuf, rotBuf, DISP_W, DISP_H, targetRot);\n')
             new_lines.append('          finalBuf = rotBuf;\n')
             new_lines.append('          if (targetRot == 1 || targetRot == 3) { finalW = DISP_H; finalH = DISP_W; }\n')
             new_lines.append('        }\n')
             new_lines.append('      }\n')
             new_lines.append('    }\n')
             new_lines.append(line)
             skip_old_eis = True
             continue

        if skip_old_eis:
             if 'char path[48]; snprintf(path, sizeof(path), "/sdcard/photo_%04d.bmp", photoCount);' in line:
                  new_lines.append('      char path[48]; snprintf(path, sizeof(path), "/sdcard/photo_%04d.bmp", photoCount);\n')
                  new_lines.append('      char payload[32]; stegoMakePayload(payload, sizeof(payload), photoCount);\n')
                  new_lines.append('      saved = saveBMP((uint8_t*)finalBuf, finalW, finalH, path, payload, (int)strlen(payload));\n')
                  skip_bmp = True
                  continue
             if skip_bmp:
                  if 'uint8_t* jpg = nullptr; size_t jLen = 0; bool ok = false;' in line:
                       new_lines.append('    } else {\n')
                       new_lines.append('      uint8_t* jpg = nullptr; size_t jLen = 0; bool ok = false;\n')
                       new_lines.append('      if (fb->format == PIXFORMAT_RGB565) {\n')
                       new_lines.append('        camera_fb_t fk = *fb; fk.buf = (uint8_t*)finalBuf; fk.width = finalW; fk.height = finalH;\n')
                       new_lines.append('        int captureQ = hdCaptureEnabled ? map(hdCaptureQuality, 1, 10, 95, 50) : 85;\n')
                       new_lines.append('        ok = frame2jpg(&fk, captureQ, &jpg, &jLen);\n')
                       skip_old_jpg = True
                       continue
             if skip_old_jpg:
                  if 'if (ok && jpg && jLen > 0) {' in line:
                       new_lines.append('      if (eisBuf) free(eisBuf);\n')
                       new_lines.append('      if (rotBuf) free(rotBuf);\n')
                       new_lines.append(line)
                       skip_old_eis = False
                       skip_old_jpg = False
                       skip_bmp = False
                       continue
                  continue

        if 'esp_camera_fb_return(fb);' in line:
             new_lines.append(line)
             skip = False
             continue

        if skip_inner:
             continue
        new_lines.append(line)
    else:
        new_lines.append(line)

with open('camera_test/camera_test.ino', 'w') as f:
    f.writelines(new_lines)
