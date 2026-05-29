def check_braces(file_path):
    with open(file_path, 'r') as f:
        content = f.read()

    stack = []
    lines = content.split('\n')
    for i, line in enumerate(lines):
        # Primitive check, ignores comments and strings
        # But should give a hint
        clean_line = line.split('//')[0]
        for char in clean_line:
            if char == '{':
                stack.append(i + 1)
            elif char == '}':
                if not stack:
                    print(f"Extra closing brace at line {i + 1}")
                    return False
                stack.pop()

    if stack:
        print(f"Unclosed braces starting at lines: {stack}")
        return False

    print("Braces look balanced.")
    return True

check_braces('camera_test/camera_test.ino')
