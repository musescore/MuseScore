#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2016 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENSE.GPL
#=============================================================================

#if (SCRIPT_INTERFACE)
      set (LIB_SCRIPT_FILES plugins.cpp)
#endif (SCRIPT_INTERFACE)

# MSVC does not depend on copying all.h to the binary directory
if (NOT MSVC)
   add_custom_command(
      OUTPUT ${PROJECT_BINARY_DIR}/all.h
      COMMAND ${CMAKE_COMMAND}
      ARGS -E copy ${PROJECT_SOURCE_DIR}/all.h ${PROJECT_BINARY_DIR}/all.h
      DEPENDS ${PROJECT_SOURCE_DIR}/all.h
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      )
endif (NOT MSVC)

if (APPLE)
        file(GLOB_RECURSE INCS "*.h")
else (APPLE)
        set(INCS "")
endif (APPLE)

if (NOT MSVC)
   set(_all_h_file "${PROJECT_BINARY_DIR}/all.h")
else (NOT MSVC)
   set(_all_h_file "${PROJECT_SOURCE_DIR}/all.h")
endif (NOT MSVC)

include_directories(
      ${PROJECT_SOURCE_DIR}/thirdparty/diff
      )

add_library (
      libmscore STATIC
      ${_all_h_file}
      ${INCS}
      ${LIB_SCRIPT_FILES}

      types.h accidental.h ambitus.h arpeggio.h articulation.h audio.h bagpembell.h barline.h beam.h bend.h
      box.h bracket.h bracketItem.h breath.h bsp.h bsymbol.h chord.h chordline.h chordlist.h chordrest.h clef.h
      cleflist.h connector.h cursor.h drumset.h dsp.h duration.h durationtype.h dynamic.h element.h
      elementmap.h excerpt.h fermata.h fifo.h figuredbass.h fingering.h fraction.h fret.h glissando.h groups.h hairpin.h
      harmony.h hook.h icon.h image.h imageStore.h iname.h input.h instrchange.h instrtemplate.h instrument.h interval.h
      jump.h key.h keylist.h keysig.h lasso.h layout.h layoutbreak.h ledgerline.h letring.h line.h location.h
      lyrics.h marker.h mcursor.h measure.h measurebase.h mscore.h mscoreview.h musescoreCore.h navigate.h note.h notedot.h
      noteevent.h noteline.h ossia.h ottava.h page.h palmmute.h part.h pedal.h pitch.h pitchspelling.h pitchvalue.h plugins.h
      pos.h property.h range.h read206.h rehearsalmark.h repeat.h repeatlist.h rest.h revisions.h score.h scoreElement.h segment.h
      segmentlist.h select.h sequencer.h shadownote.h shape.h sig.h slur.h slurtie.h spacer.h spanner.h spannermap.h spatium.h
      staff.h stafflines.h staffstate.h stafftext.h stafftextbase.h stafftype.h stafftypechange.h stafftypelist.h stem.h
      stemslash.h stringdata.h style.h sym.h symbol.h synthesizerstate.h system.h systemdivider.h systemtext.h tempo.h
      tempotext.h text.h measurenumber.h textbase.h textedit.h textframe.h textline.h textlinebase.h tie.h tiemap.h timesig.h
      tremolo.h tremolobar.h trill.h tuplet.h tupletmap.h types.h undo.h utils.h velo.h vibrato.h volta.h xml.h

      segmentlist.cpp fingering.cpp accidental.cpp arpeggio.cpp
      fermata.cpp articulation.cpp barline.cpp beam.cpp bend.cpp box.cpp
      bracket.cpp breath.cpp bsp.cpp chord.cpp chordline.cpp
      chordlist.cpp chordrest.cpp clef.cpp cleflist.cpp
      drumset.cpp durationtype.cpp dynamic.cpp edit.cpp noteentry.cpp
      element.cpp excerpt.cpp
      fifo.cpp fret.cpp glissando.cpp hairpin.cpp
      harmony.cpp hook.cpp image.cpp iname.cpp instrchange.cpp
      instrtemplate.cpp instrument.cpp interval.cpp
      key.cpp keysig.cpp lasso.cpp
      layoutbreak.cpp layout.cpp line.cpp lyrics.cpp measurebase.cpp
      measure.cpp navigate.cpp note.cpp noteevent.cpp ottava.cpp
      page.cpp part.cpp pedal.cpp letring.cpp vibrato.cpp palmmute.cpp pitch.cpp pitchspelling.cpp
      rendermidi.cpp repeat.cpp repeatlist.cpp rest.cpp
      score.cpp segment.cpp select.cpp shadownote.cpp slur.cpp tie.cpp slurtie.cpp
      spacer.cpp spanner.cpp staff.cpp staffstate.cpp
      stafftextbase.cpp stafftext.cpp systemtext.cpp stafftype.cpp stem.cpp style.cpp symbol.cpp
      sym.cpp system.cpp stringdata.cpp tempotext.cpp text.cpp measurenumber.cpp textbase.cpp textedit.cpp
      textframe.cpp textline.cpp textlinebase.cpp timesig.cpp
      tremolobar.cpp tremolo.cpp trill.cpp tuplet.cpp
      utils.cpp velo.cpp volta.cpp xmlreader.cpp xmlwriter.cpp mscore.cpp
      undo.cpp cmd.cpp scorefile.cpp revisions.cpp
      check.cpp input.cpp icon.cpp ossia.cpp
      tempo.cpp sig.cpp pos.cpp fraction.cpp duration.cpp
      figuredbass.cpp rehearsalmark.cpp transpose.cpp
      property.cpp range.cpp elementmap.cpp notedot.cpp imageStore.cpp
      audio.cpp splitMeasure.cpp joinMeasure.cpp
      cursor.cpp paste.cpp
      bsymbol.cpp marker.cpp jump.cpp stemslash.cpp ledgerline.cpp
      synthesizerstate.cpp mcursor.cpp groups.cpp mscoreview.cpp
      noteline.cpp spannermap.cpp
      bagpembell.cpp ambitus.cpp keylist.cpp scoreElement.cpp
      shape.cpp systemdivider.cpp midimapping.cpp stafflines.cpp
      read114.cpp
      read206.cpp
      read301.cpp stafftypelist.cpp stafftypechange.cpp
      bracketItem.cpp
      lyricsline.cpp
      layoutlinear.cpp
      connector.cpp location.cpp skyline.cpp
      scorediff.cpp
      )

