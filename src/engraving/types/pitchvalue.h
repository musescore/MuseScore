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

#ifndef MU_ENGRAVING_PITCHVALUE_H
#define MU_ENGRAVING_PITCHVALUE_H

#include <QList>

namespace mu::engraving {
//---------------------------------------------------------
//   PitchValue
//    used in class Bend, SquareCanvas
//
//    - time is 0 - 60 for 0-100% of the chord duration the
//      bend is attached to
//    - pitch is 100 for one semitone
//---------------------------------------------------------

struct PitchValue {
    int time = 0;
    int pitch = 0;
    bool vibrato = false;
    PitchValue() = default;
    PitchValue(int a, int b, bool c = false)
        : time(a), pitch(b), vibrato(c) {}

    inline bool operator==(const PitchValue& pv) const { return pv.time == time && pv.pitch == pitch && pv.vibrato == vibrato; }
    inline bool operator!=(const PitchValue& pv) const { return !operator==(pv); }
};

using PitchValues = QList<PitchValue>;
}

//! NOTE compat
namespace Ms {
using PitchValue = mu::engraving::PitchValue;
using PitchValues = mu::engraving::PitchValues;
}

#endif // MU_ENGRAVING_PITCHVALUE_H
