# Leland music fonts

This repository contains the **Leland music fonts, Version 0.52**.

## About the fonts

The Leland music fonts (Leland & Leland Text), Version 0.52 are available in Type 1, OpenType-CFF format. They are distributed under the [SIL Open Font License (OFL), Version 1.1](./LICENSE.txt).

The fonts have been initially developed for MuseScore (https://www.musescore.org) music composition software.

Leland is compliant with [Standard Music Font Layout (SMuFL)](https://w3c.github.io/smufl/gitbook/), which provides a standard way of mapping the thousands of musical symbols required by conventional music notation into the Private Use Area in Unicode's Basic Multilingual Plane for a single (format-independent) font.

The font is named after [Leland Smith](https://en.wikipedia.org/wiki/Leland_Smith), creator of the [SCORE](https://en.wikipedia.org/wiki/SCORE_(software)) music notation software.

## Installation

Leland is included with MuseScore 3.6 and later, so no further installation is required to use it in MuseScore.

To use the font with other applications, install the .otf font files in your OS in the usual way. Note that this font does **not** yet work in Sibelius or Finale as they do not have support for SMuFL fonts. (A Sibelius-specific version is in development.)

To use the font in Dorico (1.0.20 and later), the `metadata.json` file needs to be copied to the following location:

__Windows__: `C:\Program Files\Common Files\SMuFL\Fonts\Leland\Leland.json`  
__Mac__: `/Library/Application Support/SMuFL/Fonts/Leland/Leland.json`  
__Linux__: `/usr/share/SMuFL/Fonts/Leland/Leland.json`

Note that you will need to create a Leland folder inside the `SMuFL/Fonts` folder manually, as well as renaming the `metadata.json` file to `Leland.json`.
