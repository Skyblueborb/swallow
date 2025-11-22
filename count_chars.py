import os
import re
import sys

# Configuration
SEARCH_DIR = "."
MAX_CHAR_LENGTH = 1024
FILE_EXTENSION = ".c"

def strip_comments_and_strings(text):
    """
    Replaces comments and strings with spaces to preserve
    byte/char offsets while removing distractions for the parser.
    """
    pattern = r"(\"(\\.|[^\"\\])*\"|'(\\.|[^'\\])*')|(/\*.*?\*/|//[^\r\n]*$)"
    regex = re.compile(pattern, re.MULTILINE | re.DOTALL)

    def _replacer(match):
        return " " * len(match.group(0))

    return regex.sub(_replacer, text)

def get_context_header(text, start_index):
    """
    Looks backwards from the opening brace '{' to find the function signature.
    """
    lookback_limit = max(0, start_index - 500)
    preceding_text = text[lookback_limit:start_index]
    tokens = re.split(r'[;}]', preceding_text)
    header = tokens[-1]
    return header.strip()

def is_function_definition(header):
    """
    Heuristics to determine if the header looks like a function.
    """
    if not header:
        return False

    keywords = ['if', 'while', 'for', 'switch', 'do', 'else']
    for kw in keywords:
        if header.startswith(kw + " ") or header.startswith(kw + "("):
            return False
        if header == kw:
            return False

    if header.endswith('='):
        return False

    # Note: Standard C functions usually end with ')', but macros or K&R might not.
    # We allow loose matching here as long as it isn't a control structure.
    return True

def analyze_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            raw_content = f.read()
    except Exception as e:
        print(f"Skipping {filepath}: {e}")
        return

    # clean_content has comments/strings replaced by spaces
    clean_content = strip_comments_and_strings(raw_content)

    brace_level = 0
    start_index = -1

    for i, char in enumerate(clean_content):
        if char == '{':
            if brace_level == 0:
                start_index = i
            brace_level += 1

        elif char == '}':
            brace_level -= 1

            # Found a closed top-level block
            if brace_level == 0 and start_index != -1:
                end_index = i

                # 1. CHECK AFTER: Is it a struct/enum?
                is_data_structure = False
                for k in range(end_index + 1, len(clean_content)):
                    if clean_content[k].isspace():
                        continue
                    if clean_content[k] == ';':
                        is_data_structure = True
                    break

                if is_data_structure:
                    start_index = -1
                    continue

                # 2. CHECK BEFORE: Analyze the header
                header = get_context_header(raw_content, start_index)

                if not is_function_definition(header):
                    start_index = -1
                    continue

                # 3. Calculate Length (IGNORING WHITESPACE)
                # Extract the raw body segment
                raw_body = raw_content[start_index:end_index+1]

                # Strip comments/strings specifically for the count (optional,
                # but recommended if "code length" implies ignoring comments too).
                # If you want to count comment text but ignore whitespace, use raw_body.
                # If you want to count only CODE text and ignore whitespace, use clean_body.
                clean_body = clean_content[start_index:end_index+1]

                # Remove all whitespace (\n, \t, space)
                # Using clean_body means we don't count text inside comments,
                # but we do count the spaces that replaced them.
                # To be strictly "code characters":
                text_to_count = re.sub(r'\s+', '', clean_body)

                func_length = len(text_to_count)

                if func_length > MAX_CHAR_LENGTH:
                    match = re.search(r'(\w+)\s*\(', header)
                    func_name = match.group(1) if match else "Unknown"

                    if func_name == "Unknown":
                        func_name = header[-50:].replace('\n', ' ')

                    print(f"[ALERT] {filepath}")
                    print(f"    Function: {func_name}(...)")
                    print(f"    Length:   {func_length} non-whitespace chars (Limit: {MAX_CHAR_LENGTH})")
                    print("-" * 40)

                start_index = -1

def main():
    target = sys.argv[1] if len(sys.argv) > 1 else SEARCH_DIR
    print(f"Scanning {os.path.abspath(target)} for .c functions > {MAX_CHAR_LENGTH} non-whitespace chars...\n")

    count = 0
    for root, _, files in os.walk(target):
        for file in files:
            if file.endswith(FILE_EXTENSION):
                analyze_file(os.path.join(root, file))
                count += 1

    if count == 0:
        print("No .c files found.")

if __name__ == "__main__":
    main()
