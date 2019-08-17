Note to translators
===================

Read more about how MuseScore works together with Transifex for handling translations: http://musescore.org/node/22982


How to add a new language
=========================

You need the name of the language and the two letter code

On [Transifex](https://www.transifex.com/musescore/musescore/dashboard/)
------
* Add the language
https://www.transifex.com/organization/musescore/team/1397/
or request it https://www.transifex.com/projects/p/musescore/

In [MuseScore source](https://github.com/musescore/MuseScore)
------
* Add the language in share/locale/languages.xml

* Add the share/locale/mscore_XX.ts file, tx pull can help (`tx pull -t language_code`)
* Add the share/locale/instruments_XX.ts file, see above
* Add the share/locale/tours_XX.ts file, see above

* Finally create a PR


In [txt2s3 source](https://github.com/musescore/tx2s3)
------
* Modify languages.json (and create a PR)
