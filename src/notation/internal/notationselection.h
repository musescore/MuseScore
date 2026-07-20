/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "../inotationselection.h"

#include "igetscore.h"
#include "../notationtypes.h"
#include "../inotationelements.h"

namespace mu::notation {
//! NOTE Helper interface,
// which contains interaction methods called in the selection.
// This may not be necessary after refactoring,
// or it could be implemented differently.
class IInteractionForSelection
{
public:

    virtual ~IInteractionForSelection() = default;

    virtual void showItem(const mu::engraving::EngravingItem* item, int staffIndex = -1) = 0;
    virtual void resetHitElementContext() = 0;

    virtual bool isTextEditingStarted() const = 0;
    virtual void selectAllText() = 0;
    virtual bool isEditingElement() const = 0;
    virtual void navigateToNearText(MoveDirection direction) = 0;
    virtual void endEditElement() = 0;
    virtual void startEditElement(EngravingItem* element) = 0;

    virtual INotationElementsPtr elements() const = 0;
};

class NotationSelection : public INotationSelection
{
public:
    NotationSelection(IGetScore* getScore, IInteractionForSelection* interaction);

    bool isNone() const override;
    bool isRange() const override;
    SelectionState state() const override;

    muse::Ret canCopy() const override;
    muse::ByteArray mimeData() const override;
    QMimeData* qMimeData() const override;

    engraving::EngravingItem* element() const override;
    const std::vector<engraving::EngravingItem*>& elements() const override;

    std::vector<engraving::Note*> notes(NoteFilter filter) const override;

    muse::RectF canvasBoundingRect() const override;

    INotationSelectionRangePtr range() const override;

    engraving::EngravingItem* lastElementHit() const override;

    void onElementHit(engraving::EngravingItem*);

    mu::engraving::MeasureBase* startMeasureBase() const override;
    mu::engraving::MeasureBase* endMeasureBase() const override;
    std::vector<mu::engraving::System*> selectedSystems() const override;
    std::vector<mu::engraving::Page*> pagesContainingSelection() const override;

    bool elementsSelected(const mu::engraving::ElementTypeSet& types) const override;

    void select(SelectionTarget target);
    void select(const std::vector<EngravingItem*>& elements, SelectType type = SelectType::REPLACE, staff_idx_t staffIndex = 0);
    void clearSelection();
    muse::async::Notification selectionChanged() const;
    void notifyAboutSelectionChangedIfNeed();
    void moveSelection(MoveDirection d, MoveSelectionType type);

private:
    mu::engraving::Score* score() const;

    void doSelect(const std::vector<EngravingItem*>& elements, SelectType type, engraving::staff_idx_t staffIndex = 0);

    void selectElementsWithSameTypeOnSegment(mu::engraving::ElementType elementType, mu::engraving::Segment* segment);
    void selectFirstElement(bool frame = false);
    void selectLastElement();
    void moveElementSelection(MoveDirection d);
    void moveStringSelection(MoveDirection d);
    void moveChordNoteSelection(MoveDirection d);
    void selectTopOrBottomOfChord(MoveDirection d);
    void selectAllSimilarElements();
    void selectAllSimilarElementsInStaff();
    void selectAllSimilarElementsInRange();
    void selectAllNotesInChord();
    FilterElementsOptions elementsFilterOptions(const EngravingItem* element) const;
    void selectAll();
    void selectSection();

    engraving::EngravingItem* m_lastElementHit = nullptr;
    IGetScore* m_getScore = nullptr;
    IInteractionForSelection* m_interaction = nullptr;
    INotationSelectionRangePtr m_range;
    muse::async::Notification m_selectionChanged;
};
}
