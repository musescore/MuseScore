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
#ifndef MU_NOTATION_INOTATIONSELECTIONRANGE_H
#define MU_NOTATION_INOTATIONSELECTIONRANGE_H

#include <vector>

#include "../notationtypes.h"

namespace mu::notation {
class INotationSelectionRange
{
public:
    virtual ~INotationSelectionRange() = default;

    virtual engraving::staff_idx_t startStaffIndex() const = 0;
    virtual engraving::Segment* rangeStartSegment() const = 0;
    virtual Fraction startTick() const = 0;

    virtual engraving::staff_idx_t endStaffIndex() const = 0;
    virtual engraving::Segment* rangeEndSegment() const = 0;
    virtual Fraction endTick() const = 0;

    struct MeasureRange {
        Measure* startMeasure = nullptr;
        Measure* endMeasure = nullptr;
    };

    virtual MeasureRange measureRange() const = 0;

    virtual std::vector<const Part*> selectedParts() const = 0;

    virtual std::vector<muse::RectF> boundingArea() const = 0;
    virtual bool containsPoint(const muse::PointF& point) const = 0;
    virtual bool containsItem(const EngravingItem* item, engraving::staff_idx_t staffIdx = muse::nidx) const = 0;

    virtual bool containsMultiNoteChords() const = 0;
};

using INotationSelectionRangePtr = std::shared_ptr<INotationSelectionRange>;
}

#endif // MU_NOTATION_INOTATIONSELECTIONRANGE_H
