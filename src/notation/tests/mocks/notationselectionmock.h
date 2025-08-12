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
#ifndef MU_NOTATION_NOTATIONSELECTIONMOCK_H
#define MU_NOTATION_NOTATIONSELECTIONMOCK_H

#include <gmock/gmock.h>

#include "notation/inotationselection.h"

namespace mu::notation {
class NotationSelectionMock : public INotationSelection
{
public:
    MOCK_METHOD(bool, isNone, (), (const, override));
    MOCK_METHOD(bool, isRange, (), (const, override));
    MOCK_METHOD(SelectionState, state, (), (const, override));

    MOCK_METHOD(muse::Ret, canCopy, (), (const, override));
    MOCK_METHOD(muse::ByteArray, mimeData, (), (const, override));
    MOCK_METHOD(QMimeData*, qMimeData, (), (const, override));

    MOCK_METHOD(EngravingItem*, element, (), (const, override));
    MOCK_METHOD(const std::vector<EngravingItem*>&, elements, (), (const, override));

    MOCK_METHOD(std::vector<Note*>, notes, (NoteFilter filter), (const, override));

    MOCK_METHOD(muse::RectF, canvasBoundingRect, (), (const, override));

    MOCK_METHOD(INotationSelectionRangePtr, range, (), (const, override));

    MOCK_METHOD(EngravingItem*, lastElementHit, (), (const, override));

    MOCK_METHOD(mu::engraving::MeasureBase*, startMeasureBase, (), (const, override));
    MOCK_METHOD(mu::engraving::MeasureBase*, endMeasureBase, (), (const, override));
    MOCK_METHOD(std::vector<System*>, selectedSystems, (), (const, override));

    MOCK_METHOD(bool, elementsSelected, (const mu::engraving::ElementTypeSet&), (const, override));
};
}

#endif // MU_NOTATION_NOTATIONSELECTIONMOCK_H
