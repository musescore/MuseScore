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
#ifndef MU_INSPECTOR_COMMONTYPES_H
#define MU_INSPECTOR_COMMONTYPES_H

#include "qobjectdefs.h"

#include "dataformatter.h"
#include "libmscore/types.h"

namespace mu::inspector {
struct ElementKey
{
    Ms::ElementType type = Ms::ElementType::INVALID;
    int subtype = -1;

    ElementKey() = default;

    ElementKey(Ms::ElementType type, int subtype = -1)
        : type(type), subtype(subtype)
    {
    }

    bool operator==(const ElementKey& key) const
    {
        return type == key.type && subtype == key.subtype;
    }

    bool operator!=(const ElementKey& key) const
    {
        return !(*this == key);
    }
};

using ElementKeyList = QList<ElementKey>;
using ElementKeySet = QSet<ElementKey>;

inline uint qHash(const ElementKey& key)
{
    QString subtypePart = key.subtype >= 0 ? QString::number(key.subtype) : "";
    return qHash(QString::number(static_cast<int>(key.type)) + subtypePart);
}

class CommonTypes
{
    Q_GADGET

public:
    enum class Placement {
        PLACEMENT_TYPE_ABOVE,
        PLACEMENT_TYPE_BELOW
    };

    Q_ENUM(Placement)
};

inline double formatDoubleFunc(const QVariant& elementPropertyValue)
{
    return DataFormatter::roundDouble(elementPropertyValue.toDouble());
}
}

#endif // MU_INSPECTOR_COMMONTYPES_H
