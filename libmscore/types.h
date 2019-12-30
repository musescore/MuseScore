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

#include "config.h"

/**
 * \namespace Ms .
 */

namespace Ms {
#ifdef SCRIPT_INTERFACE
Q_NAMESPACE
#endif

//-------------------------------------------------------------------
///   \internal
///   The value of this enum determines the "stacking order"
///   of elements on the canvas.
///   Note: keep in sync with array elementNames[] in scoreElement.cpp
//-------------------------------------------------------------------

enum class ElementType {
      ///.\{
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
      STICKING,

      MAXTYPE
      ///\}
      };

//---------------------------------------------------------
//   AccidentalType
//---------------------------------------------------------
// NOTE: keep this in sync with with accList array

enum class AccidentalType : char {
      ///.\{
      NONE,
      FLAT,
      NATURAL,
      SHARP,
      SHARP2,
      FLAT2,
      //SHARP3,
      //FLAT3,
      NATURAL_FLAT,
      NATURAL_SHARP,
      SHARP_SHARP,

      // Gould arrow quartertone
      FLAT_ARROW_UP,
      FLAT_ARROW_DOWN,
      NATURAL_ARROW_UP,
      NATURAL_ARROW_DOWN,
      SHARP_ARROW_UP,
      SHARP_ARROW_DOWN,
      SHARP2_ARROW_UP,
      SHARP2_ARROW_DOWN,
      FLAT2_ARROW_UP,
      FLAT2_ARROW_DOWN,

      // Stein-Zimmermann
      MIRRORED_FLAT,
      MIRRORED_FLAT2,
      SHARP_SLASH,
      SHARP_SLASH4,

      // Arel-Ezgi-Uzdilek (AEU)
      FLAT_SLASH2,
      FLAT_SLASH,
      SHARP_SLASH3,
      SHARP_SLASH2,

      // Extended Helmholtz-Ellis accidentals (just intonation)
      DOUBLE_FLAT_ONE_ARROW_DOWN,
      FLAT_ONE_ARROW_DOWN,
      NATURAL_ONE_ARROW_DOWN,
      SHARP_ONE_ARROW_DOWN,
      DOUBLE_SHARP_ONE_ARROW_DOWN,
      DOUBLE_FLAT_ONE_ARROW_UP,

      FLAT_ONE_ARROW_UP,
      NATURAL_ONE_ARROW_UP,
      SHARP_ONE_ARROW_UP,
      DOUBLE_SHARP_ONE_ARROW_UP,
      DOUBLE_FLAT_TWO_ARROWS_DOWN,
      FLAT_TWO_ARROWS_DOWN,

      NATURAL_TWO_ARROWS_DOWN,
      SHARP_TWO_ARROWS_DOWN,
      DOUBLE_SHARP_TWO_ARROWS_DOWN,
      DOUBLE_FLAT_TWO_ARROWS_UP,
      FLAT_TWO_ARROWS_UP,
      NATURAL_TWO_ARROWS_UP,

      SHARP_TWO_ARROWS_UP,
      DOUBLE_SHARP_TWO_ARROWS_UP,
      DOUBLE_FLAT_THREE_ARROWS_DOWN,
      FLAT_THREE_ARROWS_DOWN,
      NATURAL_THREE_ARROWS_DOWN,
      SHARP_THREE_ARROWS_DOWN,

      DOUBLE_SHARP_THREE_ARROWS_DOWN,
      DOUBLE_FLAT_THREE_ARROWS_UP,
      FLAT_THREE_ARROWS_UP,
      NATURAL_THREE_ARROWS_UP,
      SHARP_THREE_ARROWS_UP,
      DOUBLE_SHARP_THREE_ARROWS_UP,

      LOWER_ONE_SEPTIMAL_COMMA,
      RAISE_ONE_SEPTIMAL_COMMA,
      LOWER_TWO_SEPTIMAL_COMMAS,
      RAISE_TWO_SEPTIMAL_COMMAS,
      LOWER_ONE_UNDECIMAL_QUARTERTONE,
      RAISE_ONE_UNDECIMAL_QUARTERTONE,

