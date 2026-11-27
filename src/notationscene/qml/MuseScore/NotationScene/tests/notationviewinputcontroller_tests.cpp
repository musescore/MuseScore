/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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
#include <gmock/gmock.h>

#include "async/processevents.h"

#include "mocks/controlledviewmock.h"
#include "global/tests/mocks/inputstatemock.h"
#include "interactive/tests/mocks/interactivemock.h"
#include "ui/tests/mocks/uiconfigurationmock.h"
#include "notation/tests/mocks/notationconfigurationmock.h"
#include "notation/tests/mocks/notationinteractionmock.h"
#include "notation/tests/mocks/notationselectionmock.h"
#include "notation/tests/mocks/notationselectionrangemock.h"
#include "playback/tests/mocks/playbackcontrollermock.h"
#include "notationscene/tests/mocks/selectinstrumentsscenariomock.h"

#include "engraving/tests/utils/scorerw.h"

#include "notationscene/qml/MuseScore/NotationScene/notationviewinputcontroller.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::InSequence;

using namespace mu;
using namespace mu::notation;
using namespace muse;

static const String TEST_SCORE_PATH(u"data/test.mscx");
static constexpr bool FLUSH_SOUND = true;

class NotationViewInputControllerTests : public ::testing::Test
{
public:

    void SetUp() override
    {
        m_interaction = std::make_shared<NiceMock<NotationInteractionMock> >();

        m_selection = std::make_shared<NiceMock<NotationSelectionMock> >();
        ON_CALL(*m_interaction, selection())
        .WillByDefault(Return(m_selection));

        m_selectionRange = std::make_shared<NiceMock<NotationSelectionRangeMock> >();
        ON_CALL(*m_selection, range())
        .WillByDefault(Return(m_selectionRange));

        ON_CALL(m_view, notationInteraction())
        .WillByDefault(Return(m_interaction));

        ON_CALL(m_view, currentScaling())
        .WillByDefault(Return(1));

        m_controller = new NotationViewInputController(&m_view, muse::modularity::globalCtx());

        m_configuration = std::make_shared<NiceMock<NotationConfigurationMock> >();
        m_controller->configuration.set(m_configuration);

        m_inputState = std::make_shared<NiceMock<muse::InputStateMock> >();
        m_controller->inputState.set(m_inputState);

        m_playbackController = std::make_shared<NiceMock<playback::PlaybackControllerMock> >();
        m_controller->playbackController.set(m_playbackController);

        m_uiConfiguration = std::make_shared<NiceMock<muse::ui::UiConfigurationMock> >();
        m_controller->uiConfiguration.set(m_uiConfiguration);

        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        ON_CALL(*m_interactive, buttonData(_)).WillByDefault([](IInteractive::Button button) {
            return IInteractive::ButtonData(button, "");
        });
        m_controller->interactive.set(m_interactive);

        m_selectInstrumentScenario = std::make_shared<NiceMock<SelectInstrumentsScenarioMock> >();
        m_controller->selectInstrumentScenario.set(m_selectInstrumentScenario);
    }

    void TearDown() override
    {
        delete m_controller;
    }

    NotationViewInputController* m_controller = nullptr;
    NiceMock<ControlledViewMock> m_view;
    std::shared_ptr<NotationConfigurationMock> m_configuration;
    std::shared_ptr<muse::InputStateMock> m_inputState;
    std::shared_ptr<NotationInteractionMock> m_interaction;
    std::shared_ptr<NotationSelectionMock> m_selection;
    std::shared_ptr<NotationSelectionRangeMock> m_selectionRange;
    std::shared_ptr<playback::PlaybackControllerMock> m_playbackController;
    std::shared_ptr<muse::ui::UiConfigurationMock> m_uiConfiguration;
    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<SelectInstrumentsScenarioMock> m_selectInstrumentScenario;
    playback::IPlaybackController::PlayParams m_playParams;

    //! An element is dragged over the view and leaves it, where there is no system drag and drop (e.g. the web):
    //! leaving drops it at the last position
    void dragOverAndLeave()
    {
        ON_CALL(*m_uiConfiguration, isSystemDragSupported()).WillByDefault(Return(false));

        muse::DragEvent moveEvent = make_dragEvent(m_dragMimeData);
        m_controller->dragMoveEvent(moveEvent);

        m_controller->dragLeaveEvent();
    }

    //! The user's answer to the question, which waits until `answer` is called
    std::function<void(IInteractive::Button)> givenTheUserIsAsked()
    {
        auto resolveAnswer = std::make_shared<async::Promise<IInteractive::Result>::Resolve>();
        ON_CALL(*m_interactive, warning(_, _, _, _, _, _)).WillByDefault([resolveAnswer]() {
            return async::make_promise<IInteractive::Result>([resolveAnswer](auto resolve) {
                *resolveAnswer = resolve;
                return async::Promise<IInteractive::Result>::dummy_result();
            }, async::PromiseType::AsyncByBody);
        });

        return [resolveAnswer](IInteractive::Button button) {
            (void)(*resolveAnswer)(IInteractive::Result(int(button)));
            for (int i = 0; i < 10; ++i) {
                async::processMessages();
            }
        };
    }

    //! The user is asked for the instrument of a dropped instrument change and answers later
    std::shared_ptr<async::Promise<InstrumentTemplate>::Resolve> givenTheUserIsAskedForAnInstrument(
        std::shared_ptr<async::Promise<InstrumentTemplate>::Reject>& reject)
    {
        auto resolveAnswer = std::make_shared<async::Promise<InstrumentTemplate>::Resolve>();
        reject = std::make_shared<async::Promise<InstrumentTemplate>::Reject>();
        ON_CALL(*m_selectInstrumentScenario, selectInstrument(_)).WillByDefault([this, resolveAnswer, reject](const InstrumentKey&) {
            ++m_instrumentQuestions;
            return async::make_promise<InstrumentTemplate>([resolveAnswer, reject](auto resolve, auto rej) {
                *resolveAnswer = resolve;
                *reject = rej;
                return async::Promise<InstrumentTemplate>::dummy_result();
            }, async::PromiseType::AsyncByBody);
        });

        return resolveAnswer;
    }

    static void processMessages()
    {
        for (int i = 0; i < 10; ++i) {
            async::processMessages();
        }
    }

    int m_instrumentQuestions = 0;

    muse::MimeData m_dragMimeData;

    static muse::DragEvent make_dragEvent(const muse::MimeData& mimeData)
    {
        muse::DragEvent ev;
        ev.position = muse::PointF(100, 100);
        ev.mimeData = mimeData;
        return ev;
    }

