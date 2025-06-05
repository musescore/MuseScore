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
#ifndef MU_ENGRAVING_SCOREHORIZONTALVIEWLAYOUT_DEV_H
#define MU_ENGRAVING_SCOREHORIZONTALVIEWLAYOUT_DEV_H

#include "layoutcontext.h"

namespace mu::engraving::rendering::score {
class ScoreHorizontalViewLayout
{
public:

    static void layoutHorizontalView(Score* score, LayoutContext& ctx, const Fraction& stick, const Fraction& etick);

private:
    static void layoutLinear(LayoutContext& ctx, bool layoutAll);
    static void layoutLinear(LayoutContext& ctx);
    static void resetSystems(LayoutContext& ctx, bool layoutAll);
    static void collectLinearSystem(LayoutContext& ctx);
    static void layoutSystemLockIndicators(System* system);

    //! puts segments on the positions according to their length
    static void layoutSegmentsWithDuration(Measure* m, const std::vector<int>& visibleParts);

    /*! \brief callulate width of segment and additional spacing of segment depends on duration of segment
     *  \return pair of {spacing, width}
     */
    static std::pair<double, double> computeCellWidth(const Segment* s, const std::vector<int>& visibleParts);

    /*! \brief get among all ChordRests of segment the ChordRest with minimum ticks,
    * take into account visibleParts
    */
    static ChordRest* chordRestWithMinDuration(const Segment* seg, const std::vector<int>& visibleParts);

    static Fraction calculateQuantumCell(const Measure* m, const std::vector<int>& visibleParts);
};
}

#endif // MU_ENGRAVING_SCOREHORIZONTALVIEWLAYOUT_DEV_H
