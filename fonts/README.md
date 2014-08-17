# MuseScore Fonts

This directory contains the fonts used internally by MuseScore and the sources used to generate them.

**FontForge** is used to work on the fonts and the sources are all in the **.sfd** format used by FontForge.

## Sub-directories

* **bravura** The _Bravura_ font(s) and the documentation for them and for SMuFL. These fonts are **not** maintained by the MsueScore community, please **DO NOT EDIT**.
* **gonville** The _Gonville_ font for score musical symbols.
* **mscore** The main font used in MuseScore scores for musical symbols (formerly known as _Emmentaler_), as well its counterpart used for texts.

Other files in the main **fonts** directories are for collateral fonts used by MuseScore for specific tasks. If an *.sfd* file is present, the font is maintained/customized by the MuseScore community.

## Notes on using FontForge

1. **Version**: FontForge version of 2014-05-27 is known to have generated .ttf fonts not working under Windows. Until the actual reason of the failure is understood, **please do not use it**. Version of 2012-07-31 is known to work.
1. **Font names**: In some Fontforge versions and/or under some platforms, Fontforge seems to skip the "FullName" line while saving the font and this makes a .ttf generated from the source unusable under Windows. To ensure that this line is always output, select "Element | Font info...", tab "PS Names" and make sure the string "Name for humans:" is different from the string "Fontname:".
1. **Kern**: To ensure fonts are properly kerned under all supported platform, **uncheck** the "Old style 'kern'" check box in the "Option" sub-dialogue box of the "Generate Font" dialogue box.
