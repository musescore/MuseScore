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
#include "engraving/dom/drumset.h"
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
#include "engraving/api/v1/instrument.h"
#include "engraving/api/v1/apistructs.h"

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
    initialInstrument.setLongName(u"Original Name");
    part->setInstrument(initialInstrument);
    EXPECT_EQ(part->instrument()->longName(), u"Original Name");

    // [WHEN] We set a new instrument name
    score->startCmd(TranslatableString::untranslatable("Set instrument name test"));
    EditPart::setInstrumentName(score, part, Fraction(0, 1), u"New Name");
    score->endCmd();

    // [THEN] The name should be changed
    EXPECT_EQ(part->instrument()->longName(), u"New Name");

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The name should be back to original
    EXPECT_EQ(part->instrument()->longName(), u"Original Name");

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
    initialInstrument.setShortName(u"Orig.");
    part->setInstrument(initialInstrument);
    EXPECT_EQ(part->instrument()->shortName(), u"Orig.");

    // [WHEN] We set a new abbreviature
    score->startCmd(TranslatableString::untranslatable("Set instrument abbreviature test"));
    EditPart::setInstrumentAbbreviature(score, part, Fraction(0, 1), u"New.");
    score->endCmd();

    // [THEN] The abbreviature should be changed
    EXPECT_EQ(part->instrument()->shortName(), u"New.");

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The abbreviature should be back to original
    EXPECT_EQ(part->instrument()->shortName(), u"Orig.");

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
    instr.setLongName(u"Piano");
    domPart->setInstrument(instr);

    apiv1::Score apiScore(domScore);
    apiv1::Part* apiPart = new apiv1::Part(domPart, apiv1::Ownership::SCORE);

    // [WHEN] We set a new instrument name via API
    apiv1::Fraction tick(Fraction(0, 1));
    apiScore.setInstrumentName(apiPart, &tick, "Grand Piano");

    // [THEN] The long name should be changed
    EXPECT_EQ(domPart->instrument()->longName(), u"Grand Piano");

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

//---------------------------------------------------------
//   testRemoveParts
//   Test removing parts and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, removeParts)
{
    // [GIVEN] A score with two parts
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part1 = new Part(score);
    score->appendPart(part1);
    score->appendStaff(Factory::createStaff(part1));

    Part* part2 = new Part(score);
    score->appendPart(part2);
    score->appendStaff(Factory::createStaff(part2));

    ASSERT_EQ(score->parts().size(), 2);

    // [WHEN] We remove the first part
    score->startCmd(TranslatableString::untranslatable("Remove parts test"));
    EditPart::removeParts(score, { part1 });
    score->endCmd();

    // [THEN] Only one part should remain
    EXPECT_EQ(score->parts().size(), 1);
    EXPECT_EQ(score->parts().front(), part2);

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] Both parts should be back
    EXPECT_EQ(score->parts().size(), 2);

    delete score;
}

//---------------------------------------------------------
//   testRemoveStaves
//   Test removing staves and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, removeStaves)
{
    // [GIVEN] A score with a part containing two staves
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    Staff* staff1 = Factory::createStaff(part);
    score->appendStaff(staff1);
    Staff* staff2 = Factory::createStaff(part);
    score->appendStaff(staff2);

    ASSERT_EQ(score->nstaves(), 2);

    // [WHEN] We remove the second staff
    score->startCmd(TranslatableString::untranslatable("Remove staves test"));
    EditPart::removeStaves(score, { staff2 });
    score->endCmd();

    // [THEN] Only one staff should remain
    EXPECT_EQ(score->nstaves(), 1);

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] Both staves should be back
    EXPECT_EQ(score->nstaves(), 2);

    delete score;
}

