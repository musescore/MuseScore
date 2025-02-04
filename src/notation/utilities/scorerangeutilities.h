/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "engraving/types/types.h"

namespace mu::engraving {
class Score;
class Segment;
class System;
}

namespace mu::notation {
class ScoreRangeUtilities
{
public:
    static std::vector<muse::RectF> boundingArea(
        const engraving::Score* score, const engraving::Segment* startSegment, const engraving::Segment* endSegment,
        engraving::staff_idx_t startStaffIndex, engraving::staff_idx_t endStaffIndex);

private:
    struct RangeSection {
        const engraving::System* system = nullptr;
        const engraving::Segment* startSegment = nullptr;
        const engraving::Segment* endSegment = nullptr;
    };

    static std::vector<RangeSection> splitRangeBySections(
        const engraving::Segment* rangeStartSegment, const engraving::Segment* rangeEndSegment);

    static engraving::staff_idx_t firstVisibleStaffIdx(
        const engraving::Score* score, const engraving::System* system, engraving::staff_idx_t startStaffIndex);
    static engraving::staff_idx_t lastVisibleStaffIdx(
        const engraving::Score* score, const engraving::System* system, engraving::staff_idx_t endStaffIndex);
};
}
