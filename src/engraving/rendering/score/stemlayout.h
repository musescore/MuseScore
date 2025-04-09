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
class Chord;
class Stem;
}

namespace mu::engraving::rendering::score {
class StemLayout
{
public:
    static double calcDefaultStemLength(Chord* item, const LayoutContext& ctx);
    static int minStaffOverlap(bool up, int staffLines, int beamCount, bool hasHook, double beamSpacing, bool useWideBeams,
                               bool isFullSize);

private:
    static int stemLengthBeamAddition(const Chord* item, const LayoutContext& ctx);
    static int maxReduction(const Chord* item, const LayoutContext& ctx, int extensionOutsideStaff);
    static int stemOpticalAdjustment(const Chord* item, int stemEndPosition);
    static int calcMinStemLength(Chord* item, const LayoutContext& ctx);
    static int calc4BeamsException(const Chord* item, int stemLength);

    static inline int calcBeamCount(const Chord* item);
};
} // namespace mu::engraving::rendering::score
