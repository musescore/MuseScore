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

// Resolve Encore face value and MIDI ticks into MuseScore duration types, dot counts and tuplet ratios.

#include "durations.h"

#include <cstdlib>
#include <utility>

#include "../parser/ticks.h"

using namespace mu::engraving;

namespace mu::iex::enc {
DurationType faceValue2DurationType(quint8 fv)
{
    switch (fv & 0x0F) {
    case 1: return DurationType::V_WHOLE;
    case 2: return DurationType::V_HALF;
    case 3: return DurationType::V_QUARTER;
    case 4: return DurationType::V_EIGHTH;
    case 5: return DurationType::V_16TH;
    case 6: return DurationType::V_32ND;
    case 7: return DurationType::V_64TH;
    case 8: return DurationType::V_128TH;
    default: return DurationType::V_QUARTER;
    }
}

// Reject rdur-based dot promotion when rdur is inflated by gap-to-next-event, not a real dotted value.
static bool inflatedDottedPromotion(qint16 realDur, quint8 fv)
{
    int faceTicks = faceValue2ticks(fv);
    return faceTicks > 0 && realDur > faceTicks && calcDots(realDur, fv) == 0;
}

DurationType realDuration2DurationType(qint16 realDur, quint8 fv)
{
    if (realDur <= 0) {
        return faceValue2DurationType(fv);
    }
    // Multi-stream MIDI overlap can shorten rdur below the written value; trust face value when rdur < faceTicks.
    const int faceTicks = faceValue2ticks(fv);
    if (faceTicks > 0 && realDur < faceTicks) {
        return faceValue2DurationType(fv);
    }
    // rdur above faceTicks but not a dotted augmentation is a trailing gap to the next event;
    // trust the written face value.
    if (inflatedDottedPromotion(realDur, fv)) {
        return faceValue2DurationType(fv);
    }
    switch (realDur) {
    case 960: return DurationType::V_WHOLE;
    case 480: return DurationType::V_HALF;
    case 240: return DurationType::V_QUARTER;
    case 120: return DurationType::V_EIGHTH;
    case  60: return DurationType::V_16TH;
    case  30: return DurationType::V_32ND;
    case  15: return DurationType::V_64TH;
    case 720: return DurationType::V_HALF;
    case 360: return DurationType::V_QUARTER;
    case 180: return DurationType::V_EIGHTH;
    case  90: return DurationType::V_16TH;
    case  45: return DurationType::V_32ND;
    // Triplet rdur (160/80/40...) falls through; detectImpliedTuplet handles them.
    // Mapping to longer type misrepresents notes not in a tuplet context.
    default:  return faceValue2DurationType(fv);
    }
}

int calcDots(qint16 realDur, quint8 fv)
{
    int base = faceValue2ticks(fv);
    if (base <= 0 || realDur <= 0 || realDur == base) {
        return 0;
    }
    // Only match when the dotted value is integral: truncated division (e.g. 112.5 to 112) would
    // falsely match a note whose rdur equals the truncated value, so the divisibility check gates it.
    if ((base * 3) % 2 == 0 && realDur == (base * 3) / 2) {
        return 1;
    }
    if ((base * 7) % 4 == 0 && realDur == (base * 7) / 4) {
        return 2;
    }
    if ((base * 15) % 8 == 0 && realDur == (base * 15) / 8) {
        return 3;
    }
    return 0;
}

int calcDotsSnap(qint16 dur, quint8 fv)
{
    static constexpr int DOTS_SNAP_TOL = 1;

    int base = faceValue2ticks(fv);
    if (base <= 0 || dur <= 0) {
        return 0;
    }
    auto near = [](int a, int b) { return std::abs(a - b) <= DOTS_SNAP_TOL; };
    if (near(dur, base)) {
        return 0;
    }
    if ((base * 3) % 2 == 0 && near(dur, (base * 3) / 2)) {
        return 1;
    }
    if ((base * 7) % 4 == 0 && near(dur, (base * 7) / 4)) {
        return 2;
    }
    if ((base * 15) % 8 == 0 && near(dur, (base * 15) / 8)) {
        return 3;
    }
    return 0;
}

Fraction dottedAdvance(DurationType durationType, int dots)
{
    Fraction multiplier = Fraction(1, 1);
    if (dots == 1) {
        multiplier = Fraction(3, 2);
    } else if (dots == 2) {
        multiplier = Fraction(7, 4);
    } else if (dots >= 3) {
        multiplier = Fraction(15, 8);
    }
    return TDuration(durationType).fraction() * multiplier;
}

int computeDotCount(quint8 dotControl, qint16 realDuration, quint8 faceValue, bool useBit0Fallback)
{
    if (dotControl > 0) {
        const int dByCtrl = calcDots(static_cast<qint16>(dotControl), faceValue);
        if (dByCtrl > 0) {
            return dByCtrl;
        }
        const int dBySnap = calcDotsSnap(realDuration, faceValue);
        if (dBySnap > 0) {
            return dBySnap;
        }
        if (useBit0Fallback && (dotControl & 1)) {
            // A dotted note lasts longer than its face value, so only force a dot when rdur >
            // faceTicks; otherwise the bit-0 flag is a spurious layout bit, not a dot indicator.
            const int faceTicks = faceValue2ticks(faceValue);
            if (faceTicks > 0 && realDuration > faceTicks) {
                return 1;
            }
        }
        return 0;
    }
    return calcDotsSnap(realDuration, faceValue);
}

bool isStandardExplicitTuplet(int actualN, int normalN)
{
    if (actualN < 2 || normalN < 1) {
        return false;
    }
    // normalN of 10/15/20 gives a fraction not representable as a TDuration; reject them.
    if (normalN == 10 || normalN == 15 || normalN == 20) {
        return false;
    }
    // TDuration-aligned ratios whose normalN is in {1,2,3,5,7} (normalN in {4,6,8} is handled by
    // the blanket rule below).
    static const std::pair<int, int> kStandardRatios[] = {
        { 3, 2 }, { 4, 3 }, { 2, 1 }, { 2, 3 }, { 5, 2 }, { 5, 3 }, { 6, 7 }, { 9, 5 }, { 4, 1 }, { 4, 2 },
    };
    for (const auto& r : kStandardRatios) {
        if (actualN == r.first && normalN == r.second) {
            return true;
        }
    }
    // normalN in {4,6,8} always yields a standard TDuration for 2..64-tuplets.
    if ((normalN == 4 || normalN == 6 || normalN == 8)
        && actualN >= 2 && actualN <= 64) {
        return true;
    }
    return false;
}

bool isCompoundBeat(quint16 rawBeatTicks, Fraction timesig)
{
    // Explicit dotted-quarter beat, or a compound x/8 meter (6/8, 9/8, 12/8, ...) whose legacy
    // files still store beatTicks=240.
    return rawBeatTicks == 360
           || (timesig.denominator() == 8
               && timesig.numerator() % 3 == 0
               && timesig.numerator() > 3);
}
} // namespace mu::iex::enc
