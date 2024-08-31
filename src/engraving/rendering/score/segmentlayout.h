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
#ifndef MU_ENGRAVING_SEGMENTLAYOUT_DEV_H
#define MU_ENGRAVING_SEGMENTLAYOUT_DEV_H

#include "dom/segment.h"
#include "layoutcontext.h"

namespace mu::engraving::rendering::score {
class SegmentLayout
{
public:

    static void layoutMeasureIndependentElements(const Segment& segment, track_idx_t track, const LayoutContext& ctx);
    static void setChordMag(const Staff* staff, const Segment& segment, track_idx_t startTrack, track_idx_t endTrack,
                            const LayoutConfiguration& conf);
    static void checkStaffMoveValidity(const Segment& segment, track_idx_t startTrack, track_idx_t endTrack);
    static void layoutChordDrumset(const Staff* staff, const Segment& segment, track_idx_t startTrack, track_idx_t endTrack,
                                   const LayoutConfiguration& conf);

    static void computeChordsUp(const Segment& segment, track_idx_t startTrack, track_idx_t endTrack, const LayoutContext& ctx);

    static void layoutChordsStem(const Segment& segment, track_idx_t startTrack, track_idx_t endTrack, const LayoutContext& ctx);
};
}

#endif // MU_ENGRAVING_SEGMENTLAYOUT_DEV_H
