//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TYPES_H__
#define __TYPES_H__

namespace Ms {
Q_NAMESPACE

//-------------------------------------------------------------------
//    The value of this enum determines the "stacking order"
//    of elements on the canvas.
//   Note: keep in sync with array elementNames[] in scoreElement.cpp
//-------------------------------------------------------------------

enum class ElementType {
      INVALID = 0,
      BRACKET_ITEM,
      PART,
      STAFF,
      SCORE,
      SYMBOL,
      TEXT,
      INSTRUMENT_NAME,
      SLUR_SEGMENT,
      TIE_SEGMENT,
      STAFF_LINES,
      BAR_LINE,
      SYSTEM_DIVIDER,
      STEM_SLASH,
      LINE,
      ARPEGGIO,
      ACCIDENTAL,
      LEDGER_LINE,
      STEM,                   // list STEM before NOTE: notes in TAB might 'break' stems
      NOTE,                   // and this requires stems to be drawn before notes
      CLEF,                   // elements from CLEF to TIMESIG need to be in the order
      KEYSIG,                 // in which they appear in a measure
      AMBITUS,
      TIMESIG,
      REST,
      BREATH,
      REPEAT_MEASURE,
      TIE,
      ARTICULATION,
      CHORDLINE,
      DYNAMIC,
      BEAM,
      HOOK,
      LYRICS,
      FIGURED_BASS,
      MARKER,
      JUMP,
      FINGERING,
      TUPLET,
      TEMPO_TEXT,
      STAFF_TEXT,
      SYSTEM_TEXT,
      REHEARSAL_MARK,
      INSTRUMENT_CHANGE,
      STAFFTYPE_CHANGE,
      HARMONY,
      FRET_DIAGRAM,
      BEND,
      TREMOLOBAR,
      VOLTA,
      HAIRPIN_SEGMENT,
      OTTAVA_SEGMENT,
      TRILL_SEGMENT,
      TEXTLINE_SEGMENT,
      VOLTA_SEGMENT,
      PEDAL_SEGMENT,
      LYRICSLINE_SEGMENT,
      GLISSANDO_SEGMENT,
      LAYOUT_BREAK,
      SPACER,
      STAFF_STATE,
      NOTEHEAD,
      NOTEDOT,
      TREMOLO,
      IMAGE,
      MEASURE,
      SELECTION,
      LASSO,
      SHADOW_NOTE,
      TAB_DURATION_SYMBOL,
      FSYMBOL,
      PAGE,
      HAIRPIN,
      OTTAVA,
      PEDAL,
      TRILL,
      TEXTLINE,
      TEXTLINE_BASE,
      NOTELINE,
      LYRICSLINE,
      GLISSANDO,
      BRACKET,
      SEGMENT,
      SYSTEM,
      COMPOUND,
      CHORD,
      SLUR,
      ELEMENT,
      ELEMENT_LIST,
      STAFF_LIST,
      MEASURE_LIST,
      HBOX,
      VBOX,
      TBOX,
      FBOX,
      ICON,
      OSSIA,
      BAGPIPE_EMBELLISHMENT,

      MAXTYPE
      };

Q_ENUM_NS(ElementType)

//---------------------------------------------------------
//   Direction
//---------------------------------------------------------

enum class Direction {
      AUTO, UP, DOWN
      };

Q_ENUM_NS(Direction)

//hack: to force the build system to run moc on this file
class Mops : public QObject {
      Q_GADGET
      };


extern Direction toDirection(const QString&);
extern const char* toString(Direction);
extern void fillComboBoxDirection(QComboBox*);


} // namespace Ms

// Q_DECLARE_METATYPE(Ms::Direction);

#endif