    struct HitContextConfig
    {
        ElementType elementType = ElementType::INVALID;
        bool start = true;
    };

    static WheelEvent make_wheelEvent(muse::Point pixelDelta,
                                      muse::Point angleDelta,
                                      muse::KeyboardModifiers modifiers = muse::KeyboardModifier::NoModifier,
                                      muse::PointF pos = muse::PointF(100, 100))
    {
        WheelEvent ev;
        ev.position = pos;
        ev.pixelDelta = pixelDelta;
        ev.angleDelta = angleDelta;
        ev.modifiers = modifiers;
        return ev;
    }

    static PointerEvent make_mousePressEvent(
        muse::MouseButton button,
        muse::KeyboardModifiers modifiers = muse::KeyboardModifier::NoModifier,
        muse::PointF pos = muse::PointF(100, 100))
    {
        PointerEvent ev;
        ev.type = PointerEvent::Type::Press;
        ev.position = pos;
        ev.button = button;
        ev.buttons = button;
        ev.modifiers = modifiers;
        return ev;
    }

    notation::EngravingItem* findElement(ElementType type, ChordRest* chord) const
    {
        switch (type) {
        case ElementType::NOTE:
            return engraving::toChord(chord)->notes().front();
        case ElementType::DYNAMIC: {
            for (notation::EngravingItem* element : chord->segment()->annotations()) {
                if (element->isDynamic()) {
                    return element;
                }
            }
            break;
        }
        case ElementType::HAIRPIN: {
            const mu::engraving::SpannerMap& spannerMap = chord->score()->spannerMap();
            auto intervals = spannerMap.findOverlapping(0, chord->score()->endTick().ticks());
            for (const auto& interval : intervals) {
                mu::engraving::Spanner* spanner = interval.value;
                if (spanner->isHairpin()) {
                    return spanner;
                }
            }
            break;
        }
        case ElementType::MEASURE:
            return chord->segment()->measure();
        default:
            break;
        }

        return nullptr;
    }

    INotationInteraction::HitElementContext hitContext(engraving::MasterScore* score, HitContextConfig config) const
    {
        INotationInteraction::HitElementContext context;

        Measure* firstMeasure = score->firstMeasure();
        Segment* segment = config.start ? firstMeasure->segments().firstCRSegment() : firstMeasure->segments().last();
        ChordRest* chord = segment->nextChordRest(0, !config.start);

        context.element = findElement(config.elementType, chord);
        context.staff = context.element->staff();

        return context;
    }

    INotationInteraction::HitElementContext hitMeasureContext(engraving::MasterScore* score, int index) const
    {
        INotationInteraction::HitElementContext context;

        engraving::MeasureBase* measure = score->measure(index);
        context.element = measure;
        context.staff = score->staff(0);

        return context;
    }
};

namespace mu::playback {
inline bool operator==(const IPlaybackController::PlayParams& p1, const IPlaybackController::PlayParams& p2)
{
    return p1.duration == p2.duration && p1.flushSound == p2.flushSound;
}
}

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
    m_controller->wheelEvent(make_wheelEvent(muse::Point(0, 180), muse::Point(0, 120)));

    //! [THEN] Should be called vertical scroll with value 80
    EXPECT_CALL(m_view, moveCanvas(0, 80))
    .Times(1);

    //! [WHEN] User scrolled mouse wheel
    m_controller->wheelEvent(make_wheelEvent(muse::Point(0, 80), muse::Point(0, 120)));

    //! [GIVEN] pixelDelta is Null
    //!  dy = (angleDelta.y() * qMax(2.0, m_view->height() / 10.0)) / QWheelEvent::DefaultDeltasPerStep;

    //! [THEN] Should be called vertical scroll with value 50  (dy = (120 * 500 / 10) / 120 = 50)
    EXPECT_CALL(m_view, moveCanvas(0, 50))
    .Times(1);

    EXPECT_CALL(m_view, height())
    .WillOnce(Return(500));

    //! [WHEN] User scrolled mouse wheel
    m_controller->wheelEvent(make_wheelEvent(muse::Point(), muse::Point(0, 120)));
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
    m_controller->wheelEvent(make_wheelEvent(muse::Point(0, 120), muse::Point(), muse::KeyboardModifier::ShiftModifier));
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

    EXPECT_CALL(m_view, toLogical(muse::PointF(100, 100)))
    .WillOnce(Return(PointF(100, 100)));

    m_controller->wheelEvent(make_wheelEvent(muse::Point(), muse::Point(0, 120), muse::KeyboardModifier::ControlModifier,
                                             muse::PointF(100, 100)));
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

    //! [GIVEN] User selected new note that was already selected
    INotationInteraction::HitElementContext newContext = hitContext(score, { ElementType::NOTE, false /*last note*/ });
    newContext.element->setSelected(true);

    std::vector<EngravingItem*> selectedElements {
        newContext.element
    };

    EXPECT_CALL(*m_interaction, hitElement(_, _))
    .WillOnce(Return(newContext.element));

    EXPECT_CALL(*m_interaction, hitStaff(_))
    .WillOnce(Return(newContext.element->staff()));

    //! [THEN] The new hit element context with new note will be set
    EXPECT_CALL(*m_interaction, setHitElementContext(newContext))
    .Times(1);

    ON_CALL(*m_interaction, hitElementContext())
    .WillByDefault(ReturnRef(newContext));

    //! [GIVEN] There is a range selection
    ON_CALL(*m_selection, isRange())
    .WillByDefault(Return(true));

    ON_CALL(*m_selection, elements())
    .WillByDefault(ReturnRef(selectedElements));

    //! [GIVEN] No note enter mode, no playing
    EXPECT_CALL(m_view, isNoteEnterMode())
    .WillOnce(Return(false));

    ON_CALL(*m_playbackController, isPlaying())
    .WillByDefault(Return(false));

    //! [THEN] seek to the element before playing it
    {
        InSequence seq;

        //! [THEN] We will seek and play selected note, but no select again
        EXPECT_CALL(*m_playbackController, seekElement(newContext.element, FLUSH_SOUND))
        .Times(1);

        std::vector<const EngravingItem*> elements = { newContext.element };
        EXPECT_CALL(*m_playbackController, playElements(elements, m_playParams, false))
        .Times(1);
    }

    std::vector<EngravingItem*> selectElements = { newContext.element };
    EXPECT_CALL(*m_interaction, select(selectElements, _, _))
    .Times(0);

    //! [WHEN] User pressed left mouse button
    m_controller->mousePressEvent(make_mousePressEvent(muse::MouseButton::LeftButton, muse::KeyboardModifier::NoModifier,
                                                       muse::PointF(100, 100)));
}

