#!/usr/bin/env python3

# If you get Unicode errors on Windows, try setting the environment variable
# PYTHONIOENCODING=utf-8. More info at https://stackoverflow.com/a/12834315

import xml.etree.ElementTree as ET
import os


# Create instruments.csv (must specify encoding and line ending on Windows)
f = open('instruments.csv', 'w', newline='\n', encoding='utf-8')

#instruments.xml
tree = ET.parse('instruments.xml')
root = tree.getroot()

previousLongName = ''
genres = []
genreIds = []
header = ['longName', 'InstrumentId', 'trackName', 'instrGroup', 'family']
for child in root:
    if child.tag == 'Genre':
        g = child.find("name")
        genres.append(g.text)
        genreIds.append(child.attrib['id'])
    elif child.tag == 'InstrumentGroup':
        if header:
            f.write('\t'.join(header + genres) + '\n')
            header = None
        instrGroup = child.attrib['id']
        genreMembers = [' ']*len(genres)
        for instrument in child.findall('Instrument'):
            instrumentId = instrument.attrib['id']
            element = instrument.find('longName')
            if element is None:
                longName = ' '
            else:
                longName = element.text
            element = instrument.find('trackName')
            if element is None:
                trackName = ' '
            else:
                trackName = element.text
            element = instrument.find('family')
            if element is None:
                family = ' '
            else:
                family = element.text
            for genre in instrument.findall('genre'):
                genreMembers[genreIds.index(genre.text)] = 'x'
                
            f.write(f'{longName}\t{instrumentId}\t{trackName}\t{instrGroup}\t{family}\t' + '\t'.join(genreMembers) + '\n')

f.close()
