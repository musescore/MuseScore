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

#include "engraving/dom/box.h"
#include "engraving/dom/measure.h" // IWYU pragma: keep
#include "engraving/dom/page.h"
#include "engraving/editing/editpagelocks.h"
#include "engraving/editing/editsystemlocks.h"
#include "engraving/editing/transaction/transaction.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

static const String PAGE_LOCKS_DATA_DIR("page_locks_data/");

class Engraving_PageLocksTests : public ::testing::Test
{
};

TEST_F(Engraving_PageLocksTests, readLocksFromFile)
{
    MasterScore* score = ScoreRW::readScore(PAGE_LOCKS_DATA_DIR + u"page_locks-1.mscx");
    ASSERT_TRUE(score);

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
    ASSERT_TRUE(score);

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
    ASSERT_TRUE(score);

    MeasureBase* thirdMeasure = score->first()->next()->next();
    ASSERT_TRUE(thirdMeasure);
    MeasureBase* sixthMeasure = thirdMeasure->next()->next()->next();
    ASSERT_TRUE(sixthMeasure);

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
    ASSERT_TRUE(score);

    MeasureBase* thirdMeasure = score->first()->next()->next();
    ASSERT_TRUE(thirdMeasure);
    MeasureBase* sixthMeasure = thirdMeasure->next()->next()->next();
    ASSERT_TRUE(sixthMeasure);

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
    ASSERT_TRUE(score);

    EXPECT_TRUE(score->pages().front()->isLocked());

    score->select(score->firstMeasure(), SelectType::RANGE);

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditPageLocks::togglePageLock(tx, score, score->selection().pagesContainingSelection());
    });

    EXPECT_FALSE(score->pages().front()->isLocked());

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditPageLocks::togglePageLock(tx, score, score->selection().pagesContainingSelection());
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

TEST_F(Engraving_PageLocksTests, removePageLockOnExpandMMRest)
{
    MasterScore* score = ScoreRW::readScore(PAGE_LOCKS_DATA_DIR + u"page_locks-1.mscx");
    ASSERT_TRUE(score);
    score->startCmd(TranslatableString::untranslatable("Enable MM rests"));
    score->undoChangeStyleVal(Sid::createMultiMeasureRests, true);
    score->endCmd();
    score->transactionManager()->transaction(TranslatableString::untranslatable("Unlock pages"), [&](auto& tx) {
        EditPageLocks::undoRemoveAllLocks(tx, score);
    });
    score->transactionManager()->transaction(TranslatableString::untranslatable("Lock compressed layout"), [&](auto& tx) {
        EditPageLocks::toggleScoreLock(tx, score);
    });
    ASSERT_FALSE(score->pageLocks()->allLocks().empty());
    const RangeLock* firstLock = score->pageLocks()->allLocks().front();
    ASSERT_TRUE(firstLock->endMB()->isMeasure());
    ASSERT_TRUE(toMeasure(firstLock->endMB())->isMMRest());
    score->startCmd(TranslatableString::untranslatable("Expand MM rests"));
    score->undoChangeStyleVal(Sid::createMultiMeasureRests, false);
    score->endCmd();
    bool retained = false;
    for (const RangeLock* lock : score->pageLocks()->allLocks()) {
        retained = retained || lock == firstLock;
    }
    EXPECT_FALSE(retained);
    delete score;
}

TEST_F(Engraving_PageLocksTests, lockSystemAtStartOfPageLock)
{
    MasterScore* score = ScoreRW::readScore(PAGE_LOCKS_DATA_DIR + u"page_locks-1.mscx");
    ASSERT_TRUE(score);

    // The second page lock, and the first two measures in it
    std::vector<const RangeLock*> pageLocks = score->pageLocks()->allLocks();
    ASSERT_GE(pageLocks.size(), 2);
    MeasureBase* pageStart = pageLocks.at(1)->startMB();
    MeasureBase* pageEnd = pageLocks.at(1)->endMB();
    MeasureBase* systemEnd = pageStart->next();
    ASSERT_TRUE(systemEnd->isBefore(pageEnd));

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving page locks tests"), [&](auto& tx) {
        EditSystemLocks::undoAddSystemLock(tx, new RangeLock(pageStart, systemEnd));
    });

    // Locking a system that starts together with a page lock leaves the page lock alone
    EXPECT_TRUE(pageStart->isStartOfSystemLock());
    EXPECT_TRUE(systemEnd->isEndOfSystemLock());
    EXPECT_TRUE(pageStart->isStartOfPageLock());
    EXPECT_EQ(pageStart->pageLock()->endMB(), pageEnd);

    delete score;
}

// Create a new page with a range starting/ending on frames (boxes)...
TEST_F(Engraving_PageLocksTests, pageLockFrameRange)
{
    MasterScore* score = ScoreRW::readScore(PAGE_LOCKS_DATA_DIR + u"page_locks-frames.mscx");
    EXPECT_TRUE(score);

    HBox* startBox = nullptr;
    TBox* endBox = nullptr;

    //! [GIVEN] A range starting at the second HBox (horizontal frame) in the given score, and
    //! ending at the first (non title) TBox...
    int boxesFound = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (mb->isHBox() && !startBox) {
            boxesFound++;
            if (boxesFound > 1) {
                startBox = toHBox(mb);
            }
        }
        if (mb->isTBox() && !endBox) {
            TBox* tBox = toTBox(mb);
            if (!tBox->isTitleFrame()) {
                endBox = tBox;
            }
            break;
        }
    }

    IF_ASSERT_FAILED(startBox && endBox) {
        delete score;
        return;
    }

    score->select(startBox, SelectType::SINGLE);
    score->select(endBox, SelectType::RANGE);

    const Selection& sel = score->selection();
    EXPECT_TRUE(sel.isRange());
    EXPECT_TRUE(sel.startMeasureBase() && sel.startMeasureBase()->isHBox());
    EXPECT_TRUE(sel.endMeasureBase() && sel.endMeasureBase()->isTBox());

    //! [WHEN] Adding a page lock over the current selection...
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditPageLocks::applyLockToSelection(tx, score);
    });

    //! [THEN] The result matches our expectations...
    EXPECT_TRUE(startBox->isStartOfPageLock());
    EXPECT_TRUE(endBox->isEndOfPageLock());
    const RangeLock* lock = score->pageLocks()->lockContaining(startBox);
    ASSERT_TRUE(lock);
    EXPECT_EQ(lock->startMB(), startBox);
    EXPECT_EQ(lock->endMB(), endBox);

    delete score;
}
