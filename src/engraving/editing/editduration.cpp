/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "editduration.h"

#include <algorithm>

#include "rw/xmlreader.h"

#include "../dom/chord.h"
#include "../dom/chordrest.h"
#include "../dom/durationtype.h"
#include "../dom/factory.h"
#include "../dom/input.h"
#include "../dom/laissezvib.h"
#include "../dom/measure.h"
#include "../dom/note.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/select.h"
#include "../dom/tie.h"
#include "../dom/utils.h"

#include "navigation.h"
#include "noteinput.h"
#include "paste.h"
#include "transaction/transaction.h"

#include "log.h"

using namespace mu::engraving;

void EditDuration::doubleDuration(Transaction& tx, Score* score)
{
    incDecDuration(tx, score, -1, false);
}

void EditDuration::halfDuration(Transaction& tx, Score* score)
{
    incDecDuration(tx, score, 1, false);
}

void EditDuration::incDurationDotted(Transaction& tx, Score* score)
{
    incDecDuration(tx, score, -1, true);
}

void EditDuration::decDurationDotted(Transaction& tx, Score* score)
{
    incDecDuration(tx, score, 1, true);
}

//---------------------------------------------------------
//   incDecDuration
//     When stepDotted is false and nSteps is 1 or -1, will halve or double the duration
//     When stepDotted is true, will step by nearest dotted or undotted note
//---------------------------------------------------------

void EditDuration::incDecDuration(Transaction& tx, Score* score, int nSteps, bool stepDotted)
{
    Selection& selection = score->selection();
    InputState& is = score->inputState();

    if (selection.isRange()) {
        if (!selection.canCopy()) {
            return;
        }
        ChordRest* firstCR = selection.firstChordRest();
        if (firstCR->isGrace()) {
            firstCR = toChordRest(firstCR->parent());
        }
        TDuration initialDuration = firstCR->ticks();
        TDuration d = initialDuration.shiftRetainDots(nSteps, stepDotted);
        if (!d.isValid()) {
            return;
        }
        Fraction scale = d.ticks() / initialDuration.ticks();
        for (ChordRest* cr : score->getSelectedChordRests()) {
            Fraction newTicks = cr->ticks() * scale;
            if (newTicks < Fraction(1, 1024)
                || (stepDotted && cr->durationType().dots() != firstCR->durationType().dots()
                    && !cr->isGrace())) {
                return;
            }
        }
        const muse::ByteArray mimeData(selection.mimeData());
        XmlReader e(mimeData);
        score->deleteRange(selection.startSegment(), selection.endSegment(), staff2track(selection.staffStart()),
                           staff2track(selection.staffEnd()), score->selectionFilter(), selection.rangeContainsMultiNoteChords());
        Paste::pasteStaff(tx, score, e, selection.startSegment(), selection.staffStart(), scale);
    } else if (selection.isList()) {
        const std::vector<Note*> notes = selection.noteList();
        const std::set<ChordRest*> crsSet = score->getSelectedChordRests();
        std::vector<ChordRest*> crs(crsSet.begin(), crsSet.end());
        std::sort(crs.begin(), crs.end(), [](const ChordRest* a, const ChordRest* b) { return a->tick() > b->tick(); });

        for (ChordRest* cr : crs) {
            // if measure rest is selected as input, then the correct initialDuration will be the
            // duration of the measure's time signature, else is just the ChordRest's duration
            TDuration initialDuration = cr->durationType();
            if (initialDuration == DurationType::V_MEASURE) {
                initialDuration = TDuration(cr->measure()->timesig(), true);

                if (initialDuration.fraction() < cr->measure()->timesig() && nSteps > 0) {
                    // Duration already shortened by truncation; shorten one step less
                    --nSteps;
                }
            }

            TDuration newDuration { stepDotted ? initialDuration.shiftRetainDots(nSteps, stepDotted) : initialDuration.shift(nSteps) };
            if (!newDuration.isValid()) {
                continue;
            }

            if (cr->isGrace()) {
                score->undoChangeChordRestLen(cr, newDuration);
            } else {
                score->changeCRlen(cr, newDuration);
            }
        }
        for (Note* n : notes) {
            if (score->canReselectItem(n)) {
                score->select(toEngravingItem(n), SelectType::ADD);
            }
        }
        if (is.noteEntryMode()) {
            const ChordRest* cr = crs.size() == 1 ? crs.front() : nullptr;
            IF_ASSERT_FAILED(cr) {
                // (At time of writing) it shouldn't be possible to have more than
                // one CR selected during note entry...
                return;
            }
            is.setDuration(cr->durationType());
            NoteInput::nextInputPos(tx, score, cr, false);
        }
    }
}

