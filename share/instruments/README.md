# Instruments list

The list of instruments is stored in this online spreadsheet:
https://docs.google.com/spreadsheets/d/1SwqZb8lq5rfv5regPSA10drWjUAoi65EuMoYtG-4k5s/edit#gid=516529997

When changes have been made to this spreadsheet, please run `update_instruments_xml.py`,
to update the following files: 
- `instruments.xml` (used by MuseScore, to load the instruments);
- `instrumentsxml.h` (a fake header file used to generate translatable strings from; 
  see also `/build/gen-instruments-projectfile` and `/build/ci/run_lupdate.sh`).
