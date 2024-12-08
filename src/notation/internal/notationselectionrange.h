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
#ifndef MU_NOTATION_NOTATIONSELECTIONRANGE_H
#define MU_NOTATION_NOTATIONSELECTIONRANGE_H

#include "inotationselectionrange.h"

#include "igetscore.h"

namespace mu::engraving {
class Score;
}

namespace mu::notation {
class NotationSelectionRange : public INotationSelectionRange
{
public:
    NotationSelectionRange(IGetScore* getScore);

    engraving::staff_idx_t startStaffIndex() const override;
    engraving::Segment* rangeStartSegment() const override;
    Fraction startTick() const override;

    engraving::staff_idx_t endStaffIndex() const override;
    engraving::Segment* rangeEndSegment() const override;
    Fraction endTick() const override;

    MeasureRange measureRange() const override;

    std::vector<const Part*> selectedParts() const override;

    std::vector<muse::RectF> boundingArea() const override;
    bool containsPoint(const muse::PointF& point) const override;
    bool containsItem(const EngravingItem* item) const override;

private:
    mu::engraving::Score* score() const;

    IGetScore* m_getScore = nullptr;
};
}

#endif // MU_NOTATION_NOTATIONSELECTIONRANGE_H
