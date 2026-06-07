import sys

with open('camera_test/camera_test.ino', 'r') as f:
    lines = f.readlines()

# Find the problematic section
start_idx = -1
end_idx = -1
for i, line in enumerate(lines):
    if 'void updateRDS() {}' in line:
        start_idx = i
    if start_idx != -1 and line.strip() == '}':
        if i > start_idx:
            end_idx = i
            # Check if this is the end of the stray block
            if i + 2 < len(lines) and 'void radioInit()' in lines[i+2]:
                break

if start_idx != -1 and end_idx != -1:
    print(f"Removing lines {start_idx} to {end_idx}")
    del lines[start_idx+1 : end_idx+1]

with open('camera_test/camera_test.ino', 'w') as f:
    f.writelines(lines)
