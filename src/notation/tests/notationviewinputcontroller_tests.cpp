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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mocks/controlledviewmock.h"
#include "mocks/notationconfigurationmock.h"
#include "mocks/notationinteractionmock.h"
#include "mocks/notationselectionmock.h"
#include "playback/tests/mocks/playbackcontrollermock.h"

#include "engraving/tests/utils/scorerw.h"

#include "notation/view/notationviewinputcontroller.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

using namespace mu;
using namespace mu::notation;
using namespace mu::framework;

static const String TEST_SCORE_PATH(u"data/test.mscx");

class NotationViewInputControllerTests : public ::testing::Test
{
public:

    void SetUp() override
    {
        m_interaction = std::make_shared<NotationInteractionMock>();

        m_selection = std::make_shared<NotationSelectionMock>();
        ON_CALL(*m_interaction, selection())
        .WillByDefault(Return(m_selection));

        ON_CALL(m_view, notationInteraction())
        .WillByDefault(Return(m_interaction));

        ON_CALL(m_view, currentScaling())
        .WillByDefault(Return(1));

        m_controller = new NotationViewInputController(&m_view);

        m_configuration = std::make_shared<NotationConfigurationMock>();
        m_controller->setconfiguration(m_configuration);

        m_playbackController = std::make_shared<playback::PlaybackControllerMock>();
        m_controller->setplaybackController(m_playbackController);

        setNoWaylandForLinux();
    }

    void TearDown() override
    {
        qDeleteAll(m_events);
        delete m_controller;
    }

    NotationViewInputController* m_controller = nullptr;
    ControlledViewMock m_view;
    std::shared_ptr<NotationConfigurationMock> m_configuration;
    std::shared_ptr<NotationInteractionMock> m_interaction;
    std::shared_ptr<NotationSelectionMock> m_selection;
    std::shared_ptr<playback::PlaybackControllerMock > m_playbackController;

    mutable QList<QInputEvent*> m_events;

    QWheelEvent* make_wheelEvent(QPoint pixelDelta,
                                 QPoint angleDelta,
                                 Qt::KeyboardModifiers modifiers = Qt::NoModifier,
                                 QPointF pos = QPointF(100, 100)) const
    {
        QPointF globalPos = pos;
        Qt::MouseButtons buttons = Qt::NoButton;

        QWheelEvent* ev = new QWheelEvent(pos,  globalPos,  pixelDelta,  angleDelta,
                                          buttons, modifiers, Qt::ScrollPhase::NoScrollPhase, false);

        m_events << ev;

        return ev;
    }

    QMouseEvent* make_mousePressEvent(
        Qt::MouseButton button,
        Qt::KeyboardModifiers modifiers = Qt::NoModifier,
        QPointF pos = QPointF(100, 100)) const
    {
        QMouseEvent* ev = new QMouseEvent(QMouseEvent::Type::MouseButtonPress, pos, button, {}, modifiers);

        m_events << ev;

        return ev;
    }

    INotationInteraction::HitElementContext hitContext(engraving::MasterScore* score, bool start) const
    {
        INotationInteraction::HitElementContext context;

        Measure* firstMeasure = score->firstMeasure();
        Segment* segment = start ? firstMeasure->segments().firstCRSegment() : firstMeasure->segments().last();
        ChordRest* firstChord = segment->nextChordRest(0, !start);

        context.element = engraving::toChord(firstChord)->notes().front();
        context.staff = context.element->staff();

        return context;
    }

    void setNoWaylandForLinux()
    {
#ifdef Q_OS_LINUX
        //! [GIVEN] No Wayland display
        setenv("WAYLAND_DISPLAY", "OFF", false);
#endif
    }
};

/**
 * @brief WheelEvent_ScrollVertical
 * @details Received wheel event, without modifiers
 */
TEST_F(NotationViewInputControllerTests, WheelEvent_ScrollVertical)
{
    //! [THEN] Should be called vertical scroll with value 180
    EXPECT_CALL(m_view, moveCanvas(0, 180))
    .Times(1);

    //! [WHEN] User scrolled mouse wheel
    m_controller->wheelEvent(make_wheelEvent(QPoint(0, 180), QPoint(0, 120)));

    //! [THEN] Should be called vertical scroll with value 80
    EXPECT_CALL(m_view, moveCanvas(0, 80))
    .Times(1);

    //! [WHEN] User scrolled mouse wheel
    m_controller->wheelEvent(make_wheelEvent(QPoint(0, 80), QPoint(0, 120)));

    //! [GIVEN] pixelDelta is Null
    //!  dy = (angleDelta.y() * qMax(2.0, m_view->height() / 10.0)) / QWheelEvent::DefaultDeltasPerStep;

    //! [THEN] Should be called vertical scroll with value 50  (dy = (120 * 500 / 10) / 120 = 50)
    EXPECT_CALL(m_view, moveCanvas(0, 50))
    .Times(1);

    EXPECT_CALL(m_view, height())
    .WillOnce(Return(500));

    //! [WHEN] User scrolled mouse wheel
    m_controller->wheelEvent(make_wheelEvent(QPoint(), QPoint(0, 120)));
}

