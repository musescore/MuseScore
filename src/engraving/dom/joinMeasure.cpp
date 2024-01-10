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

#include "factory.h"
#include "masterscore.h"
#include "measure.h"
#include "range.h"
#include "score.h"
#include "spanner.h"
#include "timesig.h"
#include "undo.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   cmdJoinMeasure
//    join measures from m1 upto (including) m2
//---------------------------------------------------------

void Score::cmdJoinMeasure(Measure* m1, Measure* m2)
{
    if (!m1 || !m2) {
        return;
    }
    masterScore()->joinMeasure(m1->tick(), m2->tick());
}

//---------------------------------------------------------
//   joinMeasure
//    join measures from tick1 upto (including) tick2
//    always acts in masterScore
//---------------------------------------------------------

void MasterScore::joinMeasure(const Fraction& tick1, const Fraction& tick2)
{
    Measure* m1 = tick2measure(tick1);
    Measure* m2 = tick2measure(tick2);

    if (!m1 || !m2) {
        return;
    }

    for (size_t staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
        if (m1->isMeasureRepeatGroupWithPrevM(static_cast<int>(staffIdx))
            || m2->isMeasureRepeatGroupWithNextM(static_cast<int>(staffIdx))) {
            MScore::setError(MsError::CANNOT_SPLIT_MEASURE_REPEAT);
            return;
        }
    }

    if (m1->isMMRest()) {
        m1 = m1->mmRestFirst();
    }
    if (m2->isMMRest()) {
        m2 = m2->mmRestLast();
    }

    deselectAll();

    ScoreRange range;
    range.read(m1->first(), m2->last());

    Fraction startTick = m1->tick();
    Fraction endTick = m2->endTick();

    auto spanners = spannerMap().findContained(startTick.ticks(), endTick.ticks());
    for (auto i : spanners) {
        doUndoRemoveElement(i.value);
    }

    for (auto i : spanner()) {
        Spanner* s = i.second;
        if (s->tick() >= startTick && s->tick() < endTick) {
            s->setStartElement(0);
        }
        if (s->tick2() >= startTick && s->tick2() < endTick) {
            s->setEndElement(0);
        }
    }

    MeasureBase* next = m2->next();
    InsertMeasureOptions options;
    options.createEmptyMeasures = true;
    options.moveSignaturesClef = false;
    options.moveStaffTypeChanges = false;
    insertMeasure(next, options);

    for (Score* s : scoreList()) {
        Measure* sM1 = s->tick2measure(startTick);
        Measure* sM2 = s->tick2measure(m2->tick());
        s->undoRemoveMeasures(sM1, sM2, true, false);
    }

    const Fraction newTimesig = m1->timesig();
    Fraction newLen;
    for (Measure* mm = m1; mm; mm = mm->nextMeasure()) {
        newLen += mm->ticks();
        if (mm == m2) {
            break;
        }
    }

    // The loop since measures are not currently linked in MuseScore
    // change nominal time sig, as inserted measure took it from next measure
    for (Score* s : scoreList()) {
        Measure* ins = s->tick2measure(startTick);
        ins->undoChangeProperty(Pid::TIMESIG_NOMINAL, newTimesig);
        // set correct barline types if needed
        // TODO: handle other end barline types; they may differ per staff
        if (m2->endBarLineType() == BarLineType::END_REPEAT) {
            ins->undoChangeProperty(Pid::REPEAT_END, true);
        }
        if (m1->getProperty(Pid::REPEAT_START).toBool()) {
            ins->undoChangeProperty(Pid::REPEAT_START, true);
        }
    }
    Measure* inserted = (next ? next->prevMeasure() : lastMeasure());
    inserted->adjustToLen(newLen, /* appendRests... */ false);

    range.write(this, m1->tick());

    // if there are some Time Signatures in joined measures, move last one to the next measure (if there is not one already)
    Segment* ts = inserted->last()->prev(SegmentType::TimeSig);
    if (ts && ts->rtick().isNotZero()) {
        for (Segment* insMSeg = inserted->last()->prev(SegmentType::TimeSig); insMSeg && insMSeg->rtick().isNotZero();
             insMSeg = insMSeg->prev(SegmentType::TimeSig)) {
            for (staff_idx_t staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                if (TimeSig* insTimeSig = toTimeSig(insMSeg->element(staffIdx * VOICES))) {
                    TimeSig* lts = nullptr;
                    for (EngravingObject* l : insTimeSig->linkList()) {
                        Score* score = l->score();
                        TimeSig* timeSig = toTimeSig(l);
                        Segment* tSeg = timeSig->segment();
                        track_idx_t track = timeSig->track();
                        Measure* sNext = next ? score->tick2measure(next->tick()) : nullptr;
                        Segment* nextTSeg = sNext ? sNext->undoGetSegmentR(SegmentType::TimeSig, Fraction(0, 1)) : nullptr;
                        if (sNext && !nextTSeg->element(track)) {
                            TimeSig* nsig = Factory::copyTimeSig(*timeSig);
                            nsig->setScore(score);
                            nsig->setTrack(track);
                            nsig->setParent(nextTSeg);

                            score->doUndoAddElement(nsig);

                            if (!lts) {
                                lts = nsig;
                            } else {
                                score->undo(new Link(lts, nsig));
                            }
                        }
                        score->doUndoRemoveElement(timeSig);
                        if (tSeg->empty()) {
                            score->doUndoRemoveElement(tSeg);
                        }
                    }
                }
            }
        }
    }
}
}
