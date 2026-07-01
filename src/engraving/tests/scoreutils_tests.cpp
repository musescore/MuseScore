/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited and others
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
#include "engraving/dom/clef.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"

using namespace mu::engraving;

class Engraving_ScoreUtilsTests : public ::testing::Test
{
public:
};

TEST_F(Engraving_ScoreUtilsTests, StaffIdxSetFromRange)
{
    // [GIVEN] Score containing a bunch of parts/staves (+ linked staves)
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    for (int i = 0; i < 3; ++i) {
        score->appendPart(new Part(score));
    }

    const std::vector<Part*>& parts = score->parts();
    score->appendStaff(Factory::createStaff(parts.at(0)));
    score->appendStaff(Factory::createStaff(parts.at(0)));

    score->appendStaff(Factory::createStaff(parts.at(1)));
    Staff* linkedStaff = static_cast<Staff*>(parts.at(1)->staves().front()->linkedClone());
    score->appendStaff(linkedStaff);

    score->appendStaff(Factory::createStaff(parts.at(2)));
    score->appendStaff(Factory::createStaff(parts.at(2)));
    score->appendStaff(Factory::createStaff(parts.at(2)));

    // [WHEN] Request staves from track 4 to 19
    std::set<staff_idx_t> actualStaffIdxSet = score->staffIdxSetFromRange(4, 19);

    // [THEN] The staves have been filtered by the track range
    std::set<staff_idx_t> expectedStaffIdxSet = { 0, 1, 2, 3, 4, 5, 6 };
    EXPECT_EQ(actualStaffIdxSet, expectedStaffIdxSet);

    // [WHEN] Request staves from track 4 to 19, and also filter out the linked staves
    actualStaffIdxSet = score->staffIdxSetFromRange(4, 19, [](const Staff& staff) {
        return staff.isPrimaryStaff();
    });

    // [THEN] The linked staves have been filtered out
    expectedStaffIdxSet = { 0, 1, 2, 4, 5, 6 };
    EXPECT_EQ(actualStaffIdxSet, expectedStaffIdxSet);

    // [WHEN] Request for more staves than exist
    actualStaffIdxSet = score->staffIdxSetFromRange(4, 9999);

    // [THEN] The set is correct
    expectedStaffIdxSet = { 0, 1, 2, 3, 4, 5, 6 };
    EXPECT_EQ(actualStaffIdxSet, expectedStaffIdxSet);

    delete score;
}

TEST_F(Engraving_ScoreUtilsTests, StaffTestMergeMatchingRests)
{
    // [GIVEN] Score containing a part and staff
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    score->appendPart(new Part(score));
    score->appendStaff(Factory::createStaff(score->parts().at(0)));

    // GIVEN score style setting is false
    score->style().set(Sid::mergeMatchingRests, false);
    // GIVEN staff setting is AUTO
    score->staff(0)->setMergeMatchingRests(AutoOnOff::AUTO);
    // [THEN] rests display unmerged
    EXPECT_FALSE(score->staff(0)->shouldMergeMatchingRests());
    // GIVEN staff setting is ON
    score->staff(0)->setMergeMatchingRests(AutoOnOff::ON);
    // [THEN] rests display merged
    EXPECT_TRUE(score->staff(0)->shouldMergeMatchingRests());
    // GIVEN staff setting is OFF
    score->staff(0)->setMergeMatchingRests(AutoOnOff::OFF);
    // [THEN] rests display unmerged
    EXPECT_FALSE(score->staff(0)->shouldMergeMatchingRests());

    // GIVEN score style setting is true
    score->style().set(Sid::mergeMatchingRests, true);
    // GIVEN staff setting is AUTO
    score->staff(0)->setMergeMatchingRests(AutoOnOff::AUTO);
    // [THEN] rests display merged
    EXPECT_TRUE(score->staff(0)->shouldMergeMatchingRests());
    // GIVEN staff setting is ON
    score->staff(0)->setMergeMatchingRests(AutoOnOff::ON);
    // [THEN] rests display merged
    EXPECT_TRUE(score->staff(0)->shouldMergeMatchingRests());
    // GIVEN staff setting is OFF
    score->staff(0)->setMergeMatchingRests(AutoOnOff::OFF);
    // [THEN] rests display unmerged
    EXPECT_FALSE(score->staff(0)->shouldMergeMatchingRests());

    delete score;
}

TEST_F(Engraving_ScoreUtilsTests, AppendPartFromTablatureTemplate)
{
    // [GIVEN] A score and the Classical Guitar (tablature) instrument template
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    const InstrumentTemplate* t = searchTemplate(String(u"guitar-nylon-tablature"));
    ASSERT_NE(t, nullptr);
    ASSERT_NE(t->staffTypePreset, nullptr);
    ASSERT_EQ(t->staffTypePreset->group(), StaffGroup::TAB);

    // [WHEN] The part is appended via the engraving API used by the plugin layer
    score->appendPart(t);

    // [THEN] The resulting staff is a TAB staff, not a default standard staff
    ASSERT_EQ(score->parts().size(), 1u);
    Part* part = score->parts().front();
    EXPECT_TRUE(part->hasTabStaff());
    EXPECT_FALSE(part->hasPitchedStaff());

    ASSERT_EQ(part->nstaves(), 1u);
    const StaffType* stt = part->staff(0)->staffType(Fraction(0, 1));
    EXPECT_EQ(stt->group(), StaffGroup::TAB);
    EXPECT_EQ(stt->lines(), 6);

    delete score;
}

TEST_F(Engraving_ScoreUtilsTests, AppendPartFromStandardTemplatePreservesClefs)
{
    // [GIVEN] A score and the Piano template (two staves, treble + bass)
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    const InstrumentTemplate* t = searchTemplate(String(u"piano"));
    ASSERT_NE(t, nullptr);

    // [WHEN] The part is appended
    score->appendPart(t);

    // [THEN] Both staves are standard, with the template's per-staff clefs applied
    ASSERT_EQ(score->parts().size(), 1u);
    Part* part = score->parts().front();
    EXPECT_TRUE(part->hasPitchedStaff());
    EXPECT_FALSE(part->hasTabStaff());

    ASSERT_EQ(part->nstaves(), 2u);
    EXPECT_EQ(part->staff(0)->defaultClefType().concertClef, ClefType::G);
    EXPECT_EQ(part->staff(1)->defaultClefType().concertClef, ClefType::F);

    delete score;
}
