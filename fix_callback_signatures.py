import sys

with open('camera_test/camera_test.ino', 'r') as f:
    lines = f.readlines()

for i, line in enumerate(lines):
    if 'void DisplayServiceName(char *name) {' in line:
        lines[i] = 'void DisplayServiceName(const char *name) {\n'
    if 'void DisplayText(char *text) {' in line:
        lines[i] = 'void DisplayText(const char *text) {\n'

with open('camera_test/camera_test.ino', 'w') as f:
    f.writelines(lines)