/**
 * @brief Mouse_Press_On_Selected_Text_Element
 * @details User pressed left mouse button on already selected text element
 *          We should change text cursor position
 */
TEST_F(NotationViewInputControllerTests, Mouse_Press_On_Selected_Text_Element)
{
    //! [GIVEN] There is a test score
    engraving::MasterScore* score = engraving::ScoreRW::readScore(TEST_SCORE_PATH);

    //! [GIVEN] Previous selected dynamic
    INotationInteraction::HitElementContext oldContext = hitContext(score, { ElementType::DYNAMIC });

    //! [GIVEN] User selected new dynamic that was already selected
    INotationInteraction::HitElementContext newContext = oldContext;
    newContext.element->setSelected(true);

    std::vector<EngravingItem*> selectedElements {
        newContext.element
    };

    EXPECT_CALL(*m_interaction, hitElement(_, _))
    .WillOnce(Return(newContext.element));

    EXPECT_CALL(*m_interaction, hitStaff(_))
    .WillOnce(Return(newContext.element->staff()));

    //! [GIVEN] The new hit element context with new dynamic will be set
    EXPECT_CALL(*m_interaction, setHitElementContext(newContext))
    .Times(1);

    //! [GIVEN] There isn't a range selection
    ON_CALL(*m_selection, isRange())
    .WillByDefault(Return(false));

    ON_CALL(*m_selection, elements())
    .WillByDefault(ReturnRef(selectedElements));

    //! [GIVEN] No note enter mode, no playing
    EXPECT_CALL(m_view, isNoteEnterMode())
    .WillOnce(Return(false));

    ON_CALL(*m_playbackController, isPlaying())
    .WillByDefault(Return(false));

    //! [THEN] We will seek and play selected dynamic, but no select again
    EXPECT_CALL(*m_playbackController, seekElement(newContext.element, FLUSH_SOUND))
    .Times(1);

    std::vector<const EngravingItem*> elements = { newContext.element };
    EXPECT_CALL(*m_playbackController, playElements(elements, m_playParams, false))
    .Times(0);

    std::vector<EngravingItem*> selectElements = { newContext.element };
    EXPECT_CALL(*m_interaction, select(selectElements, _, _))
    .Times(0);

    //! [GIVEN] Hit element is text element
    EXPECT_CALL(*m_interaction, isTextSelected())
    .WillOnce(Return(true));
    EXPECT_CALL(*m_interaction, isTextEditingStarted())
    .WillOnce(Return(true));

    //! [THEN] We will change text cursor position
    EXPECT_CALL(*m_interaction, changeTextCursorPosition(_))
    .Times(1);

    //! [WHEN] User pressed left mouse button
    m_controller->mousePressEvent(make_mousePressEvent(muse::MouseButton::LeftButton, muse::KeyboardModifier::NoModifier,
                                                       muse::PointF(100, 100)));
}

/**
 * @brief Mouse_Press_On_Selected_Non_Text_Element
 * @details User pressed left mouse button on already selected non text element
 *          We shouldn't change text cursor position
 */
TEST_F(NotationViewInputControllerTests, Mouse_Press_On_Selected_Non_Text_Element)
{
    //! [GIVEN] There is a test score
    engraving::MasterScore* score = engraving::ScoreRW::readScore(TEST_SCORE_PATH);

    //! [GIVEN] Previous selected hairpin
    INotationInteraction::HitElementContext oldContext = hitContext(score, { ElementType::HAIRPIN });

    //! [GIVEN] User selected new hairpin that was already selected
    INotationInteraction::HitElementContext newContext = oldContext;
    newContext.element->setSelected(true);

    std::vector<EngravingItem*> selectedElements {
        newContext.element
    };

    EXPECT_CALL(*m_interaction, hitElement(_, _))
    .WillOnce(Return(newContext.element));

    EXPECT_CALL(*m_interaction, hitStaff(_))
    .WillOnce(Return(newContext.element->staff()));

    //! [GIVEN] The new hit element context with new hairpin will be set
    EXPECT_CALL(*m_interaction, setHitElementContext(newContext))
    .Times(1);

    //! [GIVEN] There isn't a range selection
    ON_CALL(*m_selection, isRange())
    .WillByDefault(Return(false));

    ON_CALL(*m_selection, elements())
    .WillByDefault(ReturnRef(selectedElements));

    //! [GIVEN] No note enter mode, no playing
    EXPECT_CALL(m_view, isNoteEnterMode())
    .WillOnce(Return(false));

    ON_CALL(*m_playbackController, isPlaying())
    .WillByDefault(Return(false));

    //! [THEN] We will seek and play selected hairpin, but no select again
    EXPECT_CALL(*m_playbackController, seekElement(newContext.element, FLUSH_SOUND))
    .Times(1);

    std::vector<const EngravingItem*> elements = { newContext.element };
    EXPECT_CALL(*m_playbackController, playElements(elements, m_playParams, false))
    .Times(0);

    std::vector<EngravingItem*> selectElements = { newContext.element };
    EXPECT_CALL(*m_interaction, select(selectElements, _, _))
    .Times(0);

    //! [GIVEN] Hit element isn't text element
    EXPECT_CALL(*m_interaction, isTextSelected())
    .WillOnce(Return(false));

    //! [THEN] No cursor changing, no editing grips and no starting edit element because needStartEditingAfterSelecting is false
    EXPECT_CALL(*m_interaction, changeTextCursorPosition(_))
    .Times(0);
    EXPECT_CALL(*m_interaction, startEditGrip(newContext.element, _))
    .Times(0);
    EXPECT_CALL(*m_interaction, startEditElement(newContext.element))
    .Times(0);

    //! [WHEN] User pressed left mouse button
    m_controller->mousePressEvent(make_mousePressEvent(muse::MouseButton::LeftButton, muse::KeyboardModifier::NoModifier,
                                                       muse::PointF(100, 100)));
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
    INotationInteraction::HitElementContext oldContext = hitContext(score, { ElementType::NOTE, true /*first note*/ });

    //! [GIVEN] User selected new note that is located after the previous selected note
    INotationInteraction::HitElementContext newContext = hitContext(score, { ElementType::NOTE, false /*last note*/ });

    EXPECT_CALL(*m_interaction, hitElement(_, _))
    .WillOnce(Return(newContext.element));

    EXPECT_CALL(*m_interaction, hitStaff(_))
    .WillOnce(Return(newContext.element->staff()));

    //! [THEN] The new hit element context with new note will be set
    EXPECT_CALL(*m_interaction, setHitElementContext(newContext))
    .Times(1);

    ON_CALL(*m_interaction, hitElementContext())
    .WillByDefault(ReturnRef(newContext));

    //! [GIVEN] No note enter mode, no playing
    EXPECT_CALL(m_view, isNoteEnterMode())
    .WillOnce(Return(false));

    ON_CALL(*m_playbackController, isPlaying())
    .WillByDefault(Return(false));

    //! [THEN] We will select and play selected note, but no seek
    std::vector<EngravingItem*> selectElements = { newContext.element };
    EXPECT_CALL(*m_interaction, select(selectElements, _, _))
    .Times(1)
    .WillOnce([newContext] {
        newContext.element->setSelected(true);
        return muse::make_ok();
    });

    //! [THEN] seek to the element before playing it
    {
        InSequence seq;

        //! [THEN] We will seek first note in the range
        EXPECT_CALL(*m_playbackController, seekElement(oldContext.element, FLUSH_SOUND))
        .Times(1);

        std::vector<const EngravingItem*> playElements = { newContext.element };
        EXPECT_CALL(*m_playbackController, playElements(playElements, m_playParams, false))
        .Times(1);
    }

    EXPECT_CALL(*m_playbackController, seekElement(newContext.element, FLUSH_SOUND))
    .Times(0);

    //! [GIVEN] There is a range selection with two notes
    ON_CALL(*m_selection, isRange())
    .WillByDefault(Return(true));

    selectElements.push_back(oldContext.element);
    EXPECT_CALL(*m_selection, elements())
    .WillOnce(ReturnRef(selectElements));

    //! [WHEN] User pressed left mouse button with ShiftModifier on the new note
    m_controller->mousePressEvent(make_mousePressEvent(muse::MouseButton::LeftButton, muse::KeyboardModifier::ShiftModifier,
                                                       muse::PointF(100, 100)));
}

