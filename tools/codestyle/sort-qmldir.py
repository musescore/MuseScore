#!/usr/bin/env python3
#
# Usage:
#   sort-qmldir.py path/to/first/qmldir [path/to/second/qmldir ...]

import os.path
import sys

def sort_qmldir(filename: str) -> int:
    if os.path.basename(filename) != "qmldir":
        print("error: File is not a qmldir file.")
        return 1
    print("Sorting lines with qml files in", filename)
    lines = []
    with open(filename, 'r') as f:
        lines = f.readlines()
    if len(lines) == 0:
        print("error: couldn't read qml file or file is empty:", filename)
        return 1
    lines = [lines[0]] + sorted(lines[1:])
    with open(filename, 'w') as f:
        f.writelines(lines)
    return 0

def main() -> int:
    if len(sys.argv) == 1:
        print("error: no qmldir files specified")
        return 1
    for arg in sys.argv[1:]:
        if int(sort_qmldir(arg)) != 0:
            return 1
    return 0

if __name__ == '__main__':
    sys.exit(main())