//---------------------------------------------------------
//   testMoveParts
//   Test moving parts and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, moveParts)
{
    // [GIVEN] A score with three parts
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part1 = new Part(score);
    score->appendPart(part1);
    score->appendStaff(Factory::createStaff(part1));

    Part* part2 = new Part(score);
    score->appendPart(part2);
    score->appendStaff(Factory::createStaff(part2));

    Part* part3 = new Part(score);
    score->appendPart(part3);
    score->appendStaff(Factory::createStaff(part3));

    ASSERT_EQ(score->parts().size(), 3);
    EXPECT_EQ(score->parts()[0], part1);
    EXPECT_EQ(score->parts()[1], part2);
    EXPECT_EQ(score->parts()[2], part3);

    // [WHEN] We move part3 before part1
    score->startCmd(TranslatableString::untranslatable("Move parts test"));
    EditPart::moveParts(score, { part3 }, part1, false);
    score->endCmd();

    // [THEN] The order should be part3, part1, part2
    EXPECT_EQ(score->staves()[0]->part(), part3);
    EXPECT_EQ(score->staves()[1]->part(), part1);
    EXPECT_EQ(score->staves()[2]->part(), part2);

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The order should be back to original
    EXPECT_EQ(score->staves()[0]->part(), part1);
    EXPECT_EQ(score->staves()[1]->part(), part2);
    EXPECT_EQ(score->staves()[2]->part(), part3);

    delete score;
}

//---------------------------------------------------------
//   testMoveStaves
//   Test moving staves within a part and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, moveStaves)
{
    // [GIVEN] A score with a part containing two staves
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    Staff* staff1 = Factory::createStaff(part);
    score->appendStaff(staff1);
    Staff* staff2 = Factory::createStaff(part);
    score->appendStaff(staff2);

    ASSERT_EQ(score->nstaves(), 2);
    EXPECT_EQ(score->staves()[0], staff1);
    EXPECT_EQ(score->staves()[1], staff2);

    // [WHEN] We move staff2 before staff1
    score->startCmd(TranslatableString::untranslatable("Move staves test"));
    EditPart::moveStaves(score, { staff2 }, staff1, false);
    score->endCmd();

    // [THEN] The order should be staff2, staff1
    EXPECT_EQ(score->staves()[0], staff2);
    EXPECT_EQ(score->staves()[1], staff1);

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The order should be back to original
    EXPECT_EQ(score->staves()[0], staff1);
    EXPECT_EQ(score->staves()[1], staff2);

    delete score;
}

//---------------------------------------------------------
//   testRemovePartsApi
//   Test the Plugin API Score::removeParts() method
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, removePartsApi)
{
    // [GIVEN] A score with two parts
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart1 = new Part(domScore);
    domScore->appendPart(domPart1);
    domScore->appendStaff(Factory::createStaff(domPart1));

    Part* domPart2 = new Part(domScore);
    domScore->appendPart(domPart2);
    domScore->appendStaff(Factory::createStaff(domPart2));

    ASSERT_EQ(domScore->parts().size(), 2);

    apiv1::Score apiScore(domScore);
    apiv1::Part* apiPart1 = new apiv1::Part(domPart1, apiv1::Ownership::SCORE);

    // [WHEN] We remove the first part via API
    apiScore.removeParts({ apiPart1 });

    // [THEN] Only one part should remain
    EXPECT_EQ(domScore->parts().size(), 1);
    EXPECT_EQ(domScore->parts().front(), domPart2);

    delete apiPart1;
    delete domScore;
}

//---------------------------------------------------------
//   testMovePartsApi
//   Test the Plugin API Score::moveParts() method
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, movePartsApi)
{
    // [GIVEN] A score with two parts
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart1 = new Part(domScore);
    domScore->appendPart(domPart1);
    domScore->appendStaff(Factory::createStaff(domPart1));

    Part* domPart2 = new Part(domScore);
    domScore->appendPart(domPart2);
    domScore->appendStaff(Factory::createStaff(domPart2));

    ASSERT_EQ(domScore->parts().size(), 2);

    apiv1::Score apiScore(domScore);
    apiv1::Part* apiPart1 = new apiv1::Part(domPart1, apiv1::Ownership::SCORE);
    apiv1::Part* apiPart2 = new apiv1::Part(domPart2, apiv1::Ownership::SCORE);

    // [WHEN] We move part2 before part1 (insertMode = 0 = BEFORE)
    apiScore.moveParts({ apiPart2 }, apiPart1, 0);

    // [THEN] The order should be part2, part1
    EXPECT_EQ(domScore->staves()[0]->part(), domPart2);
    EXPECT_EQ(domScore->staves()[1]->part(), domPart1);

    delete apiPart1;
    delete apiPart2;
    delete domScore;
}

