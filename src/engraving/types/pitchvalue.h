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

#ifndef MU_ENGRAVING_PITCHVALUE_H
#define MU_ENGRAVING_PITCHVALUE_H

#include <vector>

#ifndef NO_QT_SUPPORT
#include <QVariant>
#endif

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
    static constexpr int MAX_TIME = 60;
    static constexpr int PITCH_FOR_SEMITONE = 100;
    int time = 0;
    int pitch = 0;
    bool vibrato = false;
    PitchValue() = default;
    PitchValue(int a, int b, bool c = false)
        : time(a), pitch(b), vibrato(c) {}

    inline bool operator==(const PitchValue& pv) const { return pv.time == time && pv.pitch == pitch && pv.vibrato == vibrato; }
    inline bool operator!=(const PitchValue& pv) const { return !operator==(pv); }

#ifndef NO_QT_SUPPORT
    QVariant toQVariant() const
    {
        QVariantMap map;
        map["time"] = time;
        map["pitch"] = pitch;
        map["vibrato"] = vibrato;

        return map;
    }

    static PitchValue fromQVariant(const QVariant& obj)
    {
        QVariantMap map = obj.toMap();

        PitchValue value;
        value.time = map["time"].toInt();
        value.pitch = map["pitch"].toInt();
        value.vibrato = map["vibrato"].toBool();

        return value;
    }

#endif
};

using PitchValues = std::vector<PitchValue>;

#ifndef NO_QT_SUPPORT
inline QVariant pitchValuesToQVariant(const PitchValues& values)
{
    QVariantList list;
    for (const PitchValue& value : values) {
        list << value.toQVariant();
    }

    return list;
}

inline PitchValues pitchValuesFromQVariant(const QVariant& var)
{
    PitchValues values;
    for (const QVariant& obj : var.toList()) {
        values.push_back(PitchValue::fromQVariant(obj));
    }

    return values;
}

#endif
}

#endif // MU_ENGRAVING_PITCHVALUE_H
