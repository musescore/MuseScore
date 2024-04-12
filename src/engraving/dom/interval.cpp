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

#include "interval.h"
#include "../types/types.h"
#include "utils.h"

namespace mu::engraving {
//---------------------------------------------------------
//   Interval
//---------------------------------------------------------

Interval::Interval()
    : diatonic(0), chromatic(0)
{
}

Interval::Interval(int a, int b)
    : diatonic(a), chromatic(b)
{
}

Interval::Interval(int c)
{
    chromatic = c;
    diatonic = chromatic2diatonic(c);
}

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void Interval::flip()
{
    diatonic = -diatonic;
    chromatic = -chromatic;
}

//---------------------------------------------------------
//   isZero
//---------------------------------------------------------

bool Interval::isZero() const
{
    return diatonic == 0 && chromatic == 0;
}

Interval Interval::fromOrnamentInterval(OrnamentInterval ornInt)
{
    Interval resultingInterval = Interval(0, 0);

    resultingInterval.diatonic = static_cast<int>(ornInt.step);

    int cromaticSteps = 0;
    switch (ornInt.step) {
    case IntervalStep::UNISON:
        cromaticSteps = 0;
        break;
    case IntervalStep::SECOND:
        cromaticSteps = 2;
        break;
    case IntervalStep::THIRD:
        cromaticSteps = 4;
        break;
    case IntervalStep::FOURTH:
        cromaticSteps = 5;
        break;
    case IntervalStep::FIFTH:
        cromaticSteps = 7;
        break;
    case IntervalStep::SIXTH:
        cromaticSteps = 9;
        break;
    case IntervalStep::SEVENTH:
        cromaticSteps = 11;
        break;
    case IntervalStep::OCTAVE:
        cromaticSteps = 12;
        break;
    default:
        break;
    }

    if (ornInt.isPerfect()) {
        switch (ornInt.type) {
        case IntervalType::DIMINISHED:
            cromaticSteps -= 1;
            break;
        case IntervalType::AUGMENTED:
            cromaticSteps += 1;
            break;
        default:
            break;
        }
    } else {
        switch (ornInt.type) {
        case IntervalType::DIMINISHED:
            cromaticSteps -= 2;
            break;
        case IntervalType::MINOR:
            cromaticSteps -= 1;
            break;
        case IntervalType::AUGMENTED:
            cromaticSteps += 1;
            break;
        default:
            break;
        }
    }

    resultingInterval.chromatic = cromaticSteps;

    return resultingInterval;
}
}
