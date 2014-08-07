Translation of the instrument list
---

* `generateTs.py` parses `instruments.xml` and create a fake `instrumentsxml.h` file
* `lupdate.sh` creates a TS file, `instruments_XX.ts` per language in `share/locale`
* the TS file is loaded by MuseScore and the instruments are translated when the instruments.xml file is loaded

If instruments.xml is modified
--

* run `generateTs.py` and lupdate.sh
* push the new file updated ts en_US file to transifex