//---------------------------------------------------------
//   testStaffVisiblePid
//   Test setting staff visibility via Pid and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, staffVisiblePid)
{
    // [GIVEN] A score with a visible staff
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    Staff* staff = Factory::createStaff(part);
    score->appendStaff(staff);

    EXPECT_TRUE(staff->visible());
    EXPECT_EQ(staff->getProperty(Pid::VISIBLE).toBool(), true);

    // [WHEN] We hide the staff via Pid
    score->startCmd(TranslatableString::untranslatable("Staff visible pid test"));
    staff->undoChangeProperty(Pid::VISIBLE, false);
    score->endCmd();

    // [THEN] The staff should be hidden
    EXPECT_FALSE(staff->visible());
    EXPECT_EQ(staff->getProperty(Pid::VISIBLE).toBool(), false);

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The staff should be visible again
    EXPECT_TRUE(staff->visible());

    delete score;
}

//---------------------------------------------------------
//   testStaffCutawayPid
//   Test setting staff cutaway via Pid and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, staffCutawayPid)
{
    // [GIVEN] A score with a staff (cutaway off by default)
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    Staff* staff = Factory::createStaff(part);
    score->appendStaff(staff);

    EXPECT_FALSE(staff->cutaway());

    // [WHEN] We enable cutaway via Pid
    score->startCmd(TranslatableString::untranslatable("Staff cutaway pid test"));
    staff->undoChangeProperty(Pid::STAFF_CUTAWAY, true);
    score->endCmd();

    // [THEN] Cutaway should be enabled
    EXPECT_TRUE(staff->cutaway());

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] Cutaway should be off again
    EXPECT_FALSE(staff->cutaway());

    delete score;
}

//---------------------------------------------------------
//   testStaffHideSystemBarLinePid
//   Test setting hideSystemBarLine via Pid and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, staffHideSystemBarLinePid)
{
    // [GIVEN] A score with a staff
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    Staff* staff = Factory::createStaff(part);
    score->appendStaff(staff);

    EXPECT_FALSE(staff->hideSystemBarLine());

    // [WHEN] We hide the system barline via Pid
    score->startCmd(TranslatableString::untranslatable("Hide system barline pid test"));
    staff->undoChangeProperty(Pid::STAFF_HIDE_SYSTEM_BARLINE, true);
    score->endCmd();

    // [THEN] hideSystemBarLine should be true
    EXPECT_TRUE(staff->hideSystemBarLine());

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] Should be back to false
    EXPECT_FALSE(staff->hideSystemBarLine());

    delete score;
}

//---------------------------------------------------------
//   testStaffMergeMatchingRestsPid
//   Test setting mergeMatchingRests via Pid and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, staffMergeMatchingRestsPid)
{
    // [GIVEN] A score with a staff (mergeMatchingRests defaults to AUTO)
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    Staff* staff = Factory::createStaff(part);
    score->appendStaff(staff);

    EXPECT_EQ(staff->mergeMatchingRests(), AutoOnOff::AUTO);

    // [WHEN] We set mergeMatchingRests to ON via Pid
    score->startCmd(TranslatableString::untranslatable("Merge matching rests pid test"));
    staff->undoChangeProperty(Pid::STAFF_MERGE_MATCHING_RESTS, int(AutoOnOff::ON));
    score->endCmd();

    // [THEN] mergeMatchingRests should be ON
    EXPECT_EQ(staff->mergeMatchingRests(), AutoOnOff::ON);

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] Should be back to AUTO
    EXPECT_EQ(staff->mergeMatchingRests(), AutoOnOff::AUTO);

    delete score;
}

