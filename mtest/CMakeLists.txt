#=============================================================================
#  MuseScore
#  Music Composition & Notation
#  $Id:$
#
#  Copyright (C) 2011-2016 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENSE.GPL
#=============================================================================

# set(CMAKE_VERBOSE_MAKEFILE ON)
enable_testing()

include_directories(
      ${PROJECT_BINARY_DIR}
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_SOURCE_DIR}/thirdparty/freetype/include
      )

if (OMR)
set(OMR_SRC ${PROJECT_SOURCE_DIR}/omr/importpdf.cpp)
endif (OMR)

QT5_ADD_RESOURCES(qrc_files ${PROJECT_SOURCE_DIR}/mtest/mtest.qrc
            ${PROJECT_SOURCE_DIR}/mscore/musescorefonts-MScore.qrc
            ${PROJECT_SOURCE_DIR}/mscore/musescorefonts-Gootville.qrc
            ${PROJECT_SOURCE_DIR}/mscore/musescorefonts-Bravura.qrc
            ${PROJECT_SOURCE_DIR}/mscore/musescorefonts-MuseJazz.qrc
            ${PROJECT_SOURCE_DIR}/mscore/musescorefonts-Free.qrc
            ${PROJECT_SOURCE_DIR}/mscore/musescorefonts-FreeSerif.qrc
      )

add_library (testResources STATIC
      ${qrc_files}
      )

add_library(
      testutils STATIC
      testutils.cpp
      ${PROJECT_SOURCE_DIR}/libmscore/mcursor.cpp
      ${PROJECT_SOURCE_DIR}/mscore/bb.cpp
      ${PROJECT_SOURCE_DIR}/mscore/capella.cpp
      ${PROJECT_SOURCE_DIR}/mscore/capxml.cpp
      ${PROJECT_SOURCE_DIR}/mscore/exportxml.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importgtp.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importgtp-gp4.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importgtp-gp5.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importgtp-gp6.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importptb.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importove.cpp
      ${PROJECT_SOURCE_DIR}/mscore/ove.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_operations.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_meter.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_quant.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_tuplet.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_tuplet_detect.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_tuplet_filter.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_tuplet_voice.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_tuplet_tonotes.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_chord.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_fraction.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_swing.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_drum.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_clef.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_lrhand.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_lyrics.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_tie.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_inner.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_beat.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_simplify.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_voice.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_key.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_tempo.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_model.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_instrument.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmidi/importmidi_chordname.cpp
      ${PROJECT_SOURCE_DIR}/mscore/exportmidi.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmxml.cpp               # Required by importxml.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmxmllogger.cpp         # Required by importxml.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmxmlnoteduration.cpp   # Required by importxml.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmxmlnotepitch.cpp      # Required by importxml.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmxmlpass1.cpp          # Required by importxml.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importmxmlpass2.cpp          # Required by importxml.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importxml.cpp
      ${PROJECT_SOURCE_DIR}/mscore/importxmlfirstpass.cpp
      ${PROJECT_SOURCE_DIR}/mscore/musicxmlfonthandler.cpp
      ${PROJECT_SOURCE_DIR}/mscore/musicxmlsupport.cpp
      ${PROJECT_SOURCE_DIR}/mscore/qmlplugin.cpp
      ${PROJECT_SOURCE_DIR}/mscore/preferences.cpp
      ${PROJECT_SOURCE_DIR}/mscore/shortcut.cpp
      ${PROJECT_SOURCE_DIR}/thirdparty/rtf2html/fmt_opts.cpp    # Required by capella.cpp and capxml.cpp
      ${PROJECT_SOURCE_DIR}/thirdparty/rtf2html/rtf2html.cpp    # Required by capella.cpp and capxml.cpp
      ${PROJECT_SOURCE_DIR}/thirdparty/rtf2html/rtf_keyword.cpp # Required by capella.cpp and capxml.cpp
      ${PROJECT_SOURCE_DIR}/thirdparty/rtf2html/rtf_table.cpp   # Required by capella.cpp and capxml.cpp
      ${PROJECT_SOURCE_DIR}/thirdparty/beatroot/Agent.cpp           # Required by importmidi.cpp
      ${PROJECT_SOURCE_DIR}/thirdparty/beatroot/AgentList.cpp       # Required by importmidi.cpp
      ${PROJECT_SOURCE_DIR}/thirdparty/beatroot/BeatTracker.cpp     # Required by importmidi.cpp
      ${PROJECT_SOURCE_DIR}/thirdparty/beatroot/Induction.cpp       # Required by importmidi.cpp
      ${OMR_SRC}
      omr
      )

add_executable(
      mtest
      mtest.cpp
      )

target_link_libraries(
      mtest
      ${QT_LIBRARIES}
      mscore_freetype
      )

