/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#pragma once

#include <functional>
#include <map>
#include <unordered_set>

#include "global/types/id.h"
#include "global/types/string.h"
#include "global/types/translatablestring.h"
#include "global/types/flags.h"

#include "draw/types/color.h"
#include "draw/types/geometry.h"
#include "draw/types/painterpath.h"

#include "bps.h"
#include "dimension.h"
#include "fraction.h"
#include "groupnode.h"
#include "pitchvalue.h"
#include "symid.h"

namespace mu::engraving {
using staff_idx_t = size_t;

using track_idx_t = size_t;
using TracksMap = std::multimap<track_idx_t, track_idx_t>;

using voice_idx_t = size_t;

using system_idx_t = size_t;
using part_idx_t = size_t;
using page_idx_t = size_t;

using string_idx_t = size_t;

using semitone_t = int8_t;

//-------------------------------------------------------------------
///   The value of this enum determines the "stacking order"
///   of elements on the canvas.
///   Note: keep in sync with array in TConv
//-------------------------------------------------------------------
enum class ElementType : unsigned char {
    INVALID = 0,
    BRACKET_ITEM,
    PART,
    STAFF,
    SCORE,
    TEXT,
    LAYOUT_BREAK,
    MEASURE_NUMBER,
    MMREST_RANGE,
    INSTRUMENT_NAME,
    SLUR_SEGMENT,
    TIE_SEGMENT,
    LAISSEZ_VIB_SEGMENT,
    PARTIAL_TIE_SEGMENT,
    BAR_LINE,
    STAFF_LINES,
    SYSTEM_DIVIDER,
    STEM_SLASH,
    ARPEGGIO,
    ACCIDENTAL,
    LEDGER_LINE,
    STEM,   // list STEM before NOTE: notes in TAB might 'break' stems
    HOOK,   // and this requires stems to be drawn before notes
    NOTE,   // elements from CLEF to TIMESIG need to be in the order
    SYMBOL, // in which they appear in a measure
    CLEF,
    KEYSIG,
    AMBITUS,
    TIMESIG,
    REST,
    MMREST,
    DEAD_SLAPPED,
    BREATH,
    MEASURE_REPEAT,
    TIE,
    LAISSEZ_VIB,
    PARTIAL_TIE,
    ARTICULATION,
    ORNAMENT,
    FERMATA,
    CHORDLINE,
    DYNAMIC,
    EXPRESSION,
    BEAM,
    LYRICS,
    FIGURED_BASS,
    FIGURED_BASS_ITEM,
    MARKER,
    JUMP,
    FINGERING,
    TUPLET,
    TEMPO_TEXT,
    STAFF_TEXT,
    SYSTEM_TEXT,
    SOUND_FLAG,
    PLAYTECH_ANNOTATION,
    CAPO,
    STRING_TUNINGS,
    TRIPLET_FEEL,
    REHEARSAL_MARK,
    INSTRUMENT_CHANGE,
    STAFFTYPE_CHANGE,
    HARMONY,
    FRET_DIAGRAM,
    HARP_DIAGRAM,
    BEND,
    TREMOLOBAR,
    VOLTA,
    HAIRPIN_SEGMENT,
    OTTAVA_SEGMENT,
    TRILL_SEGMENT,
    LET_RING_SEGMENT,
    GRADUAL_TEMPO_CHANGE_SEGMENT,
    VIBRATO_SEGMENT,
    PALM_MUTE_SEGMENT,
    WHAMMY_BAR_SEGMENT,
    RASGUEADO_SEGMENT,
    HARMONIC_MARK_SEGMENT,
    PICK_SCRAPE_SEGMENT,
    TEXTLINE_SEGMENT,
    VOLTA_SEGMENT,
    PEDAL_SEGMENT,
    LYRICSLINE_SEGMENT,
    PARTIAL_LYRICSLINE_SEGMENT,
    GLISSANDO_SEGMENT,
    NOTELINE_SEGMENT,
    SYSTEM_LOCK_INDICATOR,
    SPACER,
    STAFF_STATE,
    NOTEHEAD,
    NOTEDOT,
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
    GRADUAL_TEMPO_CHANGE,
    VIBRATO,
    PALM_MUTE,
    WHAMMY_BAR,
    RASGUEADO,
    HARMONIC_MARK,
    PICK_SCRAPE,
    TEXTLINE,
    TEXTLINE_BASE,
    NOTELINE,
    LYRICSLINE,
    PARTIAL_LYRICSLINE,
    GLISSANDO,
    BRACKET,
    SEGMENT,
    SYSTEM,
    CHORD,
    SLUR,
    HBOX,
    VBOX,
    TBOX,
    FBOX,
    ACTION_ICON,
    BAGPIPE_EMBELLISHMENT,
    STICKING,
    GRACE_NOTES_GROUP,
    FRET_CIRCLE,
    GUITAR_BEND,
    GUITAR_BEND_SEGMENT,
    GUITAR_BEND_HOLD,
    GUITAR_BEND_HOLD_SEGMENT,
    GUITAR_BEND_TEXT,
    TREMOLO_TWOCHORD,
    TREMOLO_SINGLECHORD,
    TIME_TICK_ANCHOR,
    PARENTHESIS,
    HAMMER_ON_PULL_OFF,
    HAMMER_ON_PULL_OFF_SEGMENT,
    HAMMER_ON_PULL_OFF_TEXT,

