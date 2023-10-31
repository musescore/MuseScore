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
#ifndef MU_INSPECTOR_BENDTYPES_H
#define MU_INSPECTOR_BENDTYPES_H

#include "qobjectdefs.h"

#include <QVariantMap>

namespace mu::inspector {
struct CurvePoint
{
    enum class MoveDirection {
        None,
        Horizontal,
        Vertical,
        Both
    };

    // time is 0 - 60 for 0-100% of the chord duration
    static constexpr int MAX_TIME = 60;

    int time = 0;
    int pitch = 0;

    QList<MoveDirection> moveDirection;
    bool endDashed = false;

    bool generated = false;

    CurvePoint() = default;
    CurvePoint(int time, int pitch, const QList<MoveDirection>& moveDirection = {}, bool endDashed = false, bool generated = false)
        : time(time), pitch(pitch), moveDirection(moveDirection), endDashed(endDashed), generated(generated) {}
    CurvePoint(int time, int pitch, bool generated)
        : time(time), pitch(pitch), generated(generated) {}

    bool canMove(MoveDirection direction = MoveDirection::None) const
    {
        switch (direction) {
        case MoveDirection::Both:
            return moveDirection.contains(MoveDirection::Both);
        case MoveDirection::None:
            return !moveDirection.empty();
        default:
            return moveDirection.contains(MoveDirection::Both) || moveDirection.contains(direction);
        }
    }

    inline bool operator==(const CurvePoint& o) const
    {
        return o.time == time && o.pitch == pitch && o.moveDirection == moveDirection && o.endDashed == endDashed;
    }

    inline bool operator!=(const CurvePoint& o) const { return !operator==(o); }

    QVariantMap toMap() const
    {
        QVariantMap map;
        map["time"] = time;
        map["pitch"] = pitch;

        QVariantList directions;
        for (MoveDirection direction : moveDirection) {
            directions << static_cast<int>(direction);
        }
        map["moveDirection"] = directions;

        map["endDashed"] = endDashed;
        map["generated"] = generated;

        return map;
    }

    static CurvePoint fromMap(const QVariant& obj)
    {
        QVariantMap map = obj.toMap();

        CurvePoint point;
        point.time = map["time"].toInt();
        point.pitch = map["pitch"].toInt();

        QList<MoveDirection> directions;
        for (const QVariant& var : map["moveDirection"].toList()) {
            directions << static_cast<MoveDirection>(var.toInt());
        }
        point.moveDirection = directions;

        point.endDashed = map["endDashed"].toBool();
        point.generated = map["generated"].toBool();

        return point;
    }
};

using CurvePoints = QList<CurvePoint>;

inline QVariant curvePointsToQVariant(const CurvePoints& points)
{
    QVariantList list;
    for (const CurvePoint& value : points) {
        list << value.toMap();
    }

    return list;
}

inline CurvePoints curvePointsFromQVariant(const QVariant& var)
{
    CurvePoints points;
    for (const QVariant& obj : var.toList()) {
        points.push_back(CurvePoint::fromMap(obj));
    }

    return points;
}
}

#endif // MU_INSPECTOR_BENDTYPES_H
