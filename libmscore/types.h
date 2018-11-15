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
      MEASURE_NUMBER,
      INSTRUMENT_NAME,
      SLUR_SEGMENT,
      TIE_SEGMENT,
      BAR_LINE,
      STAFF_LINES,
      SYSTEM_DIVIDER,
      STEM_SLASH,
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
      FERMATA,
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
      LET_RING_SEGMENT,
      VIBRATO_SEGMENT,
      PALM_MUTE_SEGMENT,
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
      LET_RING,
      VIBRATO,
      PALM_MUTE,
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

//---------------------------------------------------------
//   Direction
//---------------------------------------------------------

enum class Direction {
      AUTO, UP, DOWN
      };

//---------------------------------------------------------
//   GlissandoType
//---------------------------------------------------------

enum class GlissandoType {
      STRAIGHT, WAVY
      };

//---------------------------------------------------------
//   GlissandoStyle
//---------------------------------------------------------

enum class GlissandoStyle {
      CHROMATIC, WHITE_KEYS, BLACK_KEYS, DIATONIC
      };

//---------------------------------------------------------
//   Placement
//---------------------------------------------------------

enum class Placement {
      ABOVE, BELOW
      };

//---------------------------------------------------------
//   OffsetType
//---------------------------------------------------------

enum class OffsetType : char {
      ABS,       ///< offset in point units
      SPATIUM    ///< offset in staff space units
      };

//---------------------------------------------------------
//   Align
//---------------------------------------------------------

enum class Align : char {
      LEFT     = 0,
      RIGHT    = 1,
      HCENTER  = 2,
      TOP      = 0,
      BOTTOM   = 4,
      VCENTER  = 8,
      BASELINE = 16,
      CENTER = Align::HCENTER | Align::VCENTER,
      HMASK  = Align::LEFT    | Align::RIGHT    | Align::HCENTER,
      VMASK  = Align::TOP     | Align::BOTTOM   | Align::VCENTER | Align::BASELINE
      };

constexpr Align operator| (Align a1, Align a2) {
      return static_cast<Align>(static_cast<char>(a1) | static_cast<char>(a2));
      }
constexpr bool operator& (Align a1, Align a2) {
      return static_cast<char>(a1) & static_cast<char>(a2);
      }
constexpr Align operator~ (Align a) {
      return static_cast<Align>(~static_cast<char>(a));
      }

//---------------------------------------------------------
//   FontStyle
//---------------------------------------------------------

enum class FontStyle : char {
      Normal = 0, Bold = 1, Italic = 2, Underline = 4
      };

constexpr FontStyle operator+ (FontStyle a1, FontStyle a2) {
      return static_cast<FontStyle>(static_cast<char>(a1) | static_cast<char>(a2));
      }
constexpr FontStyle operator- (FontStyle a1, FontStyle a2) {
      return static_cast<FontStyle>(static_cast<char>(a1) & ~static_cast<char>(a2));
      }
constexpr bool operator& (FontStyle a1, FontStyle a2) {
      return static_cast<bool>(static_cast<char>(a1) & static_cast<char>(a2));
      }

//---------------------------------------------------------
//   Tuplets
//---------------------------------------------------------

enum class TupletNumberType  : char { SHOW_NUMBER, SHOW_RELATION, NO_TEXT         };
enum class TupletBracketType : char { AUTO_BRACKET, SHOW_BRACKET, SHOW_NO_BRACKET };


Q_ENUM_NS(ElementType)
Q_ENUM_NS(Direction)

//hack: to force the build system to run moc on this file
class Mops : public QObject {
      Q_GADGET
      };

extern Direction toDirection(const QString&);
extern const char* toString(Direction);
extern QString toUserString(Direction);
extern void fillComboBoxDirection(QComboBox*);


} // namespace Ms

Q_DECLARE_METATYPE(Ms::Align)


#endif
