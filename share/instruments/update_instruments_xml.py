#!/usr/bin/env python3

import argparse
import csv
import os
import requests
import sys
import xml.etree.ElementTree as ET

spreadsheet_id = '1SwqZb8lq5rfv5regPSA10drWjUAoi65EuMoYtG-4k5s'

sheet_ids = {
    'Instruments':              '516529997',
    'Drumsets':                 '272323799',
    'Channels':                 '504647632',
    'Genres':                   '1006945000',
    'Groups':                   '1568449825',
    'Families':                 '1553585382',
    'Articulations':            '408601185',
    'Articulation_Defaults':    '1165321173',
    'GM_Programs':              '1024995659',
    'GS_Drum_Kits':             '1103601299',
    'GM+GS_Percussion':         '1216482735',
}

parser = argparse.ArgumentParser(description='Fetch the latest spreadsheet and generate instruments.xml.')
parser.add_argument('-c', '--cached', action='store_true', help='Use cached version instead of downloading')
parser.add_argument('-d', '--download', action='append', choices=sheet_ids.keys(), help='Override cached option for a specific sheet')
args = parser.parse_args()

null='[null]' # value used in TSV when attributes or tags are to be omitted in XML
list_sep=';' # character used as separator in TSV when a cell contains multiple values

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def download_google_spreadsheet(sheet):
    assert sheet in sheet_ids.keys()
    path = 'tsv/download/{sheet}.tsv'.format(sheet=sheet)
    if args.cached and os.path.isfile(path) and (args.download is None or sheet not in args.download):
        eprint('Using cached spreadsheet:', path)
    else:
        eprint('Downloading spreadsheet: ', sheet)
        url = 'https://docs.google.com/spreadsheets/d/{id}/export?gid={gid}&format=tsv'
        url = url.format(id=spreadsheet_id, gid=sheet_ids[sheet])
        r = requests.get(url)
        assert r.status_code == 200
        with open(path, 'wb') as f:
            f.write(r.content)
        eprint('Spreadsheet saved to ' + path)
    return path

def table_from_tsv(filename):
    with open(filename, 'r') as f:
        return [ row for row in csv.reader(f, delimiter='\t') ]

def data_by_heading(table, headings_row=0):
    headings = table[headings_row]
    rows = []
    for entry in table[headings_row+1:]:
        data = {}
        for idx, col in enumerate(entry):
            data[headings[idx]] = col
        rows.append(data)
    return rows

def index_by_column(rows, primary_index_col, secondary_index_col=None):
    data = {}
    if secondary_index_col is None:
        for row in rows:
            primary = row[primary_index_col]
            assert primary not in data
            data[primary] = row
    else:
        for row in rows:
            primary = row[primary_index_col]
            secondary = row[secondary_index_col]
            if primary not in data:
                data[primary] = {}
            else:
                assert secondary not in data[primary]
            data[primary][secondary] = row
    return data

def google_spreadsheet_to_indexed_dict(sheet, headers_row, primary_index_col, secondary_index_col=None):
    tsv = download_google_spreadsheet(sheet)
    table = table_from_tsv(tsv)
    rows = data_by_heading(table, headers_row - 1)
    return index_by_column(rows, primary_index_col, secondary_index_col)

os.chdir(sys.path[0]) # make all paths relative to this script's directory
os.makedirs('tsv/download', exist_ok=True)

instruments     = google_spreadsheet_to_indexed_dict('Instruments', 3, 'group', 'id')
drumsets        = google_spreadsheet_to_indexed_dict('Drumsets', 3, 'instrument', 'pitch')
channels        = google_spreadsheet_to_indexed_dict('Channels', 3, 'instrument', 'channel')
genres          = google_spreadsheet_to_indexed_dict('Genres', 3, 'id')
groups          = google_spreadsheet_to_indexed_dict('Groups', 3, 'id')
families        = google_spreadsheet_to_indexed_dict('Families', 3, 'id')
articulations   = google_spreadsheet_to_indexed_dict('Articulations', 3, 'instrument', 'articulation')
articulation_defaults   = google_spreadsheet_to_indexed_dict('Articulation_Defaults', 3, 'name')
gm_programs     = google_spreadsheet_to_indexed_dict('GM_Programs', 3, 'prog')
gs_drumkits     = google_spreadsheet_to_indexed_dict('GS_Drum_Kits', 3, 'prog')
gmgs_percussion = google_spreadsheet_to_indexed_dict('GM+GS_Percussion', 3, 'pitch')

root = ET.Element('museScore')

