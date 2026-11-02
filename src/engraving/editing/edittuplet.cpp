/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "edittuplet.h"

#include <algorithm>
#include <unordered_set>

#include "../dom/chord.h"
#include "../dom/chordrest.h"
#include "../dom/input.h"
#include "../dom/note.h"
#include "../dom/score.h"
#include "../dom/select.h"

#include "log.h"

using namespace mu::engraving;

static Fraction dividingRatio(const ChordRest* chordRest, const Fraction& ratio, bool autoBaseLen)
{
    Fraction result = ratio;
    if (autoBaseLen) {
        result.setDenominator(Tuplet::computeTupletDenominator(ratio.numerator(), chordRest->ticks()));
    }
    return result;
}

// Selected chords/rests a tuplet can start at, in score order
static std::vector<ChordRest*> selectedTupletChordRests(const Score* score)
{
    // Score order, not pointer order
    const std::set<ChordRest*> selectedChordRests = score->getSelectedChordRests();
    std::vector<ChordRest*> sortedChordRests(selectedChordRests.begin(), selectedChordRests.end());
    std::sort(sortedChordRests.begin(), sortedChordRests.end(), [](const ChordRest* a, const ChordRest* b) {
        return a->tick() != b->tick() ? a->tick() < b->tick() : a->track() < b->track();
    });

    std::vector<ChordRest*> chordRests;

    // Make sure each linked group is handled only once
    std::unordered_set<const LinkedObjects*> handledLinks;

    for (ChordRest* chordRest : sortedChordRests) {
        if (chordRest->links() && !handledLinks.insert(chordRest->links()).second) {
            continue;
        }

        if (!chordRest->isGrace() && !(chordRest->isChord() && toChord(chordRest)->isTrillCueNote())) {
            chordRests.push_back(chordRest);
        }
    }

    return chordRests;
}

static void selectTupletStart(Score* score, const Tuplet* tuplet)
{
    if (tuplet->elements().empty()) {
        return;
    }

    DurationElement* firstElement = tuplet->elements().front();
    // SINGLE also moves the input position
    if (firstElement->isChord()) {
        const std::vector<Note*>& notes = toChord(firstElement)->notes();
        IF_ASSERT_FAILED(!notes.empty()) {
            score->select(firstElement, SelectType::SINGLE);
        } else {
            score->select(notes.front(), SelectType::SINGLE);
            score->select({ notes.begin() + 1, notes.end() }, SelectType::ADD);
        }
    } else {
        score->select(firstElement, SelectType::SINGLE);
    }
}

std::vector<Tuplet*> EditTuplet::addTuplets(Transaction&, Score* score, const Fraction& ratio, TupletNumberType numberType,
                                            TupletBracketType bracketType, bool autoBaseLen)
{
    Selection& selection = score->selection();
    const bool wasRangeSelection = selection.isRange();
    const Fraction startTick = selection.tickStart();
    const Fraction endTick = selection.tickEnd();
    const staff_idx_t startStaff = selection.staffStart();
    const staff_idx_t endStaff = selection.staffEnd();
    Box* startBox = selection.startBox();
    Box* endBox = selection.endBox();

    std::vector<Tuplet*> tuplets;
    for (ChordRest* chordRest : selectedTupletChordRests(score)) {
        if (Tuplet* tuplet = score->addTuplet(chordRest, dividingRatio(chordRest, ratio, autoBaseLen), numberType, bracketType)) {
            tuplets.push_back(tuplet);
        }
    }

    if (wasRangeSelection) {
        selection.setRangeTicks(startTick, endTick, startStaff, endStaff, startBox, endBox);
        selection.updateSelectedElements();
    } else if (!tuplets.empty()) {
        selectTupletStart(score, tuplets.front());
    }

    if (!tuplets.empty()) {
        score->inputState().setDuration(tuplets.front()->baseLen());
    }

    return tuplets;
}
