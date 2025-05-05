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

#include "dom/mmrest.h"
#include "dom/rest.h"

namespace mu::engraving {
class MMRest;
}

namespace mu::engraving::rendering::score {
class RestLayout
{
public:
    static void layoutRest(const Rest* item, Rest::LayoutData* ldata, const LayoutContext& ctx);
    static void fillShape(const Rest* item, Rest::LayoutData* ldata, const LayoutConfiguration& conf);

    static void resolveVerticalRestConflicts(LayoutContext& ctx, Segment* segment, staff_idx_t staffIdx);
    static void resolveRestVSChord(std::vector<Rest*>& rests, std::vector<Chord*>& chords, const Staff* staff, Segment* segment);
    static void resolveRestVSRest(std::vector<Rest*>& rests, const Staff* staff, Segment* segment, LayoutContext& ctx,
                                  bool considerBeams = false);

    static void alignRests(const System* system, LayoutContext& ctx);
    static void checkFullMeasureRestCollisions(const System* system, LayoutContext& ctx);

private:
    static void fillShape(const Rest* item, Rest::LayoutData* ldata);
    static void fillShape(const MMRest* item, MMRest::LayoutData* ldata, const LayoutConfiguration& conf);

    static int computeNaturalLine(int lines);
    static int computeVoiceOffset(const Rest* item, Rest::LayoutData* ldata); // Vertical displacement in multi-voice cases
    static int computeWholeOrBreveRestOffset(const Rest* item, int voiceOffset, int lines);

    static void updateSymbol(const Rest* item, Rest::LayoutData* ldata);
};
}