//---------------------------------------------------------
//   extendToNextNote
//---------------------------------------------------------

void EditDuration::extendToNextNote(Transaction& tx, Score* score)
{
    Selection& selection = score->selection();
    InputState& is = score->inputState();

    const Fraction startTick = selection.tickStart();
    Fraction endTick = selection.tickEnd();
    const staff_idx_t startStaff = selection.staffStart();
    const staff_idx_t endStaff = selection.staffEnd();

    std::vector<EngravingItem*> toSelect;
    const bool wasRangeSelection = selection.isRange();
    const std::vector<Note*> initialSelection = selection.noteList();

    for (ChordRest* cr : score->getSelectedChordRests()) {
        ChordRest* ncr = Navigation::nextChordRest(cr);
        if (cr->isRest() || cr->isGrace() || cr->endTick() == score->endTick()
            || (ncr && ncr->isChord() && cr->endTick() == ncr->tick())) {
            continue;
        }
        std::vector<Note*> chordNotes = toChord(cr)->notes();
        std::vector<Note*> selectedChordNotes;
        for (Note* n : chordNotes) {
            if (std::find(initialSelection.begin(), initialSelection.end(), n) != initialSelection.end()) {
                selectedChordNotes.push_back(n);
                toSelect.push_back(n);
                if (LaissezVib* lv = n->laissezVib()) {
                    score->undoRemoveElement(lv);
                }
            }
        }
        bool allNotesSelected = chordNotes.size() == selectedChordNotes.size() ? true : false;

        while (cr->endTick() != score->endTick()) { // not at end of score
            if (ncr && cr->endTick() == ncr->tick() && ncr->isChord()) {
                break;
            }
            if (!ncr || cr->endTick() != ncr->tick()  // if voices>0 have empty measures till end OR have empty measures between cr and ncr
                || cr->tuplet() != ncr->tuplet() || !allNotesSelected) {
                std::vector<Note*> notesToExtend = !allNotesSelected ? (allNotesSelected = true, selectedChordNotes) : toChord(cr)->notes();
                is.setTrack(cr->track());
                is.setSegment(cr->segment());
                is.moveToNextInputPos();
                is.setLastSegment(is.segment());

                if (!is.cr()) {
                    score->expandVoice();
                }
                is.setDuration(is.cr()->durationType());

                Note* nn = nullptr;
                for (size_t i = 0; i < notesToExtend.size(); i++) {
                    Note* note = notesToExtend[i];
                    NoteVal nval(note->noteVal());
                    nn = NoteInput::addPitch(tx, score, nval, i != 0);

                    Tie* tie =  Factory::createTie(note);
                    tie->setStartNote(note);
                    tie->setTrack(note->track());
                    tie->setTick(note->chord()->segment()->tick());
                    tie->setEndNote(nn);
                    tie->setTicks(nn->chord()->segment()->tick() - note->chord()->segment()->tick());
                    score->undoAddElement(tie);
                }
                cr = toChordRest(nn->chord());
                toSelect.push_back(cr);
            } else {
                Fraction newDur = cr->ticks() + ncr->ticks();
                score->changeCRlen(cr, newDur);
                while (toChord(cr)->notes()[0]->tieFor()) {
                    cr = Navigation::nextChordRest(cr);
                    toSelect.push_back(cr);
                }
            }
            ncr = Navigation::nextChordRest(cr);
            endTick = cr->endTick() >= endTick ? cr->endTick() : endTick;
        }
    }

    if (wasRangeSelection) {
        selection.setRangeTicks(startTick, endTick, startStaff, endStaff);
        selection.updateSelectedElements();
    } else {
        for (EngravingItem* ei : toSelect) {
            if (ei->isChord()) {
                std::vector<Note*> notes = toChord(ei)->notes();
                score->select({ notes.begin(), notes.end() }, SelectType::ADD);
            } else {
                score->select(ei, SelectType::ADD);
            }
        }
    }
}
