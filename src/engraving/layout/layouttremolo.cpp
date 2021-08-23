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

#include "libmscore/tremolo.h"
#include "libmscore/chord.h"
#include "libmscore/stem.h"

using namespace mu::engraving;
using namespace Ms;

//---------------------------------------------------------
//   extendedStemLenWithTwoNotesTremolo
//    Goal: To extend stem of one of the chords to make the tremolo less steep
//    Returns a modified pair of stem lengths of two chords
//---------------------------------------------------------

std::pair<qreal, qreal> LayoutTremolo::extendedStemLenWithTwoNoteTremolo(Tremolo* tremolo, qreal stemLen1, qreal stemLen2)
{
    const qreal spatium = tremolo->spatium();
    Chord* c1 = tremolo->chord1();
    Chord* c2 = tremolo->chord2();
    Stem* s1 = c1->stem();
    Stem* s2 = c2->stem();
    const qreal sgn1 = c1->up() ? -1.0 : 1.0;
    const qreal sgn2 = c2->up() ? -1.0 : 1.0;
    const qreal stemTipDistance = (s1 && s2) ? (s2->pagePos().y() + stemLen2) - (s1->pagePos().y() + stemLen1)
                                  : (c2->stemPos().y() + stemLen2) - (c1->stemPos().y() + stemLen1);

    // same staff & same direction: extend one of the stems
    if (c1->staffMove() == c2->staffMove() && c1->up() == c2->up()) {
        const bool stem1Higher = stemTipDistance > 0.0;
        if (std::abs(stemTipDistance) > 1.0 * spatium) {
            if ((c1->up() && !stem1Higher) || (!c1->up() && stem1Higher)) {
                return { stemLen1 + sgn1 * (std::abs(stemTipDistance) - 1.0 * spatium), stemLen2 };
            } else {   /* if ((c1->up() && stem1Higher) || (!c1->up() && !stem1Higher)) */
                return { stemLen1, stemLen2 + sgn2 * (std::abs(stemTipDistance) - 1.0 * spatium) };
            }
        }
    }

    return { stemLen1, stemLen2 };
}
