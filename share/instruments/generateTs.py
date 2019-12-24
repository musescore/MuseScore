#!/usr/bin/env python3

# If you get Unicode errors on Windows, try setting the environment variable
# PYTHONIOENCODING=utf-8. More info at https://stackoverflow.com/a/12834315

import xml.etree.ElementTree as ET
import os

def addMessage(f, text, comment='', category='InstrumentsXML'):
    if (comment):
        f.write('QT_TRANSLATE_NOOP3("'+category+'", "' + text + '", "' + comment + '"),\n')
    else:
        f.write('QT_TRANSLATE_NOOP("'+category+'", "' + text + '"),\n')


# Create instrumentsxml.h (must specify encoding and line ending on Windows)
f = open('instrumentsxml.h', 'w', newline='\n', encoding='utf-8')

#include template names and template categories
d = "../templates"
for o in sorted(os.listdir(d)): # sort to get same ordering on all platforms
    ofullPath = os.path.join(d, o)
    if os.path.isdir(ofullPath):
        templateCategory = o.split("-")[1].replace("_", " ")
        addMessage(f, templateCategory, '', 'Templates')
        print(templateCategory)
        for t in sorted(os.listdir(ofullPath)): # sort to get same ordering on all platforms
            if (os.path.isfile(os.path.join(ofullPath, t))):
                templateName = os.path.splitext(t)[0].split("-")[1].replace("_", " ")
                addMessage(f, templateName, '', 'Templates')
                print("    " + templateName)

#instruments.xml
tree = ET.parse('instruments.xml')
root = tree.getroot()

previousLongName = ""
for child in root:
    if child.tag == "Genre":
        genre = child.find("name")
        print("Genre " + genre.text)
        addMessage(f, genre.text)
    elif child.tag == "InstrumentGroup":
        instrGroup = child.find("name")
        print("Instr Group : " + instrGroup.text)
        addMessage(f, instrGroup.text)
        instruments = child.findall("Instrument")
        for instrument in instruments:
            longName = instrument.find("longName")
            if longName is not None:
                print("  longName : " + longName.text)
                addMessage(f, longName.text)
                previousLongName = longName.text

            shortName = instrument.find("shortName")
            if shortName is not None:
                print("  shortName : " + shortName.text)
                addMessage(f, shortName.text, previousLongName)
                previousLongName = ""

            trackName = instrument.find("trackName")
            if trackName is not None:
                print("  trackName " + trackName.text)
                addMessage(f, trackName.text)
                previousLongName = ""

            channels = instrument.findall("Channel")
            for channel in channels:
                channelName = channel.get("name")
                if channelName is not None:
                    print("  Channel name : " + channelName)
                    addMessage(f, channelName)
                cMidiActions = channel.findall("MidiAction")
                for cma in cMidiActions:
                    cmaName = cma.get("name")
                    if cmaName is not None:
                        print("    Channel, MidiAction name :" + cmaName)
                        addMessage(f, cma)

            iMidiActions = instrument.findall("MidiAction")
            for ima in iMidiActions:
                imaName = ima.get("name")
                if imaName is not None:
                    print("  Instrument, MidiAction name :" + imaName)
                    addMessage(f, ima)

f.close()