      LOWER_ONE_TRIDECIMAL_QUARTERTONE,
      RAISE_ONE_TRIDECIMAL_QUARTERTONE,

      DOUBLE_FLAT_EQUAL_TEMPERED,
      FLAT_EQUAL_TEMPERED,
      NATURAL_EQUAL_TEMPERED,
      SHARP_EQUAL_TEMPERED,
      DOUBLE_SHARP_EQUAL_TEMPERED,
      QUARTER_FLAT_EQUAL_TEMPERED,
      QUARTER_SHARP_EQUAL_TEMPERED,

      // Persian
      SORI,
      KORON,
      END
      ///\}
      };

//---------------------------------------------------------
//   NoteType
//---------------------------------------------------------

enum class NoteType {
      ///.\{
      NORMAL        = 0,
      ACCIACCATURA  = 0x1,
      APPOGGIATURA  = 0x2,       // grace notes
      GRACE4        = 0x4,
      GRACE16       = 0x8,
      GRACE32       = 0x10,
      GRACE8_AFTER  = 0x20,
      GRACE16_AFTER = 0x40,
      GRACE32_AFTER = 0x80,
      INVALID       = 0xFF
      ///\}
      };

constexpr NoteType operator| (NoteType t1, NoteType t2) {
      return static_cast<NoteType>(static_cast<int>(t1) | static_cast<int>(t2));
      }
constexpr bool operator& (NoteType t1, NoteType t2) {
      return static_cast<int>(t1) & static_cast<int>(t2);
      }


//---------------------------------------------------------
//   Direction
//---------------------------------------------------------

enum class Direction {
      ///.\{
      AUTO, UP, DOWN
      ///\}
      };

//---------------------------------------------------------
//   GlissandoType
//---------------------------------------------------------

enum class GlissandoType {
      ///.\{
      STRAIGHT, WAVY
      ///\}
      };

//---------------------------------------------------------
//   GlissandoStyle
//---------------------------------------------------------

enum class GlissandoStyle {
      ///.\{
      CHROMATIC, WHITE_KEYS, BLACK_KEYS, DIATONIC
      ///\}
      };

//---------------------------------------------------------
//   Placement
//---------------------------------------------------------

enum class Placement {
      ///.\{
      ABOVE, BELOW
      ///\}
      };

//---------------------------------------------------------
//   OffsetType
//---------------------------------------------------------

enum class OffsetType : char {
      ABS,       ///< offset in point units
      SPATIUM    ///< offset in staff space units
      };

//-------------------------------------------------------------------
//   SegmentType
//
//    Type values determine the order of segments for a given tick
//-------------------------------------------------------------------

enum class SegmentType {
      ///.\{
      Invalid            = 0x0,
      BeginBarLine       = 0x1,
      HeaderClef         = 0x2,
      KeySig             = 0x4,
      Ambitus            = 0x8,
      TimeSig            = 0x10,
      StartRepeatBarLine = 0x20,
      Clef               = 0x40,
      BarLine            = 0x80,
      Breath             = 0x100,
      //--
      ChordRest          = 0x200,
      //--
      EndBarLine         = 0x400,
      KeySigAnnounce     = 0x800,
      TimeSigAnnounce    = 0x1000,
      All                = -1, ///< Includes all barline types
      /// Alias for `BeginBarLine | StartRepeatBarLine | BarLine | EndBarLine`
      BarLineType        = BeginBarLine | StartRepeatBarLine | BarLine | EndBarLine
      ///\}
      };

constexpr SegmentType operator| (const SegmentType t1, const SegmentType t2) {
      return static_cast<SegmentType>(static_cast<int>(t1) | static_cast<int>(t2));
      }
constexpr bool operator& (const SegmentType t1, const SegmentType t2) {
      return static_cast<int>(t1) & static_cast<int>(t2);
      }

//-------------------------------------------------------------------
//   Tid
///   Enumerates the list of built-in text substyles
///   \internal
///   Must be in sync with textStyles array (in style.cpp)
//-------------------------------------------------------------------

enum class Tid {
      ///.\{
      DEFAULT,
      TITLE,
      SUBTITLE,
      COMPOSER,
      POET,
      LYRICS_ODD,
      LYRICS_EVEN,
      FINGERING,
      LH_GUITAR_FINGERING,
      RH_GUITAR_FINGERING,
      STRING_NUMBER,
      INSTRUMENT_LONG,
      INSTRUMENT_SHORT,
      INSTRUMENT_EXCERPT,
      DYNAMICS,
      EXPRESSION,
      TEMPO,
      METRONOME,
      MEASURE_NUMBER,
      TRANSLATOR,
      TUPLET,
      SYSTEM,
      STAFF,
      HARMONY_A,
      HARMONY_B,
      HARMONY_ROMAN,
      HARMONY_NASHVILLE,
      REHEARSAL_MARK,
      REPEAT_LEFT,       // align to start of measure
      REPEAT_RIGHT,      // align to end of measure
      FRAME,
      TEXTLINE,
      GLISSANDO,
      OTTAVA,
      VOLTA,
      PEDAL,
      LET_RING,
      PALM_MUTE,
      HAIRPIN,
      BEND,
      HEADER,
      FOOTER,
      INSTRUMENT_CHANGE,
      STICKING,
      USER1,
      USER2,
      USER3,
      USER4,
      USER5,
      USER6,
      // special, no-contents, styles used while importing older scores
      TEXT_STYLES,           // used for user-defined styles
      IGNORED_STYLES         // used for styles no longer relevant (mainly Figured bass text style)
      ///.\}
      };

//---------------------------------------------------------
//   Align
//---------------------------------------------------------

enum class Align : char {
      ///.\{
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
      ///.\}
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
//   PlayEventType
/// Determines whether oranaments are automatically generated
/// when playing a score and whether the PlayEvents are saved
/// in the score file.
//---------------------------------------------------------

enum class PlayEventType : char {
      ///.\{
      Auto,       ///< Play events for all notes are calculated by MuseScore.
      User,       ///< Some play events are modified by user. Those events are written into the mscx file.
      ///.\}
      };

//---------------------------------------------------------
//   Tuplets
//---------------------------------------------------------

enum class TupletNumberType  : char { SHOW_NUMBER, SHOW_RELATION, NO_TEXT         };
enum class TupletBracketType : char { AUTO_BRACKET, SHOW_BRACKET, SHOW_NO_BRACKET };

#ifdef SCRIPT_INTERFACE
Q_ENUM_NS(ElementType);
Q_ENUM_NS(Direction);
Q_ENUM_NS(GlissandoType);
Q_ENUM_NS(GlissandoStyle);
Q_ENUM_NS(Placement);
Q_ENUM_NS(SegmentType);
Q_ENUM_NS(Tid);
Q_ENUM_NS(Align);
Q_ENUM_NS(NoteType);
Q_ENUM_NS(PlayEventType);
Q_ENUM_NS(AccidentalType);
#endif

//hack: to force the build system to run moc on this file
/// \private
class Mops : public QObject {
      Q_GADGET
      };

extern Direction toDirection(const QString&);
extern const char* toString(Direction);
extern QString toUserString(Direction);
extern void fillComboBoxDirection(QComboBox*);


} // namespace Ms

Q_DECLARE_METATYPE(Ms::Align);

Q_DECLARE_METATYPE(Ms::Direction);

Q_DECLARE_METATYPE(Ms::NoteType);

Q_DECLARE_METATYPE(Ms::PlayEventType);

Q_DECLARE_METATYPE(Ms::AccidentalType);

#endif