    ROOT_ITEM,
    DUMMY,

    MAXTYPE
};

constexpr size_t TOT_ELEMENT_TYPES = static_cast<size_t>(ElementType::MAXTYPE);

using ElementTypeSet = std::unordered_set<ElementType>;

// ========================================
// PropertyValue
// ========================================
// --- Common ---
using String = muse::String;
using StringList = muse::StringList;
using TranslatableString = muse::TranslatableString;
using Char = muse::Char;
using AsciiStringView = muse::AsciiStringView;
using real_t = muse::real_t;
using ID = muse::ID;

// --- Geometry ---
using Point = muse::Point;
using PointF = muse::PointF;              // P_TYPE::POINT
using RectF = muse::RectF;
using LineF = muse::LineF;
using SizeF = muse::SizeF;                // P_TYPE::SIZE
using PainterPath = muse::draw::PainterPath; // P_TYPE::PATH
using ScaleF = muse::ScaleF;              // P_TYPE::SCALE
using PairF = muse::PairF;                // P_TYPE::PAIR_REAL
using PolygonF = muse::PolygonF;

// --- Draw ---
using Color = muse::draw::Color;        // P_TYPE::COLOR

enum class OrnamentStyle : unsigned char {
    DEFAULT, BAROQUE
};

// P_TYPE::GLISS_STYLE
enum class GlissandoStyle : unsigned char {
    CHROMATIC, WHITE_KEYS, BLACK_KEYS, DIATONIC, PORTAMENTO
};

// --- Layout ---

// P_TYPE::ALIGN
enum class AlignV : unsigned char {
    TOP,
    VCENTER,
    BOTTOM,
    BASELINE
};

enum class AlignH : unsigned char {
    LEFT,
    RIGHT,
    HCENTER
};

struct Align {
    AlignH horizontal = AlignH::LEFT;
    AlignV vertical = AlignV::TOP;

    Align() = default;
    Align(AlignH ah, AlignV av)
        : horizontal(ah), vertical(av) {}

    Align(AlignH a)
        : horizontal(a) {}
    Align(AlignV a)
        : vertical(a) {}

    inline Align& operator =(AlignH a) { horizontal = a; return *this; }
    inline Align& operator =(AlignV a) { vertical = a; return *this; }

    inline bool operator ==(const Align& a) const { return a.vertical == vertical && a.horizontal == horizontal; }
    inline bool operator !=(const Align& a) const { return !operator ==(a); }

    inline bool operator ==(const AlignV& a) const { return a == vertical; }
    inline bool operator !=(const AlignV& a) const { return !operator ==(a); }

