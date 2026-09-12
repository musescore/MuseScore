/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "edittie.h"

#include "../dom/chord.h"
#include "../dom/factory.h"
#include "../dom/input.h"
#include "../dom/laissezvib.h"
#include "../dom/note.h"
#include "../dom/partialtie.h"
#include "../dom/score.h"
#include "../dom/select.h"
#include "../dom/tie.h"
#include "../dom/tiejumppointlist.h"
#include "../dom/utils.h"

#include "editchord.h"
#include "noteinput.h"
#include "transaction/transaction.h"

#include "log.h"

using namespace mu::engraving;

void ChangeTieJumpPointActive::flip()
{
    TieJumpPoint* jumpPoint = m_jumpPointList->findJumpPoint(m_id);
    if (!jumpPoint) {
        return;
    }
    bool oldActive = jumpPoint->active();

    if (m_active && jumpPoint->endTie() && jumpPoint->endTie()->jumpPoint() != jumpPoint) {
        jumpPoint->endTie()->setJumpPoint(jumpPoint);
    }

    jumpPoint->setActive(m_active);
    m_active = oldActive;
}

std::vector<Note*> EditTie::cmdTieNoteList(const Selection& selection, bool noteEntryMode)
{
    EngravingItem* el = selection.element();
    if (Note* n = InputState::note(el)) {
        if (noteEntryMode) {
            return n->chord()->notes();
        } else {
            return { n };
        }
    } else {
        ChordRest* cr = InputState::chordRest(el);
        if (cr && cr->isChord()) {
            return toChord(cr)->notes();
        }
    }
    return selection.noteList();
}

static Tie* createAndAddTie(Note* startNote, Note* endNote)
{
    Score* score = startNote->score();
    Tie* tie = endNote ? Factory::createTie(startNote) : Factory::createPartialTie(startNote);
    tie->setStartNote(startNote);
    tie->setTrack(startNote->track());
    tie->setTick(startNote->chord()->segment()->tick());
    if (endNote) {
        if (endNote->tieBack()) {
            score->undoRemoveElement(endNote->tieBack());
        }
        tie->setEndNote(endNote);
        tie->setTicks(endNote->chord()->segment()->tick() - startNote->chord()->segment()->tick());
    }
    score->undoAddElement(tie);

    tie->addTiesToJumpPoints();
    if (!tie->endNote() && tie->tieJumpPoints() && tie->tieJumpPoints()->empty()) {
        score->undoRemoveElement(tie);
        tie = nullptr;
    }

    return tie;
}

void EditTie::cmdAddTie(Score* score, bool addToChord)
{
    std::vector<Note*> noteList = cmdTieNoteList(score->selection(), score->noteEntryMode());
    if (noteList.empty()) {
        LOGD("no notes selected");
        return;
    }

    std::sort(noteList.begin(), noteList.end(), [](const Note* a, const Note* b) { return a->track() < b->track(); });
    track_idx_t track = noteList.at(0)->track();

    std::vector<EngravingItem*> toSelect;

    Chord* lastAddedChord = nullptr;

    Transaction& tx = score->transactionManager()->currentOrDummyTransaction();

    InputState& is = score->inputState();

    for (Note* note : noteList) {
        if (note->tieFor()) {
            LOGD("cmdAddTie: note %p has already tie? noteFor: %p", note, note->tieFor());
            if (addToChord) {
                continue;
            } else {
                score->undoRemoveElement(note->tieFor());
            }
        }

        ChordRest* cr = nullptr;
        Chord* c = note->chord();
        int staffMove = c->staffMove();

        // set cursor at position after note
        if (c->isGraceBefore()) {
            // tie grace note before to main note
            cr = toChord(c->ownershipParent());
            addToChord = true;
        } else {
            is.setTrack(note->chord()->track());
            is.setSegment(note->chord()->segment());
            is.moveToNextInputPos();
            if (is.beyondScore()) {
                score->appendMeasures(1);
                is.moveToNextInputPos();
            }
            is.setLastSegment(is.segment());

            if (!is.cr()) {
                score->expandVoice();
            }
            cr = is.cr();
        }
        if (!cr) {
            break;
        }

        bool addFlag = lastAddedChord != nullptr;
        if (c->track() != track) {
            addFlag = false;
            track = c->track();
        }
        // try to re-use existing note or chord
        Note* n = nullptr;
        if (addToChord && cr->isChord()) {
            Chord* chord = toChord(cr);
            Note* nn = chord->findNote(note->pitch());
            if (nn && nn->tpc() == note->tpc()) {
                n = nn;                     // re-use note
            } else {
                addFlag = true;             // re-use chord
            }
        }

        // if no note to re-use, create one
        NoteVal nval(note->noteVal());
        if (!n) {
            n = NoteInput::addPitch(tx, score, nval, addFlag);
            if (staffMove != 0) {
                score->undo(new ChangeChordStaffMove(n->chord(), staffMove));
            }
        } else {
            score->select(n);
        }

        if (n) {
            if (!lastAddedChord) {
                lastAddedChord = n->chord();
            }
            // n is not necessarily next note if duration span over measure
            Note* nnote = searchTieNote(note);
            while (nnote) {
                // DEBUG: if duration spans over measure
                // this does not set line for intermediate notes
                // tpc was set correctly already
                //n->setLine(note->line());
                //n->setTpc(note->tpc());
                createAndAddTie(note, nnote);

                if (!addFlag || nnote->chord()->tick() >= lastAddedChord->tick() || nnote->chord()->isGrace()) {
                    break;
                } else {
                    note = nnote;
                    is.setLastSegment(is.segment());
                    nnote = NoteInput::addPitch(tx, score, nval, true);
                }
            }
            if (staffMove != 0) {
                for (Note* tiedNote : n->tiedNotes()) {
                    score->undo(new ChangeChordStaffMove(tiedNote->chord(), staffMove));
                }
            }
        }
        toSelect.push_back(n);
    }
    if (lastAddedChord) {
        NoteInput::nextInputPos(tx, score, lastAddedChord, false);
    }
    for (EngravingItem* e : toSelect) {
        if (score->canReselectItem(e)) {
            score->select(e, SelectType::ADD);
        }
    }
}

