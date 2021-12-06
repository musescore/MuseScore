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

#include <QString>
#include "infrastructure/draw/color.h"
#include "infrastructure/draw/geometry.h"
#include "infrastructure/draw/painterpath.h"
#include "dimension.h"
#include "fraction.h"

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

//---------------------------------------------------------
///   Align
///   Because the Align enum has Top = 0 and Left = 0,
///   align() & Align::Top will always return false.
///   @warning Do not use if (align() & Align::Top) { doSomething(); }
///   because doSomething() will never be executed!
///   use this instead:
///   `if ((static_cast<char>(align()) & static_cast<char>(Align::VMASK)) == Align::Top) { doSomething(); }`
///   Same applies to Align::Left.
//---------------------------------------------------------
// P_TYPE::ALIGN
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
    HMASK  = Align::LEFT | Align::RIGHT | Align::HCENTER,
    VMASK  = Align::TOP | Align::BOTTOM | Align::VCENTER | Align::BASELINE
             ///.\}
};

constexpr Align operator|(Align a1, Align a2)
{
    return static_cast<Align>(static_cast<char>(a1) | static_cast<char>(a2));
}

constexpr bool operator&(Align a1, Align a2)
{
    return static_cast<char>(a1) & static_cast<char>(a2);
}

constexpr Align operator~(Align a)
{
    return static_cast<Align>(~static_cast<char>(a));
}

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

// P_TYPE::BEAM_MODE
enum class BeamMode : signed char {
    AUTO, BEGIN, MID, END, NONE, BEGIN32, BEGIN64, INVALID = -1
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

// P_TYPE::HEAD_TYPE
enum class NoteHeadType : signed char {
    HEAD_AUTO    = -1,
    HEAD_WHOLE   = 0,
    HEAD_HALF    = 1,
    HEAD_QUARTER = 2,
    HEAD_BREVIS  = 3,
    HEAD_TYPES
};
} // mu::engraving

//! NOTE compat
namespace Ms {
using OrnamentStyle = mu::engraving::OrnamentStyle;
using Align = mu::engraving::Align;
using PlacementV = mu::engraving::PlacementV;
using PlacementH = mu::engraving::PlacementH;
using DirectionV = mu::engraving::DirectionV;
using DirectionH = mu::engraving::DirectionH;
using LayoutBreakType = mu::engraving::LayoutBreakType;
using VeloType = mu::engraving::VeloType;
using BeamMode = mu::engraving::BeamMode;
using TextPlace = mu::engraving::TextPlace;
using GlissandoStyle = mu::engraving::GlissandoStyle;
using BarLineType = mu::engraving::BarLineType;
using NoteHeadType = mu::engraving::NoteHeadType;
}

#endif // MU_ENGRAVING_TYPES_H
