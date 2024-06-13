#!/usr/bin/env python3

import sys
def eprint(*args, **kwargs):
    print(*args, **kwargs, file=sys.stderr)

import locale
if locale.getpreferredencoding().lower() != 'utf-8':
    encoding = locale.getpreferredencoding()
    eprint(f"Error: Encoding is '{encoding}': Python is not running in UTF-8 mode.")
    eprint('  Please set environment variable PYTHONUTF8=1 to enable UTF-8 mode.')
    raise SystemExit(1)

import csv
import os
import sys
import xml.etree.ElementTree as ET

null='[null]' # value to use in TSV when data is not specified in XML

def first_leaf_node(indexes):
    first_value = next(iter(indexes.values()))
    if isinstance(first_value, dict):
        return first_leaf_node(first_value)
    return indexes

def write_indexes(writer, indexes):
    assert isinstance(indexes, dict)
    first_value = next(iter(indexes.values()))
    if isinstance(first_value, dict):
        # assume all values are of type dict
        for subindexes in indexes.values():
            write_indexes(writer, subindexes)
    else:
        # assume all values are of type str
        writer.writerow(indexes.values())

def create_tsv(fname, indexes):
    with open(fname, 'w', newline='\n', encoding='utf-8') as f:
        writer = csv.writer(f, delimiter='\t') # TSV
        writer.writerow(first_leaf_node(indexes).keys()) # header row
        write_indexes(writer, indexes)

def load_translations(file):
    translations = {}
    for message in ET.parse(file).getroot().findall('context/message'):
        translations[message.find('source').text] = message.find('translation').text
    return translations

def find_text(element, tagName, default='[null]'):
    tag = element.find(tagName)
    return default if tag is None else tag.text

os.chdir(os.path.dirname(os.path.realpath(__file__))) # make all paths relative to this script's directory
os.makedirs('tsv', exist_ok=True)

# InstrumentsXML
root = ET.parse('instruments.xml').getroot()

# Translations
italian = load_translations('../locale/instruments_it.ts')
german = load_translations('../locale/instruments_de.ts')

genres = {}
for genre in root.findall('Genre'):
    id = genre.attrib['id']
    genres[id] = {
        'id': id,
        'name': genre.find('name').text,
    }
create_tsv('tsv/genres.tsv', genres)

families = {}
for family in root.findall('Family'):
    id = family.attrib['id']
    families[id] = {
        'id': id,
        'name': family.find('name').text,
    }
create_tsv('tsv/families.tsv', families)

articulation_names = {}
for articulation in root.findall('Articulation'):
    name = articulation.get('name', default='[null]')
    articulation_names[name] = {
        'name':     name,
        'velocity': articulation.find('velocity').text,
        'gateTime': articulation.find('gateTime').text,
    }
create_tsv('tsv/articulation_names.tsv', articulation_names)

