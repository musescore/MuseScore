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

    void updateElementList(const QList<Ms::Element*>& newRawElementList) override;
    QList<Ms::Element*> findElementsByType(const Ms::ElementType elementType) const override;
    QList<Ms::Element*> findElementsByType(const Ms::ElementType elementType,
                                           std::function<bool(const Ms::Element*)> filterFunc) const override;
    QList<Ms::Element*> takeAllElements() const override;

signals:
    void elementsUpdated() override;

private:
    QList<Ms::Element*> m_elementList;

    QList<Ms::Element*> exposeRawElements(const QList<Ms::Element*>& rawElementList) const;

    QList<Ms::Element*> findChords() const;
    QList<Ms::Element*> findNotes() const;
    QList<Ms::Element*> findStems() const;
    QList<Ms::Element*> findHooks() const;
    QList<Ms::Element*> findBeams() const;
    QList<Ms::Element*> findGlissandos() const;
    QList<Ms::Element*> findHairpins() const;
    QList<Ms::Element*> findStaffs() const;
    QList<Ms::Element*> findSectionBreaks() const;
    QList<Ms::Element*> findPedals() const;
    QList<Ms::Element*> findPairedClefs() const;
    QList<Ms::Element*> findTexts() const;
    QList<Ms::Element*> findTremolos() const;
};
}

#endif // MU_INSPECTOR_ELEMENTREPOSITORYSERVICE_H
