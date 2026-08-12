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

#include "regroup.h"

#include "editspanner.h"
#include "transaction/transaction.h"

#include "../dom/chord.h"
#include "../dom/factory.h"
#include "../dom/input.h"
#include "../dom/measure.h"
#include "../dom/note.h"
#include "../dom/rest.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/select.h"
#include "../dom/sig.h"
#include "../dom/slur.h"
#include "../dom/staff.h"
#include "../dom/tie.h"
#include "../dom/utils.h"

#include "log.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   regroupNotesAndRests
//    * combine consecutive rests into fewer rests of longer duration.
//    * combine tied notes/chords into fewer notes of longer duration.
//    Only operates on one voice - protects manual layout adjustment, etc.
//---------------------------------------------------------

void Regroup::regroupNotesAndRests(Transaction& tx, Score* score, const Fraction& startTick, const Fraction& endTick,
                                   track_idx_t track)
{
    InputState& is = score->inputState();
    Segment* inputSegment = is.segment();   // store this so we can get back to it later.
    Segment* seg = score->tick2segment(startTick, true, SegmentType::ChordRest);
    for (Measure* msr = seg->measure(); msr && msr->tick() < endTick; msr = msr->nextMeasure()) {
        Fraction maxTick = endTick > msr->endTick() ? msr->endTick() : endTick;
        if (!seg || seg->measure() != msr) {
            seg = msr->first(SegmentType::ChordRest);
        }
        for (; seg; seg = seg->next(SegmentType::ChordRest)) {
            ChordRest* curr = seg->cr(track);
            if (!curr) {
                continue;         // this voice is empty here (CR overlaps with CR in other track)
            }
            if (curr->endTick() > maxTick) {
                break;         // outside range
            }
            if (curr->isRest() && !(curr->tuplet()) && !(toRest(curr)->isGap())) {
                // combine consecutive rests
                ChordRest* lastRest = curr;
                for (Segment* s = seg->next(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                    ChordRest* cr = s->cr(track);
                    if (!cr) {
                        continue;             // this voice is empty here
                    }
                    if (!cr->isRest() || cr->tuplet() || cr->endTick() > maxTick || toRest(cr)->isGap()) {
                        break;             // next element in the same voice is not a rest, or it exceeds the selection, or it is a gap
                    }
                    lastRest = cr;
                }
                Fraction restTicks = (lastRest->endTick() - curr->tick()) * curr->staff()->timeStretch(curr->tick());
                seg = score->setNoteRest(seg, curr->track(), NoteVal(), restTicks, DirectionV::AUTO, false, {}, true);
            } else if (curr->isChord()) {
                // combine tied chords
                Chord* chord = toChord(curr);
                Chord* lastTiedChord = chord;
                for (Chord* next = chord->nextTiedChord(); next && next->endTick() <= maxTick; next = next->nextTiedChord()) {
                    lastTiedChord = next;
                }
                if (!lastTiedChord) {
                    lastTiedChord = chord;
                }
                Fraction noteTicks = (lastTiedChord->endTick() - chord->tick()) * chord->staff()->timeStretch(chord->tick());
                if (!(curr->tuplet())) {
                    // store start/end note for backward/forward ties ending/starting on the group of notes being rewritten
                    size_t numNotes = chord->notes().size();
                    std::vector<Note*> tieBack(numNotes);
                    std::vector<Note*> tieFor(numNotes);
                    for (size_t i = 0; i < numNotes; i++) {
                        Note* n = chord->notes()[i];
                        Note* nn = lastTiedChord->notes()[i];
                        if (n->tieBack()) {
                            tieBack[i] = n->tieBack()->startNote();
                        } else {
                            tieBack[i] = 0;
                        }
                        if (nn->tieFor()) {
                            tieFor[i] = nn->tieFor()->endNote();
                        } else {
                            tieFor[i] = 0;
                        }
                    }
                    Fraction tick = seg->tick();
                    track_idx_t tr = chord->track();
                    Fraction sd   = noteTicks;
                    std::vector<Tie*> ties;
                    Segment* segment = seg;
                    ChordRest* cr = toChordRest(segment->element(tr));
                    Chord* nchord = toChord(chord->clone());
                    for (size_t i = 0; i < numNotes; i++) {           // strip ties from cloned chord
                        Note* n = nchord->notes()[i];
                        if (Tie* tieFor2 = n->tieFor()) {
                            n->setTieFor(nullptr);
                            delete tieFor2;
                        }
                        if (Tie* tieBack2 = n->tieBack()) {
                            n->setTieBack(nullptr);
                            delete tieBack2;
                        }
                    }
                    Chord* startChord = nchord;
                    Measure* measure = nullptr;
                    bool firstpart = true;
                    for (;;) {
                        if (tr % VOICES) {
                            score->expandVoice(segment, tr);
                        }
                        // the returned gap ends at the measure boundary or at tuplet end
                        Fraction dd = score->makeGap(segment, tr, sd, cr->tuplet());
                        if (dd.isZero()) {
                            break;
                        }
                        measure = segment->measure();
                        std::vector<TDuration> dl;
                        dl = toRhythmicDurationList(dd, false, segment->rtick(), score->sigmap()->timesig(
                                                        tick.ticks()).nominal(), measure, 1,
                                                    score->staff(track2staff(track))->timeStretch(tick));
                        size_t n = dl.size();
                        for (size_t i = 0; i < n; ++i) {
                            const TDuration& d = dl[i];
                            Chord* nchord2 = toChord(nchord->clone());
                            if (!firstpart) {
                                nchord2->removeMarkings(true);
                            }
                            nchord2->setDurationType(d);
                            nchord2->setTicks(d.fraction());
                            std::vector<Note*> nl1 = nchord->notes();
                            std::vector<Note*> nl2 = nchord2->notes();
                            if (!firstpart) {
                                for (size_t j = 0; j < nl1.size(); ++j) {
                                    Tie* tie = Factory::createTie(score->dummy());
                                    tie->setStartNote(nl1[j]);
                                    tie->setEndNote(nl2[j]);
                                    tie->setTick(tie->startNote()->tick());
                                    tie->setTick2(tie->endNote()->tick());
                                    tie->setTrack(tr);
                                    nl1[j]->setTieFor(tie);
                                    nl2[j]->setTieBack(tie);
                                    ties.push_back(tie);
                                }
                            }
                            score->undoAddCR(nchord2, measure, tick);
                            segment = nchord2->segment();
                            tick += nchord2->actualTicks();
                            nchord = nchord2;
                            firstpart = false;
                        }
                        sd -= dd;
                        if (sd.isZero()) {
                            break;
                        }
                        Segment* nseg = score->tick2segment(tick, false, SegmentType::ChordRest);
                        if (nseg == 0) {
                            break;
                        }
                        segment = nseg;
                        cr = toChordRest(segment->element(tr));
                        if (cr == 0) {
                            if (tr % VOICES) {
                                cr = score->addRest(segment, tr, TDuration(DurationType::V_MEASURE), 0);
                            } else {
                                break;
                            }
                        }
                    }
                    if (is.slur()) {
                        // extend slur
                        is.slur()->undoChangeProperty(Pid::SPANNER_TICKS, nchord->tick() - is.slur()->tick());
                        for (EngravingObject* e : is.slur()->linkList()) {
                            Slur* slur = toSlur(e);
                            for (EngravingObject* ee : nchord->linkList()) {
                                EngravingItem* e1 = static_cast<EngravingItem*>(ee);
                                if (e1->score() == slur->score() && e1->track() == slur->track2()) {
                                    tx.push(new ChangeSpannerElements(slur, slur->startElement(), e1));
                                    break;
                                }
                            }
                        }
                    }
                    // recreate previously stored pending ties
                    for (size_t i = 0; i < numNotes; i++) {
                        Note* n = startChord->notes()[i];
                        Note* nn = nchord->notes()[i];
                        if (tieBack[i]) {
                            Tie* tie = Factory::createTie(score->dummy());
                            tie->setStartNote(tieBack[i]);
                            tie->setEndNote(n);
                            tie->setTick(tie->startNote()->tick());
                            tie->setTick2(tie->endNote()->tick());
                            tie->setTrack(track);
                            n->setTieBack(tie);
                            tieBack[i]->setTieFor(tie);
                            ties.push_back(tie);
                        }
                        if (tieFor[i]) {
                            Tie* tie = Factory::createTie(score->dummy());
                            tie->setStartNote(nn);
                            tie->setEndNote(tieFor[i]);
                            tie->setTick(tie->startNote()->tick());
                            tie->setTick2(tie->endNote()->tick());
                            tie->setTrack(track);
                            nn->setTieFor(tie);
                            tieFor[i]->setTieBack(tie);
                            ties.push_back(tie);
                        }
                    }
                    if (!ties.empty()) {         // at least one tie was created
                        for (Tie* tie : ties) {
                            score->undoAddElement(tie);
                        }
                        score->connectTies();
                    }
                }
            }
        }
    }
    // now put the input state back where it was before
    is.setSegment(inputSegment);
}

void Regroup::regroupNotesAndRestsInSelection(Transaction& tx, Score* score)
{
    bool noSelection = score->selection().isNone();
    if (noSelection) {
        score->cmdSelectAll();
    } else if (!score->selection().isRange()) {
        LOGD("no system or staff selected");
        return;
    }

    // save selection values because selection changes during grouping
    Fraction sTick = score->selection().tickStart();
    Fraction eTick = score->selection().tickEnd();
    staff_idx_t sStaff = score->selection().staffStart();
    staff_idx_t eStaff = score->selection().staffEnd();

    for (staff_idx_t staff = sStaff; staff < eStaff; staff++) {
        track_idx_t sTrack = staff * VOICES;
        track_idx_t eTrack = sTrack + VOICES;
        for (track_idx_t track = sTrack; track < eTrack; track++) {
            if (score->selectionFilter().canSelectVoice(track)) {
                regroupNotesAndRests(tx, score, sTick, eTick, track);
            }
        }
    }

    if (noSelection) {
        score->deselectAll();
        return;
    }

    // Reset selection to original selection
    score->selection().setRangeTicks(sTick, eTick, sStaff, eStaff);
    score->selection().updateSelectedElements();
}
