import re

with open('camera_test/camera_test.ino', 'r') as f:
    content = f.read()

# Define the new blocks
jpg_block = """    if (ft == GFILE_JPG) {
      char path[64]; snprintf(path, sizeof(path), "/sdcard/%s", galleryFiles[idx]);
      bool ok = false;
      FILE* f = fopen(path, "rb");
      if (f) {
        fseek(f, 0, SEEK_END);
        size_t fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        if (fsize > 0 && fsize < 250000) {
          uint8_t* buf = (uint8_t*)ps_malloc(fsize);
          if (buf) {
            if (fread(buf, 1, fsize, f) == fsize) {
              uint16_t w, h;
              TJpgDec.setJpgScale(4);
              if (TJpgDec.getJpgSize(&w, &h, buf, fsize) == JDR_OK) {
                int tw = w / 4, th = h / 4;
                int tx = x + 2 + (72 - tw) / 2;
                int ty = y + 2 + (42 - th) / 2;
                lcd.setClipRect(x + 2, y + 2, 72, 42);
                if (TJpgDec.drawJpg(tx, ty, buf, fsize) == JDR_OK) ok = true;
                lcd.clearClipRect();
              }
            }
            free(buf);
          }
        }
        fclose(f);
      }
      if (!ok) {
        lcd.setTextColor(COL_GRAY_7);
        lcd.drawString("JPG", x + 28, y + 20);
      }
    } else if (ft == GFILE_BMP) {
      for (int r = 0; r < 42; r++) {
        uint16_t gCol = lcd.color565(0, 0, 20 + r / 2);
        lcd.drawFastHLine(x + 2, y + 2 + r, 72, gCol);
      }
      lcd.setTextColor(COL_BMP_ACCENT);
      lcd.drawString("BMP", x + (76 - lcd.textWidth("BMP")) / 2, y + 18);
    } else if (ft == GFILE_VIDEO) {
      lcd.fillRect(x + 2, y + 2, 72, 42, COL_GRAY_2);
      int cx = x + 38, cy = y + 21;
      lcd.fillTriangle(cx - 8, cy - 8, cx - 8, cy + 8, cx + 10, cy, COL_GRAY_7);
      char vidNum[12] = ""; int vn = 0;
      for (int j = 0; galleryFiles[idx][j] && vn < 11; j++)
        if (isdigit(galleryFiles[idx][j])) vidNum[vn++] = galleryFiles[idx][j];
      vidNum[vn] = '\0';
      lcd.setTextColor(COL_VID_ACCENT);
      lcd.drawString(vidNum, x + (76 - lcd.textWidth(vidNum)) / 2, y + 34);
    }"""

# Find the start and end of the GFILE blocks in drawGalleryGrid
pattern = r'if \(ft == GFILE_JPG\) \{.*?\} else if \(ft == GFILE_VIDEO\) \{.*?\}'
content = re.sub(pattern, jpg_block, content, flags=re.DOTALL)

# Add TJpgDec.setJpgScale(1) after the loop
# The loop ends before the bar drawing logic
loop_end_pattern = r'(lcd\.drawString\(idxBuf, x \+ 4, y \+ 46\);\s+\}\s+)(if \(galleryCount > 16\))'
content = re.sub(loop_end_pattern, r'\1TJpgDec.setJpgScale(1);\n  \2', content)

with open('camera_test/camera_test.ino', 'w') as f:
    f.write(content)
