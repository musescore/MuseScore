#!/usr/bin/env python

import os
import xml.etree.ElementTree as ET


def addMessage(f, text, comment=''):
    text = text.replace('"', r'\"')
    text = text.replace('\n', r'\n')
    if (comment):
        f.write('QT_TRANSLATE_NOOP3("TourXML", "' + text.encode('utf8') + '", "' + comment.encode('utf8') + '"),\n')
    else:
        f.write('QT_TRANSLATE_NOOP("TourXML", "' + text.encode('utf8') + '"),\n')


scriptPath = os.path.dirname(os.path.realpath(__file__))

#find all tours
tours = []
for file in os.listdir(scriptPath):
    if file.endswith(".tour"):
        tours.append(os.path.join(scriptPath, file))

# create tourxml.h
f = open(scriptPath + '/' + 'tourxml.h', 'w')

for tour in tours:
    tree = ET.parse(tour)
    root = tree.getroot()
    tourName = root.attrib["name"]
    for child in root:
        if child.tag == "Message":
            t = child.find("Text")
            addMessage(f, t.text, tourName)

f.close()
