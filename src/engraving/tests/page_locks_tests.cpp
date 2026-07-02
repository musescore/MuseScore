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

#include "engraving/dom/page.h"
#include "engraving/editing/editpagelocks.h"
#include "engraving/editing/transaction/transaction.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"
#include "utils/testutils.h"

using namespace mu::engraving;

static const String PAGE_LOCKS_DATA_DIR("page_locks_data/");

class Engraving_PageLocksTests : public ::testing::Test
{
};

TEST_F(Engraving_PageLocksTests, readLocksFromFile)
{
    MasterScore* score = ScoreRW::readScore(PAGE_LOCKS_DATA_DIR + u"page_locks-1.mscx");
    EXPECT_TRUE(score);

    std::vector<const RangeLock*> locks = score->pageLocks()->allLocks();
    EXPECT_FALSE(locks.empty());

    for (MeasureBase* mb = score->first()->next(); mb; mb = mb->next()) {
        EXPECT_TRUE(mb->pageLock());
    }

    for (Page* page : score->pages()) {
        EXPECT_TRUE(page->isLocked());
    }

    for (const RangeLock* lock : locks) {
        int measureCount = 0;
        for (MeasureBase* mb = lock->startMB(); mb && mb->isBeforeOrEqual(lock->endMB()); mb = mb->next()) {
            ++measureCount;
        }
        EXPECT_EQ(measureCount, 4);
    }

    delete score;
}

TEST_F(Engraving_PageLocksTests, lockMeasuresPerPage)
{
    MasterScore* score = ScoreRW::readScore(PAGE_LOCKS_DATA_DIR + u"page_locks-1.mscx");
    EXPECT_TRUE(score);

    const RangeLocks* pagelocks = score->pageLocks();
    std::vector<const RangeLock*> allLocks = pagelocks->allLocks();
    EXPECT_FALSE(allLocks.empty());

    score->startCmd(TranslatableString::untranslatable("Engraving page locks tests"));
    score->cmdSelectAll();
    score->endCmd();

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditPageLocks::addRemovePageLocks(tx, score, 0, false); // Remove all locks
    });

    allLocks = pagelocks->allLocks();
    EXPECT_TRUE(allLocks.empty());

    std::vector<MeasureBase*> measuresAtPageStart;
    std::vector<MeasureBase*> measuresAtPageEnd;
    for (Page* page : score->pages()) {
        measuresAtPageStart.push_back(page->firstMeasureBase());
        measuresAtPageEnd.push_back(page->lastMeasureBase());
    }

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditPageLocks::addRemovePageLocks(tx, score, 0, true); // Lock current layout
    });

    for (MeasureBase* mb : measuresAtPageStart) {
        EXPECT_TRUE(mb->isStartOfPageLock());
    }
    for (MeasureBase* mb : measuresAtPageEnd) {
        EXPECT_TRUE(mb->isEndOfPageLock());
    }

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditPageLocks::addRemovePageLocks(tx, score, 4, false); // Add locks every 4 measures
    });

    allLocks = pagelocks->allLocks();
    for (const RangeLock* lock : allLocks) {
        int measureCount = 0;
        for (MeasureBase* mb = lock->startMB(); mb && mb->isBeforeOrEqual(lock->endMB()); mb = mb->next()) {
            ++measureCount;
        }
        EXPECT_EQ(measureCount, 4);
    }

    delete score;
}

TEST_F(Engraving_PageLocksTests, makeIntoPage)
{
    MasterScore* score = ScoreRW::readScore(PAGE_LOCKS_DATA_DIR + u"page_locks-1.mscx");
    EXPECT_TRUE(score);

    MeasureBase* thirdMeasure = score->first()->next()->next();
    EXPECT_TRUE(thirdMeasure);
    MeasureBase* sixthMeasure = thirdMeasure->next()->next()->next();
    EXPECT_TRUE(sixthMeasure);

    EXPECT_NE(thirdMeasure->system(), sixthMeasure->system());

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditPageLocks::makeIntoPage(tx, score, thirdMeasure, sixthMeasure);
    });

    EXPECT_TRUE(thirdMeasure->prev()->isEndOfPageLock());

    EXPECT_TRUE(thirdMeasure->isStartOfPageLock());
    EXPECT_TRUE(sixthMeasure->isEndOfPageLock());

    EXPECT_TRUE(sixthMeasure->next()->isStartOfPageLock());

    delete score;
}

TEST_F(Engraving_PageLocksTests, moveToPreviousNext)
{
    MasterScore* score = ScoreRW::readScore(PAGE_LOCKS_DATA_DIR + u"page_locks-1.mscx");
    EXPECT_TRUE(score);

    MeasureBase* thirdMeasure = score->first()->next()->next();
    EXPECT_TRUE(thirdMeasure);
    MeasureBase* sixthMeasure = thirdMeasure->next()->next()->next();
    EXPECT_TRUE(sixthMeasure);

    EXPECT_NE(thirdMeasure->system(), sixthMeasure->system());

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditPageLocks::moveMeasuresToPrevPage(tx, score, sixthMeasure, sixthMeasure);
    });

    EXPECT_TRUE(sixthMeasure->isEndOfPageLock());
    EXPECT_TRUE(sixthMeasure->next()->isStartOfPageLock());

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditPageLocks::moveMeasuresToNextPage(tx, score, thirdMeasure, thirdMeasure);
    });

    EXPECT_TRUE(thirdMeasure->prev()->isEndOfPageLock());
    EXPECT_TRUE(thirdMeasure->isStartOfPageLock());

    delete score;
}

TEST_F(Engraving_PageLocksTests, togglePageLock)
{
    MasterScore* score = ScoreRW::readScore(PAGE_LOCKS_DATA_DIR + u"page_locks-1.mscx");
    EXPECT_TRUE(score);

    EXPECT_TRUE(score->pages().front()->isLocked());

    score->select(score->firstMeasure(), SelectType::RANGE);

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditPageLocks::togglePageLock(tx, score, score->selection().selectedPages());
    });

    EXPECT_FALSE(score->pages().front()->isLocked());

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditPageLocks::togglePageLock(tx, score, score->selection().selectedPages());
    });

    EXPECT_TRUE(score->pages().front()->isLocked());

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditPageLocks::toggleScoreLock(tx, score);
    });

    for (Page* page : score->pages()) {
        EXPECT_FALSE(page->isLocked());
    }

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditPageLocks::toggleScoreLock(tx, score);
    });

    for (Page* page : score->pages()) {
        EXPECT_TRUE(page->isLocked());
    }

    delete score;
}
