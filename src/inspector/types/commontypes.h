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
#ifndef MU_INSPECTOR_COMMONTYPES_H
#define MU_INSPECTOR_COMMONTYPES_H

#include "qobjectdefs.h"

#include "ui/view/iconcodes.h"
#include "dataformatter.h"

#include "engraving/types/types.h"

namespace mu::inspector {
struct ElementKey
{
    mu::engraving::ElementType type = mu::engraving::ElementType::INVALID;
    int subtype = -1;

    ElementKey() = default;

    ElementKey(mu::engraving::ElementType type, int subtype = -1)
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

    bool operator<(const ElementKey& other) const
    {
        return std::tie(type, subtype) < std::tie(other.type, other.subtype);
    }
};

using ElementKeySet = std::set<ElementKey>;

class CommonTypes
{
    Q_GADGET

public:
    enum class Placement {
        PLACEMENT_TYPE_ABOVE,
        PLACEMENT_TYPE_BELOW
    };

    enum class AutoOnOff {
        AUTO_ON_OFF_AUTO,
        AUTO_ON_OFF_ON,
        AUTO_ON_OFF_OFF,
    };

    Q_ENUM(AutoOnOff);
    Q_ENUM(Placement)
};

inline double formatDoubleFunc(const QVariant& elementPropertyValue)
{
    return muse::DataFormatter::roundDouble(elementPropertyValue.toDouble());
}

template<typename T>
inline QVariant object(T type, QString title, muse::ui::IconCode::Code iconCode = muse::ui::IconCode::Code::NONE)
{
    QVariantMap obj;
    obj["value"] = static_cast<int>(type);
    obj["text"] = title;
    obj["iconCode"] = static_cast<int>(iconCode);

    return obj;
}
}

#endif // MU_INSPECTOR_COMMONTYPES_H
