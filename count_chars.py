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
    # Regex to match:
    # 1. Strings (double or single quotes)
    # 2. Block comments /* ... */
    # 3. Line comments // ...
    pattern = r"(\"(\\.|[^\"\\])*\"|'(\\.|[^'\\])*')|(/\*.*?\*/|//[^\r\n]*$)"
    regex = re.compile(pattern, re.MULTILINE | re.DOTALL)

    def _replacer(match):
        # Replace match with spaces of the same length
        return " " * len(match.group(0))

    return regex.sub(_replacer, text)

def get_context_header(text, start_index):
    """
    Looks backwards from the opening brace '{' to find the function signature.
    Stop at the previous '}' or ';'.
    """
    # Look back up to 500 chars
    lookback_limit = max(0, start_index - 500)
    preceding_text = text[lookback_limit:start_index]

    # Split by ';' or '}' to find the immediate statement before the brace
    # We reverse to find the closest one
    tokens = re.split(r'[;}]', preceding_text)
    header = tokens[-1] # The last chunk is our candidate
    return header.strip()

def is_function_definition(header):
    """
    Heuristics to determine if the header looks like a function.
    """
    if not header:
        return False

    # 1. Ignore Control Structures
    keywords = ['if', 'while', 'for', 'switch', 'do', 'else']
    # Check if the header ends with a keyword (e.g. "else")
    # or starts with a keyword followed by parens (e.g. "if (x)")
    for kw in keywords:
        # Case: "if (...)"
        if header.startswith(kw + " ") or header.startswith(kw + "("):
            return False
        # Case: "else"
        if header == kw:
            return False

    # 2. Ignore Assignments (Arrays/Structs) e.g., "int x[] ="
    if header.endswith('='):
        return False

    # 3. Must likely end with a closing parenthesis ')'
    # (Standard C functions: "void foo(int x)")
    # We strip whitespace first.
    if not header.endswith(')'):
        # Special handling for K&R C or obscure formatting,
        # but usually if it doesn't end in ')', it's not a function header.
        # Exception: "void func(void) const" (C++ish) or macros.
        # But for your specific error, the header ended in ';',
        # which we already filtered in get_context_header by splitting.
        pass

    return True

def analyze_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            raw_content = f.read()
    except Exception as e:
        print(f"Skipping {filepath}: {e}")
        return

    # Use clean content for parsing braces, but raw_content for reporting
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
                # Look ahead for a semicolon (ignoring whitespace)
                is_data_structure = False
                for k in range(end_index + 1, len(clean_content)):
                    if clean_content[k].isspace():
                        continue
                    if clean_content[k] == ';':
                        is_data_structure = True
                    break # Stop checking after first non-space char

                if is_data_structure:
                    start_index = -1
                    continue

                # 2. CHECK BEFORE: Analyze the header
                header = get_context_header(raw_content, start_index)

                # If the header is empty or looks like "if (..)", skip
                if not is_function_definition(header):
                    start_index = -1
                    continue

                # 3. Calculate Length
                func_length = (end_index - start_index) + 1

                if func_length > MAX_CHAR_LENGTH:
                    # Try to extract a pretty name
                    # Find the word before the opening parenthesis
                    match = re.search(r'(\w+)\s*\(', header)
                    func_name = match.group(1) if match else "Unknown"

                    # If "Unknown", use the whole header for context (truncated)
                    if func_name == "Unknown":
                        func_name = header[-50:].replace('\n', ' ')

                    print(f"[ALERT] {filepath}")
                    print(f"    Function: {func_name}(...)")
                    print(f"    Length:   {func_length} chars (Limit: {MAX_CHAR_LENGTH})")
                    print("-" * 40)

                start_index = -1

def main():
    target = sys.argv[1] if len(sys.argv) > 1 else SEARCH_DIR
    print(f"Scanning {os.path.abspath(target)} for .c functions > {MAX_CHAR_LENGTH} chars...\n")

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
