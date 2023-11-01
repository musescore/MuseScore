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

num_errors = 0

# Translation errors allowed in these files:
ignored_files = {
    'src/notation/view/widgets/editstyle.ui',   # many straight quotes (let's not bother translators yet)
    'src/engraving/types/symnames.cpp',         # SMuFL symbol names use straight quotes (')
}

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def tr_error(message_element, description, resolution, hint=''):
    global num_errors
    locations = [
        (
            os.path.relpath(os.path.join('share/locale', location.get('filename'))).replace('\\', '/'),
            location.get('line'),
        )
        for location in message_element.findall('location')
    ]
    locations = [ (file, line) for file, line in locations if file not in ignored_files ]
    if not locations:
        return False # error is ignored
    num_errors += 1
    eprint(f'Error in translatable string: "{message.find("source").text}"')
    for file, line in locations:
        eprint(f'    Location: {file}:{line}')
    eprint(f'    Problem: {description}')
    eprint(f'    Solution: {resolution}')
    if hint:
        eprint(f'    Hint: {hint}')
    eprint()
    return True # error

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
            tr_error(message, f'Translatable string contains illegal byte(s): {values}.',
                'Remove the illegal bytes or provide them in a non-translatable string.')
            continue

        # use the source as basis for the translation
        tr_txt = source.text

        if not tr_txt:
            # Sadly, this test only works for empty strings in QML files. If a translated
            # string is empty in a C++ file then lupdate doesn't include it in the TS file.
            tr_error(message, 'Translatable string is empty.',
                'Provide a non-empty string or use "" untranslated if it really needs to be empty.')
            continue

        tr_stripped = tr_txt.strip()

        if not tr_stripped:
            tr_error(message, 'Translatable string only contains whitespace characters.',
                'Include non-whitespace characters or provide the whitepace as untranslated text.')
            continue

        if tr_txt != tr_stripped:
            tr_error(message, 'Translatable string contains leading and/or trailing whitespace.',
                'Remove the whitepace or provide it separately as non-translatable text.',
                'Use .arg() and %1 tags if you need to insert text or numbers into a translatable string.')
            continue

        if '  ' in tr_txt:
            tr_error(message, "Translatable string contains consecutive space characters (  ).",
                'Use a single space character ( ), or provide the spaces as untranslated text.')

        if '...' in tr_txt:
            tr_error(message, "Translatable string contains three consecutive dot characters (...).",
                'Use the ellipsis character (…) instead.')

        if "'" in tr_txt:
            tr_error(message, "Translatable string contains the straight single quote mark (').",
                'Use left (‘) or right (’) curly single quote mark, or prime (′).')
            continue

        if '"' in re.sub(r'<a href="[^"]*">', '', tr_txt):
            tr_error(message, 'Translatable string contains the straight double quote mark (").',
                'Use left (“) or right (”) curly double quote mark, or double prime (″).')
            continue

        # identify QString arg() markers '%1', '%2', etc. as their
        # arguments will require separate translation
        tr_txt = re.sub(r'%([1-9]+)', r'⌜%\1⌝', tr_txt)

        # identify start and end of translatable string
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

if num_errors == 0:
    eprint(f'{sys.argv[0]}: Success!')
elif args.warn_only:
    eprint(f'{sys.argv[0]}: Success! {num_errors} translation errors ignored due to the --warn-only option.')
else:
    eprint(f'{sys.argv[0]}: Failed! {num_errors} translation errors were detected.')
    raise SystemExit(1)
