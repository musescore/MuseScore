/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __TYPES_H__
#define __TYPES_H__

#include "config.h"

#ifdef SCRIPT_INTERFACE
#include <QObject>
#endif

#include <unordered_set>

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
    MMREST_RANGE,
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
    STEM,                     // list STEM before NOTE: notes in TAB might 'break' stems
    NOTE,                     // and this requires stems to be drawn before notes
    CLEF,                     // elements from CLEF to TIMESIG need to be in the order
    KEYSIG,                   // in which they appear in a measure
    AMBITUS,
    TIMESIG,
    REST,
    MMREST,
    BREATH,
    MEASURE_REPEAT,
    TIE,
    ARTICULATION,
    FERMATA,
    CHORDLINE,
    SLIDE,
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
    PLAYTECH_ANNOTATION,
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
    TEMPO_RANGED_CHANGE_SEGMENT,
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
    TEMPO_RANGED_CHANGE,
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
    ACTION_ICON,
    OSSIA,
    BAGPIPE_EMBELLISHMENT,
    STICKING,

    ROOT_ITEM,
    DUMMY,

    MAXTYPE
    ///\}
};

using ElementTypeSet = std::unordered_set<ElementType>;

inline uint qHash(const ElementType& key)
{
    return static_cast<uint>(key);
}

//---------------------------------------------------------
//   AccidentalType
//---------------------------------------------------------
// NOTE: keep this in sync with with accList array in accidentals.cpp

enum class AccidentalType {
    ///.\{
    NONE,
    FLAT,
    NATURAL,
    SHARP,
    SHARP2,
    FLAT2,
    SHARP3,
    FLAT3,
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
    ARROW_DOWN,
    ARROW_UP,

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

    FLAT_17,
    SHARP_17,
    FLAT_19,
    SHARP_19,
    FLAT_23,
    SHARP_23,
    FLAT_31,
    SHARP_31,
    FLAT_53,
    SHARP_53,
    //EQUALS_ALMOST,
    //EQUALS,
    //TILDE,

    // Persian
    SORI,
    KORON,

    // Wyschnegradsky
    TEN_TWELFTH_FLAT,
    TEN_TWELFTH_SHARP,
    ELEVEN_TWELFTH_FLAT,
    ELEVEN_TWELFTH_SHARP,
    ONE_TWELFTH_FLAT,
    ONE_TWELFTH_SHARP,
    TWO_TWELFTH_FLAT,
    TWO_TWELFTH_SHARP,
    THREE_TWELFTH_FLAT,
    THREE_TWELFTH_SHARP,
    FOUR_TWELFTH_FLAT,
    FOUR_TWELFTH_SHARP,
    FIVE_TWELFTH_FLAT,
    FIVE_TWELFTH_SHARP,
    SIX_TWELFTH_FLAT,
    SIX_TWELFTH_SHARP,
    SEVEN_TWELFTH_FLAT,
    SEVEN_TWELFTH_SHARP,
    EIGHT_TWELFTH_FLAT,
    EIGHT_TWELFTH_SHARP,
    NINE_TWELFTH_FLAT,
    NINE_TWELFTH_SHARP,

    // (Spartan) Sagittal
    SAGITTAL_5V7KD,
    SAGITTAL_5V7KU,
    SAGITTAL_5CD,
    SAGITTAL_5CU,
    SAGITTAL_7CD,
    SAGITTAL_7CU,
    SAGITTAL_25SDD,
    SAGITTAL_25SDU,
    SAGITTAL_35MDD,
    SAGITTAL_35MDU,
    SAGITTAL_11MDD,
    SAGITTAL_11MDU,
    SAGITTAL_11LDD,
    SAGITTAL_11LDU,
    SAGITTAL_35LDD,
    SAGITTAL_35LDU,
    SAGITTAL_FLAT25SU,
    SAGITTAL_SHARP25SD,
    SAGITTAL_FLAT7CU,
    SAGITTAL_SHARP7CD,
    SAGITTAL_FLAT5CU,
    SAGITTAL_SHARP5CD,
    SAGITTAL_FLAT5V7KU,
    SAGITTAL_SHARP5V7KD,
    SAGITTAL_FLAT,
    SAGITTAL_SHARP,

    // Turkish folk music
    ONE_COMMA_FLAT,
    ONE_COMMA_SHARP,
    TWO_COMMA_FLAT,
    TWO_COMMA_SHARP,
    THREE_COMMA_FLAT,
    THREE_COMMA_SHARP,
    FOUR_COMMA_FLAT,
    //FOUR_COMMA_SHARP,
    FIVE_COMMA_SHARP,

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
    APPOGGIATURA  = 0x2,         // grace notes
    GRACE4        = 0x4,
    GRACE16       = 0x8,
    GRACE32       = 0x10,
    GRACE8_AFTER  = 0x20,
    GRACE16_AFTER = 0x40,
    GRACE32_AFTER = 0x80,
    INVALID       = 0xFF
                    ///\}
};

