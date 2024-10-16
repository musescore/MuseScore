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
#ifndef MU_ENGRAVING_SYSTEMLAYOUT_DEV_H
#define MU_ENGRAVING_SYSTEMLAYOUT_DEV_H

#include <vector>

#include "../layoutoptions.h"
#include "layoutcontext.h"

namespace mu::engraving {
class Chord;
class Score;
class Segment;
class Spanner;
class System;
class Measure;
class Bracket;
class BracketItem;
class SkylineLine;
}

namespace mu::engraving::rendering::score {
class SystemLayout
{
public:
    static System* collectSystem(LayoutContext& ctx);
    static void layoutSystemElements(System* system, LayoutContext& ctx);

    static void layoutSystem(System* system, LayoutContext& ctx, double xo1, bool isFirstSystem = false, bool firstSystemIndent = false);

    static void hideEmptyStaves(System* system, LayoutContext& ctx, bool isFirstSystem);

    static void layout2(System* system, LayoutContext& ctx);
    static void restoreLayout2(System* system, LayoutContext& ctx);
    static void setMeasureHeight(System* system, double height, const LayoutContext& ctx);
    static void layoutBracketsVertical(System* system, LayoutContext& ctx);
    static void layoutInstrumentNames(System* system, LayoutContext& ctx);

    static void setInstrumentNames(System* system, LayoutContext& ctx, bool longName, Fraction tick = { 0, 1 });

    static double minDistance(const System* top, const System* bottom, const LayoutContext& ctx);

    static void centerElementsBetweenStaves(const System* system);

    static void updateSkylineForElement(EngravingItem* element, const System* system, double yMove);

private:
    static System* getNextSystem(LayoutContext& lc);
    static void processLines(System* system, LayoutContext& ctx, std::vector<Spanner*> lines, bool align);
    static void layoutTies(Chord* ch, System* system, const Fraction& stick, LayoutContext& ctx);
    static void doLayoutTies(System* system, std::vector<Segment*> sl, const Fraction& stick, const Fraction& etick, LayoutContext& ctx);
    static void doLayoutNoteSpannersLinear(System* system, LayoutContext& ctx);
    static void layoutNoteAnchoredSpanners(System* system, Chord* chord);
    static void layoutGuitarBends(const std::vector<Segment*>& sl, LayoutContext& ctx);
    static void justifySystem(System* system, double curSysWidth, double targetSystemWidth);
    static void updateCrossBeams(System* system, LayoutContext& ctx);
    static void restoreTiesAndBends(System* system, LayoutContext& ctx);
    static void manageNarrowSpacing(System* system, LayoutContext& ctx, double& curSysWidth, double targetSysWidth, const Fraction minTicks,
                                    const Fraction maxTicks);

    static double instrumentNamesWidth(System* system, LayoutContext& ctx, bool isFirstSystem);
    static double totalBracketOffset(LayoutContext& ctx);
    static double layoutBrackets(System* system, LayoutContext& ctx);
    static void addBrackets(System* system, Measure* measure, LayoutContext& ctx);
    static Bracket* createBracket(System* system, LayoutContext& ctx, BracketItem* bi, size_t column, staff_idx_t staffIdx,
                                  std::vector<Bracket*>& bl, Measure* measure);
    static double minVertSpaceForCrossStaffBeams(System* system, staff_idx_t staffIdx1, staff_idx_t staffIdx2, LayoutContext& ctx);

    static bool elementShouldBeCenteredBetweenStaves(const EngravingItem* item, const System* system);
    static bool elementHasAnotherStackedOutside(const EngravingItem* element, const Shape& elementShape, const SkylineLine& skylineLine);
    static void centerElementBetweenStaves(EngravingItem* element, const System* system);
};
}

#endif // MU_ENGRAVING_SYSTEMLAYOUT_DEV_H
