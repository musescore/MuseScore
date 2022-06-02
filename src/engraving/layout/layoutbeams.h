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
#ifndef MU_ENGRAVING_LAYOUTBEAMS_H
#define MU_ENGRAVING_LAYOUTBEAMS_H

#include <vector>

namespace mu::engraving {
class Score;
class Measure;
class Chord;
class ChordRest;
}

namespace mu::engraving {
class LayoutContext;
class LayoutBeams
{
public:

    static bool isTopBeam(mu::engraving::ChordRest* cr);
    static bool notTopBeam(mu::engraving::ChordRest* cr);
    static void createBeams(mu::engraving::Score* score, LayoutContext& lc, mu::engraving::Measure* measure);
    static void restoreBeams(mu::engraving::Measure* m);
    static void breakCrossMeasureBeams(const LayoutContext& ctx, mu::engraving::Measure* measure);
    static void respace(const std::vector<mu::engraving::ChordRest*>& elements);

private:
    static void beamGraceNotes(mu::engraving::Score* score, mu::engraving::Chord* mainNote, bool after);
};
}

#endif // MU_ENGRAVING_LAYOUTBEAMS_H
