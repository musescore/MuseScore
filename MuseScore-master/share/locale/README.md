Note to translators
===================

Read more about how MuseScore works together with Transifex for handling translations: http://musescore.org/node/22982


How to add a new language
=========================

You need the name of the language and the two letter code

On Transifex
------
* Add the language
https://www.transifex.com/organization/musescore/team/1397/
or request it https://www.transifex.com/projects/p/musescore/

In MuseScore source 
------
* Add the language in share/locale/languages.xml

* Add the language mscore_XX.qm in share/locale/CMakeLists.txt
* Add the language instruments_XX.qm in share/locale/CMakeLists.txt

* Add the language mscore_XX.ts to build/gen-qt-projectfile.bat and build/gen-instruments-projectfile
* Add the language instruments_XX.ts to build/gen-instruments-projectfile.bat and build/gen-qt-projectfile

* Add the share/locale/mscore_XX.ts file, tx pull can help (`tx pull -t language_code)
* Add the share/locale/instruments_XX.ts file, see above





On the update translation server
------
* Modify languages.json
* rm share/locale/details.json

