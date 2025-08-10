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
#ifndef MU_NOTATION_NOTATIONSELECTION_H
#define MU_NOTATION_NOTATIONSELECTION_H

#include "../inotationselection.h"

#include "igetscore.h"

namespace mu::engraving {
class MeasureBase;
class Score;
class System;
}

namespace mu::notation {
class NotationSelection : public INotationSelection
{
public:
    NotationSelection(IGetScore* getScore);

    bool isNone() const override;
    bool isRange() const override;
    SelectionState state() const override;

    muse::Ret canCopy() const override;
    QMimeData* mimeData() const override;

    EngravingItem* element() const override;
    const std::vector<EngravingItem*>& elements() const override;

    std::vector<Note*> notes(NoteFilter filter) const override;

    muse::RectF canvasBoundingRect() const override;

    INotationSelectionRangePtr range() const override;

    EngravingItem* lastElementHit() const override;

    void onElementHit(EngravingItem*);

    mu::engraving::MeasureBase* startMeasureBase() const override;
    mu::engraving::MeasureBase* endMeasureBase() const override;
    std::vector<mu::engraving::System*> selectedSystems() const override;

    bool elementsSelected(const mu::engraving::ElementTypeSet& types) const override;

private:
    mu::engraving::Score* score() const;
    EngravingItem* m_lastElementHit;

    IGetScore* m_getScore = nullptr;
    INotationSelectionRangePtr m_range;
};
}

#endif // MU_NOTATION_NOTATIONSELECTION_H