constexpr NoteType operator|(NoteType t1, NoteType t2)
{
    return static_cast<NoteType>(static_cast<int>(t1) | static_cast<int>(t2));
}

constexpr bool operator&(NoteType t1, NoteType t2)
{
    return static_cast<int>(t1) & static_cast<int>(t2);
}

//---------------------------------------------------------
//   GlissandoType
//---------------------------------------------------------

enum class GlissandoType {
    ///.\{
    STRAIGHT, WAVY
    ///\}
};

//---------------------------------------------------------
//   HarmonyType
//---------------------------------------------------------

enum class HarmonyType {
    ///.\{
    STANDARD,
    ROMAN,
    NASHVILLE
    ///\}
};

//---------------------------------------------------------
//   MMRestRangeBracketType
//---------------------------------------------------------

enum class MMRestRangeBracketType {
    ///.\{
    BRACKETS, PARENTHESES, NONE
    ///\}
};

//---------------------------------------------------------
//   OffsetType
//---------------------------------------------------------

enum class OffsetType : char {
    ABS,         ///< offset in point units
    SPATIUM      ///< offset in staff space units
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
    All                = -1,   ///< Includes all barline types
    /// Alias for `BeginBarLine | StartRepeatBarLine | BarLine | EndBarLine`
    BarLineType        = BeginBarLine | StartRepeatBarLine | BarLine | EndBarLine
                         ///\}
};

constexpr SegmentType operator|(const SegmentType t1, const SegmentType t2)
{
    return static_cast<SegmentType>(static_cast<int>(t1) | static_cast<int>(t2));
}

constexpr bool operator&(const SegmentType t1, const SegmentType t2)
{
    return static_cast<int>(t1) & static_cast<int>(t2);
}

//---------------------------------------------------------
//   FontStyle
//---------------------------------------------------------

enum class FontStyle : char {
    Undefined = -1,
    Normal = 0,
    Bold = 1 << 0,
    Italic = 1 << 1,
    Underline = 1 << 2,
    Strike = 1 << 3
};

constexpr FontStyle operator+(FontStyle a1, FontStyle a2)
{
    return static_cast<FontStyle>(static_cast<char>(a1) | static_cast<char>(a2));
}

constexpr FontStyle operator-(FontStyle a1, FontStyle a2)
{
    return static_cast<FontStyle>(static_cast<char>(a1) & ~static_cast<char>(a2));
}

constexpr bool operator&(FontStyle a1, FontStyle a2)
{
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
    Auto,         ///< Play events for all notes are calculated by MuseScore.
    User,         ///< Some play events are modified by user. Those events are written into the mscx file.
    ///.\}
};

//---------------------------------------------------------
//   Tuplets
//---------------------------------------------------------

enum class TupletNumberType : char {
    SHOW_NUMBER, SHOW_RELATION, NO_TEXT
};
enum class TupletBracketType : char {
    AUTO_BRACKET, SHOW_BRACKET, SHOW_NO_BRACKET
};

struct ScoreChangesRange {
    int tickFrom = -1;
    int tickTo = -1;
    int staffIdxFrom = -1;
    int staffIdxTo = -1;

    ElementTypeSet changedTypes;

    bool isValidBoundary() const
    {
        bool tickRangeValid = (tickFrom != -1 && tickTo != -1);
        bool staffRangeValid = (staffIdxFrom != -1 && staffIdxTo != -1);

        return tickRangeValid && staffRangeValid;
    }

    bool isValid() const
    {
        return isValidBoundary() || !changedTypes.empty();
    }
};

#ifdef SCRIPT_INTERFACE
Q_ENUM_NS(ElementType);
Q_ENUM_NS(GlissandoType);
Q_ENUM_NS(SegmentType);
Q_ENUM_NS(NoteType);
Q_ENUM_NS(PlayEventType);
Q_ENUM_NS(AccidentalType);
Q_ENUM_NS(HarmonyType);
#endif

//hack: to force the build system to run moc on this file
/// \private
class Mops : public QObject
{
    Q_GADGET
};
} // namespace Ms

Q_DECLARE_METATYPE(Ms::NoteType);

Q_DECLARE_METATYPE(Ms::PlayEventType);

Q_DECLARE_METATYPE(Ms::AccidentalType);

#endif