/**
 * @brief Mouse_Press_On_Selected_Selected_Range
 * @details User pressed left mouse button on already selected range
 *           The selection shouldn't be changed, but the first measure in the range should be seeked
 */
TEST_F(NotationViewInputControllerTests, Mouse_Press_On_Already_Selected_Range)
{
    //! [GIVEN] There is a test score
    engraving::MasterScore* score = engraving::ScoreRW::readScore(TEST_SCORE_PATH);

    //! [GIVEN] User selects a measure
    INotationInteraction::HitElementContext context = hitMeasureContext(score, 0 /*first measure*/);

    EXPECT_CALL(*m_interaction, hitElement(_, _))
    .WillOnce(Return(context.element));

    EXPECT_CALL(*m_interaction, hitStaff(_))
    .WillOnce(Return(context.staff));

    //! [THEN] The new hit element context with new measure will be set
    EXPECT_CALL(*m_interaction, setHitElementContext(context))
    .Times(1);

    ON_CALL(*m_interaction, hitElementContext())
    .WillByDefault(ReturnRef(context));

    //! [GIVEN] There is a range selection
    ON_CALL(*m_selection, isRange())
    .WillByDefault(Return(true));
    ON_CALL(*m_selectionRange, containsItem(context.element, _))
    .WillByDefault(Return(true));

    std::vector<EngravingItem*> selectElements = { context.element };
    EXPECT_CALL(*m_selection, elements())
    .WillOnce(ReturnRef(selectElements));

    //! [THEN] We should seek measure from the range
    EXPECT_CALL(*m_playbackController, seekElement(context.element, FLUSH_SOUND))
    .Times(1);

    //! [THEN] No selection change
    EXPECT_CALL(*m_interaction, select(_, _, _))
    .Times(0);

    //! [WHEN] User pressed left mouse button
    m_controller->mousePressEvent(make_mousePressEvent(muse::MouseButton::LeftButton, muse::KeyboardModifier::NoModifier,
                                                       muse::PointF(100, 100)));
}

/**
 * @brief Mouse_Press_Shift_On_Selected_Selected_Range
 * @details User pressed left mouse button with Shift on already selected range
 *          This should result in a call to `select`, to extend/diminish the selection
 */
TEST_F(NotationViewInputControllerTests, Mouse_Press_Shift_On_Already_Selected_Range)
{
    //! [GIVEN] There is a test score
    engraving::MasterScore* score = engraving::ScoreRW::readScore(TEST_SCORE_PATH);

    //! [GIVEN] User selects a measure
    INotationInteraction::HitElementContext context = hitMeasureContext(score, 0 /*first measure*/);

    EXPECT_CALL(*m_interaction, hitElement(_, _))
    .WillOnce(Return(context.element));

    EXPECT_CALL(*m_interaction, hitStaff(_))
    .WillOnce(Return(context.staff));

    //! [THEN] The new hit element context with new measure will be set
    EXPECT_CALL(*m_interaction, setHitElementContext(context))
    .Times(1);

    ON_CALL(*m_interaction, hitElementContext())
    .WillByDefault(ReturnRef(context));

    //! [GIVEN] There is a range selection
    ON_CALL(*m_selection, isRange())
    .WillByDefault(Return(true));
    ON_CALL(*m_selectionRange, containsItem(context.element, _))
    .WillByDefault(Return(true));

    //! [THEN] We should seek measure from the range
    EXPECT_CALL(*m_playbackController, seekElement(context.element, FLUSH_SOUND))
    .Times(1);

    //! [THEN] Selection is extended/diminished
    std::vector<EngravingItem*> selectElements = { context.element };
    EXPECT_CALL(*m_interaction, select(selectElements, _, _))
    .Times(1);

    EXPECT_CALL(*m_selection, elements())
    .WillOnce(ReturnRef(selectElements));

    //! [WHEN] User pressed left mouse button
    m_controller->mousePressEvent(make_mousePressEvent(muse::MouseButton::LeftButton, muse::KeyboardModifier::ShiftModifier,
                                                       muse::PointF(100, 100)));
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
    INotationInteraction::HitElementContext oldContext = hitContext(score, { ElementType::NOTE });
    oldContext.element->setSelected(true);

    //! [GIVEN] User pressed on the previous selected note
    INotationInteraction::HitElementContext newContext = oldContext;

    EXPECT_CALL(*m_interaction, hitElement(_, _))
    .WillOnce(Return(newContext.element));

    EXPECT_CALL(*m_interaction, hitStaff(_))
    .WillOnce(Return(newContext.element->staff()));

    EXPECT_CALL(*m_interaction, setHitElementContext(newContext))
    .Times(1);

    std::vector<EngravingItem*> selectedElements = { oldContext.element };
    ON_CALL(*m_selection, elements())
    .WillByDefault(ReturnRef(selectedElements));

    //! [GIVEN] No note enter mode, no playing
    EXPECT_CALL(m_view, isNoteEnterMode())
    .WillOnce(Return(false));

    ON_CALL(*m_playbackController, isPlaying())
    .WillByDefault(Return(false));

    //! [THEN] We will no select already selected note, but play and seek
    EXPECT_CALL(*m_interaction, select(_, _, _))
    .Times(0);

    //! [THEN] seek to the element before playing it
    {
        InSequence seq;

        EXPECT_CALL(*m_playbackController, seekElement(newContext.element, FLUSH_SOUND))
        .Times(1);

        std::vector<const EngravingItem*> playElements = { newContext.element };
        EXPECT_CALL(*m_playbackController, playElements(playElements, m_playParams, false))
        .Times(1);
    }

    //! [GIVEN] There is no a range selection
    ON_CALL(*m_selection, isRange())
    .WillByDefault(Return(false));

    //! [WHEN] User pressed left mouse button with NoModifier on the new note
    m_controller->mousePressEvent(make_mousePressEvent(muse::MouseButton::LeftButton, muse::KeyboardModifier::NoModifier,
                                                       muse::PointF(100, 100)));
}

