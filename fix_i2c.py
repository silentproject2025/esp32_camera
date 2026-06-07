import re

with open('camera_test/camera_test.ino', 'r') as f:
    content = f.read()

# Remove TwoWire WireRadio = TwoWire(1);
content = re.sub(r'TwoWire WireRadio = TwoWire\(1\);', '', content)

# Update radioInit
def fix_radio_init(match):
    block = match.group(0)
    # Remove WireRadio.begin(...)
    block = re.sub(r'WireRadio\.begin\(.*?\);', '', block)
    # Ensure radio.initWire(Wire) is present
    if 'radio.initWire' in block:
        block = re.sub(r'radio\.initWire\(.*?\);', 'radio.initWire(Wire);', block)
    else:
        # If it was accidentally removed, add it back after the opening brace
        block = re.sub(r'(void radioInit\(\) \{)', r'\1\n  radio.initWire(Wire);', block)
    return block

content = re.sub(r'void radioInit\(\) \{.*?\}', fix_radio_init, content, flags=re.DOTALL)

with open('camera_test/camera_test.ino', 'w') as f:
    f.write(content)
