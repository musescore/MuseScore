How to add a new language
=========================

You need the name of the language and the code

On transifex
------
* Add the new language to the resource on Transifex ( http://musescore.org/node/22982 )

In MuseScore source 
------
* Add the language in share/locale/languages.xml
* Add the language mscore_XX.qm in share/locale/CMakeLists.txt
* Add the language in build/gen-qt-projectfile.bat and build/gen-qt-projectfile
* Add the share/locale/mscore_XX.ts file, tx pull can help

On the update translation server
------
* Modify languages.txt
* rm share/locale/details.json

