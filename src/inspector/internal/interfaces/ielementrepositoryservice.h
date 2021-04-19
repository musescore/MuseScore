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
#ifndef MU_INSPECTOR_IELEMENTREPOSITORYSERVICE_H
#define MU_INSPECTOR_IELEMENTREPOSITORYSERVICE_H

#include "libmscore/element.h"
#include <QList>
#include <QObject>
#include <functional>

namespace mu::inspector {
class IElementRepositoryService
{
public:
    virtual ~IElementRepositoryService() = default;

    virtual QObject* getQObject() = 0;

    virtual void updateElementList(const QList<Ms::Element*>& newRawElementList) = 0;
    virtual QList<Ms::Element*> findElementsByType(const Ms::ElementType elementType) const = 0;
    virtual QList<Ms::Element*> findElementsByType(const Ms::ElementType elementType,
                                                   std::function<bool(const Ms::Element*)> filterFunc) const = 0;
    virtual QList<Ms::Element*> takeAllElements() const = 0;

signals:
    virtual void elementsUpdated() = 0;
};
}

#endif // MU_INSPECTOR_IELEMENTREPOSITORYSERVICE_H