    inline bool operator ==(const AlignH& a) const { return a == horizontal; }
    inline bool operator !=(const AlignH& a) const { return !operator ==(a); }
};

// P_TYPE::PLACEMENT_V
enum class PlacementV : unsigned char {
    ABOVE, BELOW
};

// P_TYPE::PLACEMENT_H
enum class PlacementH : unsigned char {
    LEFT, CENTER, RIGHT
};

// P_TYPE::TEXT_PLACE
enum class TextPlace : unsigned char {
    AUTO, ABOVE, BELOW, LEFT
};

// P_TYPE::DIRECTION
enum class DirectionV : unsigned char {
    AUTO, UP, DOWN
};

// P_TYPE::DIRECTION_H
enum class DirectionH : unsigned char {
    AUTO, LEFT, RIGHT
};

// P_TYPE::ORIENTATION
enum class Orientation : signed char {
    VERTICAL,
    HORIZONTAL
};

enum class AutoOnOff : unsigned char {
    AUTO,
    ON,
    OFF
};

//! Note: from lowest to highest priority
enum class VoiceAssignment : unsigned char {
    ALL_VOICE_IN_INSTRUMENT,
    ALL_VOICE_IN_STAFF,
    CURRENT_VOICE_ONLY
};

// P_TYPE::BEAM_MODE
//! Note: for historical reasons, these have strange names
//!
enum class BeamMode : signed char {
    INVALID = -1,
    AUTO,
    NONE,
    BEGIN,
    // TODO:
    // strange names aside, mscx files refer to BEGIN16 and BEGIN32 as begin32/begin64, which would describe the 3rd and 4th beams,
    // which is wildly incorrect.These enum names are CORRECT, but I haven't touched the mscx files yet.
    // changing this for the save files would necessitate some serious file version / import work. -A
    BEGIN16,
    BEGIN32,
    MID,
    END
};

enum class DurationType : signed char {
    V_LONG, V_BREVE, V_WHOLE, V_HALF, V_QUARTER, V_EIGHTH, V_16TH,
    V_32ND, V_64TH, V_128TH, V_256TH, V_512TH, V_1024TH,
    V_ZERO, V_MEASURE,  V_INVALID
};

// P_TYPE::DURATION_TYPE_WITH_DOTS
struct DurationTypeWithDots
{
    DurationType type = DurationType::V_INVALID;
    int dots = 0;
    DurationTypeWithDots() = default;
    DurationTypeWithDots(DurationType t, int d = 0)
        : type(t), dots(d) {}

    inline bool operator ==(const DurationTypeWithDots& other) const { return other.type == type && other.dots == dots; }
};

// --- Types ---

// P_TYPE::LAYOUTBREAK_TYPE
enum class LayoutBreakType : unsigned char {
    PAGE, LINE, SECTION, NOBREAK
};

// P_TYPE::VELO_TYPE
enum class VeloType : unsigned char {
    OFFSET_VAL, USER_VAL
};

// P_TYPE::BARLINE_TYPE
enum class BarLineType {
    NORMAL           = 1,
    SINGLE           = BarLineType::NORMAL,
    DOUBLE           = 2,
    START_REPEAT     = 4,
    LEFT_REPEAT      = BarLineType::START_REPEAT,
    END_REPEAT       = 8,
    RIGHT_REPEAT     = BarLineType::END_REPEAT,
    BROKEN           = 0x10,
    DASHED           = BarLineType::BROKEN,
    END              = 0x20,
    FINAL            = BarLineType::END,
    END_START_REPEAT = 0x40,
    LEFT_RIGHT_REPEAT= BarLineType::END_START_REPEAT,
    DOTTED           = 0x80,
    REVERSE_END      = 0x100,
    REVERSE_FINALE   = BarLineType::REVERSE_END,
    HEAVY            = 0x200,
    DOUBLE_HEAVY     = 0x400,
};

constexpr BarLineType operator|(BarLineType t1, BarLineType t2)
{
    return static_cast<BarLineType>(static_cast<int>(t1) | static_cast<int>(t2));
}

constexpr bool operator&(BarLineType t1, BarLineType t2)
{
    return static_cast<int>(t1) & static_cast<int>(t2);
}

// P_TYPE::NOTEHEAD_TYPE
enum class NoteHeadType : signed char {
    HEAD_AUTO    = -1,
    HEAD_WHOLE   = 0,
    HEAD_HALF    = 1,
    HEAD_QUARTER = 2,
    HEAD_BREVIS  = 3,
    HEAD_TYPES
};

// P_TYPE::NOTEHEAD_SCHEME
enum class NoteHeadScheme : signed char {
    HEAD_AUTO = -1,
    HEAD_NORMAL,
    HEAD_PITCHNAME,
    HEAD_PITCHNAME_GERMAN,
    HEAD_SOLFEGE,
    HEAD_SOLFEGE_FIXED,
    HEAD_SHAPE_NOTE_4,
    HEAD_SHAPE_NOTE_7_AIKIN,
    HEAD_SHAPE_NOTE_7_FUNK,
    HEAD_SHAPE_NOTE_7_WALKER,
    HEAD_SCHEMES
};

// P_TYPE::NOTEHEAD_GROUP
enum class NoteHeadGroup : signed char {
    HEAD_NORMAL = 0,
    HEAD_CROSS,
    HEAD_PLUS,
    HEAD_XCIRCLE,
    HEAD_WITHX,
    HEAD_TRIANGLE_UP,
    HEAD_TRIANGLE_DOWN,
    HEAD_SLASHED1,
    HEAD_SLASHED2,
    HEAD_DIAMOND,
    HEAD_DIAMOND_OLD,
    HEAD_CIRCLED,
    HEAD_CIRCLED_LARGE,
    HEAD_LARGE_ARROW,
    HEAD_BREVIS_ALT,

