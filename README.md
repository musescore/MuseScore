![MuseScore](assets/musescore_logo_full.png)  
Music notation and composition software

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html)

MuseScore is an open source and free music notation software. For support, contribution, and bug reports visit MuseScore.org. Fork and make pull requests!

## Features

- WYSIWYG design, notes are entered on a "virtual notepaper"
- TrueType font(s) for printing & display allows for high quality scaling to all sizes
- Easy & fast note entry
- Many editing functions
- MusicXML import/export
- MIDI (SMF) import/export
- MuseData import
- MIDI input for note entry
- Integrated sequencer and software synthesizer to play the score
- Print or create pdf files

## More info
- [MuseScore Homepage](https://musescore.org)
- [MuseScore Git workflow instructions](https://musescore.org/en/developers-handbook/git-workflow)
- [How to compile MuseScore?](https://musescore.org/en/developers-handbook/compilation)

## License
MuseScore is licensed under GPL version 2.0. See [LICENSE.GPL](https://github.com/musescore/MuseScore/blob/master/LICENSE.GPL) in the same directory.

## Packages
See [Code Structure on Wiki](https://github.com/musescore/MuseScore/wiki/CodeStructure)


## Building
**Read the developer handbook for a [complete build walkthrough](https://musescore.org/en/developers-handbook/compilation) and a list of dependencies.**

### Getting sources
If using git to download repo of entire code history, type:

    git clone https://github.com/musescore/MuseScore.git
    cd MuseScore

Otherwise, you can just download the latest source release tarball from the [Releases page](https://github.com/musescore/MuseScore/releases), and then from your download directory type:

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
See [mtest/README.md](/mtest/README.md) or [the developer handbook](https://musescore.org/handbook/developers-handbook/finding-your-way-around/automated-tests) for instructions on how to run the test suite.

The new [script testing facility](https://musescore.org/node/278278) is also available to create your own automated tests. Please try it out!

### Code Formatting

Run `./hooks/install.sh` to install a pre-commit hook that will format your staged files. Requires that you install `uncrustify`.

If you have problems, please report them. To uninstall, run `./hooks/uninstall.sh`.
