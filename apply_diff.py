import sys

def apply_diff(file_path, diff):
    with open(file_path, 'r') as f:
        content = f.read()

    sections = diff.split('<<<<<<< SEARCH\n')
    for section in sections[1:]:
        search_replace = section.split('=======\n')
        if len(search_replace) != 2:
            print("Error: Missing ======= divider")
            continue
        search_part, replace_part = search_replace
        replace_part, footer = replace_part.split('>>>>>>> REPLACE\n')

        if search_part in content:
            content = content.replace(search_part, replace_part, 1)
        else:
            print(f"Error: Search part not found:\n{search_part}")
            sys.exit(1)

    with open(file_path, 'w') as f:
        f.write(content)

if __name__ == "__main__":
    file_path = sys.argv[1]
    diff = sys.stdin.read()
    apply_diff(file_path, diff)