/**
 * @brief Mouse_Press_On_Range
 * @details User pressed selected new measure with ShiftModifier
 *          The new measure should be selected, shouldn't be played, previous measure should be seeked
 */
TEST_F(NotationViewInputControllerTests, Mouse_Press_On_Range)
{
    //! [GIVEN] There is a test score
    engraving::MasterScore* score = engraving::ScoreRW::readScore(TEST_SCORE_PATH);

    //! [GIVEN] Previous selected measure
    INotationInteraction::HitElementContext oldContext = hitMeasureContext(score, 0 /*first measure*/);

    //! [GIVEN] User selected new measure that is located after the previous selected measure
    INotationInteraction::HitElementContext newContext = hitMeasureContext(score, 1 /*second measure*/);
    EXPECT_CALL(*m_interaction, hitElement(_, _))
    .WillOnce(Return(newContext.element));

    EXPECT_CALL(*m_interaction, hitStaff(_))
    .WillOnce(Return(newContext.staff));

    //! [THEN] The new hit element context with new measure will be set
    EXPECT_CALL(*m_interaction, setHitElementContext(newContext))
    .Times(1);

    ON_CALL(*m_interaction, hitElementContext())
    .WillByDefault(ReturnRef(newContext));

    //! [GIVEN] No note enter mode, no playing
    EXPECT_CALL(m_view, isNoteEnterMode())
    .WillOnce(Return(false));

    ON_CALL(*m_playbackController, isPlaying())
    .WillByDefault(Return(false));

    //! [THEN] We will select new measure
    std::vector<EngravingItem*> selectElements = { newContext.element };
    EXPECT_CALL(*m_interaction, select(selectElements, _, _))
    .Times(1)
    .WillOnce([newContext] {
        newContext.element->setSelected(true);
        return muse::make_ok();
    });

    //! [GIVEN] There is a range selection with two measures
    ON_CALL(*m_selection, isRange())
    .WillByDefault(Return(true));

    //! [THEN] No play measure
    std::vector<const EngravingItem*> playElements = { newContext.element };
    EXPECT_CALL(*m_playbackController, playElements(playElements, m_playParams, false))
    .Times(0);

    //! [THEN] Because it's a range selection, we should start playing from first measure in the range
    EXPECT_CALL(*m_playbackController, seekElement(oldContext.element, FLUSH_SOUND))
    .Times(1);

    selectElements.push_back(oldContext.element);
    EXPECT_CALL(*m_selection, elements())
    .WillOnce(ReturnRef(selectElements));

    //! [WHEN] User pressed left mouse button with ShiftModifier on the new note
    m_controller->mousePressEvent(make_mousePressEvent(muse::MouseButton::LeftButton, muse::KeyboardModifier::ShiftModifier,
                                                       muse::PointF(100, 100)));
}

/**
 * @brief Mouse_Press_On_Range_Context_Menu
 * @details The user pressed the right button on the selected measure
 *          The selection shouldn't be changed, context menu should be opened for selected measure
 */
TEST_F(NotationViewInputControllerTests, Mouse_Press_On_Range_Context_Menu)
{
    //! [GIVEN] There is a test score
    engraving::MasterScore* score = engraving::ScoreRW::readScore(TEST_SCORE_PATH);

    //! [GIVEN] User selected a measure
    INotationInteraction::HitElementContext selectMeasureContext = hitMeasureContext(score, 0 /*first measure*/);

    //! [GIVEN] User pressed the right button on the selected measure, same context
    INotationInteraction::HitElementContext contextMenuOnMeasureContext = selectMeasureContext;

    EXPECT_CALL(*m_interaction, hitElement(_, _))
    .WillOnce(Return(selectMeasureContext.element))
    //! right button click
    .WillOnce(Return(contextMenuOnMeasureContext.element));

    EXPECT_CALL(*m_interaction, hitStaff(_))
    .WillOnce(Return(selectMeasureContext.staff))
    //! right button click
    .WillOnce(Return(contextMenuOnMeasureContext.staff));

    //! [THEN] The new hit element context with new measure will be set
    EXPECT_CALL(*m_interaction, setHitElementContext(selectMeasureContext))
    .WillOnce(Return())
    //! right button click
    .WillOnce(Return());

    //! [GIVEN] No note enter mode, no playing
    EXPECT_CALL(m_view, isNoteEnterMode())
    .WillRepeatedly(Return(false));

    EXPECT_CALL(*m_playbackController, isPlaying())
    .WillRepeatedly(Return(false));

    //! [THEN] We will select new measure only one time
    std::vector<EngravingItem*> selectElements = { selectMeasureContext.element };
    EXPECT_CALL(*m_interaction, select(selectElements, _, _))
    .Times(1);

    EXPECT_CALL(*m_selection, isRange())
    .WillRepeatedly(Return(true));

    //! [GIVEN] Element is in selected range at the moment that it is selected for the second time
    EXPECT_CALL(*m_selectionRange, containsItem(contextMenuOnMeasureContext.element, _))
    .WillOnce(Return(true));

    std::vector<const EngravingItem*> playElements = { selectMeasureContext.element };
    EXPECT_CALL(*m_playbackController, playElements(playElements, m_playParams, false))
    .Times(0);

    EXPECT_CALL(*m_playbackController, seekElement(selectMeasureContext.element, FLUSH_SOUND))
    .WillOnce(Return())
    //! right button click
    .WillOnce(Return());

    //! [THEN] The selection shouldn't be changed
    EXPECT_CALL(*m_selection, elements())
    .WillRepeatedly(ReturnRef(selectElements));

    //! Note: the context menu itself is shown by AbstractNotationPaintView::event

    //! [WHEN] User pressed left mouse button with ShiftModifier on the new measure
    m_controller->mousePressEvent(make_mousePressEvent(muse::MouseButton::LeftButton, muse::KeyboardModifier::ShiftModifier,
                                                       muse::PointF(100, 100)));

    //! [WHEN] User pressed right mouse button with NoModifier on the selected measure
    m_controller->mousePressEvent(make_mousePressEvent(muse::MouseButton::RightButton, muse::KeyboardModifier::NoModifier,
                                                       muse::PointF(100, 100)));
}