//---------------------------------------------------------
//   testStaffReflectTranspositionPid
//   Test setting reflectTranspositionInLinkedTab via Pid and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, staffReflectTranspositionPid)
{
    // [GIVEN] A score with a staff (reflectTransposition defaults to true)
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    Staff* staff = Factory::createStaff(part);
    score->appendStaff(staff);

    EXPECT_TRUE(staff->reflectTranspositionInLinkedTab());

    // [WHEN] We disable reflectTransposition via Pid
    score->startCmd(TranslatableString::untranslatable("Reflect transposition pid test"));
    staff->undoChangeProperty(Pid::STAFF_REFLECT_TRANSPOSITION, false);
    score->endCmd();

    // [THEN] reflectTransposition should be false
    EXPECT_FALSE(staff->reflectTranspositionInLinkedTab());

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] Should be back to true
    EXPECT_TRUE(staff->reflectTranspositionInLinkedTab());

    delete score;
}

//---------------------------------------------------------
//   testStaffPropertiesApi
//   Test setting staff properties via Plugin API wrapper
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, staffPropertiesApi)
{
    // [GIVEN] A score with a staff
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart = new Part(domScore);
    domScore->appendPart(domPart);
    Staff* domStaff = Factory::createStaff(domPart);
    domScore->appendStaff(domStaff);

    // Wrap in API objects
    apiv1::Staff apiStaff(domStaff, apiv1::Ownership::SCORE);

    // [THEN] Default values should match
    EXPECT_EQ(apiStaff.property("visible").toBool(), true);
    EXPECT_EQ(apiStaff.property("cutaway").toBool(), false);
    EXPECT_EQ(apiStaff.property("hideSystemBarLine").toBool(), false);
    EXPECT_EQ(apiStaff.property("mergeMatchingRests").toInt(), int(AutoOnOff::AUTO));
    EXPECT_EQ(apiStaff.property("reflectTranspositionInLinkedTab").toBool(), true);

    // [WHEN] We set properties via the API wrapper
    domScore->startCmd(TranslatableString::untranslatable("Staff properties api test"));
    apiStaff.setProperty("visible", false);
    apiStaff.setProperty("cutaway", true);
    apiStaff.setProperty("hideSystemBarLine", true);
    apiStaff.setProperty("mergeMatchingRests", int(AutoOnOff::ON));
    apiStaff.setProperty("reflectTranspositionInLinkedTab", false);
    domScore->endCmd();

    // [THEN] Properties should be updated
    EXPECT_EQ(domStaff->visible(), false);
    EXPECT_EQ(domStaff->cutaway(), true);
    EXPECT_EQ(domStaff->hideSystemBarLine(), true);
    EXPECT_EQ(domStaff->mergeMatchingRests(), AutoOnOff::ON);
    EXPECT_EQ(domStaff->reflectTranspositionInLinkedTab(), false);

    // [WHEN] We undo
    domScore->undoRedo(true, nullptr);

    // [THEN] All should be back to defaults
    EXPECT_EQ(domStaff->visible(), true);
    EXPECT_EQ(domStaff->cutaway(), false);
    EXPECT_EQ(domStaff->hideSystemBarLine(), false);
    EXPECT_EQ(domStaff->mergeMatchingRests(), AutoOnOff::AUTO);
    EXPECT_EQ(domStaff->reflectTranspositionInLinkedTab(), true);

    delete domScore;
}

//---------------------------------------------------------
//   testAppendStaff
//   Test appending a new staff to a part and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, appendStaff)
{
    // [GIVEN] A score with a part containing one staff
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    score->appendStaff(Factory::createStaff(part));

    ASSERT_EQ(part->nstaves(), 1);

    // [WHEN] We append a new staff
    score->startCmd(TranslatableString::untranslatable("Append staff test"));
    Staff* newStaff = EditPart::appendStaff(score, part);
    score->endCmd();

    // [THEN] The part should have 2 staves
    ASSERT_NE(newStaff, nullptr);
    EXPECT_EQ(part->nstaves(), 2);

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The part should have 1 staff again
    EXPECT_EQ(part->nstaves(), 1);

    delete score;
}

