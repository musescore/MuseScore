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

#include "layoutoptions.h"
#include "layoutcontext.h"

namespace mu::engraving {
class Chord;
class Score;
class Segment;
class Spanner;
class System;

class LayoutSystem
{
public:
    static System* collectSystem(const LayoutOptions& options, LayoutContext& lc, Score* score);
    static void layoutSystemElements(const LayoutOptions& options, LayoutContext& lc, Score* score, System* system);

private:
    static System* getNextSystem(LayoutContext& lc);
    static void hideEmptyStaves(Score* score, System* system, bool isFirstSystem);
    static void processLines(System* system, std::vector<Spanner*> lines, bool align);
    static void layoutTies(Chord* ch, System* system, const Fraction& stick);
    static void doLayoutTies(System* system, std::vector<Segment*> sl, const Fraction& stick, const Fraction& etick);
    static void justifySystem(System* system, double curSysWidth, double targetSystemWidth);
};
}

#endif // MU_ENGRAVING_LAYOUTSYSTEM_H
