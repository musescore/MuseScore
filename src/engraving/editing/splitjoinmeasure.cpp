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

#include "splitjoinmeasure.h"

#include "dom/chordrest.h"
#include "dom/factory.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/range.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/spanner.h"
#include "dom/staff.h"
#include "dom/stafftypechange.h"
#include "dom/timesig.h"

#include "addremoveelement.h"
#include "editspanner.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   splitMeasure
//---------------------------------------------------------

void SplitJoinMeasure::splitMeasure(MasterScore* score, const Fraction& tick)
{
    Segment* segment = score->tick2segment(tick, false, SegmentType::ChordRest);

    if (segment->rtick().isZero()) {
        MScore::setError(MsError::CANNOT_SPLIT_MEASURE_FIRST_BEAT);
        return;
    }
    if (segment->splitsTuplet()) {
        MScore::setError(MsError::CANNOT_SPLIT_MEASURE_TUPLET);
        return;
    }

    Measure* measure = score->tick2measure(tick);

    for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        if (measure->isMeasureRepeatGroup(static_cast<int>(staffIdx))) {
            MScore::setError(MsError::CANNOT_SPLIT_MEASURE_REPEAT);
            return;
        }
    }

    ScoreRange range;
    range.read(measure->first(), measure->last());

    Fraction stick = measure->tick();
    Fraction etick = measure->endTick();

    std::list<std::tuple<Spanner*, Fraction, Fraction> > sl;
    for (auto i : score->spanner()) {
        Spanner* s = i.second;
        EngravingItem* start = s->startElement();
        EngravingItem* end = s->endElement();
        if (s->tick() >= stick && s->tick() < etick) {
            start = nullptr;
        }
        if (s->tick2() >= stick && s->tick2() < etick) {
            end = nullptr;
        }
        if (start != s->startElement() || end != s->endElement()) {
            sl.push_back(std::make_tuple(s, s->tick(), s->ticks()));
            score->undo(new ChangeStartEndSpanner(s, start, end));
        }
        if (s->tick() < stick && s->tick2() > stick) {
            sl.push_back(std::make_tuple(s, s->tick(), s->ticks()));
        }
    }

    MeasureBase* nm = measure->next();

    // create empty measures:
    Score::InsertMeasureOptions options;
    options.createEmptyMeasures = true;
    options.moveSignaturesClef = false;
    options.moveStaffTypeChanges = false;
    options.ignoreBarLines = true;

    score->insertMeasure(nm, options);
    Measure* m2 = toMeasure(nm ? nm->prev() : score->lastMeasure());
    score->insertMeasure(m2, options);
    Measure* m1 = toMeasure(m2->prev());

    for (Score* s : score->scoreList()) {
        Measure* m = s->tick2measure(tick);
        s->undoRemoveMeasures(m, m, true, false);
    }
    score->undoInsertTime(measure->tick(), -measure->ticks());

    m1->setTick(measure->tick());
    m2->setTick(tick);

    Fraction ticks1 = tick - measure->tick();
    Fraction ticks2 = measure->ticks() - ticks1;

    m1->setTimesig(measure->timesig());
    m2->setTimesig(measure->timesig());

    ticks1.reduce();
    ticks2.reduce();
    // Now make sure this reduction doesn't go 'beyond' the original measure's
    // actual denominator for both resultant measures.
    if (ticks1.denominator() < measure->ticks().denominator()) {
        if (measure->ticks().denominator() % m1->timesig().denominator() == 0) {
            int mult = measure->ticks().denominator() / ticks1.denominator();
            // *= operator automatically reduces via GCD, so rather do literal multiplication:
            ticks1.setDenominator(ticks1.denominator() * mult);
            ticks1.setNumerator(ticks1.numerator() * mult);
        }
    }
    if (ticks2.denominator() < measure->ticks().denominator()) {
        if (measure->ticks().denominator() % m2->timesig().denominator() == 0) {
            int mult = measure->ticks().denominator() / ticks2.denominator();
            ticks2.setDenominator(ticks2.denominator() * mult);
            ticks2.setNumerator(ticks2.numerator() * mult);
        }
    }
    if (ticks1.denominator() > 128 || ticks2.denominator() > 128) {
        MScore::setError(MsError::CANNOT_SPLIT_MEASURE_TOO_SHORT);
        return;
    }

    m1->adjustToLen(ticks1, false);
    m2->adjustToLen(ticks2, false);

    // set correct barline types if needed
    for (Score* s : score->scoreList()) {
        Measure* sM1 = s->tick2measure(m1->tick());
        Measure* sM2 = s->tick2measure(m2->tick());
        // TODO: handle other end barline types; they may differ per staff
        if (measure->endBarLineType() == BarLineType::END_REPEAT) {
            sM2->undoChangeProperty(Pid::REPEAT_END, true);
        }
        if (measure->getProperty(Pid::REPEAT_START).toBool()) {
            sM1->undoChangeProperty(Pid::REPEAT_START, true);
        }
    }

    range.write(score, m1->tick());

    for (auto i : sl) {
        Spanner* s      = std::get<0>(i);
        Fraction t      = std::get<1>(i);
        Fraction ticks  = std::get<2>(i);
        if (s->tick() != t) {
            s->undoChangeProperty(Pid::SPANNER_TICK, t);
        }
        if (s->ticks() != ticks) {
            s->undoChangeProperty(Pid::SPANNER_TICKS, ticks);
        }
    }

    for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        Staff* staff = score->staves().at(staffIdx);
        if (staff->isStaffTypeStartFrom(stick)) {
            StaffTypeChange* stc = Factory::createStaffTypeChange(m1);
            stc->setParent(m1);
            stc->setTrack(staffIdx * VOICES);
            score->addElement(stc);
        }
    }
}

