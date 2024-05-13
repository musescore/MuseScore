/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#ifndef MUSE_EXTENSIONS_API_QMLLISTPROPERTY_H
#define MUSE_EXTENSIONS_API_QMLLISTPROPERTY_H

#include <QQmlListProperty>

namespace muse::extensions::api {
template<typename T>
class QmlListProperty : public QQmlListProperty<T>
{
public:
    using AppendFunction = void (*)(QQmlListProperty<T>*, T*);
    using CountFunction = qsizetype (*)(QQmlListProperty<T>*);
    using AtFunction = T * (*)(QQmlListProperty<T>*, qsizetype);
    using ClearFunction = void (*)(QQmlListProperty<T>*);

    QmlListProperty(QObject* o, void* d, AppendFunction a, CountFunction c, AtFunction t,
                    ClearFunction r)
        : QQmlListProperty<T>(o, d, a, c, t, r)
    {}

    QmlListProperty(QObject* o, void* d, CountFunction c, AtFunction a)
        : QQmlListProperty<T>(o, d, c, a)
    {}
};
}

#endif // MUSE_EXTENSIONS_API_QMLLISTPROPERTY_H
