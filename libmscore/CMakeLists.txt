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

add_library (
      libmscore STATIC
      segmentlist.cpp fingering.cpp accidental.cpp arpeggio.cpp
      articulation.cpp barline.cpp beam.cpp bend.cpp box.cpp
      bracket.cpp breath.cpp bsp.cpp chord.cpp chordline.cpp
      chordlist.cpp chordrest.cpp clef.cpp cleflist.cpp
      drumset.cpp durationtype.cpp dynamic.cpp edit.cpp
      element.cpp elementlayout.cpp event.cpp excerpt.cpp
      fifo.cpp fret.cpp glissando.cpp hairpin.cpp
      harmony.cpp hook.cpp image.cpp iname.cpp instrchange.cpp
      instrtemplate.cpp instrument.cpp interval.cpp
      key.cpp keyfinder.cpp keysig.cpp lasso.cpp
      layoutbreak.cpp layout.cpp line.cpp lyrics.cpp measurebase.cpp
      measure.cpp navigate.cpp note.cpp noteevent.cpp ottava.cpp
      page.cpp part.cpp pedal.cpp pitch.cpp pitchspelling.cpp
      rendermidi.cpp repeat.cpp repeatlist.cpp rest.cpp
      score.cpp segment.cpp select.cpp shadownote.cpp slur.cpp
      spacer.cpp spanner.cpp staff.cpp staffstate.cpp
      stafftext.cpp stafftype.cpp stem.cpp style.cpp symbol.cpp
      sym.cpp system.cpp tablature.cpp tempotext.cpp text.cpp
      textframe.cpp textline.cpp timesig.cpp
      tremolobar.cpp tremolo.cpp trill.cpp tuplet.cpp
      utils.cpp velo.cpp volta.cpp xml.cpp mscore.cpp
      undo.cpp cmd.cpp scorefile.cpp revisions.cpp
      check.cpp input.cpp icon.cpp ossia.cpp
      dsp.cpp tempo.cpp sig.cpp pos.cpp fraction.cpp duration.cpp
      figuredbass.cpp simpletext.cpp rehearsalmark.cpp transpose.cpp
      property.cpp range.cpp elementmap.cpp notedot.cpp imageStore.cpp
      qzip.cpp audio.cpp splitMeasure.cpp joinMeasure.cpp midifile.cpp
      exportmidi.cpp read114.cpp
      )
set_target_properties (
      libmscore
      PROPERTIES
         COMPILE_FLAGS "-g -include ${PROJECT_BINARY_DIR}/all.h -Wall -Wextra -Winvalid-pch"
      )