    HEAD_SLASH,
    HEAD_LARGE_DIAMOND,

    HEAD_SOL,
    HEAD_LA,
    HEAD_FA,
    HEAD_MI,
    HEAD_DO,
    HEAD_RE,
    HEAD_TI,

    HEAD_HEAVY_CROSS,
    HEAD_HEAVY_CROSS_HAT,

    // not exposed from here
    HEAD_DO_WALKER,
    HEAD_RE_WALKER,
    HEAD_TI_WALKER,
    HEAD_DO_FUNK,
    HEAD_RE_FUNK,
    HEAD_TI_FUNK,

    HEAD_DO_NAME,
    HEAD_DI_NAME,
    HEAD_RA_NAME,
    HEAD_RE_NAME,
    HEAD_RI_NAME,
    HEAD_ME_NAME,
    HEAD_MI_NAME,
    HEAD_FA_NAME,
    HEAD_FI_NAME,
    HEAD_SE_NAME,
    HEAD_SOL_NAME,
    HEAD_LE_NAME,
    HEAD_LA_NAME,
    HEAD_LI_NAME,
    HEAD_TE_NAME,
    HEAD_TI_NAME,
    HEAD_SI_NAME,

    HEAD_A_SHARP,
    HEAD_A,
    HEAD_A_FLAT,
    HEAD_B_SHARP,
    HEAD_B,
    HEAD_B_FLAT,
    HEAD_C_SHARP,
    HEAD_C,
    HEAD_C_FLAT,
    HEAD_D_SHARP,
    HEAD_D,
    HEAD_D_FLAT,
    HEAD_E_SHARP,
    HEAD_E,
    HEAD_E_FLAT,
    HEAD_F_SHARP,
    HEAD_F,
    HEAD_F_FLAT,
    HEAD_G_SHARP,
    HEAD_G,
    HEAD_G_FLAT,
    HEAD_H,
    HEAD_H_SHARP,

    HEAD_SWISS_RUDIMENTS_FLAM,
    HEAD_SWISS_RUDIMENTS_DOUBLE,

