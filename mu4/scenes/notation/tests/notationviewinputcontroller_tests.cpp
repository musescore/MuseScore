//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::Return;

#include "scenes/notation/view/notationviewinputcontroller.h"
#include "mocks/controlledviewmock.h"
#include "mocks/notationsceneconfigurationmock.h"

using namespace mu;
using namespace mu::scene::notation;
using namespace mu::framework;

class NotationViewInputControllerTests : public ::testing::Test
{
public:

    ~NotationViewInputControllerTests()
    {
        qDeleteAll(m_events);
    }

    struct Env {
        ControlledViewMock view;
        NotationViewInputController* controller = nullptr;
        std::shared_ptr<NotationSceneConfigurationMock> configuration;

        Env()
        {
            controller = new NotationViewInputController(&view);

            configuration = std::make_shared<NotationSceneConfigurationMock>();
            controller->setconfiguration(configuration);
        }

        ~Env()
        {
            delete controller;
        }
    };

    mutable QList<QInputEvent*> m_events;

    QWheelEvent* make_wheelEvent(QPoint pixelDelta,
                                 QPoint angleDelta,
                                 Qt::KeyboardModifiers modifiers = Qt::NoModifier,
                                 QPointF pos = QPointF(100, 100)) const
    {
        QPointF globalPos = pos;
        int qt4Delta = 60;
        Qt::Orientation qt4Orientation = Qt::Vertical;
        Qt::MouseButtons buttons = Qt::NoButton;

        QWheelEvent* ev = new QWheelEvent(pos,  globalPos,  pixelDelta,  angleDelta,
                                          qt4Delta, qt4Orientation, buttons, modifiers);

        m_events << ev;

        return ev;
    }
};

TEST_F(NotationViewInputControllerTests, WheelEvent_ScrollVertical)
{
    //! CASE Received wheel event, without modifiers
    Env env;

    //! GIVEN Has pixelDelta

    //! CHECK Should be called vertical scroll with value 180
    EXPECT_CALL(env.view, scrollVertical(180))
    .Times(1);

    env.controller->wheelEvent(make_wheelEvent(QPoint(0, 180), QPoint(0, 120)));

    //! CHECK Should be called vertical scroll with value 80
    EXPECT_CALL(env.view, scrollVertical(80))
    .Times(1);

    env.controller->wheelEvent(make_wheelEvent(QPoint(0, 80), QPoint(0, 120)));

    //! GIVEN pixelDelta is Null
    //!  dy = (angleDelta.y() * qMax(2.0, m_view->height() / 10.0)) / QWheelEvent::DefaultDeltasPerStep;

    //! CHECK Should be called vertical scroll with value 50  (dy = (120 * 500 / 10) / 120 = 50)
    EXPECT_CALL(env.view, scrollVertical(50))
    .Times(1);

    EXPECT_CALL(env.view, height())
    .WillOnce(Return(500));

    env.controller->wheelEvent(make_wheelEvent(QPoint(), QPoint(0, 120)));
}

TEST_F(NotationViewInputControllerTests, WheelEvent_ScrollHorizontal)
{
    //! CASE Received wheel event, with key modifier ShiftModifier
    Env env;

    //! CHECK Should be called vertical scroll with value 120
    EXPECT_CALL(env.view, scrollHorizontal(120))
    .Times(1);

    env.controller->wheelEvent(make_wheelEvent(QPoint(0, 120), QPoint(), Qt::ShiftModifier));
}

TEST_F(NotationViewInputControllerTests, WheelEvent_Zoom)
{
    //! CASE Received wheel event, with key modifier ControlModifier
    Env env;

    ValCh<int> currentZoom;
    currentZoom.val = 100;

    ON_CALL(*(env.configuration), currentZoom())
    .WillByDefault(Return(currentZoom));

    //! CHECK Should be called zoomStep with value 110
    EXPECT_CALL(env.view, setZoom(110, QPoint(100, 100)))
    .Times(1);

    EXPECT_CALL(env.view, height())
    .WillOnce(Return(500));

    EXPECT_CALL(env.view, toLogical(_))
    .WillOnce(Return(QPoint(100, 100)));

    env.controller->wheelEvent(make_wheelEvent(QPoint(), QPoint(0, 120), Qt::ControlModifier, QPointF(100, 100)));
}