EditTie::TieAnalysis EditTie::analyzeTieTargets(Score* score)
{
    bool noteEntryMode = score->noteEntryMode();
    std::vector<Note*> noteList = cmdTieNoteList(score->selection(), noteEntryMode);

    std::vector<Note*> tieNoteList(noteList.size());
    bool singleTick = true;
    bool someHaveExistingNextNoteToTieTo = false;
    bool allHaveExistingNextNoteToTieTo = true;

    for (size_t i = 0; i < noteList.size(); ++i) {
        Note* n = noteList[i];
        if (n->chord()->tick() != noteList.front()->tick()) {
            singleTick = false;
        }
        if (n->tieFor()) {
            tieNoteList[i] = nullptr;
        } else {
            Note* tieNote = searchTieNote(n);
            tieNoteList[i] = tieNote;
            if (tieNote || n->chord()->hasFollowingJumpItem()) {
                someHaveExistingNextNoteToTieTo = true;
            } else {
                allHaveExistingNextNoteToTieTo = false;
            }
        }
    }

    TranslatableString actionName = (singleTick && !allHaveExistingNextNoteToTieTo) || someHaveExistingNextNoteToTieTo
                                    ? TranslatableString("undoableAction", "Add tie")
                                    : TranslatableString("undoableAction", "Remove tie");

    return TieAnalysis {
        std::move(noteList),
        std::move(tieNoteList),
        singleTick,
        someHaveExistingNextNoteToTieTo,
        allHaveExistingNextNoteToTieTo,
        std::move(actionName)
    };
}

Note* findTiePartnerInSelection(std::vector<Note*>& noteList, size_t i)
{
    Note* note = noteList[i];
    for (size_t j = i + 1; j < noteList.size(); ++j) {
        Note* candidate = noteList[j];
        if (!candidate) {
            continue;
        }
        const bool samePart = note->part() == candidate->part();
        const bool samePitch = note->pitch() == candidate->pitch();
        const bool sameUnisonIdx = note->unisonIndex() == candidate->unisonIndex();
        const bool diffTick = note->tick() != candidate->tick();
        if (samePart && samePitch && sameUnisonIdx && diffTick) {
            noteList[j] = nullptr;
            return candidate;
        }
    }
    return nullptr;
}

bool toggleOffTie(Score* score, Note* note)
{
    Tie* oldTie = note->tieFor();
    if (!oldTie) {
        return false;
    }
    if (oldTie->tieJumpPoints()) {
        oldTie->undoRemoveTiesFromJumpPoints();
    }
    score->undoRemoveElement(oldTie);
    return true;
}

Tie* tieBetween(Note* a, Note* b)
{
    Note* startNote = a->tick() <= b->tick() ? a : b;
    Note* endNote = startNote == b ? a : b;
    return createAndAddTie(startNote, endNote);
}

Tie* EditTie::cmdToggleTie(Score* score)
{
    EditTie::TieAnalysis info = analyzeTieTargets(score);

    if (info.noteList.empty()) {
        LOGD("no notes selected");
        return nullptr;
    }

    if (info.singleTick /* i.e. all notes are in the same tick */ && !info.allHaveExistingNextNoteToTieTo) {
        cmdAddTie(score);
        return nullptr;
    }
    const bool shouldTieListSelection = info.noteList.size() >= 2 && !info.singleTick;

    Tie* tie = nullptr;

    for (size_t i = 0; i < info.noteList.size(); ++i) {
        Note* note = info.noteList[i];

        if (!note) {
            continue;
        }

        // Tie to adjacent unselected note
        if (info.someHaveExistingNextNoteToTieTo && info.tieNoteList[i]) {
            tie = tieBetween(note, info.tieNoteList[i]);
            continue;
        }

        if (toggleOffTie(score, note)) {
            continue;
        }

        if (note->chord()->hasFollowingJumpItem()) {
            // Create partial tie
            tie = createAndAddTie(note, nullptr);
            continue;
        }

        if (!shouldTieListSelection || i > info.noteList.size() - 2) {
            continue;
        }

        // Tie to next appropriate note in selection
        Note* partner = findTiePartnerInSelection(info.noteList, i);
        if (!partner) {
            continue;
        }

        tie = tieBetween(note, partner);
    }

    return tie;
}

void EditTie::cmdToggleLaissezVib(Score* score)
{
    const std::vector<Note*> noteList = score->selection().noteList();

    if (noteList.empty()) {
        LOGD("no notes selected");
        return;
    }

    for (Note* note: noteList) {
        if (LaissezVib* lv = note->laissezVib()) {
            score->undoRemoveElement(lv);
        } else if (note->tieFor()) {
            continue;
        } else {
            LaissezVib* lvTie = Factory::createLaissezVib(note);
            lvTie->setOwnershipParent(note);
            score->undoAddElement(lvTie);
        }
    }
}