    HEAD_CUSTOM,
    HEAD_GROUPS,
    HEAD_INVALID = -1
};

// P_TYPE::CLEF_TYPE
// keep in sync with clefTable in clef.cpp, CLEF_TYPES in typesconv.cpp and CLEF_INFOS in exportxml.cpp
enum class ClefType : signed char {
    INVALID = -1,
    G = 0,
    G15_MB,
    G8_VB,
    G8_VA,
    G15_MA,
    G8_VB_O,
    G8_VB_P,
    G_1,
    C1,
    C2,
    C3,
    C4,
    C5,
    C_19C,
    C1_F18C,
    C3_F18C,
    C4_F18C,
    C1_F20C,
    C3_F20C,
    C4_F20C,
    F,
    F15_MB,
    F8_VB,
    F_8VA,
    F_15MA,
    F_B,
    F_C,
    F_F18C,
    F_19C,
    PERC,
    PERC2,
    TAB,
    TAB4,
    TAB_SERIF,
    TAB4_SERIF,
    // new clefs to be added between here
    C4_8VB,
    G8_VB_C,
    // and here in oder to not break TAB clef style
    MAX
};

enum class ClefToBarlinePosition : unsigned char {
    AUTO,
    BEFORE,
    AFTER
};

// P_TYPE::DYNAMIC_TYPE
enum class DynamicType : unsigned char {
    OTHER,
    PPPPPP,
    PPPPP,
    PPPP,
    PPP,
    PP,
    P,
    MP,
    MF,
    F,
    FF,
    FFF,
    FFFF,
    FFFFF,
    FFFFFF,
    FP,
    PF,
    SF,
    SFZ,
    SFF,
    SFFZ,
    SFFF,
    SFFFZ,
    SFP,
    SFPP,
    RFZ,
    RF,
    FZ,
    M,
    R,
    S,
    Z,
    N,
    LAST
};

//! OBSOLETE. Use VoiceAssignment
enum class DynamicRange : unsigned char {
    STAFF, PART, SYSTEM
};

inline VoiceAssignment dynamicRangeToVoiceAssignment(DynamicRange range)
{
    switch (range) {
    case DynamicRange::STAFF:
        return VoiceAssignment::ALL_VOICE_IN_STAFF;
    case DynamicRange::PART:
    case DynamicRange::SYSTEM:
        break;
    }

    return VoiceAssignment::ALL_VOICE_IN_INSTRUMENT;
}

// P_TYPE::DYNAMIC_SPEED
enum class DynamicSpeed : unsigned char {
    SLOW, NORMAL, FAST
};

// P_TYPE::LINE_TYPE
enum class LineType : unsigned char {
    SOLID, DASHED, DOTTED
};

// P_TYPE::HOOK_TYPE
enum class HookType : unsigned char {
    NONE, HOOK_90, HOOK_45, HOOK_90T
};

// P_TYPE::KEY_MODE
enum class KeyMode : signed char {
    UNKNOWN = -1,
    NONE,
    MAJOR,
    MINOR,
    DORIAN,
    PHRYGIAN,
    LYDIAN,
    MIXOLYDIAN,
    AEOLIAN,
    IONIAN,
    LOCRIAN
};

// P_TYPE::INT
enum class ArpeggioType : unsigned char {
    NORMAL, UP, DOWN, BRACKET, UP_STRAIGHT, DOWN_STRAIGHT
};

enum class IntervalStep : unsigned char {
    UNISON,
    SECOND,
    THIRD,
    FOURTH,
    FIFTH,
    SIXTH,
    SEVENTH,
    OCTAVE
};

enum class IntervalType : unsigned char {
    AUTO,
    AUGMENTED,
    MAJOR,
    PERFECT,
    MINOR,
    DIMINISHED
};

enum class InstrumentLabelVisibility : unsigned char {
    LONG,
    SHORT,
    HIDE
};

struct OrnamentInterval
{
    IntervalStep step = IntervalStep::SECOND;
    IntervalType type = IntervalType::AUTO;

    OrnamentInterval() = default;
    OrnamentInterval(IntervalStep s, IntervalType t)
        : step(s), type(t) {}

    inline bool operator ==(const OrnamentInterval& interval) const { return step == interval.step && type == interval.type; }
    inline bool operator !=(const OrnamentInterval& interval) const { return !operator ==(interval); }

    static bool isPerfectStep(IntervalStep step)
    {
        static const std::unordered_set<IntervalStep> perfectSteps {
            IntervalStep::UNISON,
            IntervalStep::FOURTH,
            IntervalStep::FIFTH,
            IntervalStep::OCTAVE
        };
        return muse::contains(perfectSteps, step);
    }

    bool isPerfect() const
    {
        return isPerfectStep(step);
    }
};

static const OrnamentInterval DEFAULT_ORNAMENT_INTERVAL = OrnamentInterval(IntervalStep::SECOND, IntervalType::AUTO);

enum class OrnamentShowAccidental : unsigned char {
    DEFAULT,
    ANY_ALTERATION,
    ALWAYS,
};

enum class PartialSpannerDirection : signed char {
    NONE = -1,
    INCOMING,
    OUTGOING,
    BOTH
};

enum ChordStylePreset : unsigned char {
    STANDARD,
    JAZZ,
    LEGACY,
    CUSTOM
};

//-------------------------------------------------------------------
//   Tid
///   Enumerates the list of built-in text substyles
///   \internal
///   Must be in sync with textStyles (in textstyle.cpp)
//-------------------------------------------------------------------
// P_TYPE::TEXT_STYLE
enum class TextStyleType : unsigned char {
    DEFAULT,

