Translation of the instrument list
---

* `generateTs.py` parses `instruments.xml` and create a fake `instrumentsxml.h` file
* gen-instruments-projectfile creates a pro file for the translations, so we can run lupdate and lrelease on it
* the TS file is loaded by MuseScore and the instruments are translated when the instruments.xml file is loaded

If instruments.xml is modified
--

* run `generateTs.py` and lupdate.sh
* push the new file updated ts en_US file to transifex