/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "dom/factory.h"
#include "dom/part.h"
#include "dom/staff.h"

#include "compat/scoreaccess.h"

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
