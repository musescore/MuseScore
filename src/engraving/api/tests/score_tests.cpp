/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"
#include "engraving/editing/editpart.h"

#include "engraving/api/v1/score.h"
#include "engraving/api/v1/part.h"

#include "mocks/globalcontextmock.h"
#include "mocks/notationmock.h"
#include "mocks/notationpartsmock.h"

using namespace mu::engraving;
using namespace mu::notation;
using namespace mu::context;
using ::testing::Return;
using ::testing::_;
using ::testing::NiceMock;

class Engraving_ApiScoreTests : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_globalContext = std::make_shared<NiceMock<GlobalContextMock> >();
        m_notation = std::make_shared<NiceMock<NotationMock> >();
        m_notationParts = std::make_shared<NiceMock<NotationPartsMock> >();
    }

    void TearDown() override
    {
        m_notationParts.reset();
        m_notation.reset();
        m_globalContext.reset();
    }

protected:
    std::shared_ptr<NiceMock<GlobalContextMock> > m_globalContext;
    std::shared_ptr<NiceMock<NotationMock> > m_notation;
    std::shared_ptr<NiceMock<NotationPartsMock> > m_notationParts;
};

//---------------------------------------------------------
//   testReplaceInstrumentAtDomLevel
//   Test that ChangePart correctly replaces the instrument
//   This tests the underlying mechanism used by the Plugin API
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, replaceInstrumentAtDomLevel)
{
    // [GIVEN] A score with a part
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    // Create a part with a default instrument
    Part* part = new Part(score);
    score->appendPart(part);
    score->appendStaff(Factory::createStaff(part));

    ASSERT_EQ(score->parts().size(), 1);
    ASSERT_NE(part, nullptr);

    // Set initial instrument
    Instrument initialInstrument;
    initialInstrument.setId(u"test.initial");
    initialInstrument.setTrackName(u"Initial Instrument");
    part->setInstrument(initialInstrument);

    // Verify initial instrument
    EXPECT_EQ(part->instrumentId(), QString("test.initial"));

    // [WHEN] We replace the instrument using ChangePart
    Instrument newInstrument;
    newInstrument.setId(u"test.replaced");
    newInstrument.setTrackName(u"Replaced Instrument");

    score->startCmd(TranslatableString::untranslatable("Replace instrument test"));
    score->undo(new ChangePart(part, new Instrument(newInstrument), u"Replaced Part"));
    score->endCmd();

    // [THEN] The part's instrument should be changed
    EXPECT_EQ(part->instrumentId(), QString("test.replaced"));
    EXPECT_EQ(part->instrument()->trackName(), muse::String(u"Replaced Instrument"));

    delete score;
}

//---------------------------------------------------------
//   testReplaceInstrumentUndo
//   Test that instrument replacement can be undone
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, replaceInstrumentUndo)
{
    // [GIVEN] A score with a part
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    score->appendStaff(Factory::createStaff(part));

    // Set initial instrument
    Instrument initialInstrument;
    initialInstrument.setId(u"test.original");
    initialInstrument.setTrackName(u"Original Instrument");
    part->setInstrument(initialInstrument);

    EXPECT_EQ(part->instrumentId(), QString("test.original"));

    // [WHEN] We replace the instrument
    Instrument newInstrument;
    newInstrument.setId(u"test.new");
    newInstrument.setTrackName(u"New Instrument");

    score->startCmd(TranslatableString::untranslatable("Replace instrument test"));
    score->undo(new ChangePart(part, new Instrument(newInstrument), u"New Part"));
    score->endCmd();

    // Verify it changed
    EXPECT_EQ(part->instrumentId(), QString("test.new"));

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The instrument should be back to original
    EXPECT_EQ(part->instrumentId(), QString("test.original"));

    delete score;
}