def to_attribute(el, data, key, attr=None):
    if attr is None:
        attr = key
    val = data[key]
    if val and val != null:
        el.set(attr, val)

def to_subelement(el, data, key, tag=None):
    if tag is None:
        tag = key
    val = data[key]
    if val and val != null:
        ET.SubElement(el, tag).text = val

def to_comment(el, data, key):
    val = data[key]
    if val and val != null:
        el.append(ET.Comment(val))

def pitch_range(el, instrument, min_key, max_key, range_tag):
    min_val = instrument[min_key]
    max_val = instrument[max_key]
    if min_val and max_val and min_val != null and max_val != null:
        ET.SubElement(el, range_tag).text = min_val + '-' + max_val

def to_list(str):
    if str:
        return str.split(list_sep)
    return []

for genre in genres.values():
    el = ET.SubElement(root, 'Genre')
    to_attribute(el, genre, 'id')
    to_subelement(el, genre, 'name')

for family in families.values():
    if family['id'] == null or not family['id']:
        continue
    el = ET.SubElement(root, 'Family')
    to_attribute(el, family, 'id')
    to_subelement(el, family, 'name')

for articulation in articulation_defaults.values():
    el = ET.SubElement(root, 'Articulation')
    to_attribute(el, articulation, 'name')
    to_subelement(el, articulation, 'velocity')
    to_subelement(el, articulation, 'gateTime')

