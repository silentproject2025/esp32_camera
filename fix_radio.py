import sys

with open('camera_test/camera_test.ino', 'r') as f:
    lines = f.readlines()

# Find drawRadioUI and handleModeRadio
draw_radio_idx = -1
handle_radio_idx = -1
handle_exp_idx = -1

for i, line in enumerate(lines):
    if 'void drawRadioUI() {' in line:
        draw_radio_idx = i
    if 'void handleModeRadio(ButtonEvent evt) {' in line:
        handle_radio_idx = i
    if 'void handleModeMenuExpAdj(ButtonEvent evt){' in line:
        handle_exp_idx = i

if draw_radio_idx != -1 and handle_radio_idx != -1:
    # We want to replace the hint section at the end of drawRadioUI
    # The hint section is roughly at handle_radio_idx - 6 to handle_radio_idx - 2
    new_hint = [
        '  // Footer Hint\n',
        '  lcd.drawFastHLine(mx + 10, my + mh - 25, mw - 20, COL_GRAY_3);\n',
        '  lcd.setTextColor(COL_GRAY_8);\n',
        '  lcd.setFont(&fonts::Font0);\n',
        '  const char* hint = "C/D: Tune (Hold:Seek) B:Vol (Hold:Exit)";\n',
        '  lcd.drawString(hint, mx + (mw - lcd.textWidth(hint)) / 2, my + mh - 16);\n',
        '}\n',
        '\n'
    ]
    # Find where Footer Hint starts
    footer_start = -1
    for i in range(draw_radio_idx, handle_radio_idx):
        if '// Footer Hint' in lines[i]:
            footer_start = i
            break

    if footer_start != -1:
        # Replace from footer_start to handle_radio_idx
        lines[footer_start:handle_radio_idx] = new_hint

# Recalculate indices after replacement
for i, line in enumerate(lines):
    if 'void handleModeRadio(ButtonEvent evt) {' in line:
        handle_radio_idx = i
    if 'void handleModeMenuExpAdj(ButtonEvent evt){' in line:
        handle_exp_idx = i

if handle_radio_idx != -1 and handle_exp_idx != -1:
    new_handle_radio = [
        'void handleModeRadio(ButtonEvent evt) {\n',
        '  static uint32_t lastUpdate = 0;\n',
        '  if (millis() - lastUpdate > 500) {\n',
        '    radioFreq = radio.getFrequency();\n',
        '    RADIO_INFO info;\n',
        '    radio.checkRDS(); radio.getRadioInfo(&info);\n',
        '    radioStereo = info.stereo;\n',
        '    lastUpdate = millis();\n',
        '    drawRadioUI();\n',
        '  }\n',
        '\n',
        '  if (!evt.valid) return;\n',
        '\n',
        '  if (evt.pin == BTN_B) {\n',
        '    if (evt.isLong) {\n',
        '      radio.setMute(true);\n',
        '      appMode = MODE_FEATURES;\n',
        '      drawFeaturesMenu(menuFeatSel);\n',
        '      resetAllButtons();\n',
        '      return;\n',
        '    } else {\n',
        '      radioVol = (radioVol + 1) % 16;\n',
        '      radio.setVolume(radioVol);\n',
        '    }\n',
        '  } else if (evt.pin == BTN_C) {\n',
        '    if (evt.isLong) {\n',
        '      radio.seekDown(true);\n',
        '    } else {\n',
        '      radioFreq -= 10;\n',
        '      if (radioFreq < 8750) radioFreq = 10800;\n',
        '      radio.setFrequency(radioFreq);\n',
        '    }\n',
        '  } else if (evt.pin == BTN_D) {\n',
        '    if (evt.isLong) {\n',
        '      radio.seekUp(true);\n',
        '    } else {\n',
        '      radioFreq += 10;\n',
        '      if (radioFreq > 10800) radioFreq = 8750;\n',
        '      radio.setFrequency(radioFreq);\n',
        '    }\n',
        '  } else if (evt.pin == BTN_BOOT) {\n',
        '    radioMute = !radioMute;\n',
        '    radio.setMute(radioMute);\n',
        '  }\n',
        '\n',
        '  drawRadioUI();\n',
        '}\n',
        '\n'
    ]
    lines[handle_radio_idx:handle_exp_idx] = new_handle_radio

with open('camera_test/camera_test.ino', 'w') as f:
    f.writelines(lines)
