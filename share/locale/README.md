How to add a new language
=========================

You need the name of the language and the code

On transifex
------
* Add a resource

In MuseScore source 
------
* Add the language in languages.xml
* Add the language mscore_XX.qm in share/locale/CMakeLists.txt
* Add the language in gen-qt-projectfile.bat and gen-qt-projectfile
* Add the mscore_XX.ts file, tx pull can help

On the update translation server
------
* Modify languages.txt
* rm share/locale/details.json

