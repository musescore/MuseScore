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

#include <utility>
#include <functional>
#include <unordered_set>
#include <QString>

#include "id.h"

#include "infrastructure/draw/color.h"
#include "infrastructure/draw/geometry.h"
#include "infrastructure/draw/painterpath.h"
#include "dimension.h"
#include "fraction.h"
#include "symid.h"
#include "pitchvalue.h"
#include "bps.h"
#include "groupnode.h"

#ifndef MU_ENGRAVING_TYPES_H
#define MU_ENGRAVING_TYPES_H

namespace mu::engraving {
// ========================================
// PropertyValue
// ========================================

// --- Geometry ---
using PointF = mu::PointF;              // P_TYPE::POINT
using SizeF = mu::SizeF;                // P_TYPE::SIZE
using PainterPath = mu::PainterPath;    // P_TYPE::PATH
using ScaleF = mu::ScaleF;              // P_TYPE::SCALE
using PairF = mu::PairF;                // P_TYPE::PAIR_REAL

// --- Draw ---
using Color = draw::Color;              // P_TYPE::COLOR

enum class OrnamentStyle : char {
    DEFAULT, BAROQUE
};

// P_TYPE::GLISS_STYLE
enum class GlissandoStyle {
    CHROMATIC, WHITE_KEYS, BLACK_KEYS, DIATONIC, PORTAMENTO
};

// --- Layout ---

// P_TYPE::ALIGN
enum class AlignV {
    TOP,
    VCENTER,
    BOTTOM,
    BASELINE
};

enum class AlignH {
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
enum class PlacementV {
    ABOVE, BELOW
};

// P_TYPE::PLACEMENT_H
enum class PlacementH {
    LEFT, CENTER, RIGHT
};

// P_TYPE::TEXT_PLACE
enum class TextPlace : char {
    AUTO, ABOVE, BELOW, LEFT
};

// P_TYPE::DIRECTION
enum class DirectionV {
    AUTO, UP, DOWN
};

// P_TYPE::DIRECTION_H
enum class DirectionH : char {
    AUTO, LEFT, RIGHT
};

// P_TYPE::ORIENTATION
enum class Orientation : signed char {
    VERTICAL,
    HORIZONTAL
};

// P_TYPE::BEAM_MODE
enum class BeamMode : signed char {
    AUTO, BEGIN, MID, END, NONE, BEGIN32, BEGIN64, INVALID = -1
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
enum class LayoutBreakType {
    PAGE, LINE, SECTION, NOBREAK
};

// P_TYPE::VELO_TYPE
enum class VeloType : char {
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

    HEAD_SOL,
    HEAD_LA,
    HEAD_FA,
    HEAD_MI,
    HEAD_DO,
    HEAD_RE,
    HEAD_TI,
    // not exposed from here
    HEAD_DO_WALKER,
    HEAD_RE_WALKER,
    HEAD_TI_WALKER,
    HEAD_DO_FUNK,
    HEAD_RE_FUNK,
    HEAD_TI_FUNK,

    HEAD_DO_NAME,
    HEAD_RE_NAME,
    HEAD_MI_NAME,
    HEAD_FA_NAME,
    HEAD_SOL_NAME,
    HEAD_LA_NAME,
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

    HEAD_CUSTOM,
    HEAD_GROUPS,
    HEAD_INVALID = -1
};

// P_TYPE::CLEF_TYPE
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
    C3_F20C,
    C1_F20C,
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
    MAX
};

// P_TYPE::DYNAMIC_TYPE
enum class DynamicType : char {
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

// P_TYPE::DYNAMIC_RANGE
enum class DynamicRange : char {
    STAFF, PART, SYSTEM
};

// P_TYPE::DYNAMIC_SPEED
enum class DynamicSpeed : char {
    SLOW, NORMAL, FAST
};

// P_TYPE::HOOK_TYPE
enum class HookType : char {
    NONE, HOOK_90, HOOK_45, HOOK_90T
};

// P_TYPE::KEY_MODE
enum class KeyMode : char {
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

//-------------------------------------------------------------------
//   Tid
///   Enumerates the list of built-in text substyles
///   \internal
///   Must be in sync with textStyles (in textstyle.cpp)
//-------------------------------------------------------------------
// P_TYPE::TEXT_STYLE
enum class TextStyleType {
    DEFAULT,

