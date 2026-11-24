/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include "modularity/ioc.h"

#include "engraving/tests/utils/scorerw.h"

#include "engraving/dom/dummyparent.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/instrchange.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"

#include "notation/inotationinteraction.h"
#include "notation/inotationundostack.h"
#include "notation/internal/notation.h"
#include "notation/tests/mocks/notationconfigurationmock.h"
#include "notationscene/tests/mocks/selectinstrumentsscenariomock.h"

using namespace ::testing;
using namespace muse;
using namespace mu::engraving;
using namespace mu::notation;

namespace {
const String TEST_SCORE_PATH(u"data/test.mscx");
const std::string TEST_MODULE("notation_utests");
const modularity::IoCID TEST_CONTEXT_ID = 1;
}

class Notation_NotationInteractionTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_iocCtx = std::make_shared<modularity::Context>(TEST_CONTEXT_ID);

        m_selectInstrumentScenario = std::make_shared<NiceMock<SelectInstrumentsScenarioMock> >();
        modularity::ioc(m_iocCtx)->registerExport<ISelectInstrumentsScenario>(TEST_MODULE, m_selectInstrumentScenario);

        m_configuration = std::make_shared<NiceMock<NotationConfigurationMock> >();
        modularity::globalIoc()->registerExport<INotationConfiguration>(TEST_MODULE, m_configuration);

        m_score.reset(ScoreRW::readScore(TEST_SCORE_PATH));
        m_notation = std::make_shared<Notation>(nullptr, m_iocCtx, m_score.get());
    }

    void TearDown() override
    {
        //! NOTE The notation doesn't own the score
        m_notation.reset();
        m_score.reset();

        modularity::globalIoc()->unregister<INotationConfiguration>(TEST_MODULE);
        modularity::removeIoC(m_iocCtx);
    }

    //! NOTE The answer is given later, as by the user
    async::Promise<InstrumentTemplate> pendingAnswer()
    {
        return async::make_promise<InstrumentTemplate>([this](auto resolve, auto reject) {
            m_resolve = resolve;
            m_reject = reject;
            return async::Promise<InstrumentTemplate>::dummy_result();
        }, async::PromiseType::AsyncByBody);
    }

    void selectFirstChordRest()
    {
        Segment* segment = score()->firstMeasure()->first(SegmentType::ChordRest);
        m_notation->interaction()->select({ segment->element(0) }, SelectType::SINGLE);
    }

    InstrumentChange* findInstrumentChange() const
    {
        for (Segment* segment = score()->firstSegment(SegmentType::ChordRest); segment;
             segment = segment->next1(SegmentType::ChordRest)) {
            for (EngravingItem* annotation : segment->annotations()) {
                if (annotation->isInstrumentChange()) {
                    return toInstrumentChange(annotation);
                }
            }
        }

        return nullptr;
    }

    MasterScore* score() const
    {
        return m_score.get();
    }

    modularity::ContextPtr m_iocCtx;
    std::shared_ptr<SelectInstrumentsScenarioMock> m_selectInstrumentScenario;
    std::shared_ptr<NotationConfigurationMock> m_configuration;
    std::unique_ptr<MasterScore> m_score;
    std::shared_ptr<Notation> m_notation;
    async::Promise<InstrumentTemplate>::Resolve m_resolve;
    async::Promise<InstrumentTemplate>::Reject m_reject;
};

TEST_F(Notation_NotationInteractionTests, AddInstrumentChange_UserChoosesAnInstrument_AddsItInOneUndoStep)
{
    //! [GIVEN] A chord is selected, and the user is asked for the instrument
    selectFirstChordRest();
    EXPECT_CALL(*m_selectInstrumentScenario, selectInstrument(_)).WillOnce(Return(pendingAnswer()));

    //! [WHEN] Adding an instrument change
    m_notation->interaction()->addText(TextStyleType::INSTRUMENT_CHANGE);
    async::processMessages();

    //! [THEN] Nothing is changed until the user answers
    EXPECT_EQ(findInstrumentChange(), nullptr);
    EXPECT_FALSE(m_notation->undoStack()->canUndo());

    //! [WHEN] The user chooses an instrument
    const InstrumentTemplate* flute = searchTemplate(u"flute");
    ASSERT_TRUE(flute);
    (void)m_resolve(*flute);
    async::processMessages();

    //! [THEN] The instrument change to that instrument is added
    InstrumentChange* instrumentChange = findInstrumentChange();
    ASSERT_TRUE(instrumentChange);
    EXPECT_TRUE(instrumentChange->init());
    EXPECT_EQ(instrumentChange->instrument()->id(), flute->id);

    //! [THEN] It is undone in one step
    m_notation->undoStack()->undo(nullptr);
    EXPECT_EQ(findInstrumentChange(), nullptr);
    EXPECT_FALSE(m_notation->undoStack()->canUndo());
}

TEST_F(Notation_NotationInteractionTests, AddInstrumentChange_UserCancels_AddsNothing)
{
    //! [GIVEN] A chord is selected, and the user is asked for the instrument
    selectFirstChordRest();
    EXPECT_CALL(*m_selectInstrumentScenario, selectInstrument(_)).WillOnce(Return(pendingAnswer()));

    m_notation->interaction()->addText(TextStyleType::INSTRUMENT_CHANGE);
    async::processMessages();

    //! [WHEN] The user cancels
    (void)m_reject(int(Ret::Code::Cancel), "");
    async::processMessages();

    //! [THEN] Nothing is added, and there is nothing to undo
    EXPECT_EQ(findInstrumentChange(), nullptr);
    EXPECT_FALSE(m_notation->undoStack()->canUndo());
}