//---------------------------------------------------------
//   testAppendLinkedStaff
//   Test appending a linked staff and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, appendLinkedStaff)
{
    // [GIVEN] A score with a part containing one staff
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    Staff* sourceStaff = Factory::createStaff(part);
    score->appendStaff(sourceStaff);

    ASSERT_EQ(part->nstaves(), 1);

    // [WHEN] We append a linked staff
    score->startCmd(TranslatableString::untranslatable("Append linked staff test"));
    Staff* linkedStaff = EditPart::appendLinkedStaff(score, sourceStaff, part);
    score->endCmd();

    // [THEN] The part should have 2 staves
    ASSERT_NE(linkedStaff, nullptr);
    EXPECT_EQ(part->nstaves(), 2);

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The part should have 1 staff again
    EXPECT_EQ(part->nstaves(), 1);

    delete score;
}

//---------------------------------------------------------
//   testAppendStaffApi
//   Test the Plugin API Score::appendStaff() method
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, appendStaffApi)
{
    // [GIVEN] A score with a part containing one staff
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart = new Part(domScore);
    domScore->appendPart(domPart);
    domScore->appendStaff(Factory::createStaff(domPart));

    ASSERT_EQ(domPart->nstaves(), 1);

    apiv1::Score apiScore(domScore);
    apiv1::Part* apiPart = new apiv1::Part(domPart, apiv1::Ownership::SCORE);

    // [WHEN] We append a staff via API
    apiv1::Staff* apiStaff = apiScore.appendStaff(apiPart);

    // [THEN] The part should have 2 staves
    EXPECT_NE(apiStaff, nullptr);
    EXPECT_EQ(domPart->nstaves(), 2);

    delete apiPart;
    delete domScore;
}

//---------------------------------------------------------
//   testAppendLinkedStaffApi
//   Test the Plugin API Score::appendLinkedStaff() method
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, appendLinkedStaffApi)
{
    // [GIVEN] A score with a part containing one staff
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart = new Part(domScore);
    domScore->appendPart(domPart);
    Staff* domStaff = Factory::createStaff(domPart);
    domScore->appendStaff(domStaff);

    ASSERT_EQ(domPart->nstaves(), 1);

    apiv1::Score apiScore(domScore);
    apiv1::Part* apiPart = new apiv1::Part(domPart, apiv1::Ownership::SCORE);
    apiv1::Staff* apiSourceStaff = new apiv1::Staff(domStaff, apiv1::Ownership::SCORE);

    // [WHEN] We append a linked staff via API
    apiv1::Staff* apiLinkedStaff = apiScore.appendLinkedStaff(apiSourceStaff, apiPart);

    // [THEN] The part should have 2 staves
    EXPECT_NE(apiLinkedStaff, nullptr);
    EXPECT_EQ(domPart->nstaves(), 2);

    delete apiSourceStaff;
    delete apiPart;
    delete domScore;
}

//---------------------------------------------------------
//   testSetVoiceVisibleOnMainScore
//   Test that setVoiceVisible returns false on main score
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, setVoiceVisibleOnMainScore)
{
    // [GIVEN] A main score (not an excerpt)
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    Staff* staff = Factory::createStaff(part);
    score->appendStaff(staff);

    // [WHEN] We try to set voice visible on the main score
    bool result = EditPart::setVoiceVisible(score, staff, 0, false);

    // [THEN] It should return false (only works on excerpts)
    EXPECT_FALSE(result);

    delete score;
}

//---------------------------------------------------------
//   testSetVoiceVisibleApi
//   Test the Plugin API Score::setVoiceVisible() method
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, setVoiceVisibleApi)
{
    // [GIVEN] A main score (not an excerpt)
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart = new Part(domScore);
    domScore->appendPart(domPart);
    Staff* domStaff = Factory::createStaff(domPart);
    domScore->appendStaff(domStaff);

    apiv1::Score apiScore(domScore);
    apiv1::Staff* apiStaff = new apiv1::Staff(domStaff, apiv1::Ownership::SCORE);

    // [WHEN] We try to set voice visible via API on main score
    bool result = apiScore.setVoiceVisible(apiStaff, 0, false);

    // [THEN] It should return false
    EXPECT_FALSE(result);

    delete apiStaff;
    delete domScore;
}

