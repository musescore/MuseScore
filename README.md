![MuseScore](mscore/data/musescore_logo_full.png)  
 Music notation and composition software

[![Travis CI](https://secure.travis-ci.org/musescore/MuseScore.svg)](https://travis-ci.org/musescore/MuseScore)
[![Appveyor](https://ci.appveyor.com/api/projects/status/bp3ww6v985i64ece/branch/master?svg=true)](https://ci.appveyor.com/project/MuseScore/musescore/branch/master)
[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html)

MuseScore is an open source and free music notation software. For support, contribution, and bug reports visit MuseScore.org. Fork and make pull requests!

## Features

* WYSIWYG design, notes are entered on a "virtual notepaper"
* TrueType font(s) for printing & display allows for high quality scaling to all sizes
* Easy & fast note entry
* Many editing functions
* MusicXML import/export
* MIDI (SMF) import/export
* MuseData import
* MIDI input for note entry
* Integrated sequencer and software synthesizer to play the score
* Print or create pdf files

## More info
* [MuseScore Homepage](https://musescore.org)
* [MuseScore Git workflow instructions](https://musescore.org/en/developers-handbook/git-workflow).
* [How to compile MuseScore?](https://musescore.org/en/developers-handbook/compilation)

## License
MuseScore is licensed under GPL version 2.0. See LICENSE.GPL in the same directory.

## Packages
* **aeolus** Clone of [Aeolus](http://kokkinizita.linuxaudio.org/linuxaudio/aeolus/)
Disabled by default in the stable releases. See http://dev-list.musescore.org/Aeolus-Organ-Synth-td7578364.html
Kept as an example of how to integrate with a complex synthesizer.

* **assets** Graphical assets, use them if you need a MuseScore icon. For logo, color etc... see https://musescore.org/en/about/logos-and-graphics

* **awl** Audio Widget Library, from the MusE project

* **build** Utility files for build

* **bww2mxml** Command line tool to convert BWW files to MusicXML. BWW parser is used by MuseScore to import BWW files.

* **demos** A few MuseScore files to demonstrate what can be done

* **fluid** Clone of [FluidSynth](https://sourceforge.net/projects/fluidsynth/), ported to C++ and customized

* **fonts** Contains fontforge source (sfd) + ttf/otf fonts. MuseScore includes the "Emmentaler" font from the Lilypond project.

* **libmscore** Data model of MuseScore

* **mscore** Main code for the MuseScore UI

* **msynth** Abstract interface to Fluid + Aeolus

* **mtest** Unit testing using QTest

* **omr** Optical music recognition

* **share** Files moved to /usr/share/... on install

* **test** Old tests. Should move to mtest

* **vtest** Visual tests. Compare reference images with current implementation

* **thirdparty** Contains projects which are included for convenience, usually to integrate them into the build system to make them available for all supported platforms.

    * **thirdparty/rtf2html**
    Used for capella import

    * **thirdparty/diff**
    Not used currently. [Diff, Match and Patch Library](https://code.google.com/p/google-diff-match-patch/)

    * **thirdparty/ofqf**
    OSC server interface. Based on [OSC for Qt4](http://www.arnoldarts.de/projects/ofqf/)

    * **thirdparty/singleapp**
    Clone from [Qt Single Application](https://github.com/qtproject/qt-solutions/tree/master/qtsingleapplication)

    * **thirdparty/portmidi**
    Clone from [PortMidi](https://sourceforge.net/projects/portmedia/)

    * **thirdparty/beatroot**
    It's a core part of BeatRoot Vamp Plugin by Simon Dixon and Chris Cannam,
    used in MIDI import for beat detection. (https://code.soundsoftware.ac.uk/projects/beatroot-vamp/repository)


## Building
**Read the developer handbook for a [complete build walkthrough](https://musescore.org/en/developers-handbook/compilation) and a list of dependencies.**

### Getting sources
If using git to download repo of entire code history, type:

    git clone https://github.com/musescore/MuseScore.git
    cd MuseScore

Otherwise, you can just download the latest source release tarball from https://github.com/musescore/MuseScore/releases, and then from your download directory type:

    tar xzf MuseScore-x.x.x.tar.gz
    cd MuseScore-x.x.x

### Release Build
To compile MuseScore, type:

    make release

If something goes wrong, then remove the whole build subdirectory with `make clean` and start new with `make release`.

### Running
To start MuseScore, type:

    ./build.release/mscore/mscore

The Start Center window will appear on every invocation until you disable that setting via the "Preferences" dialog.

### Installing
To install to default prefix using root user, type:

    sudo make install

### Debug Build
A debug version can be built by doing `make debug` instead of `make release`.

To run the debug version, type:

    ./build.debug/mscore/mscore

### Testing
See [mtest/README.md](/mtest/README.md) or https://musescore.org/en/developers-handbook/testing for instructions on how to run the test suite.
