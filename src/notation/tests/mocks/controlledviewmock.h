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
#ifndef MU_NOTATION_CONTROLLEDVIEWMOCK_H
#define MU_NOTATION_CONTROLLEDVIEWMOCK_H

#include <gmock/gmock.h>

#include "notation/view/notationviewinputcontroller.h"

namespace mu {
namespace notation {
class ControlledViewMock : public IControlledView
{
public:

    MOCK_METHOD(qreal, width, (), (const, override));
    MOCK_METHOD(qreal, height, (), (const, override));
    MOCK_METHOD(qreal, scale, (), (const, override));
    MOCK_METHOD(QPoint, toLogical, (const QPoint& p), (const, override));

    MOCK_METHOD(void, moveCanvas, (int dx, int dy), (override));
    MOCK_METHOD(void, scrollVertical, (int dy), (override));
    MOCK_METHOD(void, scrollHorizontal, (int dx), (override));
    MOCK_METHOD(void, setZoom, (int, const QPoint&), (override));

    MOCK_METHOD(bool, isNoteEnterMode, (), (const, override));
    MOCK_METHOD(void, showShadowNote, (const QPointF& pos), (override));

    MOCK_METHOD(INotationInteraction*, notationInteraction, (), (const, override));
    MOCK_METHOD(INotationPlayback*, notationPlayback, (), (const, override));
};
}
}

#endif // MU_NOTATION_CONTROLLEDVIEWMOCK_H
