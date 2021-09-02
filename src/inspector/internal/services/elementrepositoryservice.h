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
#ifndef MU_INSPECTOR_ELEMENTREPOSITORYSERVICE_H
#define MU_INSPECTOR_ELEMENTREPOSITORYSERVICE_H

#include "internal/interfaces/ielementrepositoryservice.h"

#include <QObject>

namespace mu::inspector {
class ElementRepositoryService : public QObject, public IElementRepositoryService
{
    Q_OBJECT
public:
    explicit ElementRepositoryService(QObject* parent);

    QObject* getQObject() override;

    void updateElementList(const QList<Ms::EngravingItem*>& newRawElementList) override;
    QList<Ms::EngravingItem*> findElementsByType(const Ms::ElementType elementType) const override;
    QList<Ms::EngravingItem*> findElementsByType(const Ms::ElementType elementType,
                                                 std::function<bool(const Ms::EngravingItem*)> filterFunc) const override;
    QList<Ms::EngravingItem*> takeAllElements() const override;

signals:
    void elementsUpdated() override;

private:
    QList<Ms::EngravingItem*> m_elementList;

    QList<Ms::EngravingItem*> exposeRawElements(const QList<Ms::EngravingItem*>& rawElementList) const;

    QList<Ms::EngravingItem*> findChords() const;
    QList<Ms::EngravingItem*> findNotes() const;
    QList<Ms::EngravingItem*> findStems() const;
    QList<Ms::EngravingItem*> findHooks() const;
    QList<Ms::EngravingItem*> findBeams() const;
    QList<Ms::EngravingItem*> findGlissandos() const;
    QList<Ms::EngravingItem*> findHairpins() const;
    QList<Ms::EngravingItem*> findStaffs() const;
    QList<Ms::EngravingItem*> findSectionBreaks() const;
    QList<Ms::EngravingItem*> findPedals() const;
    QList<Ms::EngravingItem*> findPairedClefs() const;
    QList<Ms::EngravingItem*> findTexts() const;
    QList<Ms::EngravingItem*> findTremolos() const;
};
}

#endif // MU_INSPECTOR_ELEMENTREPOSITORYSERVICE_H
