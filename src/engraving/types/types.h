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

#ifndef MU_ENGRAVING_TYPES_H
#define MU_ENGRAVING_TYPES_H

namespace mu::engraving {
// ========================================
// PropertyValue
// ========================================

// --- Base ---
// bool (std)                           // P_TYPE::BOOL
// int  (std)                           // P_TYPE::INT
// qreal (Qt)                           // P_TYPE::REAL
// QString (Qt)                         // P_TYPE::STRING

// --- Geometry ---
using PointF = mu::PointF;              // P_TYPE::POINT
using SizeF = mu::SizeF;                // P_TYPE::SIZE
using PainterPath = mu::PainterPath;    // P_TYPE::PATH
using ScaleF = mu::ScaleF;              // P_TYPE::SCALE
// Spatium (dimension.h)                // P_TYPE::SPATIUM
// Milimetre (dimension.h)              // P_TYPE::MILIMETRE
using PairF = mu::PairF;                // P_TYPE::PAIR_REAL

// --- Draw ---
using Color = draw::Color;              // P_TYPE::COLOR

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
} // mu::engraving

//! NOTE compat
namespace Ms {
using Align = mu::engraving::Align;
using PlacementV = mu::engraving::PlacementV;
using PlacementH = mu::engraving::PlacementH;
}

#endif // MU_ENGRAVING_TYPES_H