    // Page-oriented styles
    TITLE,
    SUBTITLE,
    COMPOSER,
    LYRICIST,
    TRANSLATOR,
    FRAME,
    INSTRUMENT_EXCERPT,
    INSTRUMENT_LONG,
    INSTRUMENT_SHORT,
    INSTRUMENT_CHANGE,
    HEADER,
    FOOTER,
    COPYRIGHT,
    PAGE_NUMBER,

    // Measure-oriented styles
    MEASURE_NUMBER,
    MMREST_RANGE,

    // System-level styles
    TEMPO,
    TEMPO_CHANGE,
    METRONOME,
    REPEAT_LEFT,       // align to start of measure
    REPEAT_RIGHT,      // align to end of measure
    REHEARSAL_MARK,
    SYSTEM,

    // Staff oriented styles
    STAFF,
    EXPRESSION,
    DYNAMICS,
    HAIRPIN,
    LYRICS_ODD,
    LYRICS_EVEN,
    HARMONY_A,
    HARMONY_B,
    HARMONY_ROMAN,
    HARMONY_NASHVILLE,

    // Note oriented styles
    TUPLET,
    STICKING,
    FINGERING,
    LH_GUITAR_FINGERING,
    RH_GUITAR_FINGERING,
    HAMMER_ON_PULL_OFF,
    STRING_NUMBER,
    STRING_TUNINGS,
    FRET_DIAGRAM_FINGERING,
    FRET_DIAGRAM_FRET_NUMBER,
    HARP_PEDAL_DIAGRAM,
    HARP_PEDAL_TEXT_DIAGRAM,

    // Line-oriented styles
    TEXTLINE,
    NOTELINE,
    VOLTA,
    OTTAVA,
    GLISSANDO,
    PEDAL,
    BEND,
    LET_RING,
    PALM_MUTE,

    // User styles
    USER1,
    USER2,
    USER3,
    USER4,
    USER5,
    USER6,
    USER7,
    USER8,
    USER9,
    USER10,
    USER11,
    USER12,
    // special, no-contents, styles used while importing older scores
    TEXT_TYPES,           // used for user-defined types
    IGNORED_TYPES         // used for types no longer relevant (mainly Figured bass text type)
};

enum class AnnotationCategory : signed char {
    Undefined = -1,
    TempoAnnotation,
    PlayingAnnotation,
    Other,
};

enum class PlayingTechniqueType : signed char {
    Undefined = -1,
    Natural,
    Pizzicato,
    Open,
    Mute,
    Tremolo,
    Detache,
    Martele,
    ColLegno,
    SulPonticello,
    SulTasto,
    Vibrato,
    Legato,
    Distortion,
    Overdrive,
    Harmonics,
    JazzTone,
};

enum class GradualTempoChangeType : signed char {
    Undefined = -1,
    Accelerando,
    Allargando,
    Calando,
    Lentando,
    Morendo,
    Precipitando,
    Rallentando,
    Ritardando,
    Smorzando,
    Sostenuto,
    Stringendo
};

// P_TYPE::CHANGE_METHOD
enum class ChangeMethod : signed char {
    NORMAL,
    EXPONENTIAL,
    EASE_IN,
    EASE_OUT,
    EASE_IN_OUT        // and shake it all about
};

enum class ChangeDirection : signed char {
    INCREASING,
    DECREASING
};

// P_TYPE::ACCIDENTAL_ROLE
enum class AccidentalRole : unsigned char {
    AUTO,                 // layout created accidental
    USER                  // user created accidental
};

enum class AccidentalVal : signed char {
    SHARP3  = 3,
    SHARP2  = 2,
    SHARP   = 1,
    NATURAL = 0,
    FLAT    = -1,
    FLAT2   = -2,
    FLAT3   = -3,
    MIN     = FLAT3,
    MAX     = SHARP3
};

enum class FermataType : signed char {
    Undefined = -1,
    VeryShort,
    Short,
    ShortHenze,
    Normal,
    Long,
    LongHenze,
    VeryLong
};

enum class ChordLineType : unsigned char {
    NOTYPE, FALL, DOIT,
    PLOP, SCOOP
};

enum class SlurStyleType : signed char {
    Undefined = -1,
    Solid,
    Dotted,
    Dashed,
    WideDashed
};

struct InstrumentTrackId {
    muse::ID partId = 0;
    String instrumentId;