//---------------------------------------------------------
//   testReplaceDrumset
//   Test replacing a drumset and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, replaceDrumset)
{
    // [GIVEN] A score with a percussion part
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    score->appendStaff(Factory::createStaff(part));

    // Set up a percussion instrument with a drumset
    Instrument percInstrument;
    percInstrument.setId(u"drumset");
    Drumset ds;
    ds.drum(36).name = u"Bass Drum";
    ds.drum(36).notehead = NoteHeadGroup::HEAD_NORMAL;
    ds.drum(36).line = 7;
    ds.drum(36).voice = 0;
    ds.drum(36).stemDirection = DirectionV::DOWN;
    percInstrument.setDrumset(&ds);
    part->setInstrument(percInstrument);

    EXPECT_EQ(part->instrument()->drumset()->name(36), u"Bass Drum");

    // [WHEN] We replace the drumset with modified values
    Drumset newDs(ds);
    newDs.drum(36).name = u"Kick Drum";

    score->startCmd(TranslatableString::untranslatable("Replace drumset test"));
    EditPart::replaceDrumset(score, part, u"drumset", newDs);
    score->endCmd();

    // [THEN] The drumset should have the new name
    EXPECT_EQ(part->instrument()->drumset()->name(36), u"Kick Drum");

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] The drumset should be back to original
    EXPECT_EQ(part->instrument()->drumset()->name(36), u"Bass Drum");

    delete score;
}

//---------------------------------------------------------
//   testReplaceDrumsetApi
//   Test the Plugin API replaceDrumset workflow
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, replaceDrumsetApi)
{
    // [GIVEN] A score with a percussion part
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart = new Part(domScore);
    domScore->appendPart(domPart);
    domScore->appendStaff(Factory::createStaff(domPart));

    // Set up a percussion instrument with a drumset
    Instrument percInstrument;
    percInstrument.setId(u"drumset");
    Drumset ds;
    ds.drum(36).name = u"Bass Drum";
    ds.drum(36).notehead = NoteHeadGroup::HEAD_NORMAL;
    ds.drum(36).line = 7;
    ds.drum(36).voice = 0;
    ds.drum(36).stemDirection = DirectionV::DOWN;
    percInstrument.setDrumset(&ds);
    domPart->setInstrument(percInstrument);

    EXPECT_EQ(domPart->instrument()->drumset()->name(36), u"Bass Drum");

    apiv1::Score apiScore(domScore);
    apiv1::Part* apiPart = new apiv1::Part(domPart, apiv1::Ownership::SCORE);

    // [WHEN] We clone the drumset via API, modify it, and replace
    apiv1::Instrument apiInstr(domPart->instrument(), domPart);
    apiv1::Drumset* cloned = apiInstr.cloneDrumset();
    ASSERT_NE(cloned, nullptr);

    cloned->setName(36, "Kick Drum");
    apiScore.replaceDrumset(apiPart, cloned);

    // [THEN] The drumset should have the new name
    EXPECT_EQ(domPart->instrument()->drumset()->name(36), u"Kick Drum");

    delete cloned;
    delete apiPart;
    delete domScore;
}

