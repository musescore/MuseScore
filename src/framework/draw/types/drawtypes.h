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
#ifndef MUSE_DRAW_DRAWTYPES_H
#define MUSE_DRAW_DRAWTYPES_H

#include "global/types/flags.h"

namespace muse::draw {
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

enum class FillRule {
    OddEvenFill,
    WindingFill
};

enum AlignmentFlag {
    AlignLeft = 0x0001,
    AlignLeading = AlignLeft,
    AlignRight = 0x0002,
    AlignTrailing = AlignRight,
    AlignHCenter = 0x0004,
    AlignJustify = 0x0008,
    AlignAbsolute = 0x0010,
    AlignHorizontal_Mask = AlignLeft | AlignRight | AlignHCenter | AlignJustify | AlignAbsolute,

    AlignTop = 0x0020,
    AlignBottom = 0x0040,
    AlignVCenter = 0x0080,
    AlignBaseline = 0x0100,
    // Note that 0x100 will clash with Qt::TextSingleLine = 0x100 due to what the comment above
    // this enum declaration states. However, since Qt::AlignBaseline is only used by layouts,
    // it doesn't make sense to pass Qt::AlignBaseline to QPainter::drawText(), so there
    // shouldn't really be any ambiguity between the two overlapping enum values.
    AlignVertical_Mask = AlignTop | AlignBottom | AlignVCenter | AlignBaseline,

    AlignCenter = AlignVCenter | AlignHCenter
};

DECLARE_FLAGS(Alignment, AlignmentFlag)
DECLARE_OPERATORS_FOR_FLAGS(Alignment)

enum TextFlag {
    TextSingleLine = 0x0100,
    TextDontClip = 0x0200,
    TextExpandTabs = 0x0400,
    TextShowMnemonic = 0x0800,
    TextWordWrap = 0x1000,
    TextWrapAnywhere = 0x2000,
    TextDontPrint = 0x4000,
    TextIncludeTrailingSpaces = 0x08000000,
    TextHideMnemonic = 0x8000,
    TextJustificationForced = 0x10000,
    TextForceLeftToRight = 0x20000,
    TextForceRightToLeft = 0x40000,
    // Ensures that the longest variant is always used when computing the
    // size of a multi-variant string.
    TextLongestVariant = 0x80000
};
}

#endif // MUSE_DRAW_DRAWTYPES_H
