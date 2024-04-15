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
#ifndef MU_INSPECTOR_BENDTYPES_H
#define MU_INSPECTOR_BENDTYPES_H

#include "qobjectdefs.h"

#include <QVariantMap>

namespace mu::inspector {
class BendTypes
{
    Q_GADGET

public:
    enum class ShowHoldLine {
        SHOW_HOLD_AUTO = 0,
        SHOW_HOLD_SHOW  = 1,
        SHOW_HOLD_HIDE = 2,
    };

    Q_ENUM(ShowHoldLine)
};

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
    bool limitMoveVerticallyByNearestPoints = true;

    bool endDashed = false;
    QString name;

    bool generated = false;

    CurvePoint() = default;
    CurvePoint(int time, int pitch, const QList<MoveDirection>& moveDirection = {}, bool endDashed = false, const QString& name = QString(),
               bool limitMoveVerticallyByNearestPoints = true)
        : time(time), pitch(pitch), moveDirection(moveDirection),
        limitMoveVerticallyByNearestPoints(limitMoveVerticallyByNearestPoints), endDashed(endDashed), name(name) {}
    CurvePoint(int time, int pitch, bool generated, bool endDashed = false)
        : time(time), pitch(pitch), endDashed(endDashed), generated(generated) {}

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
        map["limitMoveVerticallyByNearestPoints"] = limitMoveVerticallyByNearestPoints;

        map["endDashed"] = endDashed;
        map["name"] = name;
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
        point.limitMoveVerticallyByNearestPoints = map["limitMoveVerticallyByNearestPoints"].toBool();

        point.endDashed = map["endDashed"].toBool();
        point.name = map["name"].toString();
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
