# MuseScore Fonts

This directory contains the fonts used internally by MuseScore and the sources used to generate them.

**FontForge** is used to work on the fonts and the sources are all in the **.sfd** format used by FontForge.

## Sub-directories

* **bravura** The [_Bravura_](https://github.com/steinbergmedia/bravura/) fonts and the documentation for them. These fonts are **not** maintained by the MuseScore community, please **DO NOT EDIT** these fonts.
* **campania** The [_Campania_](https://github.com/MarcSabatella/Campania) font for Roman numeral analysis. This font is not maintained by the MuseScore project, but it is open source and contributions from the MuseScore community are encouraged at the main site for the font.
* **edwin** The [_Edwin_](https://github.com/MuseScoreFonts/Edwin/) text font family. This is maintained by the MuseScore project.
* **gootville** The [_Gonville_](http://www.chiark.greenend.org.uk/~sgtatham/gonville/) fonts for score musical symbols. This font is modified and maintained by the MuseScore project.
* **leland** The [_Leland_](https://github.com/MuseScoreFonts/Leland/) font as of 3.6 is the main font used in MuseScore scores for musical symbols. This is maintained by the MuseScore project.
* **mscore** Prior to 3.6 this was the main font used in MuseScore scores for musical symbols (formerly known as [_Emmentaler_](http://lilypond.org/doc/v2.18/Documentation/notation/the-feta-font)), as well its counterpart used for texts.
* **musejazz** The _MuseJazz_ and _MuseJazzText_ fonts used for notation and text in a handwritten style. These are maintained by the MuseScore project.
* **petaluma** The [_Petaluma_](https://github.com/steinbergmedia/petaluma/) fonts and the documentation for them. These fonts are **not** maintained by the MuseScore community, please **DO NOT EDIT** these fonts.
* **finalemaestro** The [_Finale Maestro_](https://makemusic.zendesk.com/hc/en-us/articles/1500013053461-MakeMusic-Fonts-and-Licensing-Information) fonts and the documentation for them. These fonts are **not** maintained by the MuseScore community, please **DO NOT EDIT** these fonts.
* **finalebroadway** The [_Finale Broadway_](https://makemusic.zendesk.com/hc/en-us/articles/1500013053461-MakeMusic-Fonts-and-Licensing-Information) fonts and the documentation for them. These fonts are **not** maintained by the MuseScore community, please **DO NOT EDIT** these fonts.
* **smufl** The [_SMuFL_](https://github.com/w3c/smufl/) files and documentation for [_SMuFL_](http://www.smufl.org). These are **not** maintained by the MuseScore community, please **DO NOT EDIT** these files.

Other files in the main **fonts** directories are for collateral fonts used by MuseScore for specific tasks. If an *.sfd* file is present, the font is maintained/customized by the MuseScore community.

## Notes on using FontForge

1. **Version**: FontForge version of 2014-05-27 is known to have generated .ttf fonts not working under Windows. Until the actual reason of the failure is understood, **please do not use it**. Version of 2012-07-31 is known to work.
2. **Font names**: In some Fontforge versions and/or under some platforms, Fontforge seems to skip the "FullName" line while saving the font and this makes a .ttf generated from the source unusable under Windows. To ensure that this line is always output, select "Element | Font info...", tab "PS Names" and make sure the string "Name for humans:" is different from the string "Fontname:".
3. **Kern**: To ensure fonts are properly kerned under all supported platforms, **uncheck** the "Old style 'kern'" checkbox in the "Option" subdialog of the "Generate Font" dialog box.
4. **OpenType**: For MuseJazz at least but perhaps other fonts, it had at one time been determined that MacOS requires you to check the "OpenType" box in order for kerning to work, although Linux and Windows required it to be **unchecked**. It seems this has changed since then (probably on account of a change within Qt) and now it seems proper to always check "OpenType". But please try to verify that fonts you generate from *.sfd* sources kern properly on all platforms, and be aware that playing with the "Apple" / "OpenType" / "Old style kern" options may be necessary, regardless of what we believe **should** work in theory.