TEST_F(Notation_NotationInteractionTests, AddInstrumentChange_NotationClosedBeforeTheAnswer_DoesNotCrash)
{
    //! [GIVEN] The user is asked for the instrument
    selectFirstChordRest();
    EXPECT_CALL(*m_selectInstrumentScenario, selectInstrument(_)).WillOnce(Return(pendingAnswer()));

    m_notation->interaction()->addText(TextStyleType::INSTRUMENT_CHANGE);
    async::processMessages();

    //! [WHEN] The notation is closed, and then the user chooses an instrument
    m_notation.reset();

    const InstrumentTemplate* flute = searchTemplate(u"flute");
    ASSERT_TRUE(flute);
    (void)m_resolve(*flute);
    async::processMessages();

    //! [THEN] The answer is ignored (checked by ASan)
}

TEST_F(Notation_NotationInteractionTests, ApplyInstrumentChangeFromPalette_UserChoosesAnInstrument_AppliesItInOneUndoStep)
{
    //! [GIVEN] A chord is selected, and the user is asked for the instrument
    selectFirstChordRest();
    EXPECT_CALL(*m_selectInstrumentScenario, selectInstrument(_)).WillOnce(Return(pendingAnswer()));

    //! [WHEN] Applying an instrument change from the palette, which is gone before the answer
    {
        std::unique_ptr<InstrumentChange> paletteElement(Factory::createInstrumentChange(score()->dummy()));
        EXPECT_TRUE(m_notation->interaction()->applyPaletteElement(paletteElement.get()));
    }
    async::processMessages();

    //! [THEN] Nothing is changed until the user answers
    EXPECT_EQ(findInstrumentChange(), nullptr);
    EXPECT_FALSE(m_notation->undoStack()->canUndo());

    //! [WHEN] The user chooses an instrument
    const InstrumentTemplate* flute = searchTemplate(u"flute");
    ASSERT_TRUE(flute);
    (void)m_resolve(*flute);
    async::processMessages();

    //! [THEN] The instrument change to that instrument is added
    InstrumentChange* instrumentChange = findInstrumentChange();
    ASSERT_TRUE(instrumentChange);
    EXPECT_TRUE(instrumentChange->init());
    EXPECT_EQ(instrumentChange->instrument()->id(), flute->id);

    //! [THEN] It is undone in one step
    m_notation->undoStack()->undo(nullptr);
    EXPECT_EQ(findInstrumentChange(), nullptr);
    EXPECT_FALSE(m_notation->undoStack()->canUndo());
}

TEST_F(Notation_NotationInteractionTests, ApplyInstrumentChangeFromPalette_UserCancels_AppliesNothing)
{
    //! [GIVEN] A chord is selected, and the user is asked for the instrument
    selectFirstChordRest();
    EXPECT_CALL(*m_selectInstrumentScenario, selectInstrument(_)).WillOnce(Return(pendingAnswer()));

    std::unique_ptr<InstrumentChange> paletteElement(Factory::createInstrumentChange(score()->dummy()));
    m_notation->interaction()->applyPaletteElement(paletteElement.get());
    async::processMessages();

    //! [WHEN] The user cancels
    (void)m_reject(int(Ret::Code::Cancel), "");
    async::processMessages();

    //! [THEN] Nothing is applied, and there is nothing to undo
    EXPECT_EQ(findInstrumentChange(), nullptr);
    EXPECT_FALSE(m_notation->undoStack()->canUndo());
}

TEST_F(Notation_NotationInteractionTests, DropInstrumentChange_WithTheChosenInstrument_AddsItInOneUndoStep)
{
    //! [GIVEN] An instrument change is dragged onto the first chord
    std::unique_ptr<InstrumentChange> paletteElement(Factory::createInstrumentChange(score()->dummy()));
    ASSERT_TRUE(m_notation->interaction()->startDropSingle(paletteElement->mimeData().toQByteArray()));
    EXPECT_TRUE(m_notation->interaction()->isDroppingInstrumentChange());

    Segment* segment = score()->firstMeasure()->first(SegmentType::ChordRest);
    m_notation->interaction()->setDropTarget(segment->element(0));

    //! [THEN] The instrument is not asked during the drop, it is chosen before
    EXPECT_CALL(*m_selectInstrumentScenario, selectInstrument(_)).Times(0);

    //! [WHEN] It is dropped with the chosen instrument
    const InstrumentTemplate* flute = searchTemplate(u"flute");
    ASSERT_TRUE(flute);
    EXPECT_TRUE(m_notation->interaction()->dropSingle(PointF(), Qt::NoModifier, *flute));
    m_notation->interaction()->endDrop();

    //! [THEN] The instrument change to that instrument is added
    InstrumentChange* instrumentChange = findInstrumentChange();
    ASSERT_TRUE(instrumentChange);
    EXPECT_TRUE(instrumentChange->init());
    EXPECT_EQ(instrumentChange->instrument()->id(), flute->id);

    //! [THEN] It is undone in one step
    m_notation->undoStack()->undo(nullptr);
    EXPECT_EQ(findInstrumentChange(), nullptr);
    EXPECT_FALSE(m_notation->undoStack()->canUndo());
}
