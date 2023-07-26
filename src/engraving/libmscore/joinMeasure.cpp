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

#include "masterscore.h"
#include "measure.h"
#include "range.h"
#include "score.h"
#include "spanner.h"
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
    startCmd();

    deselectAll();

    ScoreRange range;
    range.read(m1->first(), m2->last());

    Fraction tick1 = m1->tick();
    Fraction tick2 = m2->endTick();

    auto spanners = m_spanner.findContained(tick1.ticks(), tick2.ticks());
    for (auto i : spanners) {
        undo(new RemoveElement(i.value));
    }

    for (auto i : spanner()) {
        Spanner* s = i.second;
        if (s->tick() >= tick1 && s->tick() < tick2) {
            s->setStartElement(0);
        }
        if (s->tick2() >= tick1 && s->tick2() < tick2) {
            s->setEndElement(0);
        }
    }

    deleteMeasures(m1, m2, true);

    MeasureBase* next = m2->next();
    const Fraction newTimesig = m1->timesig();
    Fraction newLen;
    for (Measure* mm = m1; mm; mm = mm->nextMeasure()) {
        newLen += mm->ticks();
        if (mm == m2) {
            break;
        }
    }

    InsertMeasureOptions options;
    options.createEmptyMeasures = true;

    insertMeasure(ElementType::MEASURE, next, options);
    // The loop since measures are not currently linked in MuseScore
    for (Score* s : masterScore()->scoreList()) {
        Measure* ins = s->tick2measure(tick1);
        ins->undoChangeProperty(Pid::TIMESIG_NOMINAL, newTimesig);
//             TODO: there was a commented chunk of code regarding setting bar
//             line types. Should we handle them here too?
//             m->setEndBarLineType(m2->endBarLineType(), m2->endBarLineGenerated(),
//             m2->endBarLineVisible(), m2->endBarLineColor());
    }
    Measure* inserted = (next ? next->prevMeasure() : lastMeasure());
    inserted->adjustToLen(newLen, /* appendRests... */ false);

    range.write(this, m1->tick());

    endCmd();
}
}
