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
#include "layouttremolo.h"

#include "libmscore/chord.h"
#include "libmscore/stem.h"
#include "libmscore/tremolo.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   extendedStemLenWithTwoNotesTremolo
//    Goal: To extend stem of one of the chords to make the tremolo less steep
//    Returns a modified pair of stem lengths of two chords
//---------------------------------------------------------

std::pair<double, double> LayoutTremolo::extendedStemLenWithTwoNoteTremolo(Tremolo* tremolo, double stemLen1, double stemLen2)
{
    const double spatium = tremolo->spatium();
    Chord* c1 = tremolo->chord1();
    Chord* c2 = tremolo->chord2();
    Stem* s1 = c1->stem();
    Stem* s2 = c2->stem();
    const double sgn1 = c1->up() ? -1.0 : 1.0;
    const double sgn2 = c2->up() ? -1.0 : 1.0;
    const double stemTipDistance = (s1 && s2) ? (s2->pagePos().y() + stemLen2) - (s1->pagePos().y() + stemLen1)
                                   : (c2->stemPos().y() + (sgn2 * stemLen2)) - (c1->stemPos().y() + (sgn1 * stemLen1));

    // same staff & same direction: extend one of the stems
    if (c1->staffMove() == c2->staffMove() && c1->up() == c2->up()) {
        const bool stem1Higher = stemTipDistance > 0.0;
        if (std::abs(stemTipDistance) > 1.0 * spatium) {
            if ((c1->up() && !stem1Higher) || (!c1->up() && stem1Higher)) {
                return { stemLen1 + (std::abs(stemTipDistance) - spatium), stemLen2 };
            } else {   /* if ((c1->up() && stem1Higher) || (!c1->up() && !stem1Higher)) */
                return { stemLen1, stemLen2 + (std::abs(stemTipDistance) - spatium) };
            }
        }
    }

    return { stemLen1, stemLen2 };
}
