# Auto translated by GPT-4o from the C++ version wrote by Peter Pirkelbauer. 

# With additional comments

"""
This program reads a JSON file, parses its contents, and pretty-prints the JSON data in a structured format.

### How it works:
1. The program takes a JSON file as a command-line argument.
2. It reads the file, parses the JSON content into Python objects.
3. It formats the parsed JSON data into a human-readable structure with indentation.
4. Handles escaped characters in strings (representing C++ codes) for better readability.

### Input:
A JSON file containing valid JSON data.
Example (Partial):

[{"role":"user","content":"Given the following input code in C++:\n```cpp\n#include <stdexcept>\n\n#include \"simplematrix.h\"\n\nSimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)\n{\n  if (lhs.columns() != rhs.rows())\n    throw std::runtime_error{\"lhs.columns() != rhs.rows()\"};\n\n  SimpleMatrix res{lhs.rows(), rhs.columns()};\n\n  for (int i = 0; i < res.rows(); ++i)\n  {\n    for (int j = 0; j < res.columns(); ++j)\n    {\n      res(i,j) = 0;\n\n      for (int k = 0; k < lhs.columns(); ++k)\n        res(i,j) += lhs(i, k) * rhs(k, j);\n    }\n  }\n\n  return res;\n}\n\n```

### Output (Partial):

[
    {
        "role": "user",
        "content": "Given the following input code in C++:
```cpp
#include <stdexcept>

#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  SimpleMatrix res{lhs.rows(), rhs.columns()};

  for (int i = 0; i < res.rows(); ++i)
  {
    for (int j = 0; j < res.columns(); ++j)
    {
      res(i,j) = 0;

      for (int k = 0; k < lhs.columns(); ++k)
        res(i,j) += lhs(i, k) * rhs(k, j);
    }
  }

  return res;
}

```


### Usage:
Run the program from the command line with the JSON file as an argument:
$ python prettyjson.py <filename>

If the input file is not valid JSON, the program will raise an error and display an appropriate message.
"""

import json
import sys


def read_json_file(file):
    """
    Reads a JSON file and parses it into a Python object.
    """
    try:
        data = json.load(file)
    except json.JSONDecodeError as e:
        raise RuntimeError(f"Unable to parse JSON file: {e}") from e
    return data


def parse_file(filename):
    """
    Opens a file and parses its contents as JSON.
    """
    with open(filename, 'r') as file:
        return read_json_file(file)

"""
The form feed (\f) is an escape character used to indicate a page break or advance to the next "form" (page). 
Historically, it was relevant in environments where printers or terminal displays handled page-oriented output. 
It's less commonly encountered in modern programming but still part of text-processing standards like ASCII.
"""

def print_unescaped(output, code):
    """
    Handles escaped characters in strings and prints them to output.
    """
    last = ' '
    for ch in code:
        if last == '\\':
            if ch == 'f':
                output.write('\n\n') # Convert escaped form feed (\f) to two newlines 
            elif ch == 'n':
                output.write('\n') # Convert escaped newline (\n) to a single newline
            elif ch == 't':
                output.write('  ') # Convert escaped tab (\t) to two spaces
            elif ch in ('\'', '"', '?', '\\'):
                output.write(ch) # Preserve escaped quotes, question marks, and backslashes
            last = ' ' # Reset `last` to indicate the escape sequence is handled
        elif ch == '\\':
            last = ch  # Mark the start of an escape sequence
        else:
            output.write(ch) # Print non-escaped characters as-is

"""
Core Logic
Check the Type of value: The function uses isinstance to determine the type of value and handles each type differently:

Dictionary (dict):
------------------------
Start with an opening brace ({).
Increase the indentation level for inner elements.
Iterate through key-value pairs:
  Print each key followed by its corresponding value.
  Recursively call pretty_print for the value.
  Separate key-value pairs with a comma except for the last pair.
End with a closing brace (}), adjusting the indentation back to the parent level.

List (list):
------------------------
Start with an opening bracket ([).
Increase the indentation level for inner elements.
Iterate through list items:
  Print each item, recursively calling pretty_print for nested structures.
  Separate items with a comma except for the last item.
End with a closing bracket (]), adjusting the indentation back to the parent level.


String (str):
------------------------

Serialize the string using json.dumps to handle quotes and special characters.
Pass the serialized string to print_unescaped for additional processing (e.g., handling escape sequences).

Number, Boolean, or None:
------------------------

Directly serialize these values using json.dumps and write them to the output.
Write a Newline at the End (Base Case): If indent is empty, it indicates the top level of the JSON structure. A newline is written to ensure the output ends neatly.


Indentation Handling

Indentation grows by appending four spaces (' ') for each level of nesting.

For example:
  Top-level: No spaces.
  Second-level: Four spaces.
  Third-level: Eight spaces, and so on.


"""
def pretty_print(output, value, indent=""):
    """
    Pretty-prints JSON data with custom formatting.
    """
    if isinstance(value, dict):
        output.write("{\n")
        new_indent = indent + '    '
        for i, (key, val) in enumerate(value.items()):
            output.write(f"{new_indent}{json.dumps(key)}: ")
            pretty_print(output, val, new_indent)
            if i < len(value) - 1:
                output.write(',\n')
        output.write(f'\n{indent}}}')
    elif isinstance(value, list):
        output.write('[\n')
        new_indent = indent + '    '
        for i, item in enumerate(value):
            output.write(new_indent)
            pretty_print(output, item, new_indent)
            if i < len(value) - 1:
                output.write(',\n')
        output.write(f'\n{indent}]')
    elif isinstance(value, str):
        print_unescaped(output, json.dumps(value))
    elif isinstance(value, (int, float, bool)) or value is None:
        output.write(json.dumps(value))
    if not indent:
        output.write('\n')


def main():
    if len(sys.argv) != 2:
        print("Usage: pretty <filename>", file=sys.stderr)
        sys.exit(1)

    try:
        # Parse the file as JSON
        filename = sys.argv[1]
        parsed_json = parse_file(filename)

        # Pretty-print the parsed JSON
        pretty_print(sys.stdout, parsed_json)
    except Exception as e:
        print(f"Caught exception: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()


