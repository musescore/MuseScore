/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <vector>

#include "layoutcontext.h"

namespace mu::engraving {
class ChordBracket;
class Segment;
class Shape;
}

namespace mu::engraving::rendering::score {
class ChordBracketLayout
{
public:
    static void layoutSegment(Segment* segment, LayoutContext& ctx);
    static void updateHorizontalSpacing(const Segment* firstSegment, const Segment* secondSegment, staff_idx_t staffIdx,
                                        double squeezeFactor, double& minDistance);
    static void updateVerticalGeometry(ChordBracket* bracket, LayoutContext& ctx);

private:
    struct BracketVisualSpan {
        staff_idx_t topStaff = 0;
        staff_idx_t bottomStaff = 0;

        // Staff-local coordinates.
        double topY = 0.0;
        double bottomY = 0.0;
    };

    static std::vector<ChordBracket*> chordBracketsInSegment(const Segment* segment);
    static BracketVisualSpan bracketVisualSpan(const ChordBracket* bracket);
    static Shape bracketCollisionShape(const ChordBracket* bracket, const BracketVisualSpan& span, staff_idx_t staffIdx, double x);
    static void layoutHorizontal(ChordBracket* bracket);
};
} // namespace mu::engraving::rendering::score
