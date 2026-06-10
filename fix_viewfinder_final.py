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
        new_lines.append('      uint16_t bktCol=recActive?0xF800:(COL_GRAY_E);\n')
        new_lines.append('      drawCornerBrackets(bktCol);\n')
        new_lines.append('      if (recActive) {\n')
        new_lines.append('        lcd.fillCircle(10, 10, 4, (millis()/500%2) ? 0xF800 : 0x4000);\n')
        new_lines.append('      }\n')
        new_lines.append('      if (hudEnabled) {\n')
        new_lines.append('        char fpsBuf[12]; snprintf(fpsBuf,sizeof(fpsBuf),"%.0f fps",fpsValue);\n')
        new_lines.append('        drawPill(32,10,fpsBuf,COL_PILL_BG,COL_GRAY_A);\n')
        new_lines.append('        if (!recActive && !g_tilted && !g_shake) neoBreath(0, 0, 80);\n')
        new_lines.append('        char sensorPill[20];\n')
        new_lines.append('        snprintf(sensorPill,sizeof(sensorPill),"%s%s",sensorName,ledFlashEnabled?" *":"");\n')
        new_lines.append('        drawPill(DISP_W-42,10,sensorPill,COL_PILL_BG,COL_GRAY_A);\n')
        new_lines.append('        if(expPreset>0){\n')
        new_lines.append('          char expBuf[12];\n')
        new_lines.append('          if(expPreset==5) snprintf(expBuf,sizeof(expBuf),"M %d",expManualVal);\n')
        new_lines.append('          else             snprintf(expBuf,sizeof(expBuf),"%s",expPresetNames[expPreset]);\n')
        new_lines.append('          drawPill(DISP_W/2,10,expBuf,COL_PILL_BG,COL_GRAY_E);\n')
        new_lines.append('        }\n')
        new_lines.append('        const char* fmtTag;\n')
        new_lines.append('        if(detectedSensor==PID_GC2145) fmtTag=(gc2145CaptureFormat==GFMT_BMP)?"BMP":"JPG";\n')
        new_lines.append('        else fmtTag="JPG";\n')
        new_lines.append('        char shotBuf[12]; snprintf(shotBuf,sizeof(shotBuf),"#%04d %s",photoCount+1,fmtTag);\n')
        new_lines.append('        drawPill(38,DISP_H-10,shotBuf,COL_PILL_BG,COL_GRAY_8);\n')
        new_lines.append('        drawPill(DISP_W-36,DISP_H-10,sdReady?"SD  OK":"SD  --",\n')
        new_lines.append('                 COL_PILL_BG,sdReady?COL_GRAY_8:COL_GRAY_5);\n')
        new_lines.append('        lcd.setFont(&fonts::Font0); lcd.setTextColor(COL_GRAY_3);\n')
        new_lines.append('        lcd.drawString("Clong=AI", 70, DISP_H-10); lcd.drawString("Dshort=FEAT", 170, DISP_H-10);\n')
        new_lines.append('        if(recActive) drawRecIndicator();\n')
        new_lines.append('        if(hdrEnabled) drawPill(DISP_W/2, 35, "HDR", COL_PILL_BG, 0x07E0);\n')
        new_lines.append('        if(eisEnabled) { char eb[12]; snprintf(eb, sizeof(eb), recActive ? "EIS●" : "EIS"); drawPill(DISP_W/2 - (hdrEnabled ? 45 : 0), 35, eb, COL_PILL_BG, 0xCE59); }\n')
        new_lines.append('        mpuDrawIndicator();\n')
        new_lines.append('      }\n')
        new_lines.append('      updateFPS();\n')
        new_lines.append('    } else {\n')
        new_lines.append('      lcd.fillScreen(COL_BLACK);\n')
        new_lines.append('      lcd.setFont(&fonts::Font0);lcd.setTextColor(COL_GRAY_5);\n')
        new_lines.append('      lcd.drawString("format not rgb565",10,110);\n')
        new_lines.append('    }\n')
        new_lines.append('    esp_camera_fb_return(fb);\n')
        new_lines.append('  }\n')
        new_lines.append('}\n')
        skip = True
        continue

    if skip:
        if 'void captureAndPreview' in line:
            skip = False
        else:
            continue

    new_lines.append(line)

with open('camera_test/camera_test.ino', 'w') as f:
    f.writelines(new_lines)
