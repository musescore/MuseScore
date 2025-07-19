/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#ifndef MU_NOTATION_NOTATIONSELECTIONRANGEMOCK_H
#define MU_NOTATION_NOTATIONSELECTIONRANGEMOCK_H

#include <gmock/gmock.h>

#include "notation/internal/inotationselectionrange.h"

namespace mu::notation {
class NotationSelectionRangeMock : public INotationSelectionRange
{
public:
    MOCK_METHOD(engraving::staff_idx_t, startStaffIndex, (), (const, override));
    MOCK_METHOD(engraving::Segment*, rangeStartSegment, (), (const, override));
    MOCK_METHOD(Fraction, startTick, (), (const, override));

    MOCK_METHOD(engraving::staff_idx_t, endStaffIndex, (), (const, override));
    MOCK_METHOD(engraving::Segment*, rangeEndSegment, (), (const, override));
    MOCK_METHOD(Fraction, endTick, (), (const, override));

    MOCK_METHOD(MeasureRange, measureRange, (), (const, override));

    MOCK_METHOD(std::vector<const Part*>, selectedParts, (), (const, override));

    MOCK_METHOD(std::vector<muse::RectF>, boundingArea, (), (const, override));
    MOCK_METHOD(bool, containsPoint, (const muse::PointF&), (const, override));
    MOCK_METHOD(bool, containsItem, (const engraving::EngravingItem*, engraving::staff_idx_t), (const, override));

    MOCK_METHOD(bool, containsMultiNoteChords, (), (const, override));
};
}

#endif // MU_NOTATION_NOTATIONSELECTIONRANGEMOCK_H
