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

import argparse
import glob
import io
import os
import re
import sys
import xml.etree.ElementTree as ET

parser = argparse.ArgumentParser(description='Generate fake translations for testing purposes.')
parser.add_argument('--warn-only', action='store_true',
    help='exit with zero status even when translation errors are detected')
args = parser.parse_args()

exit_status = 0

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def tr_error(message_element, description, resolution, hint=''):
    location = message_element.find('location')
    filename = location.get('filename')
    filename = os.path.relpath(os.path.join('share/locale', filename))
    line_num = location.get('line')
    eprint(f'Translation error at line {line_num} in file {filename}:')
    eprint(f'    Problem: {description}')
    eprint(f'    Solution: {resolution}')
    if hint:
        eprint(f'    Hint: {hint}')

superscript_numbers = '¹²³⁴⁵⁶⁷⁸⁹'

os.chdir(sys.path[0] + '/../..') # make all paths relative to repository root

source_lang = 'en'
target_lang = 'en@placeholder'  # Substring before '@' must correspond to a real
                                # locale for plurals to work. We must provide as
                                # many plural forms as are required by the locale.

source_lang_ts = source_lang + '.ts'
target_lang_ts = target_lang + '.ts'

eprint(f'{sys.argv[0]}: Generating fake translations for testing purposes.')

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

        bytes = source.findall('byte')
        if bytes:
            values = ', '.join([ f"'{b.get('value')}'" for b in bytes ])
            if len(bytes) == 1:
                tr_error(message, f'Translated string contains illegal byte: {values}.',
                    'Remove the illegal byte or provide it untranslated.')
            else:
                tr_error(message, f'Translated string contains illegal bytes: {values}.',
                    'Remove the illegal bytes or provide them untranslated.')
            exit_status = 1
            continue

        # use the source as basis for the translation
        tr_txt = source.text

        if not tr_txt:
            # Sadly, this test only works for empty strings in QML files. If a translated
            # string is empty in a C++ file then lupdate doesn't include it in the TS file.
            tr_error(message, 'Translated string is empty.',
                'Provide a non-empty string or use "" untranslated if it really needs to be empty.')
            exit_status = 1
            continue

        tr_stripped = tr_txt.strip()

        if not tr_stripped:
            tr_error(message, 'Translated string only contains whitespace characters.',
                'Include non-whitespace characters or provide the whitepace as untranslated text.')
            exit_status = 1
            continue

        if tr_txt != tr_stripped:
            tr_error(message, 'Translated string contains leading and/or trailing whitespace.',
                'Remove the whitepace or provide it separately as untranslated text.',
                'Use .arg() and %1 tags if you need to insert text or numbers into a translated string.')
            exit_status = 1
            continue

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
    # Tweak ElementTree's XML formatting to match that of Qt's lupdate. The aim
    # is to minimise the diff between source_file and target_file.
    for el in root.findall('.//'):
        if el.text:
            # lupdate XML-escapes more characters than ElementTree, but ElementTree
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

if exit_status == 0:
    eprint(f'{sys.argv[0]}: Success!')
elif args.warn_only:
    eprint(f'{sys.argv[0]}: Success! Some errors were ignored because of the --warn-only option.')
else:
    eprint(f'{sys.argv[0]}: Failed! Translation errors were detected.')
    raise SystemExit(exit_status)
