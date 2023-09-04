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

#include "swing.h"

#include "realfn.h"

#include "chord.h"
#include "measure.h"
#include "navigate.h"
#include "staff.h"

using namespace mu::engraving;

bool Swing::ChordDurationAdjustment::isNull() const
{
    return RealIsNull(remainingDurationMultiplier) && RealIsNull(durationMultiplier);
}

//---------------------------------------------------------
//   isSubdivided
//   Check for subdivided beat
//---------------------------------------------------------

static bool isSubdivided(const ChordRest* chord, int swingUnit)
{
    if (!chord) {
        return false;
    }

    const ChordRest* prev = prevChordRest(chord);
    if (chord->actualTicks().ticks() < swingUnit || (prev && prev->actualTicks().ticks() < swingUnit)) {
        return true;
    }

    return false;
}

void Swing::swingAdjustParams(const Chord* chord, const SwingParameters& params, int& onTime, int& gateTime)
{
    Fraction tick = chord->rtick();

    // adjust for anacrusis
    const Measure* cm = chord->measure();
    const MeasureBase* pm = cm->prev();
    ElementType pt = pm ? pm->type() : ElementType::INVALID;
    if (!pm || pm->lineBreak() || pm->pageBreak() || pm->sectionBreak()
        || pt == ElementType::VBOX || pt == ElementType::HBOX
        || pt == ElementType::FBOX || pt == ElementType::TBOX) {
        Fraction offset = cm->timesig() - cm->ticks();
        if (offset > Fraction(0, 1)) {
            tick += offset;
        }
    }

    int swingBeat            = params.swingUnit * 2;
    double ticksDuration     = (double)chord->actualTicks().ticks();
    double swingTickAdjust   = ((double)swingBeat) * (((double)(params.swingRatio - 50)) / 100.0);
    double swingActualAdjust = (swingTickAdjust / ticksDuration) * 1000.0;
    int startTick            = tick.ticks();

    //Check the position of the chord to apply changes accordingly
    if (startTick % swingBeat == params.swingUnit) {
        if (!isSubdivided(chord, params.swingUnit)) {
            onTime = onTime + swingActualAdjust;
            gateTime = (200 - (gateTime + (swingActualAdjust / 10)));
        }
    }

    int endTick = startTick + ticksDuration;
    if (endTick % swingBeat == params.swingUnit) {
        const ChordRest* ncr = nextChordRest(chord);

        if (!isSubdivided(ncr, params.swingUnit)) {
            gateTime = gateTime + (swingActualAdjust / 10);
        }
    }
}

Swing::ChordDurationAdjustment Swing::applySwing(const Chord* chord, const SwingParameters& params)
{
    int onTime = 0;
    int gateTime = 100;

    swingAdjustParams(chord, params, onTime, gateTime);

    if (gateTime <= 0) {
        gateTime = 100;
    }

    ChordDurationAdjustment result;
    result.remainingDurationMultiplier = static_cast<double>(onTime) / 1000.;
    result.durationMultiplier = static_cast<double>(gateTime) / 100.;

    return result;
}
