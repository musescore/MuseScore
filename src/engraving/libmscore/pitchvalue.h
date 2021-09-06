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

#ifndef __PITCHVALUE_H__
#define __PITCHVALUE_H__

#include <QVariant>

namespace Ms {
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
    PitchValue() {}
    PitchValue(int a, int b, bool c = false)
        : time(a), pitch(b), vibrato(c) {}
    inline bool operator==(const PitchValue& pv) const
    {
        return pv.time == time && pv.pitch == pitch && pv.vibrato == vibrato;
    }
};

inline QVariant pitchValuesToVariant(const QList<PitchValue>& values)
{
    QVariantList result;

    for (const PitchValue& value : values) {
        result << QVariant::fromValue(value);
    }

    return result;
}

inline QList<PitchValue> pitchValuesFromVariant(const QVariant& values)
{
    QList<PitchValue> result;

    for (const QVariant& value : values.toList()) {
        result << value.value<PitchValue>();
    }

    return result;
}
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::PitchValue)

#endif
