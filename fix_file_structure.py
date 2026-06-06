import sys

def fix():
    with open('camera_test/camera_test.ino', 'r') as f:
        content = f.read()

    lines = content.splitlines(keepends=True)

    # 1. Identify Blocks
    # Globals
    start_g = content.find("// Bluetooth MP3 globals")
    end_g = content.find("AudioOutputBT *outBT = nullptr;") + len("AudioOutputBT *outBT = nullptr;") + 1
    g_block = content[start_g:end_g]

    # Logic (btStartScan to end of handleModeBTMP3Player)
    start_l = content.find("void btStartScan() {")
    end_tag = "void handleModeFeatures(ButtonEvent evt) {"
    end_l = content.find(end_tag)
    l_block = content[start_l:end_l]

    # AppMode enum and rest
    start_am = content.find("enum AppMode {")

    # Temporarily remove g_block and l_block
    content = content.replace(g_block, "")
    content = content.replace(l_block, "")

    # Now find insertion points in the modified content
    # Insertion point for globals: before drawFeaturesMenu
    pos_dfm = content.find("void drawFeaturesMenu(int sel) {")
    content = content[:pos_dfm] + g_block + "\\n\\n" + content[pos_dfm:]

    # Insertion point for logic: after drawFeaturesMenu's end and before handleModeFeatures
    pos_hmf = content.find("void handleModeFeatures(ButtonEvent evt) {")
    content = content[:pos_hmf] + l_block + "\\n\\n" + content[pos_hmf:]

    # Final cleanup of double newlines and escaped newlines
    content = content.replace("\\n", "\\n") # ensures they are escaped for next step

    with open('camera_test/camera_test.ino', 'w') as f:
        f.write(content)

if __name__ == "__main__":
    fix()
