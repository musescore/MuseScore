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
#include "engraving/dom/stafftype.h"
#include "engraving/editing/editpart.h"
#include "engraving/types/types.h"

#include "engraving/api/v1/score.h"
#include "engraving/api/v1/part.h"
#include "engraving/api/v1/elements.h"

using namespace mu::engraving;

class Engraving_ApiScoreTests : public ::testing::Test
{
public:
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
    EXPECT_EQ(part->instrumentId(), u"test.initial");

    // [WHEN] We replace the instrument using ChangePart
    Instrument newInstrument;
    newInstrument.setId(u"test.replaced");
    newInstrument.setTrackName(u"Replaced Instrument");

    score->startCmd(TranslatableString::untranslatable("Replace instrument test"));
    score->undo(new ChangePart(part, new Instrument(newInstrument), u"Replaced Part"));
    score->endCmd();

    // [THEN] The part's instrument should be changed
    EXPECT_EQ(part->instrumentId(), u"test.replaced");
    EXPECT_EQ(part->instrument()->trackName(), u"Replaced Instrument");

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

    EXPECT_EQ(part->instrumentId(), u"test.original");

    // [WHEN] We replace the instrument
    Instrument newInstrument;
    newInstrument.setId(u"test.new");
    newInstrument.setTrackName(u"New Instrument");

    score->startCmd(TranslatableString::untranslatable("Replace instrument test"));
    score->undo(new ChangePart(part, new Instrument(newInstrument), u"New Part"));
    score->endCmd();

    // Verify it changed
    EXPECT_EQ(part->instrumentId(), u"test.new");

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The instrument should be back to original
    EXPECT_EQ(part->instrumentId(), u"test.original");

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

    EXPECT_EQ(part->instrumentId(), u"test.replaced");

    // Undo
    score->undoRedo(true, nullptr);
    EXPECT_EQ(part->instrumentId(), u"test.initial");

    // [WHEN] We redo
    score->undoRedo(false, nullptr);

    // [THEN] The instrument should be replaced again
    EXPECT_EQ(part->instrumentId(), u"test.replaced");

    delete score;
}

//---------------------------------------------------------
//   testReplaceInstrumentApi
//   Test the Plugin API Score::replaceInstrument() method
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, replaceInstrumentApi)
{
    // [GIVEN] A score with a part
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart = new Part(domScore);
    domScore->appendPart(domPart);
    domScore->appendStaff(Factory::createStaff(domPart));

    // Set initial instrument
    Instrument initialInstrument;
    initialInstrument.setId(u"test.flute");
    initialInstrument.setTrackName(u"Flute");
    domPart->setInstrument(initialInstrument);

    EXPECT_EQ(domPart->instrumentId(), u"test.flute");

    // Create API wrappers
    apiv1::Score apiScore(domScore);
    apiv1::Part* apiPart = new apiv1::Part(domPart, apiv1::Ownership::SCORE);

    // [WHEN] We call replaceInstrument through the API with a valid instrument
    apiScore.replaceInstrument(apiPart, "violin");

    // [THEN] The instrument should be changed to violin
    EXPECT_EQ(domPart->instrumentId(), u"violin");

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
    apiScore.replaceInstrument(nullptr, "violin");

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

    // Set initial instrument
    Instrument initialInstrument;
    initialInstrument.setId(u"test.initial");
    domPart->setInstrument(initialInstrument);

    apiv1::Score apiScore(domScore);
    apiv1::Part* apiPart = new apiv1::Part(domPart, apiv1::Ownership::SCORE);

    // [WHEN] Calling with invalid instrument ID
    apiScore.replaceInstrument(apiPart, "nonexistent.instrument.xyz");

    // [THEN] Instrument should remain unchanged
    EXPECT_EQ(domPart->instrumentId(), u"test.initial");

    delete apiPart;
    delete domScore;
}

