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

    score->startCmd(TranslatableString("undoableAction", "Add tie"));
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
            // tie grace note before to next grace note or main note
            Note* nextNote = searchTieNote(note);
            if (nextNote && nextNote->chord()->isGrace()) {
                cr = nextNote->chord();
                addToChord = true;
            } else {
                ChordRest* host = toChordRest(c->explicitParent());
                if (!host->isChord()) {
                    continue; // rest has no main note to tie to
                }
                cr = host;
                addToChord = true;
            }
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
    score->endCmd();
}

Tie* EditTie::cmdToggleTie(Score* score)
{
    std::vector<Note*> noteList = cmdTieNoteList(score->selection(), score->noteEntryMode());

    if (noteList.empty()) {
        LOGD("no notes selected");
        return nullptr;
    }

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

    const bool shouldTieListSelection = noteList.size() >= 2 && !singleTick;

    if (singleTick /* i.e. all notes are in the same tick */ && !allHaveExistingNextNoteToTieTo) {
        cmdAddTie(score);
        return nullptr;
    }

    const TranslatableString actionName = someHaveExistingNextNoteToTieTo
                                          ? TranslatableString("undoableAction", "Add tie")
                                          : TranslatableString("undoableAction", "Remove tie");

    score->startCmd(actionName);

    Tie* tie = nullptr;

    for (size_t i = 0; i < noteList.size(); ++i) {
        Note* note = noteList[i];
        Note* tieToNote = tieNoteList[i];

        if (!note) {
            continue;
        }

        // Tie to adjacent unselected note
        if (someHaveExistingNextNoteToTieTo && tieToNote) {
            Note* startNote = note->tick() <= tieToNote->tick() ? note : tieToNote;
            Note* endNote = startNote == tieToNote ? note : tieToNote;
            tie = createAndAddTie(startNote, endNote);
            continue;
        }

        Tie* oldTie = note->tieFor();
        Chord* chord = note->chord();
        if (oldTie) {
            // Toggle existing tie off
            if (oldTie->tieJumpPoints()) {
                oldTie->undoRemoveTiesFromJumpPoints();
            }
            score->undoRemoveElement(oldTie);
            continue;
        }

        if (chord->hasFollowingJumpItem()) {
            // Create partial tie
            tie = createAndAddTie(note, nullptr);
            continue;
        }

        if (!shouldTieListSelection || i > noteList.size() - 2) {
            continue;
        }

        // Tie to next appropriate note in selection
        Note* note2 = nullptr;

        for (size_t j = i + 1; j < noteList.size(); ++j) {
            Note* candidateNote = noteList[j];
            if (!candidateNote) {
                continue;
            }
            const bool samePart = note->part() == candidateNote->part();
            const bool samePitch = note->pitch() == candidateNote->pitch();
            const bool sameUnisonIdx = note->unisonIndex() == candidateNote->unisonIndex();
            const bool diffTick = note->tick() != candidateNote->tick();
            if (samePart && samePitch && sameUnisonIdx && diffTick) {
                note2 = candidateNote;
                noteList[j] = nullptr;
                break;
            }
        }

        if (!(note && note2)) {
            continue;
        }

        Note* startNote = note->tick() <= note2->tick() ? note : note2;
        Note* endNote = startNote == note2 ? note : note2;

        tie = createAndAddTie(startNote, endNote);
    }

    score->endCmd();

    return tie;
}

void EditTie::cmdToggleLaissezVib(Score* score)
{
    const std::vector<Note*> noteList = score->selection().noteList();

    if (noteList.empty()) {
        LOGD("no notes selected");
        return;
    }

    score->startCmd(TranslatableString("undoableAction", "Toggle laissez vibrer"));

    for (Note* note: noteList) {
        if (LaissezVib* lv = note->laissezVib()) {
            score->undoRemoveElement(lv);
        } else if (note->tieFor()) {
            continue;
        } else {
            LaissezVib* lvTie = Factory::createLaissezVib(note);
            lvTie->setParent(note);
            score->undoAddElement(lvTie);
        }
    }

    score->endCmd();
}
