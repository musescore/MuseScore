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
// Convert Encore tick/xoffset coordinates to MuseScore Fractions and snapped tick positions.

#include "coords.h"

#include <cstdlib>
#include <limits>

#include "../parser/elem.h"
#include "../parser/ticks.h"

using namespace mu::engraving;

namespace mu::iex::enc {
int encWholeNoteTicks(const EncMeasure& measure)
{
    if (measure.durTicks && measure.timeSigNum && measure.timeSigDen) {
        return (static_cast<int>(measure.durTicks) * static_cast<int>(measure.timeSigDen))
               / static_cast<int>(measure.timeSigNum);
    }
    return kEncWholeTicks;
}

Fraction snapStartTickByXoffset(Fraction defaultTick, const EncMeasure& encMeas,
                                int staffIdx, int ornXoffset, Fraction measTick)
{
    const int wholeTicks = encWholeNoteTicks(encMeas);
    const Fraction relTick = defaultTick - measTick;
    const int defaultEncTick = (relTick.numerator() * wholeTicks)
                               / std::max(1, relTick.denominator());

    // xoffset of the note/rest at the element's own tick (any voice on the staff).
    int defaultCrXoff = -1;
    forEachStaffNoteXoff(encMeas, staffIdx, /*includeRests*/ true, /*lineSlotByRawByte*/ nullptr,
                         [&](const EncMeasureElem* em, int xoff) {
        if (static_cast<int>(em->tick) != defaultEncTick) {
            return true;
        }
        defaultCrXoff = xoff;
        return false;   // first match at that tick wins
    });
    // Glyph sits at or after its own note: trust the tick.
    if (defaultCrXoff >= 0 && ornXoffset >= defaultCrXoff) {
        return defaultTick;
    }
    // Glyph drawn left of the note: snap back to the latest preceding chord/rest whose
    // xoffset is <= the glyph xoffset. Also covers elements stored at durTicks (no CR there).
    int bestTick = -1;
    forEachStaffNoteXoff(encMeas, staffIdx, /*includeRests*/ true, /*lineSlotByRawByte*/ nullptr,
                         [&](const EncMeasureElem* em, int xoff) {
        if (static_cast<int>(em->tick) >= defaultEncTick) {
            return true;
        }
        if (xoff <= ornXoffset && static_cast<int>(em->tick) > bestTick) {
            bestTick = static_cast<int>(em->tick);
        }
        return true;
    });
    if (bestTick < 0) {
        return defaultTick;
    }
    return measTick + Fraction(bestTick, wholeTicks);
}
} // namespace mu::iex::enc