//---------------------------------------------------------
//   testSetPartVisible
//   Test setting part visibility and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, setPartVisible)
{
    // [GIVEN] A score with a visible part
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    score->appendStaff(Factory::createStaff(part));
    EXPECT_TRUE(part->show());

    // [WHEN] We hide the part
    score->startCmd(TranslatableString::untranslatable("Set part visible test"));
    EditPart::setPartVisible(score, part, false);
    score->endCmd();

    // [THEN] The part should be hidden
    EXPECT_FALSE(part->show());

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The part should be visible again
    EXPECT_TRUE(part->show());

    delete score;
}

//---------------------------------------------------------
//   testSetStaffVisible
//   Test setting staff visibility and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, setStaffVisible)
{
    // [GIVEN] A score with a visible staff
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    Staff* staff = Factory::createStaff(part);
    score->appendStaff(staff);
    EXPECT_TRUE(staff->visible());

    // [WHEN] We hide the staff
    score->startCmd(TranslatableString::untranslatable("Set staff visible test"));
    EditPart::setStaffVisible(score, staff, false);
    score->endCmd();

    // [THEN] The staff should be hidden
    EXPECT_FALSE(staff->visible());

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The staff should be visible again
    EXPECT_TRUE(staff->visible());

    delete score;
}

//---------------------------------------------------------
//   testSetPartSharpFlat
//   Test setting sharp/flat preference and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, setPartSharpFlat)
{
    // [GIVEN] A score with a part
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    score->appendStaff(Factory::createStaff(part));

    // [WHEN] We set sharps preference
    score->startCmd(TranslatableString::untranslatable("Set sharp flat test"));
    EditPart::setPartSharpFlat(score, part, PreferSharpFlat::SHARPS);
    score->endCmd();

    // [THEN] The property should be changed
    EXPECT_EQ(part->getProperty(Pid::PREFER_SHARP_FLAT).toInt(), int(PreferSharpFlat::SHARPS));

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The property should be back to default
    EXPECT_NE(part->getProperty(Pid::PREFER_SHARP_FLAT).toInt(), int(PreferSharpFlat::SHARPS));

    delete score;
}

//---------------------------------------------------------
//   testSetInstrumentName
//   Test setting instrument long name and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, setInstrumentName)
{
    // [GIVEN] A score with a part
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    score->appendStaff(Factory::createStaff(part));

    Instrument initialInstrument;
    initialInstrument.setLongName(StaffName(u"Original Name"));
    part->setInstrument(initialInstrument);
    EXPECT_EQ(part->instrument()->longName().toPlainText(), u"Original Name");

    // [WHEN] We set a new instrument name
    score->startCmd(TranslatableString::untranslatable("Set instrument name test"));
    EditPart::setInstrumentName(score, part, Fraction(0, 1), u"New Name");
    score->endCmd();

    // [THEN] The name should be changed
    EXPECT_EQ(part->instrument()->longName().toPlainText(), u"New Name");

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The name should be back to original
    EXPECT_EQ(part->instrument()->longName().toPlainText(), u"Original Name");

    delete score;
}

//---------------------------------------------------------
//   testSetInstrumentAbbreviature
//   Test setting instrument short name and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, setInstrumentAbbreviature)
{
    // [GIVEN] A score with a part
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    score->appendStaff(Factory::createStaff(part));

    Instrument initialInstrument;
    initialInstrument.setShortName(StaffName(u"Orig."));
    part->setInstrument(initialInstrument);
    EXPECT_EQ(part->instrument()->shortName().toPlainText(), u"Orig.");

    // [WHEN] We set a new abbreviature
    score->startCmd(TranslatableString::untranslatable("Set instrument abbreviature test"));
    EditPart::setInstrumentAbbreviature(score, part, Fraction(0, 1), u"New.");
    score->endCmd();

    // [THEN] The abbreviature should be changed
    EXPECT_EQ(part->instrument()->shortName().toPlainText(), u"New.");

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The abbreviature should be back to original
    EXPECT_EQ(part->instrument()->shortName().toPlainText(), u"Orig.");

    delete score;
}

