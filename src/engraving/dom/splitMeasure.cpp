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

#include "chordrest.h"
#include "measure.h"
#include "range.h"
#include "score.h"
#include "segment.h"
#include "spanner.h"
#include "tie.h"
#include "undo.h"
#include "utils.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   cmdSplitMeasure
//---------------------------------------------------------

void Score::cmdSplitMeasure(ChordRest* cr)
{
    startCmd();
    splitMeasure(cr->segment());
    endCmd();
}

//---------------------------------------------------------
//   splitMeasure
//---------------------------------------------------------

void Score::splitMeasure(Segment* segment)
{
    if (segment->rtick().isZero()) {
        MScore::setError(MsError::CANNOT_SPLIT_MEASURE_FIRST_BEAT);
        return;
    }
    if (segment->splitsTuplet()) {
        MScore::setError(MsError::CANNOT_SPLIT_MEASURE_TUPLET);
        return;
    }
    Measure* measure = segment->measure();
    for (size_t staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
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
    for (auto i : spanner()) {
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
            undo(new ChangeStartEndSpanner(s, start, end));
        }
        if (s->tick() < stick && s->tick2() > stick) {
            sl.push_back(std::make_tuple(s, s->tick(), s->ticks()));
        }
    }

    // Make sure ties to the beginning of the split measure are restored.
    std::vector<Tie*> ties;
    for (size_t track = 0; track < ntracks(); track++) {
        Chord* chord = measure->findChord(stick, static_cast<int>(track));
        if (chord) {
            for (Note* note : chord->notes()) {
                Tie* tie = note->tieBack();
                if (tie) {
                    ties.push_back(tie->clone());
                }
            }
        }
    }

    MeasureBase* nm = measure->next();

    // create empty measures:
    InsertMeasureOptions options;
    options.createEmptyMeasures = true;
    options.moveSignaturesClef = false;

    insertMeasure(ElementType::MEASURE, nm, options);
    Measure* m2 = toMeasure(nm ? nm->prev() : lastMeasure());
    insertMeasure(ElementType::MEASURE, m2, options);
    Measure* m1 = toMeasure(m2->prev());

    undoRemoveMeasures(measure, measure, true);
    undoInsertTime(measure->tick(), -measure->ticks());

    Fraction tick = segment->tick();
    m1->setTick(measure->tick());
    m2->setTick(tick);
    Fraction ticks1 = segment->tick() - measure->tick();
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
    range.write(this, m1->tick());

    // Restore ties to the beginning of the split measure.
    for (auto tie : ties) {
        tie->setEndNote(searchTieNote(tie->startNote()));
        undoAddElement(tie);
    }

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
}
}
