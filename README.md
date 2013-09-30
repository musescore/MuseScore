=================================================================
      MuseScore Score Typesetter
=================================================================

Features:
      - WYSIWYG design, notes are entered on a "virtual notepaper"
      - TrueType font(s) for printing & display allows for high quality
        scaling to all sizes
      - easy & fast note entry
      - many editing functions
      - MusicXML import/export
      - Midi (SMF) import/export
      - MuseData import
      - Midi input for note entry
      - integrated sequencer and software synthesizer to
        play the score
      - print or create pdf files

Licence:
      MuseScore is distributed under the GNU General Public License (GPL).
      Please read the file LICENSE in this directory for more
      details.

Requirements:
      - Qt version >= qt5.1.x

        Note that many linux distributions split packages into a user
        and developer package and that you need to install both.

      - ALSA Version 1.0 or newer (audio/midi input/output)
      - CMake >= 2.8.9
      - libsndfile >= 1.0.19

      - portaudio 19, required for windows version

Credits:
      MuseScore uses the "Emmentaler" font from the lilypond project.

Installation:
      - unpack source distribution
            tar xvofj mscore-x.x.x.tar.bz2

      - make
            cd mscore-x.x.x
            make release

      if something goes wrong, then remove the whole build subdirectory
      with "make clean" and start new with "make release"

      - install as root user
            sudo make install

User Documentation:
      Look at the handbook section at http://www.musescore.org

Program Documentation:
      Type
            cd build
            make doxy

      to generate the program documentation with DoxyGen.

      Browse the documentation with your favourite html browser
      at:
            build/Doc/html/index.html


Test:
      Type
            cd build.release/mscore
            ./mscore

      to start mscore. On first invocation a demofile
      is shown. You probably want to change that in the
      "preferences" dialog.

Debug:
      A debug version can be built by doing "make debug" above,
      instead of "make release".

      To test the debug version, type

            cd build.debug/mscore
            ./mscore

=================================================================
      Contact
=================================================================

      Werner Schweer; ws at wschweer.de
      http://www.musescore.org/

Links
* [MuseScore Homepage](http://musescore.org)
* [MuseScore Git workflow instructions](http://musescore.org/en/developers-handbook/git-workflow).
* [How to compile MuseScore?](http://musescore.org/en/developers-handbook/compilation).
* Build status: [![Build Status](https://secure.travis-ci.org/musescore/MuseScore.png)](http://travis-ci.org/musescore/MuseScore)

Packages
* **aeolus** Clone of [Aeolus](http://kokkinizita.linuxaudio.org/linuxaudio/aeolus/)

* **awl** Audio Widget Library, from the MusE project

* **build** Utility files for build

* **bww2mxml** Command line tool to convert BWW files to MusicXML. BWW parser is used by MuseScore to import BWW files.

* **demos** A few MuseScore files to demonstrate what can be done

* **fluid** Clone of [FluidSynth](http://sourceforge.net/apps/trac/fluidsynth/), ported to C++ and customized

* **fonts** Contains fontforge source (sfd) + ttf/otf fonts

* **libmscore** Data model of MuseScore

* **mscore** Main code for the MuseScore UI

* **mstyle** Clone of KDE4 style Oxygen

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
    Not used currently. [Diff, Match and Patch Library](http://code.google.com/p/google-diff-match-patch/)

    * **thirdparty/ofqf**
    OSC server interface. Based on [OSC for Qt4](http://www.arnoldarts.de/drupal/?q=ofqf)

    * **thirdparty/singleapp**
    Clone from [Qt Single Application](http://qt.gitorious.org/qt-solutions/qt-solutions/trees/master/qtsingleapplication)

   * **thirdparty/portmidi**
   Clone from [PortMidi](http://portmedia.sourceforge.net/)