set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

if (APPLE)
   set(CMAKE_CXX_FLAGS_DEBUG   "-std=c++0x -fPIC -stdlib=libc++ -g -Wno-inconsistent-missing-override")
   set(CMAKE_CXX_FLAGS_RELEASE "-std=c++0x -fPIC -stdlib=libc++ -O2 -DNDEBUG -DQT_NO_DEBUG -Wno-inconsistent-missing-override")
else (APPLE)
   if (MINGW)
      # -mno-ms-bitfields see #22048
      set(CMAKE_CXX_FLAGS_DEBUG   "-std=gnu++0x -mno-ms-bitfields -g")
      set(CMAKE_CXX_FLAGS_RELEASE "-std=gnu++0x -mno-ms-bitfields -O2 -DNDEBUG -DQT_NO_DEBUG")
   else (MINGW)
      set(CMAKE_CXX_FLAGS_DEBUG   "-std=gnu++0x -fPIC -g")
      set(CMAKE_CXX_FLAGS_RELEASE "-std=gnu++0x -fPIC -O2 -DNDEBUG -DQT_NO_DEBUG -DQT_NO_DEBUG_OUTPUT")
   endif (MINGW)
endif(APPLE)


string(REPLACE ";" ";-I" INC "${QT_INCLUDES}")

if (APPLE AND (CMAKE_VERSION VERSION_LESS "3.5.0"))
set_target_properties (
      testutils
      PROPERTIES
      COMPILE_FLAGS "-include all.h -D TESTROOT=\\\\\"${PROJECT_SOURCE_DIR}\\\\\" -g -Wall -Wextra"
      )
else (APPLE AND (CMAKE_VERSION VERSION_LESS "3.5.0"))
set_target_properties (
      testutils
      PROPERTIES
      COMPILE_FLAGS "-include all.h -D TESTROOT=\\\"${PROJECT_SOURCE_DIR}\\\" -g -Wall -Wextra"
      )
endif (APPLE AND (CMAKE_VERSION VERSION_LESS "3.5.0"))

#      COMPILE_FLAGS "-include all.h -I ${INC} -D TESTROOT=\\\"${PROJECT_SOURCE_DIR}\\\" -g -Wall -Wextra"

add_custom_target(report
      COMMAND ant -f ${PROJECT_SOURCE_DIR}/mtest/build.xml -Droot.dir=${PROJECT_BINARY_DIR}/mtest report
      WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/mtest"
      )

add_custom_target(reporthtml
      COMMAND ant -f ${PROJECT_SOURCE_DIR}/mtest/build.xml -Droot.dir=${PROJECT_BINARY_DIR}/mtest reporthtml
      WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/mtest"
      )

subdirs (
#        libmscore/compat114
        libmscore/compat206
#        libmscore/album            # obsolete
        libmscore/barline
        libmscore/beam
        libmscore/breath
        libmscore/chordsymbol
        libmscore/clef
        libmscore/clef_courtesy
        libmscore/concertpitch
        libmscore/copypaste
        libmscore/copypastesymbollist
        libmscore/dynamic
        libmscore/durationtype
        libmscore/earlymusic
        libmscore/element
        libmscore/exchangevoices
        libmscore/hairpin
        libmscore/implode_explode
        libmscore/instrumentchange
        libmscore/join
        libmscore/keysig
        libmscore/layout
        libmscore/links
        libmscore/parts
        libmscore/measure
        libmscore/midi                 # one disabled
#        libmscore/midimapping
        libmscore/note
        libmscore/repeat
        libmscore/rhythmicGrouping
        libmscore/selectionfilter
        libmscore/selectionrangedelete
        libmscore/spanners
        libmscore/split
        libmscore/splitstaff
        libmscore/timesig
        libmscore/tools                # Some tests disabled
        libmscore/transpose
        libmscore/tuplet
#        libmscore/text        work in progress...
        importmidi
        capella
        biab
        musicxml
        guitarpro
        scripting
#        testoves
        zerberus/comments
        zerberus/envelopes
        zerberus/includes
        zerberus/global
        zerberus/opcodeparse
        zerberus/inputControls
        zerberus/loop
        )


install(FILES
      ../share/styles/chords_std.xml
      ../share/styles/chords_jazz.xml
      ../share/styles/chords.xml
      ../share/styles/stdchords.xml
      ../share/styles/jazzchords.xml
      ../share/styles/cchords_muse.xml
      ../share/styles/cchords_nrb.xml
      ../share/styles/cchords_rb.xml
      ../share/styles/cchords_sym.xml
      DESTINATION ${PROJECT_BINARY_DIR}/mtest/styles
      )


if (OMR)
subdirs(omr)
endif (OMR)
