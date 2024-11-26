# ![MuseScore](share/icons/musescore_logo_full.png)

Music notation and composition software

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.en.html)
[![Coverage](https://s3.us-east-1.amazonaws.com/extensions.musescore.org/test/code_coverage/coverage_badge.svg?)](https://github.com/Eism/MuseScore/actions/workflows/check_unit_tests.yml)

MuseScore is an open source and free music notation software. For support, contribution, and bug reports visit MuseScore.org. Fork and make pull requests!

## Features

- WYSIWYG design, notes are entered on a "virtual notepaper"
- TrueType font(s) for printing & display allows for high quality scaling to all sizes
- Easy & fast note entry
- Many editing functions
- MusicXML import/export
- MIDI (SMF) import/export
- MEI import/export
- MuseData import
- MIDI input for note entry
- Integrated sequencer and software synthesizer to play the score
- Print or create PDF files

## More info

- [MuseScore Homepage](https://musescore.org)
- [MuseScore Git workflow instructions](https://musescore.org/en/developers-handbook/git-workflow)
- [How to compile MuseScore?](https://github.com/musescore/MuseScore/wiki/Set-up-developer-environment)

## License

MuseScore is licensed under GPL version 3.0. See [license file](https://github.com/musescore/MuseScore/blob/master/LICENSE.txt) in the same directory.

## Packages

See [Code Structure on Wiki](https://github.com/musescore/MuseScore/wiki/CodeStructure)

## Building

**Read the [Compilation section](https://github.com/musescore/MuseScore/wiki/Set-up-developer-environment) of the [MuseScore Wiki](https://github.com/musescore/MuseScore/wiki) for a complete build walkthrough and a list of dependencies.**

### Getting sources

If using git to download repo of entire code history, type:

    git clone https://github.com/musescore/MuseScore.git
    cd MuseScore

Otherwise, you can just download the latest source release tarball from the [Releases page](https://github.com/musescore/MuseScore/releases), and then from your download directory type:

    tar xzf MuseScore-x.x.x.tar.gz
    cd MuseScore-x.x.x

### Release Build

To compile MuseScore for release, type:

    cmake -P build.cmake -DCMAKE_BUILD_TYPE=Release

If something goes wrong, append the word "clean" to the above command to delete the build subdirectory:

    cmake -P build.cmake -DCMAKE_BUILD_TYPE=Release clean

Then try running the first command again.

### Running

To start MuseScore, type:

    cmake -P build.cmake -DCMAKE_BUILD_TYPE=Release run

Or run the compiled executable directly.

### Debug Build

A debug version can be built and run by replacing `-DCMAKE_BUILD_TYPE=Release`
with `-DCMAKE_BUILD_TYPE=Debug` in the above commands.

If you omit the `-DCMAKE_BUILD_TYPE` option entirely then `RelWithDebInfo` is
used by default, as it provides a useful compromise between Release and Debug.

### Testing

See the [Unit tests section](https://github.com/musescore/MuseScore/wiki/Unit-tests) of the [MuseScore Wiki](https://github.com/musescore/MuseScore/wiki) for instructions on how to run the test suite.

### Code Formatting

Run `./hooks/install.sh` to install a pre-commit hook that will format your staged files. Requires that you install `uncrustify`.

If you have problems, please report them. To uninstall, run `./hooks/uninstall.sh`.
