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
#ifndef MU_DRAW_DRAWTYPES_H
#define MU_DRAW_DRAWTYPES_H

#include <QVariant>
#include "geometry.h"

namespace mu::draw {
enum class CompositionMode {
    SourceOver,
    HardLight
};

enum class PolygonMode {
    OddEven,
    Winding,
    Convex,
    Polyline
};

enum class PenStyle { // pen style
    NoPen,
    SolidLine,
    DashLine,
    DotLine,
    DashDotLine,
    DashDotDotLine,
    CustomDashLine
};

enum class PenCapStyle { // line endcap style
    FlatCap = 0x00,
    SquareCap = 0x10,
    RoundCap = 0x20
};

enum class PenJoinStyle { // line join style
    MiterJoin = 0x00,
    BevelJoin = 0x40,
    RoundJoin = 0x80
};

enum class BrushStyle { // brush style
    NoBrush,
    SolidPattern,
    Dense1Pattern,
    Dense2Pattern,
    Dense3Pattern,
    Dense4Pattern,
    Dense5Pattern,
    Dense6Pattern,
    Dense7Pattern,
    HorPattern,
    VerPattern,
    CrossPattern,
    BDiagPattern,
    FDiagPattern,
    DiagCrossPattern,
    LinearGradientPattern,
    RadialGradientPattern,
    ConicalGradientPattern,
    TexturePattern = 24
};
}

#endif // MU_DRAW_DRAWTYPES_H