//---------------------------------------------------------
//   testReplaceInstrumentRedo
//   Test that instrument replacement can be redone after undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, replaceInstrumentRedo)
{
    // [GIVEN] A score with a part
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    score->appendStaff(Factory::createStaff(part));

    // Set initial instrument
    Instrument initialInstrument;
    initialInstrument.setId(u"test.initial");
    part->setInstrument(initialInstrument);

    // Replace the instrument
    Instrument newInstrument;
    newInstrument.setId(u"test.replaced");

    score->startCmd(TranslatableString::untranslatable("Replace instrument test"));
    score->undo(new ChangePart(part, new Instrument(newInstrument), u"Replaced"));
    score->endCmd();

    EXPECT_EQ(part->instrumentId(), QString("test.replaced"));

    // Undo
    score->undoRedo(true, nullptr);
    EXPECT_EQ(part->instrumentId(), QString("test.initial"));

    // [WHEN] We redo
    score->undoRedo(false, nullptr);

    // [THEN] The instrument should be replaced again
    EXPECT_EQ(part->instrumentId(), QString("test.replaced"));

    delete score;
}

//---------------------------------------------------------
//   testReplaceInstrumentApiWithMocks
//   Test the Plugin API Score::replaceInstrument() method
//   using mocks to verify the correct call chain
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, replaceInstrumentApiWithMocks)
{
    // [GIVEN] A score with a part and mocked notation layer
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart = new Part(domScore);
    domScore->appendPart(domPart);
    domScore->appendStaff(Factory::createStaff(domPart));

    // Set initial instrument
    Instrument initialInstrument;
    initialInstrument.setId(u"wind.flutes.flute");
    initialInstrument.setTrackName(u"Flute");
    domPart->setInstrument(initialInstrument);

    // Setup mock chain: GlobalContext -> Notation -> NotationParts
    ON_CALL(*m_globalContext, currentNotation())
    .WillByDefault(Return(m_notation));
    ON_CALL(*m_notation, parts())
    .WillByDefault(Return(m_notationParts));

    // [EXPECT] replaceInstrument to be called on NotationParts with correct parameters
    EXPECT_CALL(*m_notationParts, replaceInstrument(_, _, _))
    .Times(1);

    // Register the mock in IOC
    muse::modularity::globalIoc()->registerExport<IGlobalContext>("test", m_globalContext);

    // Create API wrapper for the score
    apiv1::Score apiScore(domScore);

    // Create API wrapper for the part
    apiv1::Part* apiPart = new apiv1::Part(domPart, apiv1::Ownership::SCORE);

    // [WHEN] We call replaceInstrument through the API
    apiScore.replaceInstrument(apiPart, "violin");

    // Cleanup
    muse::modularity::globalIoc()->unregister<IGlobalContext>("test");
    delete apiPart;
    delete domScore;
}

//---------------------------------------------------------
//   testReplaceInstrumentApiNullPart
//   Test that the API handles null part gracefully
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, replaceInstrumentApiNullPart)
{
    // [GIVEN] A score
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);
    apiv1::Score apiScore(domScore);

    // [WHEN/THEN] Calling with null part should not crash
    apiScore.replaceInstrument(nullptr, "strings.violin");

    delete domScore;
}

//---------------------------------------------------------
//   testReplaceInstrumentApiInvalidInstrument
//   Test that the API handles invalid instrument ID gracefully
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, replaceInstrumentApiInvalidInstrument)
{
    // [GIVEN] A score with a part
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart = new Part(domScore);
    domScore->appendPart(domPart);
    domScore->appendStaff(Factory::createStaff(domPart));

    apiv1::Score apiScore(domScore);
    apiv1::Part* apiPart = new apiv1::Part(domPart, apiv1::Ownership::SCORE);

    // [WHEN/THEN] Calling with invalid instrument ID should not crash
    // (instrument templates are not loaded in test environment, so any ID is "invalid")
    apiScore.replaceInstrument(apiPart, "nonexistent.instrument.xyz");

    delete apiPart;
    delete domScore;
}
