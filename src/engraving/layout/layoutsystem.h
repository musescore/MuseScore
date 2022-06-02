/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_ENGRAVING_LAYOUTSYSTEM_H
#define MU_ENGRAVING_LAYOUTSYSTEM_H

#include <vector>

#include "libmscore/segment.h"

#include "layoutoptions.h"
#include "layoutcontext.h"

namespace mu::engraving {
class Score;
class System;
class Spanner;
class Chord;
}

namespace mu::engraving {
class LayoutSystem
{
public:

    static mu::engraving::System* collectSystem(const LayoutOptions& options, LayoutContext& lc, mu::engraving::Score* score);
    static void layoutSystemElements(const LayoutOptions& options, LayoutContext& lc, mu::engraving::Score* score,
                                     mu::engraving::System* system);

private:

    static mu::engraving::System* getNextSystem(LayoutContext& lc);
    static void hideEmptyStaves(mu::engraving::Score* score, mu::engraving::System* system, bool isFirstSystem);
    static void processLines(mu::engraving::System* system, std::vector<mu::engraving::Spanner*> lines, bool align);
    static void layoutTies(mu::engraving::Chord* ch, mu::engraving::System* system, const mu::engraving::Fraction& stick);
    static void doLayoutTies(mu::engraving::System* system, std::vector<mu::engraving::Segment*> sl, const Fraction& stick,
                             const Fraction& etick);
};
}

#endif // MU_ENGRAVING_LAYOUTSYSTEM_H