//---------------------------------------------------------
//   testInsertPart
//   Test inserting a part at a given index and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, insertPart)
{
    // [GIVEN] A score with one part
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part1 = new Part(score);
    score->appendPart(part1);
    score->appendStaff(Factory::createStaff(part1));

    Instrument instr1;
    instr1.setId(u"test.first");
    part1->setInstrument(instr1);

    ASSERT_EQ(score->parts().size(), 1);

    // Load instrument templates so searchTemplate works
    const InstrumentTemplate* violinTempl = searchTemplate(u"violin");
    if (!violinTempl) {
        GTEST_SKIP() << "Instrument templates not loaded";
    }

    // [WHEN] We insert a part at index 0
    score->startCmd(TranslatableString::untranslatable("Insert part test"));
    EditPart::insertPart(score, violinTempl, 0);
    score->endCmd();

    // [THEN] Two parts should exist, the new one at index 0
    EXPECT_EQ(score->parts().size(), 2);
    EXPECT_EQ(score->parts()[0]->instrumentId(), u"violin");

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] Should be back to one part
    EXPECT_EQ(score->parts().size(), 1);

    delete score;
}

//---------------------------------------------------------
//   testReplacePart
//   Test replacing a part and undo
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, replacePart)
{
    // [GIVEN] A score with one part
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Part* part = new Part(score);
    score->appendPart(part);
    score->appendStaff(Factory::createStaff(part));

    Instrument instr;
    instr.setId(u"test.original");
    part->setInstrument(instr);

    ASSERT_EQ(score->parts().size(), 1);

    const InstrumentTemplate* violinTempl = searchTemplate(u"violin");
    if (!violinTempl) {
        GTEST_SKIP() << "Instrument templates not loaded";
    }

    // [WHEN] We replace the part
    score->startCmd(TranslatableString::untranslatable("Replace part test"));
    EditPart::replacePart(score, part, violinTempl);
    score->endCmd();

    // [THEN] The part should be replaced with violin
    EXPECT_EQ(score->parts().size(), 1);
    EXPECT_EQ(score->parts()[0]->instrumentId(), u"violin");

    // [WHEN] We undo
    score->undoRedo(true, nullptr);

    // [THEN] Should be back to the original part
    EXPECT_EQ(score->parts().size(), 1);
    EXPECT_EQ(score->parts()[0]->instrumentId(), u"test.original");

    delete score;
}

//---------------------------------------------------------
//   testInsertPartApi
//   Test the Plugin API Score::insertPart() method
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, insertPartApi)
{
    // [GIVEN] A score with one part
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart = new Part(domScore);
    domScore->appendPart(domPart);
    domScore->appendStaff(Factory::createStaff(domPart));

    ASSERT_EQ(domScore->parts().size(), 1);

    const InstrumentTemplate* violinTempl = searchTemplate(u"violin");
    if (!violinTempl) {
        GTEST_SKIP() << "Instrument templates not loaded";
    }

    apiv1::Score apiScore(domScore);

    // [WHEN] We insert a part via API
    apiScore.insertPart("violin", 0);

    // [THEN] Two parts should exist
    EXPECT_EQ(domScore->parts().size(), 2);
    EXPECT_EQ(domScore->parts()[0]->instrumentId(), u"violin");

    delete domScore;
}

//---------------------------------------------------------
//   testReplacePartApi
//   Test the Plugin API Score::replacePart() method
//---------------------------------------------------------

TEST_F(Engraving_ApiScoreTests, replacePartApi)
{
    // [GIVEN] A score with one part
    MasterScore* domScore = compat::ScoreAccess::createMasterScore(nullptr);

    Part* domPart = new Part(domScore);
    domScore->appendPart(domPart);
    domScore->appendStaff(Factory::createStaff(domPart));

    Instrument instr;
    instr.setId(u"test.original");
    domPart->setInstrument(instr);

    ASSERT_EQ(domScore->parts().size(), 1);

    const InstrumentTemplate* violinTempl = searchTemplate(u"violin");
    if (!violinTempl) {
        GTEST_SKIP() << "Instrument templates not loaded";
    }

    apiv1::Score apiScore(domScore);
    apiv1::Part* apiPart = new apiv1::Part(domPart, apiv1::Ownership::SCORE);

    // [WHEN] We replace the part via API
    apiScore.replacePart(apiPart, "violin");

    // [THEN] The part should be replaced
    EXPECT_EQ(domScore->parts().size(), 1);
    EXPECT_EQ(domScore->parts()[0]->instrumentId(), u"violin");

    delete apiPart;
    delete domScore;
}
