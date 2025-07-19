/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "stafftypechange.h"
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
        if (staves().at(staffIdx)->staffTypeRange(tick2).first > tick1.ticks()) {
            MScore::setError(MsError::CANNOT_JOIN_MEASURE_STAFFTYPE_CHANGE);
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

    std::map<Spanner*, Fraction> spannersEndingInRange;
    std::multimap<int, Spanner*> spannerMap = spanner();
    for (std::pair<int, Spanner*> i : spannerMap) {
        Spanner* s = i.second;
        if (s->tick() >= startTick && s->tick() < endTick) {
            s->setStartElement(0);
        }
        if (s->tick2() >= startTick && s->tick2() < endTick) {
            s->setEndElement(0);
        }
        // Maintain length of spanners starting outside and ending within join range
        if (s->tick() < startTick && s->tick2() < endTick) {
            spannersEndingInRange.emplace(s, s->tick2());
        }
        // Remove spanners starting within join range and ending outside
        // These will be replaced on range.write
        if (s->tick() > startTick && s->tick() < endTick && s->tick2() > endTick) {
            doUndoRemoveElement(s);
        }
    }

    MeasureBase* next = m2->next();
    Measure* deleteStart = tick2measure(startTick);
    Measure* deleteEnd = tick2measure(m2->tick());
    deleteMeasures(deleteStart, deleteEnd, true);
    InsertMeasureOptions options;
    options.createEmptyMeasures = true;
    options.moveSignaturesClef = false;
    options.moveStaffTypeChanges = false;
    options.ignoreBarLines = true;
    Measure* joinedMeasure = toMeasure(insertMeasure(next, options));

    for (auto spannerToFixup : spannersEndingInRange) {
        Spanner* sp = spannerToFixup.first;
        Fraction spEndTick = spannerToFixup.second;

        sp->setTick2(spEndTick);
    }

    for (size_t staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
        Staff* staff = staves().at(staffIdx);
        if (staff->isStaffTypeStartFrom(tick1)) {
            StaffTypeChange* stc = engraving::Factory::createStaffTypeChange(joinedMeasure);
            stc->setParent(joinedMeasure);
            stc->setTrack(staffIdx * VOICES);
            addElement(stc);
        }
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
