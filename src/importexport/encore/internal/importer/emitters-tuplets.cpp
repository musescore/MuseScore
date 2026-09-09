/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// TupletTracker: build and close MuseScore tuplets as notes are emitted.

#include "emitters-tuplets.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/mscore.h"
#include "../parser/ticks.h"

using namespace mu::engraving;

namespace mu::iex::enc {
bool TupletTracker::groupFull() const
{
    // Close when accumulated face values reach actualN × baseLen.
    // >= handles both standard groups ({8,8,8}: exact) and mixed-duration groups ({16,16,Q}: exceeds threshold).
    return inTuplet() && fullFaceSum > Fraction(0, 1) && faceTicks >= fullFaceSum;
}

// True when a Fraction fits exactly in a TDuration (power-of-two, up to 4 dots).
// Guards tuplet setTicks: beam layout calls TDuration(ticks, truncate=false) and asserts on non-fitting fractions.
bool fitsTDuration(const Fraction& f)
{
    if (f.numerator() <= 0) {
        return false;
    }
    TDuration snap(f, true /*truncate*/);
    return snap.isValid() && snap.fraction() == f;
}

// Finish the current tuplet group and reset the per-group accumulators for the next one.
void TupletTracker::closeTuplet()
{
    if (currentTuplet && placedTicks > Fraction(0, 1)) {
        const Fraction expected = TDuration(currentTuplet->baseLen()).fraction()
                                  * currentTuplet->ratio().denominator();
        if (currentTuplet->ticks() == Fraction(0, 1)) {
            // Beam layout builds TDuration(tuplet->ticks()) and asserts on a fraction that is
            // not a real duration, so never store a placed length that does not fit a TDuration:
            // snap a non-fitting partial length down to the nearest representable duration.
            const Fraction t = fitsTDuration(placedTicks)
                               ? placedTicks
                               : TDuration(placedTicks, true /*truncate*/).fraction();
            if (t > Fraction(0, 1)) {
                currentTuplet->setTicks(t);
            }
        } else {
            const bool mixedValueOvershoot = (placedTicks > expected)
                                             && (faceTicks > fullFaceSum);
            const bool willSet = (placedTicks < expected || mixedValueOvershoot)
                                 && fitsTDuration(placedTicks);
            if (willSet) {
                currentTuplet->setTicks(placedTicks);
            }
        }
    }
    currentTuplet = nullptr;
    actualN = 0;
    normalN = 0;
    placedTicks = Fraction(0, 1);
    faceTicks   = Fraction(0, 1);
    fullFaceSum = Fraction(0, 1);
}

// Create a MuseScore Tuplet for an [aN:normalN_] group at tick and begin tracking it.
Tuplet* TupletTracker::startTuplet(Measure* measure, Fraction tick,
                                   int aN, int normalN_, DurationType baseType, track_idx_t track_)
{
    closeTuplet();
    currentTuplet = Factory::createTuplet(measure);
    currentTuplet->setRatio(Fraction(aN, normalN_));
    currentTuplet->setBaseLen(TDuration(baseType));
    currentTuplet->setTick(tick);
    currentTuplet->setTrack(track_);
    Fraction tupletDuration = TDuration(baseType).fraction() * normalN_;
    if (fitsTDuration(tupletDuration)) {
        currentTuplet->setTicks(tupletDuration);
    }
    measure->add(currentTuplet);
    actualN = aN;
    normalN = normalN_;
    placedTicks = Fraction(0, 1);
    faceTicks   = Fraction(0, 1);
    fullFaceSum = TDuration(baseType).fraction() * aN;
    return currentTuplet;
}

// Tick advance contributed by one tuplet member of the given base duration (scaled by the ratio).
Fraction TupletTracker::noteAdvance(DurationType baseType) const
{
    if (!inTuplet()) {
        return TDuration(baseType).fraction();
    }
    return TDuration(baseType).fraction() * Fraction(normalN, actualN);
}
} // namespace mu::iex::enc
