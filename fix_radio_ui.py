import re

with open('camera_test/camera_test.ino', 'r') as f:
    content = f.read()

# Fix openRadio to call drawRadioUI() immediately
content = re.sub(
    r'void openRadio\(\) \{(.*?)\}',
    r'void openRadio() {\1  drawRadioUI();\n}',
    content,
    flags=re.DOTALL
)

# Fix handleModeRadio to include checkRDS and update RDS variables
def fix_handle_mode_radio(match):
    block = match.group(0)
    # Add RDS variables update in the timer block
    rds_logic = """
    radio.checkRDS();
    radio.getRadioInfo(&info);
    radioStereo = info.stereo;
    """
    # Replace existing logic
    if 'radio.getRadioInfo(&info);' in block:
        block = re.sub(r'radio\.getRadioInfo\(&info\);', 'radio.checkRDS(); radio.getRadioInfo(&info);', block)

    # We should also update radioRDSStation and radioRDSText
    # but that might be better done in a separate callback or if the library provides it.
    # For now, let's just make sure it doesn't hang.
    return block

content = re.sub(r'void handleModeRadio\(ButtonEvent evt\) \{.*?\}', fix_handle_mode_radio, content, flags=re.DOTALL)

with open('camera_test/camera_test.ino', 'w') as f:
    f.write(content)
