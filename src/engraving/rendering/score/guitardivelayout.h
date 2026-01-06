/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "layoutcontext.h"

namespace mu::engraving {
class GuitarBend;
class GuitarBendSegment;
class LineSegment;
}

namespace mu::engraving::rendering::score {
class GuitarDiveLayout
{
public:
    friend class GuitarBendLayout;

    static void updateDiveSequences(const std::vector<mu::engraving::GuitarBend*>& bends, const LayoutContext& ctx);
    static void layoutDiveTabStaff(GuitarBendSegment* item, LayoutContext&);

private:
    static PointF computeStartPosOnStaff(GuitarBendSegment* item, LayoutContext& ctx);
    static PointF computeEndPosOnStaff(GuitarBendSegment* item, LayoutContext& ctx);

    static PointF computeStartPosAboveStaff(GuitarBendSegment* item, LayoutContext& ctx);
    static PointF computeEndPosAboveStaff(GuitarBendSegment* item, LayoutContext&);

    static LineSegment* findPrevHoldOrBendSegment(GuitarBendSegment* item, bool excludeFullReleaseDive = false);

    static void layoutDip(GuitarBendSegment* item, LayoutContext& ctx);
    static void layoutScoop(GuitarBendSegment* item);
};
}