//---------------------------------------------------------
//   joinMeasure
//---------------------------------------------------------

void SplitJoinMeasure::joinMeasures(MasterScore* score, const Fraction& tick1, const Fraction& tick2)
{
    Measure* m1 = score->tick2measure(tick1);
    Measure* m2 = score->tick2measure(tick2);

    if (!m1 || !m2) {
        return;
    }

    for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        if (m1->isMeasureRepeatGroupWithPrevM(static_cast<int>(staffIdx))
            || m2->isMeasureRepeatGroupWithNextM(static_cast<int>(staffIdx))) {
            MScore::setError(MsError::CANNOT_SPLIT_MEASURE_REPEAT);
            return;
        }
        if (score->staves().at(staffIdx)->staffTypeRange(tick2).first > tick1.ticks()) {
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

    score->deselectAll();

    ScoreRange range;
    range.read(m1->first(), m2->last());

    Fraction startTick = m1->tick();
    Fraction endTick = m2->endTick();

    auto spanners = score->spannerMap().findContained(startTick.ticks(), endTick.ticks());
    for (auto i : spanners) {
        score->doUndoRemoveElement(i.value);
    }

    std::map<Spanner*, Fraction> spannersEndingInRange;
    std::multimap<int, Spanner*> spannerMap = score->spanner();
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
            score->doUndoRemoveElement(s);
        }
    }

    MeasureBase* next = m2->next();
    score->deleteMeasures(m1, m2, true);

    Score::InsertMeasureOptions options;
    options.createEmptyMeasures = true;
    options.moveSignaturesClef = false;
    options.moveStaffTypeChanges = false;
    options.ignoreBarLines = true;
    Measure* joinedMeasure = toMeasure(score->insertMeasure(next, options));

    for (auto spannerToFixup : spannersEndingInRange) {
        Spanner* sp = spannerToFixup.first;
        Fraction spEndTick = spannerToFixup.second;

        sp->setTick2(spEndTick);
    }

    for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        Staff* staff = score->staves().at(staffIdx);
        if (staff->isStaffTypeStartFrom(tick1)) {
            StaffTypeChange* stc = engraving::Factory::createStaffTypeChange(joinedMeasure);
            stc->setParent(joinedMeasure);
            stc->setTrack(staffIdx * VOICES);
            score->addElement(stc);
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
    for (Score* s : score->scoreList()) {
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
    Measure* inserted = (next ? next->prevMeasure() : score->lastMeasure());
    inserted->adjustToLen(newLen, /* appendRests... */ false);

    range.write(score, m1->tick());

    // if there are some Time Signatures in joined measures, move last one to the next measure (if there is not one already)
    Segment* ts = inserted->last()->prev(SegmentType::TimeSig);
    if (ts && ts->rtick().isNotZero()) {
        for (Segment* insMSeg = inserted->last()->prev(SegmentType::TimeSig); insMSeg && insMSeg->rtick().isNotZero();
             insMSeg = insMSeg->prev(SegmentType::TimeSig)) {
            for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
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
