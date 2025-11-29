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

#pragma once

#include "ielementrepositoryservice.h"

namespace mu::inspector {
class ElementRepositoryService : public IElementRepositoryService
{
public:
    ElementRepositoryService() = default;

    bool needUpdateElementList(const QList<mu::engraving::EngravingItem*>& newRawElementList,
                               engraving::SelState selectionState) const override;
    void updateElementList(const QList<mu::engraving::EngravingItem*>& newRawElementList, engraving::SelState selectionState) override;

    QList<mu::engraving::EngravingItem*> findElementsByType(const mu::engraving::ElementType elementType) const override;
    QList<mu::engraving::EngravingItem*> findElementsByType(const mu::engraving::ElementType elementType,
                                                            std::function<bool(const mu::engraving::EngravingItem*)> filterFunc) const
    override;
    QList<mu::engraving::EngravingItem*> takeAllElements() const override;

    muse::async::Channel<QList<mu::engraving::EngravingItem*> > elementsUpdated() const override;

private:
    QList<mu::engraving::EngravingItem*> exposeRawElements(const QList<mu::engraving::EngravingItem*>& rawElementList) const;

    QList<mu::engraving::EngravingItem*> findChords() const;
    QList<mu::engraving::EngravingItem*> findNotes() const;
    QList<mu::engraving::EngravingItem*> findElementsForNotes() const;
    QList<mu::engraving::EngravingItem*> findNoteHeads() const;
    QList<mu::engraving::EngravingItem*> findStems() const;
    QList<mu::engraving::EngravingItem*> findHooks() const;
    QList<mu::engraving::EngravingItem*> findBeams() const;
    QList<mu::engraving::EngravingItem*> findStaffs() const;
    QList<mu::engraving::EngravingItem*> findSectionBreaks() const;
    QList<mu::engraving::EngravingItem*> findTexts() const;
    std::vector<mu::engraving::EngravingItem*> findTextDelegates(mu::engraving::EngravingItem* element) const;
    QList<mu::engraving::EngravingItem*> findBrackets() const;
    QList<mu::engraving::EngravingItem*> findLines(mu::engraving::ElementType lineType) const;
    QList<mu::engraving::EngravingItem*> findRests() const;
    QList<mu::engraving::EngravingItem*> findOrnaments() const;
    QList<mu::engraving::EngravingItem*> findLyrics() const;

    QList<mu::engraving::EngravingItem*> m_exposedElementList;
    QList<mu::engraving::EngravingItem*> m_rawElementList;
    mu::engraving::SelState m_selectionState = mu::engraving::SelState::NONE;

    muse::async::Channel<QList<mu::engraving::EngravingItem*> > m_elementsUpdated;
};
}
