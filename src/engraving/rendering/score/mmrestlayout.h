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

#include "layoutcontext.h"

namespace mu::engraving {
class Measure;
class Segment;
}

namespace mu::engraving::rendering::score {
class MMRestLayout
{
public:

    static void createMultiMeasureRestsIfNeed(Measure* firstMeasure, LayoutContext& ctx);
    static void layoutMMRestRange(Measure* m, LayoutContext& ctx);

private:
    static void createMMRest(LayoutContext& ctx, Measure* firstMeasure, Measure* lastMeasure, const Fraction& len);

    static void reuseExistingMMRest(LayoutContext& ctx, Measure* mmrMeasure, Measure* lastMeasure, Fraction len);
    static void removeMMRestElements(Measure* mmRestMeasure, LayoutContext& ctx);
    static void changeMeasureElParents(Measure* firstMeasure, Measure* lastMeasure, Measure* mmrMeasure, LayoutContext& ctx);
    static void restoreMeasureElParents(Measure* firstMeasure, Measure* lastMeasure, Measure* mmrMeasure, LayoutContext& ctx);
    static void changeAnnotationsParent(Segment* oldParent, Segment* newParent, const LayoutContext& ctx, bool checkVisibility);
    static Segment* changeElementsParent(Segment* oldSeg, Measure* newMeasure, const Fraction& newSegTick, LayoutContext& ctx,
                                         bool checkVisibility);

    static bool validMMRestMeasure(const LayoutContext& ctx, const Measure* m);
    static bool breakMMRForElement(Measure* measure, Measure* prevMeasure, const LayoutContext& ctx);
    static bool breakMultiMeasureRest(const LayoutContext& ctx, Measure* m);
};
}