    // Page-orientde styles
    TITLE,
    SUBTITLE,
    COMPOSER,
    POET,
    TRANSLATOR,
    FRAME,
    INSTRUMENT_EXCERPT,
    INSTRUMENT_LONG,
    INSTRUMENT_SHORT,
    INSTRUMENT_CHANGE,
    HEADER,
    FOOTER,

    // Measure-oriented styles
    MEASURE_NUMBER,
    MMREST_RANGE,

    // Sytem-level styles
    TEMPO,
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
    STRING_NUMBER,

    // Line-oriented styles
    TEXTLINE,
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

enum class AnnotationCategory {
    Undefined = -1,
    TempoAnnotation,
    PlayingAnnotation,
    Other,
};

enum class PlayingTechniqueType {
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
    Overdrive
};

enum class TempoTechniqueType {
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
enum class AccidentalRole : char {
    AUTO,                 // layout created accidental
    USER                  // user created accidental
};

enum class FermataType {
    Undefined = -1,
    VeryShort,
    Short,
    ShortHenze,
    Normal,
    Long,
    LongHenze,
    VeryLong
};

enum class ChordLineType : char {
    NOTYPE, FALL, DOIT,
    PLOP, SCOOP
};

struct InstrumentTrackId {
    ID partId = 0;
    std::string instrumentId;

    bool operator ==(const InstrumentTrackId& other) const
    {
        return partId == other.partId && instrumentId == other.instrumentId;
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

using InstrumentTrackIdSet = std::unordered_set<InstrumentTrackId>;
} // mu::engraving

template<>
struct std::hash<mu::engraving::InstrumentTrackId>
{
    std::size_t operator()(const mu::engraving::InstrumentTrackId& s) const noexcept
    {
        std::size_t h1 = std::hash<int> {}(s.partId.toUint64());
        std::size_t h2 = std::hash<std::string> {}(s.instrumentId);
        return h1 ^ (h2 << 1);
    }
};

//! NOTE compat
namespace Ms {
using OrnamentStyle = mu::engraving::OrnamentStyle;
using AlignV = mu::engraving::AlignV;
using AlignH = mu::engraving::AlignH;
using Align = mu::engraving::Align;
using PlacementV = mu::engraving::PlacementV;
using PlacementH = mu::engraving::PlacementH;
using DirectionV = mu::engraving::DirectionV;
using DirectionH = mu::engraving::DirectionH;
using Orientation = mu::engraving::Orientation;
using LayoutBreakType = mu::engraving::LayoutBreakType;
using VeloType = mu::engraving::VeloType;
using BeamMode = mu::engraving::BeamMode;
using TextPlace = mu::engraving::TextPlace;
using GlissandoStyle = mu::engraving::GlissandoStyle;
using BarLineType = mu::engraving::BarLineType;
using NoteHeadType = mu::engraving::NoteHeadType;
using NoteHeadScheme = mu::engraving::NoteHeadScheme;
using NoteHeadGroup = mu::engraving::NoteHeadGroup;
using ClefType = mu::engraving::ClefType;
using DynamicType = mu::engraving::DynamicType;
using DynamicRange = mu::engraving::DynamicRange;
using DynamicSpeed = mu::engraving::DynamicSpeed;
using HookType = mu::engraving::HookType;
using KeyMode = mu::engraving::KeyMode;
using TextStyleType = mu::engraving::TextStyleType;
using ChangeMethod = mu::engraving::ChangeMethod;
using ChangeDirection = mu::engraving::ChangeDirection;
using AccidentalRole = mu::engraving::AccidentalRole;
using DurationType = mu::engraving::DurationType;
using DurationTypeWithDots = mu::engraving::DurationTypeWithDots;
using PlayingTechniqueType = mu::engraving::PlayingTechniqueType;
using TempoChangeType = mu::engraving::TempoTechniqueType;
using InstrumentTrackId = mu::engraving::InstrumentTrackId;
using InstrumentTrackIdSet = mu::engraving::InstrumentTrackIdSet;
using FermataType = mu::engraving::FermataType;
using ChordLineType = mu::engraving::ChordLineType;
}

#endif // MU_ENGRAVING_TYPES_H
