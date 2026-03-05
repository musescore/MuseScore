/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "draw/types/geometry.h"

namespace mu::engraving::rendering::score {
struct ArcRange {
    double startAngleRadians = 0.0;
    double endAngleRadians = 0.0;

    double spanRadians() const { return endAngleRadians - startAngleRadians; }
};

// Compute visible arc ranges for an ellipse with cutouts from adjacent ellipses.
// Returns ranges in radians where the main ellipse is visible (not occluded).
// Angles are measured counter-clockwise from 3 o'clock position (standard math convention).
std::vector<ArcRange> computeVisibleArcRanges(
    const muse::RectF& mainRect, const std::vector<muse::RectF>& adjacentRects, double lineWidth);
}
