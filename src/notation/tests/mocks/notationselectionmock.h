/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

    MOCK_METHOD(mu::Ret, canCopy, (), (const, override));
    MOCK_METHOD(QMimeData*, mimeData, (), (const, override));

    MOCK_METHOD(EngravingItem*, element, (), (const, override));
    MOCK_METHOD(std::vector<EngravingItem*>, elements, (), (const, override));

    MOCK_METHOD(std::vector<Note*>, notes, (NoteFilter filter), (const, override));

    MOCK_METHOD(RectF, canvasBoundingRect, (), (const, override));

    MOCK_METHOD(INotationSelectionRangePtr, range, (), (const, override));

    MOCK_METHOD(EngravingItem*, lastElementHit, (), (const, override));
};
}

#endif // MU_NOTATION_NOTATIONSELECTIONMOCK_H
