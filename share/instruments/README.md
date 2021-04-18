Translation of the instrument list
---

* `generateTs.py` parses `instruments.xml` and creates a fake `instrumentsxml.h` file
* gen-instruments-projectfile creates a pro file for the translations, so we can run lupdate (to create/update the TS files) and lrelease on it (to generate the QM files)
* the QM files are loaded by MuseScore and the instruments are translated when the instruments.xml file is loaded

If instruments.xml is modified
--

* run `generateTs.py` and lupdate.sh
* push the new file updated ts en_US file to transifex