/**
 * @brief WheelEvent_ScrollHorizontal
 * @details Received wheel event, with key modifier ShiftModifier
 */
TEST_F(NotationViewInputControllerTests, WheelEvent_ScrollHorizontal)
{
    //! [THEN] Should be called horizontal scroll with value 120
    EXPECT_CALL(m_view, moveCanvasHorizontal(120))
    .Times(1);

    //! [WHEN] User scrolled mouse wheel
    m_controller->wheelEvent(make_wheelEvent(QPoint(0, 120), QPoint(), Qt::ShiftModifier));
}

/**
 * @brief WheelEvent_ScrollHorizontal
 * @details Received wheel event, with key modifier ControlModifier
 */
TEST_F(NotationViewInputControllerTests, DISABLED_WheelEvent_Zoom)
{
    //! CASE Received wheel event, with key modifier ControlModifier
    ValCh<int> currentZoom;
    currentZoom.val = 100;

//    ON_CALL(*m_configuration, currentZoom())
//    .WillByDefault(Return(currentZoom));

    //! CHECK Should be called zoomStep with value 110
//    EXPECT_CALL(env.view, setZoom(110, QPoint(100, 100)))
//    .Times(1);

    EXPECT_CALL(m_view, height())
    .WillOnce(Return(500));

    EXPECT_CALL(m_view, toLogical(QPointF(100, 100)))
    .WillOnce(Return(PointF(100, 100)));

    m_controller->wheelEvent(make_wheelEvent(QPoint(), QPoint(0, 120), Qt::ControlModifier, QPointF(100, 100)));
}

/**
 * @brief Mouse_Press_Range_Start_Drag_From_Selected_Element
 * @details User pressed left mouse button on already selected note
 *          The new note should be seeked and played, but no selected again
 */
TEST_F(NotationViewInputControllerTests, Mouse_Press_Range_Start_Drag_From_Selected_Element)
{
    //! [GIVEN] There is a test score
    engraving::MasterScore* score = engraving::ScoreRW::readScore(TEST_SCORE_PATH);

    //! [GIVEN] Previous selected note
    INotationInteraction::HitElementContext oldContext = hitContext(score, true /*first note*/);

    //! [GIVEN] User selected new note that was already selected
    INotationInteraction::HitElementContext newContext = hitContext(score, false /*last note*/);
    newContext.element->setSelected(true);

    EXPECT_CALL(*m_interaction, hitElement(_, _))
    .WillOnce(Return(newContext.element));

    EXPECT_CALL(*m_interaction, hitStaff(_))
    .WillOnce(Return(newContext.element->staff()));

    //! [GIVEN] The hew hit element context with new note will be set
    EXPECT_CALL(*m_interaction, setHitElementContext(newContext))
    .Times(1);

    EXPECT_CALL(*m_interaction, hitElementContext())
    .Times(2)
    .WillOnce(ReturnRef(oldContext))
    .WillOnce(ReturnRef(newContext));

    //! [GIVEN] There is a range selection
    ON_CALL(*m_selection, isRange())
    .WillByDefault(Return(true));

    //! [GIVEN] No note enter mode, no playing
    EXPECT_CALL(m_view, isNoteEnterMode())
    .WillOnce(Return(false));

    EXPECT_CALL(*m_playbackController, isPlaying())
    .WillOnce(Return(false));

    //! [THEN] We will seek and play selected note, but no select again
    EXPECT_CALL(*m_playbackController, seekElement(newContext.element))
    .Times(1);

    std::vector<const EngravingItem*> elements = { newContext.element };
    EXPECT_CALL(*m_playbackController, playElements(elements))
    .Times(1);

    std::vector<EngravingItem*> selectElements = { newContext.element };
    EXPECT_CALL(*m_interaction, select(selectElements, _, _))
    .Times(0);

    //! [WHEN] User pressed left mouse button
    m_controller->mousePressEvent(make_mousePressEvent(Qt::LeftButton, Qt::NoModifier, QPoint(100, 100)));
}

/**
 * @brief Mouse_Press_Range_Start_Play_From_First_Playable_Element
 * @details User selected a range in a note that is located after the previous selected note
 *          The new note should be selected and played, but no seeked
 *          The first note from a range should be seeked
 */
