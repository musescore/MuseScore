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

#include <vector>

#include "layoutcontext.h"

namespace mu::engraving {
class Segment;
class System;
}

namespace mu::engraving::rendering::score {
// Help class.
// Contains harmonies/fretboard per segment.
class HarmonyList : public std::vector<EngravingItem*>
{
    // muse::OBJECT_ALLOCATOR(mu::engraving, HarmonyList);

    std::map<const Segment*, std::vector<EngravingItem*> > elements;
    std::vector<EngravingItem*> modified;

    EngravingItem* getReferenceElement(const Segment* s, bool above, bool visible) const;

public:
    HarmonyList()
    {
        elements.clear();
        modified.clear();
    }

    void append(const Segment* s, EngravingItem* e) { elements[s].push_back(e); }

    double getReferenceHeight(bool above) const;

    bool align(bool above, double reference, double maxShift);

    void addToSkyline(const System* system);
};

class HarmonyLayout
{
public:

    static void layoutHarmonies(const std::vector<Segment*>& sl, LayoutContext& ctx);
    static void alignHarmonies(const System* system, const std::vector<Segment*>& sl, bool harmony, const double maxShiftAbove,
                               const double maxShiftBelow);
};
}
