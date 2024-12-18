/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "dom/masterscore.h"
#include "dom/spanner.h"

#include "utils/scorerw.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

static const String REMOVE_DATA_DIR("remove_data/");

class Engraving_RemoveTests : public ::testing::Test
{
};

//---------------------------------------------------------
//    For passing to the defined below inStaff() function.
//---------------------------------------------------------

struct StaffCheckData {
    staff_idx_t staffIdx;
    bool staffHasElements;
};

//---------------------------------------------------------
//    for usage with Score::scanElements to check whether
//    the element belongs to a staff with a certain number.
//---------------------------------------------------------

static void inStaff(void* staffCheckData, EngravingItem* e)
{
    StaffCheckData* checkData = static_cast<StaffCheckData*>(staffCheckData);
    if (e->staffIdx() == checkData->staffIdx) {
        LOGD() << e->typeName() << " is in staff " << checkData->staffIdx;
        checkData->staffHasElements = true;
    }
}

static bool staffHasElements(Score* score, staff_idx_t staffIdx)
{
    for (auto i = score->spannerMap().cbegin(); i != score->spannerMap().cend(); ++i) {
        Spanner* s = i->second;
        if (s->staffIdx() == staffIdx) {
            LOGD() << s->typeName() << " is in staff " << staffIdx;
            return true;
        }
    }
    for (Spanner* s : score->unmanagedSpanners()) {
        if (s->staffIdx() == staffIdx) {
            LOGD() << s->typeName() << " is in staff " << staffIdx;
            return true;
        }
    }
    StaffCheckData checkData { staffIdx, false };
    score->scanElements(&checkData, inStaff, true);
    return checkData.staffHasElements;
}

//---------------------------------------------------------
//   removeStaff
//    Checks that after a staff removal all elements
//    belonging to it are not removed in excerpts.
//---------------------------------------------------------

TEST_F(Engraving_RemoveTests, removeStaff)
{
    MasterScore* score = ScoreRW::readScore(REMOVE_DATA_DIR + u"remove_staff.mscx");
    EXPECT_TRUE(score);

    // Remove the second staff and see what happens
    score->startCmd(TranslatableString::untranslatable("Engraving remove tests"));
    score->cmdRemoveStaff(1);
    score->endCmd(false, /*layoutAllParts = */ true);

    EXPECT_FALSE(staffHasElements(score, 1));
    for (Excerpt* ex : score->excerpts()) {
        Score* s = ex->excerptScore();
        EXPECT_TRUE(staffHasElements(s, 1));
    }

    delete score;
}