//---------------------------------------------------------
//   testSetStaffType
//   Test setting staff type and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, setStaffType)
{
    // [GIVEN] A score with a standard staff
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    Staff* staff = Factory::createStaff(part);
    score->appendStaff(staff);

    EXPECT_EQ(staff->staffType(Fraction(0, 1))->type(), StaffTypes::STANDARD);

    // [WHEN] We change to percussion staff type
    score->startCmd(TranslatableString::untranslatable("Set staff type test"));
    EditPart::setStaffType(score, staff, StaffTypes::PERC_DEFAULT);
    score->endCmd();

    // [THEN] The staff type should be changed
    EXPECT_EQ(staff->staffType(Fraction(0, 1))->type(), StaffTypes::PERC_DEFAULT);

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The staff type should be back to standard
    EXPECT_EQ(staff->staffType(Fraction(0, 1))->type(), StaffTypes::STANDARD);

    delete score;
}

//---------------------------------------------------------
//   testSetPartVisibleApi
//   Test the Plugin API Score::setPartVisible() method
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, setPartVisibleApi)
{
    // [GIVEN] A score with a visible part
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart = new Part(domScore);
    domScore->appendPart(domPart);
    domScore->appendStaff(Factory::createStaff(domPart));

    apiv1::Score apiScore(domScore);
    apiv1::Part* apiPart = new apiv1::Part(domPart, apiv1::Ownership::SCORE);

    EXPECT_TRUE(domPart->show());

    // [WHEN] We hide the part via API
    apiScore.setPartVisible(apiPart, false);

    // [THEN] The part should be hidden
    EXPECT_FALSE(domPart->show());

    delete apiPart;
    delete domScore;
}

//---------------------------------------------------------
//   testSetStaffVisibleApi
//   Test the Plugin API Score::setStaffVisible() method
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, setStaffVisibleApi)
{
    // [GIVEN] A score with a visible staff
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart = new Part(domScore);
    domScore->appendPart(domPart);
    Staff* domStaff = Factory::createStaff(domPart);
    domScore->appendStaff(domStaff);

    apiv1::Score apiScore(domScore);
    apiv1::Staff* apiStaff = new apiv1::Staff(domStaff, apiv1::Ownership::SCORE);

    EXPECT_TRUE(domStaff->visible());

    // [WHEN] We hide the staff via API
    apiScore.setStaffVisible(apiStaff, false);

    // [THEN] The staff should be hidden
    EXPECT_FALSE(domStaff->visible());

    delete apiStaff;
    delete domScore;
}

//---------------------------------------------------------
//   testSetInstrumentNameApi
//   Test the Plugin API Score::setInstrumentName() method
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, setInstrumentNameApi)
{
    // [GIVEN] A score with a part
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart = new Part(domScore);
    domScore->appendPart(domPart);
    domScore->appendStaff(Factory::createStaff(domPart));

    Instrument instr;
    instr.setLongName(StaffName(u"Piano"));
    domPart->setInstrument(instr);

    apiv1::Score apiScore(domScore);
    apiv1::Part* apiPart = new apiv1::Part(domPart, apiv1::Ownership::SCORE);

    // [WHEN] We set a new instrument name via API
    apiScore.setInstrumentName(apiPart, "Grand Piano");

    // [THEN] The long name should be changed
    EXPECT_EQ(domPart->instrument()->longName().toPlainText(), u"Grand Piano");

    delete apiPart;
    delete domScore;
}

//---------------------------------------------------------
//   testSetStaffTypeApi
//   Test the Plugin API Score::setStaffType() method
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, setStaffTypeApi)
{
    // [GIVEN] A score with a standard staff
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart = new Part(domScore);
    domScore->appendPart(domPart);
    Staff* domStaff = Factory::createStaff(domPart);
    domScore->appendStaff(domStaff);

    apiv1::Score apiScore(domScore);
    apiv1::Staff* apiStaff = new apiv1::Staff(domStaff, apiv1::Ownership::SCORE);

    EXPECT_EQ(domStaff->staffType(Fraction(0, 1))->type(), StaffTypes::STANDARD);

    // [WHEN] We change to percussion type via API
    apiScore.setStaffType(apiStaff, int(StaffTypes::PERC_DEFAULT));

    // [THEN] The staff type should be changed
    EXPECT_EQ(domStaff->staffType(Fraction(0, 1))->type(), StaffTypes::PERC_DEFAULT);

    delete apiStaff;
    delete domScore;
}