/**
 * @brief Mouse_Press_On_Range_Context_Menu_New_Selection
 * @details The user pressed the right button on the new measure
 *          The selection should be changed, context menu should be opened for new measure
 */
TEST_F(NotationViewInputControllerTests, Mouse_Press_On_Range_Context_Menu_New_Selection)
{
    //! [GIVEN] There is a test score
    engraving::MasterScore* score = engraving::ScoreRW::readScore(TEST_SCORE_PATH);

    //! [GIVEN] User selected a measure
    INotationInteraction::HitElementContext selectMeasureContext = hitMeasureContext(score, 0 /*first measure*/);

    //! [GIVEN] User pressed the right button on the selected measure, same context
    INotationInteraction::HitElementContext contextMenuOnMeasureContext = hitMeasureContext(score, 1 /*second measure*/);

    EXPECT_CALL(*m_interaction, hitElement(_, _))
    .WillOnce(Return(selectMeasureContext.element))
    //! right button click
    .WillOnce(Return(contextMenuOnMeasureContext.element));

    EXPECT_CALL(*m_interaction, hitStaff(_))
    .WillOnce(Return(selectMeasureContext.staff))
    //! right button click
    .WillOnce(Return(contextMenuOnMeasureContext.staff));

    EXPECT_CALL(*m_interaction, setHitElementContext(selectMeasureContext))
    .WillOnce(Return());
    EXPECT_CALL(*m_interaction, setHitElementContext(contextMenuOnMeasureContext))
    .WillOnce(Return());

    //! [GIVEN] No note enter mode, no playing
    EXPECT_CALL(m_view, isNoteEnterMode())
    .WillRepeatedly(Return(false));

    EXPECT_CALL(*m_playbackController, isPlaying())
    .WillRepeatedly(Return(false));

    EXPECT_CALL(*m_selection, isRange())
    .WillRepeatedly(Return(true));

    //! [THEN] The selection should be changed
    std::vector<EngravingItem*> selectElements = { selectMeasureContext.element };
    EXPECT_CALL(*m_interaction, select(selectElements, _, _))
    .Times(1);

    std::vector<EngravingItem*> contextMenuSelectElements = { contextMenuOnMeasureContext.element };
    EXPECT_CALL(*m_interaction, select(contextMenuSelectElements, _, _))
    .WillOnce(Return(muse::make_ok()));

    EXPECT_CALL(*m_selection, elements())
    .WillOnce(ReturnRef(selectElements))
    .WillOnce(ReturnRef(contextMenuSelectElements));

    //! [GIVEN] New element is not in selected range at the moment that it is selected
    EXPECT_CALL(*m_selectionRange, containsItem(contextMenuOnMeasureContext.element, _))
    .WillOnce(Return(false));

    EXPECT_CALL(*m_playbackController, playElements(_, m_playParams, false))
    .Times(0);

    //! [THEN] We will seek each measures
    EXPECT_CALL(*m_playbackController, seekElement(selectMeasureContext.element, FLUSH_SOUND))
    .WillOnce(Return());

    EXPECT_CALL(*m_playbackController, seekElement(contextMenuOnMeasureContext.element, FLUSH_SOUND))
    .WillOnce(Return());

    //! Note: the context menu itself is shown by AbstractNotationPaintView::event

    //! [WHEN] User pressed left mouse button with ShiftModifier on the new measure
    m_controller->mousePressEvent(make_mousePressEvent(muse::MouseButton::LeftButton, muse::KeyboardModifier::ShiftModifier,
                                                       muse::PointF(100, 100)));

    //! [WHEN] User pressed right mouse button with NoModifier on the selected measure
    m_controller->mousePressEvent(make_mousePressEvent(muse::MouseButton::RightButton, muse::KeyboardModifier::NoModifier,
                                                       muse::PointF(100, 100)));
}

TEST_F(NotationViewInputControllerTests, Drop_NeedsConfirmation_UserAgrees_DropsAfterTheAnswer)
{
    //! [GIVEN] Dropping the element needs a confirmation, and the user is asked
    ON_CALL(*m_interaction, canDropSingle(_)).WillByDefault(Return(Ret(int(Ret::Code::UnknownError), "Drop A?")));
    std::function<void(IInteractive::Button)> answer = givenTheUserIsAsked();

    int drops = 0;
    int endedDrops = 0;
    ON_CALL(*m_interaction, dropSingle(_, _, _))
    .WillByDefault([&drops, &endedDrops](const muse::PointF&, muse::KeyboardModifiers, const std::optional<InstrumentTemplate>&) {
        EXPECT_EQ(endedDrops, 0) << "the drop is not ended before the element is dropped";
        ++drops;
        return true;
    });
    ON_CALL(*m_interaction, endDrop()).WillByDefault([&endedDrops]() {
        ++endedDrops;
    });

    //! [WHEN] The element is dropped...
    dragOverAndLeave();

    //! [THEN] ...the drop is kept while the user is asked
    EXPECT_EQ(drops, 0);
    EXPECT_EQ(endedDrops, 0);

    //! [WHEN] The user agrees...
    answer(IInteractive::Button::Ok);

    //! [THEN] ...the element is dropped, and the drop is ended
    EXPECT_EQ(drops, 1);
    EXPECT_EQ(endedDrops, 1);
}

TEST_F(NotationViewInputControllerTests, Drop_NeedsConfirmation_UserCancels_EndsTheDropAfterTheAnswer)
{
    //! [GIVEN] Dropping the element needs a confirmation, and the user is asked
    ON_CALL(*m_interaction, canDropSingle(_)).WillByDefault(Return(Ret(int(Ret::Code::UnknownError), "Drop A?")));
    std::function<void(IInteractive::Button)> answer = givenTheUserIsAsked();

    //! [THEN] Nothing is dropped
    EXPECT_CALL(*m_interaction, dropSingle(_, _, _)).Times(0);

    //! [WHEN] The element is dropped...
    int endedDrops = 0;
    ON_CALL(*m_interaction, endDrop()).WillByDefault([&endedDrops]() {
        ++endedDrops;
    });
    dragOverAndLeave();

    //! [THEN] ...the drop is kept while the user is asked
    EXPECT_EQ(endedDrops, 0);

    //! [WHEN] The user cancels...
    answer(IInteractive::Button::Cancel);

    //! [THEN] ...the drop is ended
    EXPECT_EQ(endedDrops, 1);
}