    bool operator ==(const InstrumentTrackId& other) const
    {
        return partId == other.partId && instrumentId == other.instrumentId;
    }

    bool operator !=(const InstrumentTrackId& other) const
    {
        return !operator ==(other);
    }

    bool operator <(const InstrumentTrackId& other) const noexcept
    {
        if (partId < other.partId) {
            return true;
        }

        return instrumentId < other.instrumentId;
    }

    bool isValid() const
    {
        return partId.isValid() && !instrumentId.empty();
    }
};

// Tremolo subtypes:
enum class TremoloType : signed char {
    INVALID_TREMOLO = -1,
    R8 = 0, R16, R32, R64, BUZZ_ROLL,    // one note tremolo (repeat)
    C8, C16, C32, C64       // two note tremolo (change)
};

inline bool isTremoloTwoChord(TremoloType type)
{
    return type >= TremoloType::C8;
}

// only applicable to minim two-note tremolo in non-TAB staves
enum class TremoloStyle : signed char {
    DEFAULT = 0, TRADITIONAL, TRADITIONAL_ALTERNATE
};

enum class TremoloChordType : unsigned char {
    TremoloNone, TremoloSingle, TremoloFirstChord, TremoloSecondChord
};

enum class BracketType : signed char {
    NORMAL, BRACE, SQUARE, LINE, NO_BRACKET = -1
};

using InstrumentTrackIdList = std::vector<InstrumentTrackId>;
using InstrumentTrackIdSet = std::unordered_set<InstrumentTrackId>;

enum EmbellishmentType : unsigned char {};

enum DrumNum : unsigned char {};

enum class GlissandoType : unsigned char {
    STRAIGHT, WAVY
};

enum class JumpType : unsigned char {
    DC,
    DC_AL_FINE,
    DC_AL_CODA,
    DS_AL_CODA,
    DS_AL_FINE,
    DS,
    DC_AL_DBLCODA,
    DS_AL_DBLCODA,
    DSS,
    DSS_AL_CODA,
    DSS_AL_DBLCODA,
    DSS_AL_FINE,
    USER
};

enum class MarkerType : unsigned char {
    SEGNO,
    VARSEGNO,
    CODA,
    VARCODA,
    CODETTA, // not in SMuFL, but still needed for 1.x compatibility, rendered as a double coda
    FINE,
    TOCODA,
    TOCODASYM,
    DA_CODA,
    DA_DBLCODA,
    USER
};

enum class StaffGroup : unsigned char {
    STANDARD, PERCUSSION, TAB
};
constexpr int STAFF_GROUP_MAX = int(StaffGroup::TAB) + 1; // out of enum to avoid compiler complains about not handled switch cases

enum class TrillType : unsigned char {
    TRILL_LINE, UPPRALL_LINE, DOWNPRALL_LINE, PRALLPRALL_LINE,
};

enum class VibratoType : unsigned char {
    GUITAR_VIBRATO, GUITAR_VIBRATO_WIDE, VIBRATO_SAWTOOTH, VIBRATO_SAWTOOTH_WIDE
};

enum class ArticulationTextType : unsigned char {
    NO_TEXT,
    TAP,
    SLAP,
    POP
};

enum class LyricsSyllabic : unsigned char {
    SINGLE, BEGIN, END, MIDDLE
};

enum class LyricsDashSystemStart : unsigned char {
    STANDARD,
    UNDER_HEADER,
    UNDER_FIRST_NOTE
};

enum class NoteLineEndPlacement : unsigned char {
    LEFT_EDGE,
    OFFSET_ENDS,
};

enum class SpannerSegmentType : unsigned char {
    SINGLE, BEGIN, MIDDLE, END
};

enum class TiePlacement : unsigned char {
    AUTO,
    INSIDE,
    OUTSIDE,
};

enum class TieDotsPlacement : unsigned char {
    AUTO,
    BEFORE_DOTS,
    AFTER_DOTS
};

enum class TimeSigPlacement : unsigned char {
    NORMAL,
    ABOVE_STAVES,
    ACROSS_STAVES
};

enum class TimeSigStyle : unsigned char {
    NORMAL,
    NARROW,
    LARGE
};

enum class TimeSigVSMargin : unsigned char {
    HANG_INTO_MARGIN,
    RIGHT_ALIGN_TO_BARLINE,
    CREATE_SPACE,
};

enum class NoteSpellingType : unsigned char {
    STANDARD = 0, GERMAN, GERMAN_PURE, SOLFEGGIO, FRENCH
};

//---------------------------------------------------------
//   Key
//---------------------------------------------------------

enum class Key : signed char {
    C_B = -7,
    G_B,
    D_B,
    A_B,
    E_B,
    B_B,
    F,
    C,      // == 0
    G,
    D,
    A,
    E,
    B,
    F_S,
    C_S,
    MIN     = Key::C_B,
    MAX     = Key::C_S,
    INVALID = Key::MIN - 1,
    NUM_OF  = Key::MAX - Key::MIN + 1,
    DELTA_ENHARMONIC = 12
};

static inline bool operator<(Key a, Key b) { return static_cast<int>(a) < static_cast<int>(b); }
static inline bool operator>(Key a, Key b) { return static_cast<int>(a) > static_cast<int>(b); }
static inline bool operator>(Key a, int b) { return static_cast<int>(a) > b; }
static inline bool operator<(Key a, int b) { return static_cast<int>(a) < b; }
static inline bool operator==(const Key a, const Key b) { return int(a) == int(b); }
static inline bool operator!=(const Key a, const Key b) { return static_cast<int>(a) != static_cast<int>(b); }
static inline Key operator+=(Key& a, const Key& b) { return a = Key(static_cast<int>(a) + static_cast<int>(b)); }
static inline Key operator-=(Key& a, const Key& b) { return a = Key(static_cast<int>(a) - static_cast<int>(b)); }

struct SwingParameters {
    int swingUnit = 0;
    int swingRatio = 0;

