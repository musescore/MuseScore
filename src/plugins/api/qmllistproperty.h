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

#ifndef MU_PLUGINS_QMLLISTPROPERTY_H
#define MU_PLUGINS_QMLLISTPROPERTY_H

#include <QQmlListProperty>

namespace mu::plugins {
template<typename T>
class QmlListProperty : public QQmlListProperty<T>
{
public:
    using AppendFunction = void (*)(QQmlListProperty<T>*, T*);
    using CountFunction = qsizetype (*)(QQmlListProperty<T>*);
    using AtFunction = T * (*)(QQmlListProperty<T>*, qsizetype);
    using ClearFunction = void (*)(QQmlListProperty<T>*);

#ifdef MU_QT5_COMPAT
    static int count_qt5(QQmlListProperty<T>* list)
    {
        QmlListProperty<T>* me = static_cast<QmlListProperty<T>*>(list);
        return static_cast<int>(me->count(list));
    }

    static T* at_qt5(QQmlListProperty<T>* list, int index)
    {
        QmlListProperty<T>* me = static_cast<QmlListProperty<T>*>(list);
        return me->at(list, static_cast<int>(index));
    }

#endif

    QmlListProperty(QObject* o, void* d, AppendFunction a, CountFunction c, AtFunction t,
                    ClearFunction r)
#ifndef MU_QT5_COMPAT
        : QQmlListProperty<T>(o, d, a, c, t, r)
#else
        : QQmlListProperty<T>(o, d, a, &count_qt5, &at_qt5, r), count(c), at(t)
#endif
    {}

    QmlListProperty(QObject* o, void* d, CountFunction c, AtFunction a)
#ifndef MU_QT5_COMPAT
        : QQmlListProperty<T>(o, d, c, a)
#else
        : QQmlListProperty<T>(o, d, &count_qt5, &at_qt5), count(c), at(a)
#endif
    {}

#ifdef MU_QT5_COMPAT
    CountFunction count;
    AtFunction at;
#endif
};
}

#endif // MU_PLUGINS_QMLLISTPROPERTY_H
