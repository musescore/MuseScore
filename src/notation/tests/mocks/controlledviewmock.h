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
#ifndef MU_NOTATION_CONTROLLEDVIEWMOCK_H
#define MU_NOTATION_CONTROLLEDVIEWMOCK_H

#include <gmock/gmock.h>

#include "notation/view/notationviewinputcontroller.h"

namespace mu::notation {
class ControlledViewMock : public IControlledView
{
public:

    MOCK_METHOD(qreal, width, (), (const, override));
    MOCK_METHOD(qreal, height, (), (const, override));

    MOCK_METHOD(muse::PointF, viewportTopLeft, (), (const, override));

    MOCK_METHOD(bool, moveCanvas, (qreal, qreal, CoordinateSystem, bool, bool));

    MOCK_METHOD(muse::RectF, notationContentRect, (), (const, override));
    MOCK_METHOD(qreal, currentScaling, (), (const, override));
    MOCK_METHOD(void, setScaling, (qreal, const muse::PointF&, bool), (override));

    MOCK_METHOD(muse::PointF, toLogical, (const muse::PointF&), (const, override));
    MOCK_METHOD(muse::PointF, toLogical, (const QPointF&), (const, override));
    MOCK_METHOD(muse::PointF, fromLogical, (const muse::PointF&), (const, override));
    MOCK_METHOD(muse::RectF, fromLogical, (const muse::RectF&), (const, override));

    MOCK_METHOD(bool, isNoteEnterMode, (), (const, override));
    MOCK_METHOD(void, showShadowNote, (const muse::PointF&), (override));

    MOCK_METHOD(void, showContextMenu, (const ElementType&, const QPointF&), (override));
    MOCK_METHOD(void, hideContextMenu, (), (override));

    MOCK_METHOD(void, showElementPopup, (const ElementType&, const muse::RectF&), (override));
    MOCK_METHOD(void, hideElementPopup, (const ElementType& elementType), (override));
    MOCK_METHOD(void, toggleElementPopup, (const ElementType&, const muse::RectF&), (override));

    MOCK_METHOD(bool, elementPopupIsOpen, (const ElementType&), (const, override));

    MOCK_METHOD(INotationInteractionPtr, notationInteraction, (), (const, override));
    MOCK_METHOD(INotationPlaybackPtr, notationPlayback, (), (const, override));

    MOCK_METHOD(QQuickItem*, asItem, (), (override));
};
}

#endif // MU_NOTATION_CONTROLLEDVIEWMOCK_H
