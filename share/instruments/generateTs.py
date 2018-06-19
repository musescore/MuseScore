#!/usr/bin/env python

import xml.etree.ElementTree as ET




def addMessage(f, text, comment=''):
    if (comment):
        f.write('QT_TRANSLATE_NOOP3("InstrumentsXML", "' + text.encode('utf8') + '", "' + comment.encode('utf8') + '"),\n')
    else:
        f.write('QT_TRANSLATE_NOOP("InstrumentsXML", "' + text.encode('utf8') + '"),\n')



f = open('instrumentsxml.h','w')

tree = ET.parse('instruments.xml')
root = tree.getroot()

previousLongName = ""
for child in root:
    if child.tag == "Genre":
        genre = child.find("name")
        print "Genre " + genre.text
        addMessage(f, genre.text)
    elif child.tag == "InstrumentGroup":
        instrGroup = child.find("name")
        print "Instr Group " + instrGroup.text
        addMessage(f, instrGroup.text)
        instruments = child.findall("Instrument")
        for instrument in instruments:
            longName = instrument.find("longName")
            if longName is not None:
                print "longName " + longName.text.encode('utf8')
                addMessage(f, longName.text)
                previousLongName = longName.text

            shortName = instrument.find("shortName")
            if shortName is not None:
                print "shortName " + shortName.text.encode('utf8')
                addMessage(f, shortName.text, previousLongName)
                previousLongName = ""

            trackName = instrument.find("trackName")
            if trackName is not None:
                print "trackName " + trackName.text.encode('utf8')
                addMessage(f, trackName.text)
                previousLongName = ""

            channels = instrument.findall("Channel")
            for channel in channels:
                channelName = channel.get("name")
                if channelName is not None:
                    print "Channel name :" + channelName
                    addMessage(f, channelName)
                cMidiActions = channel.findall("MidiAction")
                for cma in cMidiActions:
                    cmaName = cma.get("name")
                    if cmaName is not None:
                        print "Channel, MidiAction name :" + cmaName
                        addMessage(f, cma)

            iMidiActions = instrument.findall("MidiAction")
            for ima in iMidiActions:
                imaName = ima.get("name")
                if imaName is not None:
                    print "Instrument, MidiAction name :" + imaName
                    addMessage(f, ima)

f.close()
