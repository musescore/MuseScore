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

#ifndef MU_ENGRAVING_APIV1_ENUMS_H
#define MU_ENGRAVING_APIV1_ENUMS_H

#include <QQmlPropertyMap>
#include <QMetaEnum>

namespace mu::engraving::apiv1 {
//---------------------------------------------------------
///   \class Enum
///   Wrapper for enumerations
//---------------------------------------------------------

class Enum : public QQmlPropertyMap
{
    Q_OBJECT

public:
    /// \cond MS_INTERNAL
    Enum(const QMetaEnum& _enum, QObject* parent = nullptr);

    void add(const QMetaEnum& en);

    /// \endcond
};

//---------------------------------------------------------
//    wrapEnum
///   \cond PLUGIN_API \private \endcond
///   \relates Enum
//---------------------------------------------------------

template<class T>
Enum* wrapEnum(QObject* parent = nullptr)
{
    return new Enum(QMetaEnum::fromType<T>(), parent);
}
}

#endif
