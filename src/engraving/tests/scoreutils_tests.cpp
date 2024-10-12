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

#include "dom/excerpt.h"
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

TEST_F(Engraving_ScoreUtilsTests, StaffTestDefaultMergeMatchingRests)
{
    // [GIVEN] A score to add stuff to
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    const std::vector<Part*>& parts = score->parts();
    Staff* staff = nullptr;

    // [WHEN] style value is false
    score->style().set(Sid::staffDefaultMergeMatchingRests, false);
    // [THEN] appended score value is false
    score->appendPart(new Part(score));
    staff = Factory::createStaff(parts.at(0));
    score->appendStaff(staff);
    EXPECT_EQ(staff->mergeMatchingRests(), false);

    // [WHEN] style value is true
    score->style().set(Sid::staffDefaultMergeMatchingRests, true);
    // [THEN] appended score value is true
    score->appendPart(new Part(score));
    staff = Factory::createStaff(parts.at(1));
    score->appendStaff(staff);
    EXPECT_EQ(staff->mergeMatchingRests(), true);

    delete score;
}

TEST_F(Engraving_ScoreUtilsTests, StaffTestDefaultMergeMatchingRestsWithExcerpts)
{
    // [GIVEN] Score containing a bunch of parts/staves (+ linked staves)
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    for (int i = 0; i < 3; ++i) {
        score->appendPart(new Part(score));
    }
    // [WHEN] master score value is true
    score->style().set(Sid::staffDefaultMergeMatchingRests, true);

    // [THEN] staves added are true
    const std::vector<Part*>& scoreParts = score->parts();
    score->appendStaff(Factory::createStaff(scoreParts.at(0)));
    EXPECT_EQ(score->staves().at(0)->mergeMatchingRests(), true);

    // [WHEN] excerpt score value is false
    Score* nscore = score->createScore();
    nscore->style().set(Sid::staffDefaultMergeMatchingRests, false);
    std::vector<Part*> parts;
    parts.push_back(scoreParts.at(0));
    Excerpt ex(score);
    ex.setExcerptScore(nscore);
    ex.setName(u"voice");
    ex.setParts(parts);
    Excerpt::createExcerpt(&ex);
    EXPECT_TRUE(nscore);

    // [THEN] the added staff should be false
    EXPECT_EQ(ex.excerptScore()->staves().at(0)->mergeMatchingRests(), false);

    // [BUT] new score staves should be false
    score->appendStaff(Factory::createStaff(scoreParts.at(1)));
    EXPECT_EQ(score->staves().at(1)->mergeMatchingRests(), true);
    score->appendStaff(Factory::createStaff(scoreParts.at(2)));
    EXPECT_EQ(score->staves().at(1)->mergeMatchingRests(), true);

    delete score;
}
