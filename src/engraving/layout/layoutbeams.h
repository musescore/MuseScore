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

namespace Ms {
class Score;
class Measure;
class Chord;
class ChordRest;
}

namespace mu::engraving {
struct LayoutContext;
class LayoutBeams
{
public:

    static bool isTopBeam(Ms::ChordRest* cr);
    static bool notTopBeam(Ms::ChordRest* cr);
    static void createBeams(Ms::Score* score, struct LayoutContext& lc, Ms::Measure* measure);
    static void restoreBeams(Ms::Measure* m);
    static void breakCrossMeasureBeams(Ms::Measure* measure);
    static void respace(std::vector<Ms::ChordRest*>* elements);

private:
    static void beamGraceNotes(Ms::Score* score, Ms::Chord* mainNote, bool after);
};
}

#endif // MU_ENGRAVING_LAYOUTBEAMS_H
