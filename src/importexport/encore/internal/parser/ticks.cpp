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

// Implements the tick table and the 3:2 / 5:4 implied-tuplet probe.

#include "ticks.h"

namespace mu::iex::enc {
int faceValue2ticks(quint8 fv)
{
    switch (fv & 0x0F) {
    case 1: return 960;
    case 2: return 480;
    case 3: return 240;
    case 4: return 120;
    case 5: return 60;
    case 6: return 30;
    case 7: return 15;
    case 8: return 7;
    default: return 0;
    }
}

quint8 ticks2faceValue(int ticks)
{
    for (quint8 fv = 1; fv <= 8; ++fv) {
        if (faceValue2ticks(fv) <= ticks) {
            return fv;
        }
    }
    return 3;   // quarter fallback
}

int detectImpliedTuplet(qint16 realDur, quint8 fv, int& normalNotes)
{
    int base = faceValue2ticks(fv);
    if (base <= 0 || realDur <= 0) {
        normalNotes = 0;
        return 0;
    }
    // Triplet (3:2): realDuration = base * 2/3
    if (realDur * 3 == base * 2) {
        normalNotes = 2;
        return 3;
    }
    // Quintuplet (5:4): realDuration = base * 4/5
    if (realDur * 5 == base * 4) {
        normalNotes = 4;
        return 5;
    }
    normalNotes = 0;
    return 0;
}
} // namespace mu::iex::enc