TEST_F(NotationViewInputControllerTests, Drop_NoConfirmation_DropsAndEndsAtOnce)
{
    //! [GIVEN] Dropping the element needs no confirmation
    ON_CALL(*m_interaction, canDropSingle(_)).WillByDefault(Return(muse::make_ok()));

    //! [THEN] Nobody is asked, the element is dropped and the drop is ended at once
    EXPECT_CALL(*m_interactive, warning(_, _, _, _, _, _)).Times(0);
    {
        InSequence seq;
        EXPECT_CALL(*m_interaction, dropSingle(_, _, _)).WillOnce(Return(true));
        EXPECT_CALL(*m_interaction, endDrop()).Times(1);
    }

    //! [WHEN] The element is dropped
    dragOverAndLeave();
}

TEST_F(NotationViewInputControllerTests, Drop_NeedsConfirmation_AnotherDragIsNotStartedUntilTheAnswer)
{
    //! [GIVEN] A drop waits for the user's answer
    ON_CALL(*m_interaction, canDropSingle(_)).WillByDefault(Return(Ret(int(Ret::Code::UnknownError), "Drop A?")));
    std::function<void(IInteractive::Button)> answer = givenTheUserIsAsked();
    dragOverAndLeave();

    //! [THEN] Another element is not taken, it would replace the waiting drop
    EXPECT_CALL(*m_interaction, startDropSingle(_)).Times(0);

    //! [WHEN] Another element is dragged into the view...
    m_dragMimeData.setData(engraving::mimeSymbolFormat, muse::ByteArray("B"));
    muse::DragEvent enterEvent = make_dragEvent(m_dragMimeData);
    m_controller->dragEnterEvent(enterEvent);

    //! [THEN] ...it is ignored
    EXPECT_FALSE(enterEvent.isAccepted());

    answer(IInteractive::Button::Cancel);
}

TEST_F(NotationViewInputControllerTests, Drop_NeedsConfirmation_NotationClosedBeforeTheAnswer_DoesNothing)
{
    //! [GIVEN] A drop waits for the user's answer, in a notation that is closed before the answer
    auto closedInteraction = std::make_shared<NiceMock<NotationInteractionMock> >();
    ON_CALL(*closedInteraction, canDropSingle(_)).WillByDefault(Return(Ret(int(Ret::Code::UnknownError), "Drop A?")));
    ON_CALL(m_view, notationInteraction()).WillByDefault([&closedInteraction]() {
        return closedInteraction;
    });

    std::function<void(IInteractive::Button)> answer = givenTheUserIsAsked();
    dragOverAndLeave();

    std::weak_ptr<NotationInteractionMock> released = closedInteraction;
    closedInteraction.reset();

    //! [THEN] The waiting drop doesn't keep the interaction of the closed notation alive
    EXPECT_TRUE(released.expired());

    //! [WHEN] The user agrees...
    //! [THEN] ...nothing is dropped into the closed notation (and nothing is accessed)
    answer(IInteractive::Button::Ok);
}

TEST_F(NotationViewInputControllerTests, Drop_InstrumentChange_UserChoosesAnInstrument_DropsItWithTheInstrument)
{
    //! [GIVEN] An instrument change is dropped, and the user is asked for the instrument
    ON_CALL(*m_interaction, canDropSingle(_)).WillByDefault(Return(muse::make_ok()));
    ON_CALL(*m_interaction, isDroppingInstrumentChange()).WillByDefault(Return(true));
    std::shared_ptr<async::Promise<InstrumentTemplate>::Reject> reject;
    auto resolve = givenTheUserIsAskedForAnInstrument(reject);

    InstrumentTemplate flute;
    flute.id = u"flute";

    int drops = 0;
    int endedDrops = 0;
    ON_CALL(*m_interaction, dropSingle(_, _, _))
    .WillByDefault([&drops, &endedDrops](const muse::PointF&, muse::KeyboardModifiers, const std::optional<InstrumentTemplate>& instrument) {
        EXPECT_EQ(endedDrops,
                  0) << "the drop is not ended before the element is dropped";
        EXPECT_TRUE(instrument.has_value()) << "an instrument change is dropped with the chosen instrument";
        EXPECT_EQ(instrument.value_or(InstrumentTemplate()).id, u"flute");
        ++drops;
        return true;
    });
    ON_CALL(*m_interaction, endDrop()).WillByDefault([&endedDrops]() {
        ++endedDrops;
    });

    //! [WHEN] The element is dropped...
    dragOverAndLeave();

    //! [THEN] ...the drop is kept while the user is asked
    EXPECT_EQ(drops, 0);
    EXPECT_EQ(endedDrops, 0);

    //! [WHEN] The user chooses an instrument...
    (void)(*resolve)(flute);
    processMessages();

    //! [THEN] ...the element is dropped with that instrument, and the drop is ended
    EXPECT_EQ(drops, 1);
    EXPECT_EQ(endedDrops, 1);
}

TEST_F(NotationViewInputControllerTests, Drop_InstrumentChange_UserCancels_EndsTheDropAfterTheAnswer)
{
    //! [GIVEN] An instrument change is dropped, and the user is asked for the instrument
    ON_CALL(*m_interaction, canDropSingle(_)).WillByDefault(Return(muse::make_ok()));
    ON_CALL(*m_interaction, isDroppingInstrumentChange()).WillByDefault(Return(true));
    std::shared_ptr<async::Promise<InstrumentTemplate>::Reject> reject;
    givenTheUserIsAskedForAnInstrument(reject);

    //! [THEN] Nothing is dropped
    EXPECT_CALL(*m_interaction, dropSingle(_, _, _)).Times(0);

    int endedDrops = 0;
    ON_CALL(*m_interaction, endDrop()).WillByDefault([&endedDrops]() {
        ++endedDrops;
    });

    //! [WHEN] The element is dropped...
    dragOverAndLeave();

    //! [THEN] ...the drop is kept while the user is asked
    EXPECT_EQ(endedDrops, 0);

    //! [WHEN] The user cancels...
    (void)(*reject)(int(Ret::Code::Cancel), "");
    processMessages();

    //! [THEN] ...the drop is ended
    EXPECT_EQ(endedDrops, 1);
}

