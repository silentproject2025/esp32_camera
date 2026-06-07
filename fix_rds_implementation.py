import sys

with open('camera_test/camera_test.ino', 'r') as f:
    lines = f.readlines()

new_lines = []
skip = 0
for i, line in enumerate(lines):
    if skip > 0:
        skip -= 1
        continue

    # 1. Update RDS_PROCS
    if 'void RDS_PROCS(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {' in line:
        new_lines.append('void DisplayServiceName(char *name) {\n')
        new_lines.append('  if (name && strcmp(radioRDSStation, name) != 0) {\n')
        new_lines.append('    strncpy(radioRDSStation, name, sizeof(radioRDSStation)-1);\n')
        new_lines.append('  }\n')
        new_lines.append('}\n\n')
        new_lines.append('void DisplayText(char *text) {\n')
        new_lines.append('  if (text && strcmp(radioRDSText, text) != 0) {\n')
        new_lines.append('    strncpy(radioRDSText, text, sizeof(radioRDSText)-1);\n')
        new_lines.append('  }\n')
        new_lines.append('}\n\n')
        new_lines.append('void RDS_PROCS(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {\n')
        new_lines.append('  rds.processData(block1, block2, block3, block4);\n')
        new_lines.append('}\n')
        skip = 2 # Skip old RDS_PROCS body
        continue

    # 2. Nullify updateRDS
    if 'void updateRDS() {' in line:
        new_lines.append('void updateRDS() {}\n')
        # Skip until the end of the old function
        j = i
        while j < len(lines) and '}' not in lines[j]:
            j += 1
        skip = j - i
        continue

    # 3. Update radioInit
    if 'void radioInit() {' in line:
        new_lines.append(line)
        new_lines.append('  radio.initWire(Wire);\n')
        new_lines.append('  radio.attachReceiveRDS(RDS_PROCS);\n')
        new_lines.append('  rds.attachServiceNameCallback(DisplayServiceName);\n')
        new_lines.append('  rds.attachTextCallback(DisplayText);\n')
        # skip next two lines as they are replaced
        skip = 2
        continue

    new_lines.append(line)

with open('camera_test/camera_test.ino', 'w') as f:
    f.writelines(new_lines)
