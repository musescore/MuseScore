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

#ifndef __PLUGIN_API_ENUMS_H__
#define __PLUGIN_API_ENUMS_H__

#include <QQmlPropertyMap>
#include <QMetaEnum>

namespace mu::engraving {
namespace PluginAPI {
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
} // namespace PluginAPI
} // namespace Ms
#endif
