#!/usr/bin/env python3

# Generates fake translations to be displayed in MuseScore's UI when the
# language is set to «Placeholder translations» in Preferences > General.
# This enables developers to see which strings have been correctly marked for
# translation without having to wait for a proper translation to be made.

# Steps:
#   1. Add Qt's bin folder to $PATH
#   2. Call run_lupdate.sh
#   3. Run this script
#   4. Call run_lrelease.sh
#   5. Compile & run MuseScore
#   6. In Preferences > General, set language to «Placeholder translations»

import glob
import io
import os
import re
import sys
import xml.etree.ElementTree as ET

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

superscript_numbers = '¹²³⁴⁵⁶⁷⁸⁹'

os.chdir(sys.path[0] + '/../..') # make all paths relative to repository root

source_lang = 'en'
target_lang = 'en@placeholder'  # Substring before '@' must correspond to a real
                                # locale for plurals to work. You must provide as
                                # many plural forms as are required by the locale.

source_lang_ts = source_lang + '.ts'
target_lang_ts = target_lang + '.ts'

for source_file in glob.glob('share/locale/*_' + source_lang_ts):
    target_file = source_file[:-len(source_lang_ts)] + target_lang_ts

    eprint("Reading " + source_file)
    tree = ET.parse(source_file)
    root = tree.getroot()

    assert(root.tag == 'TS')
    root.set('language', target_lang)

    for message in root.findall('.//message'):
        source = message.find('source')
        translation = message.find('translation')
        plurals = translation.findall('numerusform')

        # use the source as basis for the translation
        tr_txt = source.text

        # identify QString arg() markers '%1', '%2', etc. as their
        # arguments will require separate translation
        tr_txt = re.sub(r'%([1-9]+)', r'⌜%\1⌝', tr_txt)

        # identify start and end of translated string
        tr_txt = '«' + tr_txt + '»'

        if plurals:
            for idx, plural in enumerate(plurals):
                plural.text = 'ᵗʳ' + superscript_numbers[idx] + tr_txt
        else:
            translation.text = 'ᵗʳ' + tr_txt

    eprint("Writing " + target_file)
    # Tweak ElementTree's XML formatting to match that of Qt's lrelease. The aim is to
    # minimise the diff between source_file and target_file.
    for el in root.findall('.//'):
        if el.text:
            # lrelease XML-escapes more characters than ElementTree, but ElementTree
            # won't allow us to write the & escape character so use \r instead.
            el.text = el.text.replace('\'', '\rapos;').replace('"', '\rquot;')

    # Write XML tree to memory so that we can manipulate it as raw text
    memfile = io.StringIO()
    tree.write(memfile, encoding='unicode', xml_declaration=True)

    # Manipulate raw XML and write to disk
    with open(target_file, 'w', newline='\n', encoding='utf-8') as f:
        memfile.seek(0)
        f.write(next(iter(memfile))) # write first line (the XML declaration)
        f.write('<!DOCTYPE TS>\n')
        for line in memfile:
            f.write(line
                .replace('" />', '"/>') # remove space after final attribute in opening tags
                .replace('\r', '&') # use the proper escape character in &apos; and &quot;
            )
