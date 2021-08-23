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

#include "layoutoptions.h"
#include "layoutcontext.h"

namespace Ms {
class Score;
class System;
class Spanner;
class Chord;
class Fraction;
}

namespace mu::engraving {
class LayoutSystem
{
public:

    static Ms::System* collectSystem(const LayoutOptions& options, LayoutContext& lc, Ms::Score* score);
    static void layoutSystemElements(const LayoutOptions& options, LayoutContext& lc, Ms::Score* score, Ms::System* system);

private:

    static Ms::System* getNextSystem(LayoutContext& lc, Ms::Score* score);
    static void hideEmptyStaves(Ms::Score* score, Ms::System* system, bool isFirstSystem);
    static void processLines(Ms::System* system, std::vector<Ms::Spanner*> lines, bool align);
    static void layoutTies(Ms::Chord* ch, Ms::System* system, const Ms::Fraction& stick);
};
}

#endif // MU_ENGRAVING_LAYOUTSYSTEM_H
