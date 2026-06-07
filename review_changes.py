import sys

def check_file():
    with open("camera_test/camera_test.ino", "r") as f:
        lines = f.readlines()

    # Check frequency limits
    found_limits = 0
    for line in lines:
        if "if (radioFreq < 5000) radioFreq = 11500;" in line:
            found_limits += 1
        if "if (radioFreq > 11500) radioFreq = 5000;" in line:
            found_limits += 1

    if found_limits == 2:
        print("PASS: Frequency limits updated to 5000-11500.")
    else:
        print(f"FAIL: Frequency limits not found or incorrect (found {found_limits}/2).")

    # Check BOOT button logic
    found_boot = False
    for i, line in enumerate(lines):
        if "else if (evt.pin == BTN_BOOT) {" in line:
            if "if (evt.isLong) {" in lines[i+1] and "radio.seekUp(true);" in lines[i+2]:
                found_boot = True
                break

    if found_boot:
        print("PASS: BOOT button long-press for auto-seek implemented.")
    else:
        print("FAIL: BOOT button long-press logic not found or incorrect.")

    # Check UI hint
    found_hint = False
    for line in lines:
        if 'const char* hint = "C/D: Tune (Hold:Seek) B:Vol (Hold:Exit) BOOT:Mute (Hold:Scan)";' in line:
            found_hint = True
            break

    if found_hint:
        print("PASS: UI hint updated.")
    else:
        print("FAIL: UI hint not found or incorrect.")

check_file()
