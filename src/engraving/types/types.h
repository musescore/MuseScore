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

// P_TYPE::DIRECTION
enum class DirectionV {
    AUTO, UP, DOWN
};

// P_TYPE::DIRECTION_H
enum class DirectionH : char {
    AUTO, LEFT, RIGHT
};

enum class LayoutBreakType {
    PAGE, LINE, SECTION, NOBREAK
};

enum class VeloType : char {
    OFFSET_VAL, USER_VAL
};

enum class BeamMode : signed char {
    AUTO, BEGIN, MID, END, NONE, BEGIN32, BEGIN64, INVALID = -1
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
}

#endif // MU_ENGRAVING_TYPES_H
