#=============================================================================
#  MuseScore
#  Music Composition & Notation
#  $Id:$
#
#  Copyright (C) 2011 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENSE.GPL
#=============================================================================

if (SCRIPT_INTERFACE)
   QT4_WRAP_CPP (libqtMocs
      accidental.h arpeggio.h articulation.h barline.h beam.h bend.h
      box.h bracket.h breath.h bsymbol.h chord.h chordline.h chordrest.h
      clef.h duration.h dynamic.h element.h figuredbass.h fingering.h fret.h
      glissando.h hairpin.h harmony.h hook.h icon.h image.h instrchange.h
      keysig.h lasso.h layoutbreak.h line.h lyrics.h measurebase.h measure.h
      notedot.h note.h ossia.h ottava.h page.h pedal.h rehearsalmark.h repeat.h
      rest.h segment.h shadownote.h simpletext.h slur.h tie.h spacer.h spanner.h
      stafftext.h stem.h symbol.h system.h tempotext.h textframe.h text.h
      textline.h timesig.h tremolobar.h tremolo.h trill.h tuplet.h volta.h
      score.h cursor.h page.h part.h staff.h mscore.h staffstate.h marker.h
      jump.h stemslash.h ledgerline.h noteline.h bagpembell.h
      )
endif (SCRIPT_INTERFACE)

add_custom_command(
   OUTPUT ${PROJECT_BINARY_DIR}/all.h
   COMMAND ${CMAKE_COMMAND}
   ARGS -E copy ${PROJECT_SOURCE_DIR}/all.h ${PROJECT_BINARY_DIR}/all.h
   DEPENDS ${PROJECT_SOURCE_DIR}/all.h
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )

if (APPLE)
        file(GLOB_RECURSE INCS "*.h")
else (APPLE)
        set(INCS "")
endif (APPLE)

add_library (
      libmscore STATIC
      ${PROJECT_BINARY_DIR}/all.h
      ${libqtMocs}
      ${INCS}
      segmentlist.cpp fingering.cpp accidental.cpp arpeggio.cpp
      articulation.cpp barline.cpp beam.cpp bend.cpp box.cpp
      bracket.cpp breath.cpp bsp.cpp chord.cpp chordline.cpp
      chordlist.cpp chordrest.cpp clef.cpp cleflist.cpp
      drumset.cpp durationtype.cpp dynamic.cpp edit.cpp
      element.cpp elementlayout.cpp excerpt.cpp
      fifo.cpp fret.cpp glissando.cpp hairpin.cpp
      harmony.cpp hook.cpp image.cpp iname.cpp instrchange.cpp
      instrtemplate.cpp instrument.cpp interval.cpp
      key.cpp keyfinder.cpp keysig.cpp lasso.cpp
      layoutbreak.cpp layout.cpp line.cpp lyrics.cpp measurebase.cpp
      measure.cpp navigate.cpp note.cpp noteevent.cpp ottava.cpp
      page.cpp part.cpp pedal.cpp pitch.cpp pitchspelling.cpp
      rendermidi.cpp repeat.cpp repeatlist.cpp rest.cpp
      score.cpp segment.cpp select.cpp shadownote.cpp slur.cpp tie.cpp
      spacer.cpp spanner.cpp staff.cpp staffstate.cpp
      stafftext.cpp stafftype.cpp stem.cpp style.cpp symbol.cpp
      sym.cpp system.cpp tablature.cpp tempotext.cpp text.cpp
      textframe.cpp textline.cpp timesig.cpp
      tremolobar.cpp tremolo.cpp trill.cpp tuplet.cpp
      utils.cpp velo.cpp volta.cpp xml.cpp mscore.cpp
      undo.cpp cmd.cpp scorefile.cpp revisions.cpp
      check.cpp input.cpp icon.cpp ossia.cpp
      tempo.cpp sig.cpp pos.cpp fraction.cpp duration.cpp
      figuredbass.cpp simpletext.cpp rehearsalmark.cpp transpose.cpp
      property.cpp range.cpp elementmap.cpp notedot.cpp imageStore.cpp
      qzip.cpp audio.cpp splitMeasure.cpp joinMeasure.cpp
      cursor.cpp read114.cpp paste.cpp
      bsymbol.cpp marker.cpp jump.cpp stemslash.cpp ledgerline.cpp
      synthesizerstate.cpp mcursor.cpp groups.cpp mscoreview.cpp
      noteline.cpp spannermap.cpp bagpembell.cpp
      )
if (SCRIPT_INTERFACE)
   set_target_properties (
      libmscore
      PROPERTIES
         COMPILE_FLAGS "-g ${PCH_INCLUDE} -Wall -Wextra -Winvalid-pch"
      )
else (SCRIPT_INTERFACE)
   set_target_properties (
      libmscore
      PROPERTIES
         COMPILE_FLAGS "-g ${PCH_INCLUDE} -include libmscore/noscript.h -Wall -Wextra -Winvalid-pch"
      )
endif (SCRIPT_INTERFACE)

xcode_pch(libmscore all)

ADD_DEPENDENCIES(libmscore mops1)
ADD_DEPENDENCIES(libmscore mops2)