TEST_F(NotationViewInputControllerTests, Drop_InstrumentChange_NotationClosedBeforeTheAnswer_DoesNothing)
{
    //! [GIVEN] An instrument change is dropped into a notation that is closed before the answer
    auto closedInteraction = std::make_shared<NiceMock<NotationInteractionMock> >();
    ON_CALL(*closedInteraction, canDropSingle(_)).WillByDefault(Return(muse::make_ok()));
    ON_CALL(*closedInteraction, isDroppingInstrumentChange()).WillByDefault(Return(true));
    ON_CALL(m_view, notationInteraction()).WillByDefault([&closedInteraction]() {
        return closedInteraction;
    });

    std::shared_ptr<async::Promise<InstrumentTemplate>::Reject> reject;
    auto resolve = givenTheUserIsAskedForAnInstrument(reject);
    dragOverAndLeave();

    std::weak_ptr<NotationInteractionMock> released = closedInteraction;
    closedInteraction.reset();

    //! [THEN] The waiting drop doesn't keep the interaction of the closed notation alive
    EXPECT_TRUE(released.expired());

    //! [WHEN] The user chooses an instrument
    //! [THEN] Nothing is dropped into the closed notation (and nothing is accessed)
    (void)(*resolve)(InstrumentTemplate());
    processMessages();
}

TEST_F(NotationViewInputControllerTests, Drop_InstrumentChangeNeedsConfirmation_UserAgrees_ThenChoosesTheInstrument)
{
    //! [GIVEN] Dropping an instrument change needs a confirmation, and then its instrument
    ON_CALL(*m_interaction, isDroppingInstrumentChange()).WillByDefault(Return(true));
    ON_CALL(*m_interaction, canDropSingle(_)).WillByDefault(Return(Ret(int(Ret::Code::UnknownError), "Drop A?")));
    std::function<void(IInteractive::Button)> answer = givenTheUserIsAsked();
    std::shared_ptr<async::Promise<InstrumentTemplate>::Reject> reject;
    auto resolve = givenTheUserIsAskedForAnInstrument(reject);

    InstrumentTemplate flute;
    flute.id = u"flute";

    int drops = 0;
    ON_CALL(*m_interaction, dropSingle(_, _, _))
    .WillByDefault([&drops](const muse::PointF&, muse::KeyboardModifiers, const std::optional<InstrumentTemplate>& instrument) {
        EXPECT_EQ(instrument.value_or(InstrumentTemplate()).id, u"flute");
        ++drops;
        return true;
    });

    //! [WHEN] The element is dropped
    dragOverAndLeave();

    //! [THEN] The instrument is not asked before the confirmation
    EXPECT_EQ(m_instrumentQuestions, 0);

    //! [WHEN] The user agrees
    answer(IInteractive::Button::Ok);

    //! [THEN] The instrument is asked, and nothing is dropped until it is chosen
    EXPECT_EQ(m_instrumentQuestions, 1);
    EXPECT_EQ(drops, 0);

    //! [WHEN] The user chooses an instrument
    (void)(*resolve)(flute);
    processMessages();

    //! [THEN] The element is dropped once, with that instrument
    EXPECT_EQ(drops, 1);
}

TEST_F(NotationViewInputControllerTests, Drop_InstrumentChangeNeedsConfirmation_UserCancels_DoesNotAskTheInstrument)
{
    //! [GIVEN] Dropping an instrument change needs a confirmation
    ON_CALL(*m_interaction, isDroppingInstrumentChange()).WillByDefault(Return(true));
    ON_CALL(*m_interaction, canDropSingle(_)).WillByDefault(Return(Ret(int(Ret::Code::UnknownError), "Drop A?")));
    std::function<void(IInteractive::Button)> answer = givenTheUserIsAsked();

    //! [THEN] Neither the instrument is asked, nor anything dropped, and the drop is ended
    EXPECT_CALL(*m_selectInstrumentScenario, selectInstrument(_)).Times(0);
    EXPECT_CALL(*m_interaction, dropSingle(_, _, _)).Times(0);

    int endedDrops = 0;
    ON_CALL(*m_interaction, endDrop()).WillByDefault([&endedDrops]() {
        ++endedDrops;
    });

    //! [WHEN] The element is dropped, and the user cancels
    dragOverAndLeave();
    answer(IInteractive::Button::Cancel);

    EXPECT_EQ(endedDrops, 1);
}

TEST_F(NotationViewInputControllerTests, TripleClick_PressSoonAfterDoubleClickInTheText_SelectsAllText)
{
    //! [GIVEN] The text is being edited, the double click interval is long
    ON_CALL(*m_interaction, isTextEditingStarted()).WillByDefault(Return(true));
    ON_CALL(*m_uiConfiguration, doubleClickIntervalMs()).WillByDefault(Return(60000));

    {
        InSequence seq;

        //! [THEN] The double click selects the word, the next press selects all the text
        EXPECT_CALL(*m_interaction, selectText(mu::engraving::SelectTextType::Word)).Times(1);
        EXPECT_CALL(*m_interaction, selectText(mu::engraving::SelectTextType::All)).Times(1);
    }

    //! [WHEN] Double click and then press
    m_controller->mouseDoubleClickEvent(muse::PointerEvent());
    m_controller->mousePressEvent(make_mousePressEvent(muse::MouseButton::LeftButton));
}

TEST_F(NotationViewInputControllerTests, TripleClick_PressAfterTheDoubleClickInterval_DoesNotSelectAllText)
{
    //! [GIVEN] The text is being edited, the double click interval is over at once
    ON_CALL(*m_interaction, isTextEditingStarted()).WillByDefault(Return(true));
    ON_CALL(*m_uiConfiguration, doubleClickIntervalMs()).WillByDefault(Return(0));

    //! [GIVEN] Nothing is selected, the press goes the usual way
    const std::vector<EngravingItem*> noElements;
    ON_CALL(*m_selection, elements()).WillByDefault(ReturnRef(noElements));

    //! [THEN] Only the double click selects the word
    EXPECT_CALL(*m_interaction, selectText(mu::engraving::SelectTextType::Word)).Times(1);
    EXPECT_CALL(*m_interaction, selectText(mu::engraving::SelectTextType::All)).Times(0);

    //! [WHEN] Double click and then press
    m_controller->mouseDoubleClickEvent(muse::PointerEvent());
    m_controller->mousePressEvent(make_mousePressEvent(muse::MouseButton::LeftButton));
}

TEST_F(NotationViewInputControllerTests, TextCursorRect_TextIsNotEdited_ProvidesNothing)
{
    //! [GIVEN] The text is not being edited
    ON_CALL(*m_interaction, isTextEditingStarted()).WillByDefault(Return(false));

    //! [THEN] There is no cursor of the text
    EXPECT_FALSE(m_controller->textCursorRect().has_value());
}
