/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "edittremolo.h"

#include "../dom/chord.h"
#include "../dom/measure.h"
#include "../dom/score.h"
#include "../dom/tremolotwochord.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   MoveTremolo
//---------------------------------------------------------

void MoveTremolo::redo(EditData*)
{
    // Find new tremolo chords
    Measure* m1 = score->tick2measure(chord1Tick);
    Measure* m2 = score->tick2measure(chord2Tick);
    IF_ASSERT_FAILED(m1 && m2) {
        return;
    }
    Chord* c1 = m1->findChord(chord1Tick, track);
    Chord* c2 = m2->findChord(chord2Tick, track);
    IF_ASSERT_FAILED(c1 && c2) {
        return;
    }

    // Remember the old tremolo chords
    oldC1 = trem->chord1();
    oldC2 = trem->chord2();

    // Move tremolo away from old chords
    trem->chord1()->setTremoloTwoChord(nullptr);
    trem->chord2()->setTremoloTwoChord(nullptr);

    // Delete old tremolo on c1 and c2, if present
    if (c1->tremoloTwoChord() && (c1->tremoloTwoChord() != trem)) {
        if (c2->tremoloTwoChord() == c1->tremoloTwoChord()) {
            c2->tremoloTwoChord()->setChords(c1, c2);
        } else {
            c1->tremoloTwoChord()->setChords(c1, nullptr);
        }
        TremoloTwoChord* oldTremolo  = c1->tremoloTwoChord();
        c1->setTremoloTwoChord(nullptr);
        delete oldTremolo;
    }
    if (c2->tremoloTwoChord() && (c2->tremoloTwoChord() != trem)) {
        c2->tremoloTwoChord()->setChords(nullptr, c2);
        TremoloTwoChord* oldTremolo  = c2->tremoloTwoChord();
        c2->setTremoloTwoChord(nullptr);
        delete oldTremolo;
    }

    // Move tremolo to new chords
    c1->setTremoloTwoChord(trem);
    c2->setTremoloTwoChord(trem);
    trem->setChords(c1, c2);
    trem->setParent(c1);

    // Tremolo would cross barline, so remove it
    if (m1 != m2) {
        score->undoRemoveElement(trem);
        return;
    }
    // One of the notes crosses a barline, so remove the tremolo
    if (c1->ticks() != c2->ticks()) {
        score->undoRemoveElement(trem);
    }
}

void MoveTremolo::undo(EditData*)
{
    // Move tremolo to old position
    trem->chord1()->setTremoloTwoChord(nullptr);
    trem->chord2()->setTremoloTwoChord(nullptr);
    oldC1->setTremoloTwoChord(trem);
    oldC2->setTremoloTwoChord(trem);
    trem->setChords(oldC1, oldC2);
    trem->setParent(oldC1);
}