groups = {}
instruments = {}
channels = {}
articulations = {}
drumsets = {}
for group in root.findall('InstrumentGroup'):
    group_id = group.attrib['id']
    groups[group_id] = {
        'id':   group_id,
        'name': group.find('name').text,
    }
    for instrument in group.findall('Instrument'):
        instrument_id = instrument.attrib['id']

        trackName = find_text(instrument, 'trackName')
        longName = find_text(instrument, 'longName')
        shortName = find_text(instrument, 'shortName')
        stringdata = instrument.find('StringData')
        strings_open = []
        strings_fretted = []
        if stringdata is None:
            frets = null
        else:
            frets = stringdata.find('frets').text
            for string in stringdata.findall('string'):
                if string.get('open') == '1':
                    strings_open.append(string.text)
                else:
                    strings_fretted.append(string.text)

        stafftype = instrument.find('stafftype')
        if stafftype is None:
            stafftype_name = null
            staffTypePreset = null
        else:
            stafftype_name = stafftype.text
            staffTypePreset = stafftype.get('staffTypePreset', default=null)

        try: minA, maxA = instrument.find('aPitchRange').text.split('-')
        except AttributeError: minA, maxA = (null, null)

        try: minP, maxP = instrument.find('pPitchRange').text.split('-')
        except AttributeError: minP, maxP = (null, null)

        chans = {}
        for channel in instrument.findall('Channel'):
            channel_name = channel.get('name', default=null)
            try: bank_msb = int(channel.find('controller[@ctrl="0"]').get('value'))
            except AttributeError: bank_msb = 0
            try: bank_lsb = int(channel.find('controller[@ctrl="32"]').get('value'))
            except AttributeError: bank_lsb = 0
            bank = (128 * bank_msb) + bank_lsb
            chans[channel_name] = {
                'instrument':   instrument_id,
                'group':        group_id,
                'family':       find_text(instrument, 'family'),
                'description':  find_text(instrument, 'description'),
                'channel':      channel_name,
                'Bank':         (128 * bank_msb) + bank_lsb,
                'Prog':         channel.find('program').get('value'),
            }
        if chans:
            channels[instrument_id] = chans

        instruments[instrument_id] = {
            'id':           instrument_id,
            'group':        group_id,
            'family':       find_text(instrument, 'family'),
            'longName':     longName,
            'longName_it':  italian[longName] if longName in italian else null,
            'longName_de':  german[longName] if longName in german else null,
            'shortName':    shortName,
            'shortName_it': italian[shortName] if shortName in italian else null,
            'shortName_de': german[shortName] if shortName in german else null,
            'trackName':    trackName,
            'trackName_it': italian[trackName] if trackName in italian else null,
            'trackName_de': german[trackName] if trackName in german else null,
            'init':         find_text(instrument, 'init'),
            'description':  find_text(instrument, 'description'),
            'musicXMLid':   find_text(instrument, 'musicXMLid'),
            'frets':        frets,
        'stringsOpen':      ';'.join(strings_open),
        'stringsFretted':   ';'.join(strings_fretted),
            'staves':       find_text(instrument, 'staves'),
            'clefs':        ';'.join(clef.text for clef in instrument.findall('clef')),
            'clefsT':       ';'.join(clef.text for clef in instrument.findall('transposingClef')),
            'clefsC':       ';'.join(clef.text for clef in instrument.findall('concertClef')),
            'stafftype':    stafftype_name,
        'staffTypePreset':  staffTypePreset,
            'bracket':      find_text(instrument, 'bracket'),
            'bracketSpan':  find_text(instrument, 'bracketSpan'),
            'barlineSpans': ';'.join(span.text for span in instrument.findall('barlineSpan')),
            'drumset':      find_text(instrument, 'drumset'),
            'minA':         minA,
            'maxA':         maxA,
            'minP':         minP,
            'maxP':         maxP,
            'transpDia':    find_text(instrument, 'transposeDiatonic'),
            'transpChr':    find_text(instrument, 'transposeChromatic'),
            'SND':          find_text(instrument, 'singleNoteDynamics'),
        'articulations':    ';'.join(articulation.get('name', default=null) for articulation in instrument.findall('Articulation')),
            'velocities':   ';'.join(find_text(articulation, 'velocity') for articulation in instrument.findall('Articulation')),
            'gateTimes':    ';'.join(find_text(articulation, 'gateTime') for articulation in instrument.findall('Articulation')),
            'genres':       ';'.join(genre.text for genre in instrument.findall('genre')),
        }

        artics = {}
        for articulation in instrument.findall('Articulation'):
            name = articulation.get('name', default=null)
            artics[name] = {
                'instrument':   instrument_id,
                'group':        group_id,
                'family':       find_text(instrument, 'family'),
                'description':  find_text(instrument, 'description'),
                'articulation': name,
                'velocity':     find_text(articulation, 'velocity'),
                'gateTime':     find_text(articulation, 'gateTime'),
            }
        if artics:
            articulations[instrument_id] = artics

        drums = {}
        for drum in instrument.findall('Drum'):
            pitch = drum.get('pitch', default=null)
            drums[pitch] = {
                'instrument':   instrument_id,
                'group':        group_id,
                'family':       find_text(instrument, 'family'),
                'description':  find_text(instrument, 'description'),
                'pitch':        pitch,
                'drum':         find_text(drum, 'name'),
                'head':         find_text(drum, 'head'),
                'line':         find_text(drum, 'line'),
                'voice':        find_text(drum, 'voice'),
                'stem':         find_text(drum, 'stem'),
                'shortcut':     find_text(drum, 'shortcut'),
            }
        if drums:
            drumsets[instrument_id] = drums

create_tsv('tsv/groups.tsv', groups)
create_tsv('tsv/instruments.tsv', instruments)
create_tsv('tsv/articulations.tsv', articulations)
create_tsv('tsv/channels.tsv', channels)
create_tsv('tsv/drumsets.tsv', drumsets)