for group in groups.values():
    g_el = ET.SubElement(root, 'InstrumentGroup')
    to_attribute(g_el, group, 'id')
    to_subelement(g_el, group, 'name')
    for instrument in instruments[group['id']].values():
        el = ET.SubElement(g_el, 'Instrument')
        to_attribute(el, instrument, 'id')
        to_subelement(el, instrument, 'init') # must be first subelement
        to_subelement(el, instrument, 'family')
        to_comment(el, instrument, 'comment')
        if instrument["ddName"] != '[hide]':
             to_subelement(el, instrument, 'trackName')
             to_subelement(el, instrument, 'longName')
             to_subelement(el, instrument, 'shortName')
             if instrument["ddName"]:
                 dd_el = ET.SubElement(el, 'dropdownName')
                 dd_el.text = instrument["ddName"]
                 to_attribute(dd_el, instrument, 'ddMeaning', 'meaning')
        to_subelement(el, instrument, 'description')
        to_subelement(el, instrument, 'musicXMLid')

        frets = instrument['frets']
        strings_open = to_list(instrument['stringsOpen'])
        strings_fretted = to_list(instrument['stringsFretted'])
        if frets and frets != null:
            sd_el = ET.SubElement(el, 'StringData')
            ET.SubElement(sd_el, 'frets').text = frets
            for string in strings_open:
                st_el = ET.SubElement(sd_el, 'string')
                st_el.set('open', '1')
                st_el.text = string
            for string in strings_fretted:
                ET.SubElement(sd_el, 'string').text = string
        else:
            assert not strings_open and not strings_fretted

        to_subelement(el, instrument, 'staves')

        clefs = to_list(instrument['clefs'])
        if clefs and clefs[0] and clefs[0] != null:
            ET.SubElement(el, 'clef').text = clefs[0]

        clefsT = to_list(instrument['clefsT'])
        if clefsT and clefsT[0] and clefsT[0] != null:
            ET.SubElement(el, 'transposingClef').text = clefsT[0]

        clefsC = to_list(instrument['clefsC'])
        if clefsC and clefsC[0] and clefsC[0] != null:
            ET.SubElement(el, 'concertClef').text = clefsC[0]

        stafftype = instrument['stafftype']
        staffTypePreset = instrument['staffTypePreset']
        if stafftype and stafftype != null:
            st_el = ET.SubElement(el, 'stafftype')
            if staffTypePreset and staffTypePreset != null:
                st_el.set('staffTypePreset', staffTypePreset)
            st_el.text = stafftype
        else:
            assert staffTypePreset == null or not staffTypePreset

        to_subelement(el, instrument, 'bracket')
        to_subelement(el, instrument, 'bracketSpan')

        barlineSpans = to_list(instrument['barlineSpans'])
        barlineSpanTotal = 0
        if barlineSpans:
            span = barlineSpans[0]
            if span and span != null:
                ET.SubElement(el, 'barlineSpan').text = span
                barlineSpanTotal += int(span)

        is_drumset = instrument['drumset'] == '1'
        if is_drumset:
            ET.SubElement(el, 'drumset').text = '1'

        for staff, clef in enumerate(clefs[1:], 2):
            c_el = ET.SubElement(el, 'clef')
            c_el.set('staff', str(staff))
            c_el.text = clef

        for staff, clefT in enumerate(clefsT[1:], 2):
            c_el = ET.SubElement(el, 'transposingClef')
            c_el.set('staff', str(staff))
            c_el.text = clefT

        for staff, clefC in enumerate(clefsC[1:], 2):
            c_el = ET.SubElement(el, 'concertClef')
            c_el.set('staff', str(staff))
            c_el.text = clefC

        for span in barlineSpans[1:]:
            bs_el = ET.SubElement(el, 'barlineSpan')
            bs_el.set('staff', str(barlineSpanTotal + 1))
            bs_el.text = span
            barlineSpanTotal += int(span)

        pitch_range(el, instrument, 'minA', 'maxA', 'aPitchRange')
        pitch_range(el, instrument, 'minP', 'maxP', 'pPitchRange')

        to_subelement(el, instrument, 'SND', 'singleNoteDynamics')
        to_subelement(el, instrument, 'transpDia', 'transposeDiatonic')
        to_subelement(el, instrument, 'transpChr', 'transposeChromatic')

        if instrument['id'] in drumsets:
            for drum in drumsets[instrument['id']].values():
                pitch = drum['pitch']
                d_el = ET.SubElement(el, 'Drum')
                d_el.set('pitch', pitch)
                if pitch in gmgs_percussion:
                    to_comment(d_el, gmgs_percussion[pitch], 'name')
                to_subelement(d_el, drum, 'head')
                to_subelement(d_el, drum, 'line')
                to_subelement(d_el, drum, 'voice')
                to_subelement(d_el, drum, 'drum', 'name')
                to_subelement(d_el, drum, 'stem')
                to_subelement(d_el, drum, 'shortcut')

        if instrument['id'] in channels:
            for channel in channels[instrument['id']].values():
                ch_el = ET.SubElement(el, 'Channel')
                to_attribute(ch_el, channel, 'channel', 'name')
                bank = channel['Bank']
                prog = channel['Prog']
                msb = channel['MSB']
                lsb = channel['LSB']
                sound = channel["MS General sound"]
                assert int(bank) == (int(msb) * 128) + int(lsb)
                ch_el.append(ET.Comment("MIDI: Bank {0}, Prog {1}; MS General: {2}".format(bank, prog, sound)))
                if msb != "0":
                    con_el = ET.SubElement(ch_el, 'controller')
                    con_el.set('ctrl', "0")
                    con_el.set('value', msb)
                    ch_el.append(ET.Comment("Bank MSB"))
                if lsb != "0":
                    con_el = ET.SubElement(ch_el, 'controller')
                    con_el.set('ctrl', "32")
                    con_el.set('value', lsb)
                    ch_el.append(ET.Comment("Bank LSB"))
                ET.SubElement(ch_el, 'program').set('value', channel['Prog'])
                if is_drumset:
                    try: to_comment(ch_el, gs_drumkits[channel['Prog']], 'name')
                    except KeyError: ch_el.append(ET.Comment("Non-GS drum kit"))
                else:
                    to_comment(ch_el, gm_programs[channel['Prog']], 'name')

        if instrument['id'] in articulations:
            for articulation in articulations[instrument['id']].values():
                ar_el = ET.SubElement(el, 'Articulation')
                to_attribute(ar_el, articulation, 'articulation', 'name')
                to_subelement(ar_el, articulation, 'velocity')
                to_subelement(ar_el, articulation, 'gateTime')

        for genre in to_list(instrument['genres']):
            assert genre in genres
            ET.SubElement(el, 'genre').text = genre

tree = ET.ElementTree(root)
ET.indent(tree, space='      ')
for pr_el in root.findall('.//Channel/program'):
    pr_el.tail = ' '
for pr_el in root.findall('.//Channel/controller'):
    pr_el.tail = ' '
for dr_el in root.findall('.//Instrument/Drum'):
    if dr_el.get('pitch') in gmgs_percussion:
        dr_el.text = ' '
root.tail = '\n'
import io
import shutil
memfile = io.StringIO()
tree.write(memfile, encoding='unicode', xml_declaration=True)
with open('instruments.xml', 'w', newline='\n', encoding='utf-8') as f:
    memfile.seek(0)
    f.write(next(iter(memfile))) # write first line (the XML declaration)
    f.write('<!-- Generated by {0} using data from https://docs.google.com/spreadsheets/d/{1}. -->\n'.format(os.path.basename(__file__), spreadsheet_id))
    for line in memfile:
        f.write(line.replace('" />', '"/>'))
