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
#ifndef MU_NOTATION_INOTATIONSELECTION_H
#define MU_NOTATION_INOTATIONSELECTION_H

#include <vector>

#include "notationtypes.h"
#include "internal/inotationselectionrange.h"
#include "types/ret.h"

class QMimeData;

namespace mu::notation {
class INotationSelection
{
public:
    virtual ~INotationSelection() = default;

    virtual bool isNone() const = 0;
    virtual bool isRange() const = 0;
    virtual SelectionState state() const = 0;

    virtual muse::Ret canCopy() const = 0;
    virtual QMimeData* mimeData() const = 0;

    virtual EngravingItem* element() const = 0;
    virtual const std::vector<EngravingItem*>& elements() const = 0;

    virtual std::vector<Note*> notes(NoteFilter filter = NoteFilter::All) const = 0;

    virtual muse::RectF canvasBoundingRect() const = 0;

    virtual INotationSelectionRangePtr range() const = 0;

    virtual EngravingItem* lastElementHit() const = 0;

    virtual mu::engraving::MeasureBase* startMeasureBase() const = 0;
    virtual mu::engraving::MeasureBase* endMeasureBase() const = 0;
    virtual std::vector<mu::engraving::System*> selectedSystems() const = 0;
};

using INotationSelectionPtr = std::shared_ptr<INotationSelection>;
}

#endif // MU_NOTATION_INOTATIONSELECTION_H
