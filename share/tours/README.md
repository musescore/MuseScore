Translation of the tours
---

* `generateTs.py` parses `*.tour` and creates a fake `tourxml.h` file
* gen-tours-projectfile creates a pro file for the translations, so we can run lupdate (to create/update the TS files) and lrelease on it (to generate the QM files)
* the QM files are loaded by MuseScore and the tours are translated when the *.tour file is loaded

If any *.tour is modified or added
--

* run `generateTs.py` and lupdate.sh
* push the new file updated ts en_US file to transifex
