#!/usr/bin/env python3
#
# Usage:
#   sort-qrc.py path/to/first_resource_file.qrc [path/to/second_resource_file.qrc ...]

import os.path
import sys
import xml.etree.ElementTree as ET

def sort_qrc(filename: str) -> int:
    if os.path.splitext(filename)[1] != ".qrc":
        print("error: File is not a .qrc file.")
        return 1
    print("Sorting lines with files in", filename)
    tree = ET.parse(filename)
    RCC = tree.getroot()
    for qresource in RCC:
        qresource[:] = sorted(qresource, key=lambda filetag: filetag.text.lower())
    tree.write(filename)
    with open(filename, 'a') as f:
        f.write('\n') # Newline at end of file
    return 0

def main() -> int:
    if len(sys.argv) == 1:
        print("error: no .qrc files specified")
        return 1
    for arg in sys.argv[1:]:
        if int(sort_qrc(arg)) != 0:
            return 1
    return 0

if __name__ == '__main__':
    sys.exit(main())