target_link_libraries(libmscore
      diff_match_patch
      )

##
## Code coverage. Only affects DEBUG builds.
##

set(COVERAGE_OPTIONS "")
if (NOT APPLE AND NOT MINGW AND NOT MSVC AND CMAKE_BUILD_TYPE MATCHES "DEBUG")
   if (COVERAGE)
      set(COVERAGE_OPTIONS "-O0 --coverage")
      set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
      target_link_libraries(libmscore gcov)
   endif (COVERAGE)
endif (NOT APPLE AND NOT MINGW AND NOT MSVC AND CMAKE_BUILD_TYPE MATCHES "DEBUG")



if (NOT MSVC)
   set_target_properties (
      libmscore
      PROPERTIES
         COMPILE_FLAGS "-g ${PCH_INCLUDE} -Wall -Wextra -Winvalid-pch -Woverloaded-virtual ${COVERAGE_OPTIONS}"
      )
else (NOT MSVC)
   set_target_properties (
      libmscore
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE}"
      )
endif (NOT MSVC)

xcode_pch(libmscore all)

# Use MSVC pre-compiled headers
vstudio_pch( libmscore )

# MSVC does not depend on mops1 & mops2 for PCH
if (NOT MSVC)
   ADD_DEPENDENCIES(libmscore mops1)
   ADD_DEPENDENCIES(libmscore mops2)
endif (NOT MSVC)
