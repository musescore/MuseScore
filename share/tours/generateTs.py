#!/usr/bin/env python3

# If you get Unicode errors on Windows, try setting the environment variable
# PYTHONIOENCODING=utf-8. More info at https://stackoverflow.com/a/12834315

import os
import xml.etree.ElementTree as ET


def addMessage(f, text, comment=''):
    text = text.replace('"', r'\"')
    text = text.replace('\n', r'\n')
    if (comment):
        f.write('QT_TRANSLATE_NOOP3("TourXML", "' + text + '", "' + comment + '"),\n')
    else:
        f.write('QT_TRANSLATE_NOOP("TourXML", "' + text + '"),\n')


scriptPath = os.path.dirname(os.path.realpath(__file__))

#find all tours
tours = []
for file in sorted(os.listdir(scriptPath)): # sort to get same ordering on all platforms
    if file.endswith(".tour"):
        tours.append(os.path.join(scriptPath, file))

# create tourxml.h (must specify encoding and line ending on Windows)
f = open(scriptPath + '/' + 'tourxml.h', 'w', newline='\n', encoding='utf-8')

for tour in tours:
    tree = ET.parse(tour)
    root = tree.getroot()
    tourName = root.attrib["name"]
    for child in root:
        if child.tag == "Message":
            t = child.find("Text")
            addMessage(f, t.text, tourName)

f.close()
