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

#include "editmeasurerepeat.h"

#include <vector>

#include "translation.h"

#include "../dom/barline.h"
#include "../dom/factory.h"
#include "../dom/measure.h"
#include "../dom/measurerepeat.h"
#include "../dom/rest.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/select.h"
#include "../dom/staff.h"
#include "../dom/utils.h"

#include "../infrastructure/messagebox.h"

using namespace mu;
using namespace mu::engraving;

//---------------------------------------------------------
//   addMeasureRepeat
//---------------------------------------------------------

void EditMeasureRepeat::addMeasureRepeat(Transaction& tx, Score* score, Measure* firstMeasure, int numMeasures, staff_idx_t staffIdx)
{
    //
    // make measures into group
    //
    if (!makeMeasureRepeatGroup(tx, score, firstMeasure, numMeasures, staffIdx)) {
        return;
    }

    //
    // add MeasureRepeat element
    //
    int measureWithElementNo;
    if (numMeasures % 2) {
        // odd number, element anchored to center measure of group
        measureWithElementNo = numMeasures / 2 + 1;
    } else {
        // even number, element anchored to last measure in first half of group
        measureWithElementNo = numMeasures / 2;
    }
    Measure* measureWithElement = firstMeasure;
    for (int i = 1; i < measureWithElementNo; ++i) {
        measureWithElement = measureWithElement->nextMeasure();
    }
    // MeasureRepeat element will be positioned appropriately (in center of measure / on following barline)
    // when stretchMeasure() is called on measureWithElement
    MeasureRepeat* mr = addMeasureRepeat(tx, score, measureWithElement->tick(), staff2track(staffIdx), numMeasures);
    score->select(mr, SelectType::SINGLE, 0);
}

//---------------------------------------------------------
//   the measures of a measure repeat group
//    empty if there are not enough of them, with equal durations
//---------------------------------------------------------

static std::vector<Measure*> groupMeasures(Measure* firstMeasure, int numMeasures)
{
    std::vector<Measure*> measures;
    Measure* measure = firstMeasure;
    for (int i = 1; i <= numMeasures; ++i) {
        if (!measure || measure->ticks() != firstMeasure->ticks()) {
            return {};
        }
        measures.push_back(measure);
        measure = measure->nextMeasure();
    }

    return measures;
}

//---------------------------------------------------------
//   whether there is something in the measures, deleted to make room for a measure repeat
//---------------------------------------------------------

static bool hasContentsToReplace(const std::vector<Measure*>& measures, staff_idx_t staffIdx)
{
    for (const Measure* m : measures) {
        if (m != measures.back()) {
            if (m->endBarLineType() != BarLineType::NORMAL) {
                return true;
            }
        }
        for (const Segment* seg = m->first(); seg; seg = seg->next()) {
            if (seg->segmentType() & SegmentType::ChordRest) {
                track_idx_t strack = staffIdx * VOICES;
                track_idx_t etrack = strack + VOICES;
                for (track_idx_t track = strack; track < etrack; ++track) {
                    const EngravingItem* e = seg->element(track);
                    if (e && !e->generated() && !e->isRest()) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool EditMeasureRepeat::addingReplacesContents(Measure* firstMeasure, int numMeasures, staff_idx_t staffIdx)
{
    const std::vector<Measure*> measures = groupMeasures(firstMeasure, numMeasures);
    return !measures.empty() && hasContentsToReplace(measures, staffIdx);
}

bool EditMeasureRepeat::droppingReplacesContents(EngravingItem* target, track_idx_t track, int numMeasures)
{
    if (!target) {
        return false;
    }

    if (target->isMeasure()) {
        if (track == muse::nidx) {
            return false;
        }
        return addingReplacesContents(toMeasure(target), numMeasures, track2staff(track));
    }

    if (target->isRest()) {
        Rest* rest = toRest(target);
        if (rest->durationType().type() != DurationType::V_MEASURE) {
            return false;
        }
        return addingReplacesContents(rest->measure(), numMeasures, rest->staffIdx());
    }

    return false;
}

//---------------------------------------------------------
//   makeMeasureRepeatGroup
///   clear measures, apply noBreak, set measureRepeatCount
///   returns false if these measures won't work or user aborted
//---------------------------------------------------------

bool EditMeasureRepeat::makeMeasureRepeatGroup(Transaction&, Score* score, Measure* firstMeasure, int numMeasures,
                                               staff_idx_t staffIdx)
{
    //
    // check that sufficient measures exist, with equal durations
    //
    std::vector<Measure*> measures = groupMeasures(firstMeasure, numMeasures);
    if (measures.empty()) {
        MScore::setError(MsError::INSUFFICIENT_MEASURES);
        return false;
    }

    //
    // warn user if things will have to be deleted to make room for measure repeat
    //
    if (hasContentsToReplace(measures, staffIdx)) {
        auto b = MessageBox(score->iocContext()).warning(muse::trc("engraving", "Current contents of measures will be replaced"),
                                                         muse::trc("engraving", "Continue with inserting measure repeat?"));
        if (b == MessageBox::Button::Cancel) {
            return false;
        }
    }

    //
    // group measures and clear current contents
    //

    score->deselectAll();
    int i = 1;
    for (auto m : measures) {
        score->select(m, SelectType::RANGE, staffIdx);
        if (m->isMeasureRepeatGroup(staffIdx)) {
            score->deleteItem(m->measureRepeatElement(staffIdx)); // reset measures related to an earlier MeasureRepeat
        }
        score->undoChangeMeasureRepeatCount(m, i++, staffIdx);
        if (m != measures.front()) {
            m->undoChangeProperty(Pid::REPEAT_START, false);
        }
        if (m != measures.back()) {
            m->undoSetNoBreak(true);
            Segment* seg = m->findSegmentR(SegmentType::EndBarLine, m->ticks());
            BarLine* endBarLine = toBarLine(seg->element(staff2track(staffIdx)));
            score->deleteItem(endBarLine); // also takes care of Pid::REPEAT_END
        }
    }
    score->cmdDeleteSelection();
    return true;
}

//---------------------------------------------------------
//   addMeasureRepeat
//    create one MeasureRepeat at tick of subtype numMeasures
//    create segment if necessary
//    does NOT set measureRepeatCount or do anything else with measure(s)!
//---------------------------------------------------------

MeasureRepeat* EditMeasureRepeat::addMeasureRepeat(Transaction&, Score* score, const Fraction& tick, track_idx_t track,
                                                   int numMeasures)
{
    Measure* measure = score->tick2measure(tick);
    MeasureRepeat* mr = Factory::createMeasureRepeat(score->dummy());
    mr->setNumMeasures(numMeasures);
    mr->setTicks(measure->stretchedLen(score->staff(track2staff(track))));
    mr->setTrack(track);
    score->undoAddCR(mr, measure, tick);
    return mr;
}
