import xml.etree.ElementTree as ET




def addMessage(f, text, comment=''):
    if (comment):
        f.write("//: " + comment.encode('utf8') + "\n")
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
                print "longName " +longName.text
                addMessage(f, longName.text)
                previousLongName = longName.text
            shortName = instrument.find("shortName")
            if shortName is not None:
                print "shortName " +shortName.text
                addMessage(f, shortName.text, previousLongName)
                previousLongName = ""
            trackName = instrument.find("trackName")
            if trackName is not None:
                print "trackName " + trackName.text
                addMessage(f, trackName.text)
                previousLongName = ""

f.close() 