    bool isOn() const { return swingUnit != 0; }
};

struct CapoParams {
    bool active = false;
    int fretPosition = 0;
    std::unordered_set<string_idx_t> ignoredStrings;
};

struct PartAudioSettingsCompat {
    InstrumentTrackId instrumentId;
    bool mute = false;
    bool solo = false;
    int velocity = 127;
};

struct SettingsCompat {
    std::map<muse::ID /*partid*/, PartAudioSettingsCompat> audioSettings;
};

//---------------------------------------------------------
//   UpdateMode
//    There is an implied order from least invasive update
//    to most invasive update. LayoutAll is fallback and
//    recreates all.
//---------------------------------------------------------

enum class UpdateMode : unsigned char {
    DoNothing,
    Update,             // do screen refresh of RectF "refresh"
    UpdateAll,          // do complete screen refresh
    Layout,             // do partial layout for tick range
};

//---------------------------------------------------------
//   LayoutFlag bits
//---------------------------------------------------------

enum class LayoutFlag : unsigned char {
    NO_FLAGS       = 0,
    PLAY_EVENTS    = 2,
    REBUILD_MIDI_MAPPING = 4,
};

typedef muse::Flags<LayoutFlag> LayoutFlags;
} // mu::engraving

template<>
struct std::hash<mu::engraving::InstrumentTrackId>
{
    std::size_t operator()(const mu::engraving::InstrumentTrackId& s) const noexcept
    {
        std::size_t h1 = std::hash<int> {}(static_cast<int>(s.partId.toUint64()));
        std::size_t h2 = std::hash<muse::String> {}(s.instrumentId);
        return h1 ^ (h2 << 1);
    }
};

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::engraving::BeamMode)
Q_DECLARE_METATYPE(mu::engraving::JumpType)
Q_DECLARE_METATYPE(mu::engraving::MarkerType)
Q_DECLARE_METATYPE(mu::engraving::TrillType)
Q_DECLARE_METATYPE(mu::engraving::VibratoType)
#endif
