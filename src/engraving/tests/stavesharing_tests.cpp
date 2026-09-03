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

#include <gtest/gtest.h>

#include "engraving/dom/measure.h"
#include "engraving/dom/part.h"
#include "engraving/dom/sharedpart.h"

#include "engraving/editing/editstavesharing.h"
#include "engraving/editing/transaction/transaction.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu::engraving;

static const String STAVE_SHARING_DIR(u"stavesharing_data/");

class Engraving_StaveSharingTests : public ::testing::Test
{
};

void collectSharedAndOriginParts(MasterScore* score, SharedPart** sharedPart, std::vector<Part*>& originParts)
{
    *sharedPart = nullptr;
    originParts.clear();

    for (Part* part : score->parts()) {
        if (part->isSharedPart()) {
            *sharedPart = toSharedPart(part);
        } else {
            originParts.push_back(part);
        }
    }
}

bool checkSharedPartExist(SharedPart* sharedPart, const std::vector<Part*>& originParts)
{
    if (!(sharedPart && sharedPart->originParts() == originParts)) {
        return false;
    }

    for (Part* part : originParts) {
        if (part->sharedPart() != sharedPart) {
            return false;
        }
    }

    return true;
}

bool checkSharedPartNotExist(SharedPart* sharedPart, const std::vector<Part*>& originParts)
{
    if (sharedPart) {
        return false;
    }

    for (Part* part : originParts) {
        if (part->sharedPart()) {
            return false;
        }
    }

    return true;
}

TEST_F(Engraving_StaveSharingTests, testCreateSharedPart)
{
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_00.mscz");
    EXPECT_TRUE(score);

    score->transactionManager()->transaction(muse::TranslatableString("staveSharingTest", "Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);

    EXPECT_TRUE(checkSharedPartExist(sharedPart, originParts));

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testCreateSharedPartUndoRedo)
{
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_00.mscz");
    EXPECT_TRUE(score);

    score->transactionManager()->transaction(muse::TranslatableString("staveSharingTest", "Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });

    score->undoRedo(true, nullptr);

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);

    EXPECT_TRUE(checkSharedPartNotExist(sharedPart, originParts));

    score->undoRedo(false, nullptr);

    collectSharedAndOriginParts(score, &sharedPart, originParts);

    EXPECT_TRUE(checkSharedPartExist(sharedPart, originParts));

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testDeleteSharedStaves)
{
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_00.mscz");
    EXPECT_TRUE(score);

    score->transactionManager()->transaction(muse::TranslatableString("staveSharingTest", "Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);

    EXPECT_TRUE(checkSharedPartExist(sharedPart, originParts));

    score->startCmd(muse::TranslatableString("staveSharingTest", "Remove shared part"));
    score->cmdRemovePart(sharedPart);
    score->endCmd();

    collectSharedAndOriginParts(score, &sharedPart, originParts);
    EXPECT_TRUE(checkSharedPartNotExist(sharedPart, originParts));

    score->undoRedo(true, nullptr);

    collectSharedAndOriginParts(score, &sharedPart, originParts);
    EXPECT_TRUE(checkSharedPartExist(sharedPart, originParts));

    Part* partToRemove = originParts.front();
    score->startCmd(muse::TranslatableString("staveSharingTest", "Remove origin part"));
    score->cmdRemovePart(partToRemove);
    score->endCmd();

    collectSharedAndOriginParts(score, &sharedPart, originParts);
    EXPECT_FALSE(partToRemove->sharedPart());
    EXPECT_FALSE(muse::contains(sharedPart->originParts(), partToRemove));

    partToRemove = originParts.front();
    score->startCmd(muse::TranslatableString("staveSharingTest", "Remove origin part"));
    score->cmdRemovePart(partToRemove);
    score->endCmd();

    collectSharedAndOriginParts(score, &sharedPart, originParts);
    EXPECT_FALSE(sharedPart);
    EXPECT_TRUE(originParts.empty());
}

TEST_F(Engraving_StaveSharingTests, testSaveReloadStaveSharing)
{
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_00.mscz");
    EXPECT_TRUE(score);

    score->transactionManager()->transaction(muse::TranslatableString("staveSharingTest", "Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"staveSharing", STAVE_SHARING_DIR + u"staveSharing_00_ref.mscx"));

    delete score;

    score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_00_ref.mscx");
    EXPECT_TRUE(score);

    score->doLayout();

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);

    EXPECT_TRUE(checkSharedPartExist(sharedPart, originParts));

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testChangeDurationAfterUndoingStaveSharing)
{
    // [GIVEN] A score of 1 measure with 2 staves and stave sharing disabled
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_01.mscx");
    EXPECT_TRUE(score);

    // [THEN] Check rest has no shared item yet
    Measure* m1 = score->firstMeasure();
    ChordRest* cr1 = m1->findChordRest(Fraction(0, 1), 0);
    EXPECT_TRUE(cr1 && cr1->isRest());
    ASSERT_FALSE(cr1->sharedItem());

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString("staveSharingTest", "Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });

    // [THEN] Rest should have a shared item now
    ASSERT_TRUE(cr1->sharedItem());

    // [WHEN] Stave sharing is disabled
    score->transactionManager()->transaction(muse::TranslatableString("staveSharingTest", "Disable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, false);
    });

    // [THEN] Rest should stll have a shared item, as the shared parts still exist
    ASSERT_TRUE(cr1->sharedItem());

    // [WHEN] Undo twice
    // Undo "Disable stave sharing"
    score->undoRedo(true, nullptr);
    // Undo "Enable stave sharing" - creation of the stave sharing groups is now undone
    score->undoRedo(true, nullptr);

    // [THEN] The shared item should have been removed from the score and disconnected from the origin rest
    ASSERT_FALSE(cr1->sharedItem());

    // [THEN] Shared parts should have been removed
    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    EXPECT_TRUE(checkSharedPartNotExist(sharedPart, originParts));

    // [WHEN] The rest's duration is changed
    score->startCmd(muse::TranslatableString("staveSharingTest", "Change rest duration"));
    score->changeCRlen(cr1, TDuration(DurationType::V_QUARTER));
    score->endCmd();

    // [THEN] We should not crash

    delete score;
}