TEST_F(NotationViewInputControllerTests, Mouse_Press_Range_Start_Play_From_First_Playable_Element)
{
    //! [GIVEN] There is a test score
    engraving::MasterScore* score = engraving::ScoreRW::readScore(TEST_SCORE_PATH);

    //! [GIVEN] Previous selected note
    INotationInteraction::HitElementContext oldContext = hitContext(score, true /*first note*/);

    //! [GIVEN] User selected new note that is located after the previous selected note
    INotationInteraction::HitElementContext newContext = hitContext(score, false /*last note*/);

    EXPECT_CALL(*m_interaction, hitElement(_, _))
    .WillOnce(Return(newContext.element));

    EXPECT_CALL(*m_interaction, hitStaff(_))
    .WillOnce(Return(newContext.element->staff()));

    //! [GIVEN] The hew hit element context with new note will be set
    EXPECT_CALL(*m_interaction, setHitElementContext(newContext))
    .Times(1);

    EXPECT_CALL(*m_interaction, hitElementContext())
    .Times(2)
    .WillOnce(ReturnRef(oldContext))
    .WillOnce(ReturnRef(newContext));

    //! [GIVEN] No note enter mode, no playing
    EXPECT_CALL(m_view, isNoteEnterMode())
    .WillOnce(Return(false));

    EXPECT_CALL(*m_playbackController, isPlaying())
    .WillOnce(Return(false));

    //! [THEN] We will select and play selected note, but no seek
    std::vector<EngravingItem*> selectElements = { newContext.element };
    EXPECT_CALL(*m_interaction, select(selectElements, _, _))
    .Times(1);

    std::vector<const EngravingItem*> playElements = { newContext.element };
    EXPECT_CALL(*m_playbackController, playElements(playElements))
    .Times(1);

    EXPECT_CALL(*m_playbackController, seekElement(newContext.element))
    .Times(0);

    //! [GIVEN] There is a range selection with two notes
    ON_CALL(*m_selection, isRange())
    .WillByDefault(Return(true));

    selectElements.push_back(oldContext.element);
    EXPECT_CALL(*m_selection, elements())
    .WillOnce(Return(selectElements));

    //! [THEN] We will seek first note in the range
    EXPECT_CALL(*m_playbackController, seekElement(oldContext.element))
    .Times(1);

    //! [WHEN] User pressed left mouse button with ShiftModifier on the new note
    m_controller->mousePressEvent(make_mousePressEvent(Qt::LeftButton, Qt::ShiftModifier, QPoint(100, 100)));
}

/**
 * @brief Mouse_Press_On_Already_Selected_Element
 * @details User pressed on already selected note
 *          The selected note should not be selected again, but should be played and seeked
 */
TEST_F(NotationViewInputControllerTests, Mouse_Press_On_Already_Selected_Element)
{
    //! [GIVEN] There is a test score
    engraving::MasterScore* score = engraving::ScoreRW::readScore(TEST_SCORE_PATH);

    //! [GIVEN] Previous selected note
    INotationInteraction::HitElementContext oldContext = hitContext(score, true /*first note*/);
    oldContext.element->setSelected(true);

    //! [GIVEN] User pressed on the previous selected note
    INotationInteraction::HitElementContext newContext = oldContext;

    EXPECT_CALL(*m_interaction, hitElement(_, _))
    .WillOnce(Return(newContext.element));

    EXPECT_CALL(*m_interaction, hitStaff(_))
    .WillOnce(Return(newContext.element->staff()));

    EXPECT_CALL(*m_interaction, setHitElementContext(newContext))
    .Times(1);

    EXPECT_CALL(*m_interaction, hitElementContext())
    .Times(2)
    .WillOnce(ReturnRef(oldContext))
    .WillOnce(ReturnRef(newContext));

    //! [GIVEN] No note enter mode, no playing
    EXPECT_CALL(m_view, isNoteEnterMode())
    .WillOnce(Return(false));

    EXPECT_CALL(*m_playbackController, isPlaying())
    .WillOnce(Return(false));

    //! [THEN] We will no select already selected note, but play and seek
    std::vector<EngravingItem*> selectElements = { newContext.element };
    EXPECT_CALL(*m_interaction, select(selectElements, _, _))
    .Times(0);

    std::vector<const EngravingItem*> playElements = { newContext.element };
    EXPECT_CALL(*m_playbackController, playElements(playElements))
    .Times(1);

    EXPECT_CALL(*m_playbackController, seekElement(newContext.element))
    .Times(1);

    //! [GIVEN] There is no a range selection
    ON_CALL(*m_selection, isRange())
    .WillByDefault(Return(false));

    //! [WHEN] User pressed left mouse button with ShiftModifier on the new note
    m_controller->mousePressEvent(make_mousePressEvent(Qt::LeftButton, Qt::NoModifier, QPoint(100, 100)));
}
