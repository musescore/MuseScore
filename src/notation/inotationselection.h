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
#ifndef MU_NOTATION_INOTATIONSELECTION_H
#define MU_NOTATION_INOTATIONSELECTION_H

#include <vector>
#include <QRectF>
#include <QMimeData>

#include "notationtypes.h"
#include "internal/inotationselectionrange.h"

namespace mu::notation {
class INotationSelection
{
public:
    virtual ~INotationSelection() = default;

    virtual bool isNone() const = 0;
    virtual bool isRange() const = 0;
    virtual SelectionState state() const = 0;

    virtual bool canCopy() const = 0;
    virtual QMimeData* mimeData() const = 0;

    virtual EngravingItem* element() const = 0;
    virtual std::vector<EngravingItem*> elements() const = 0;

    virtual std::vector<Note*> notes(NoteFilter filter = NoteFilter::All) const = 0;

    virtual RectF canvasBoundingRect() const = 0;

    virtual INotationSelectionRangePtr range() const = 0;

    virtual EngravingItem* lastElementHit() const = 0;
};

using INotationSelectionPtr = std::shared_ptr<INotationSelection>;
}

#endif // MU_NOTATION_INOTATIONSELECTION_H
