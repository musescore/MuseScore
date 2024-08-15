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

#ifndef MU_ENGRAVING_GUITARBENDLAYOUT_DEV_H
#define MU_ENGRAVING_GUITARBENDLAYOUT_DEV_H

#include "layoutcontext.h"

namespace mu::engraving {
class GuitarBend;
class GuitarBendSegment;
class GuitarBendHoldSegment;
class Note;
class SLine;
}

namespace mu::engraving::rendering::score {
class GuitarBendLayout
{
public:
    static void updateSegmentsAndLayout(SLine* item, LayoutContext& ctx);

    static void layoutStandardStaff(GuitarBendSegment* item, LayoutContext& ctx);
    static void layoutTabStaff(GuitarBendSegment* item, LayoutContext&);
    static void layoutHoldLine(GuitarBendHoldSegment* item);

private:
    static void layoutAngularBend(GuitarBendSegment* item, LayoutContext& ctx);
    static void computeUp(GuitarBend* item);
    static void computeIsInside(GuitarBend* item);
    static void avoidBadStaffLineIntersection(GuitarBendSegment* item, PointF& point);
    static void adjustX(GuitarBendSegment* item, PointF& startPos, PointF& endPos, const Note* startNote, const Note* endNote);
    static void layoutSlightBend(GuitarBendSegment* item, LayoutContext&);

    static PointF computeStartPos(GuitarBendSegment* item, Note* startNote, double distAboveTab, double verticalPad, double arrowHeight);
    static PointF computeEndPos(GuitarBendSegment* item, Note* endNote, double distAboveTab, double verticalPad, double arrowHeight,
                                double arrowWidth, const PointF& startPos, const PointF& prevEndPoint);
    static void checkConflictWithOtherBends(GuitarBendSegment* item);
};
}

#endif // MU_ENGRAVING_GUITARBENDLAYOUT_DEV_H
