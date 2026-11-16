/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "engraving/dom/system.h"
#include "engraving/editing/editsystemlocks.h"
#include "engraving/editing/transaction/transaction.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"
#include "utils/testutils.h"

using namespace mu::engraving;

static const String SYSTEM_LOCKS_DATA_DIR("system_locks_data/");

class Engraving_SystemLocksTests : public ::testing::Test
{
};

TEST_F(Engraving_SystemLocksTests, readLocksFromFile)
{
    MasterScore* score = ScoreRW::readScore(SYSTEM_LOCKS_DATA_DIR + u"system_locks-1.mscx");
    ASSERT_TRUE(score);

    std::vector<const RangeLock*> locks = score->systemLocks()->allLocks();
    EXPECT_FALSE(locks.empty());

    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        EXPECT_TRUE(mb->systemLock());
    }

    for (System* sys : score->systems()) {
        EXPECT_TRUE(sys->isLocked());
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

TEST_F(Engraving_SystemLocksTests, lockMeasuresPerSystem)
{
    MasterScore* score = ScoreRW::readScore(SYSTEM_LOCKS_DATA_DIR + u"system_locks-1.mscx");
    ASSERT_TRUE(score);

    const RangeLocks* systemLocks = score->systemLocks();
    std::vector<const RangeLock*> allLocks = systemLocks->allLocks();
    EXPECT_FALSE(allLocks.empty());

    score->startCmd(TranslatableString::untranslatable("Engraving system locks tests"));
    score->cmdSelectAll();
    score->endCmd();

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditSystemLocks::addRemoveSystemLocks(tx, score, 0, false); // Remove all locks
    });

    allLocks = systemLocks->allLocks();
    EXPECT_TRUE(allLocks.empty());

    std::vector<MeasureBase*> measuresAtSystemStart;
    std::vector<MeasureBase*> measuresAtSystemEnd;
    for (System* sys : score->systems()) {
        measuresAtSystemStart.push_back(sys->first());
        measuresAtSystemEnd.push_back(sys->last());
    }

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditSystemLocks::addRemoveSystemLocks(tx, score, 0, true); // Lock current layout
    });

    for (MeasureBase* mb : measuresAtSystemStart) {
        EXPECT_TRUE(mb->isStartOfSystemLock());
    }
    for (MeasureBase* mb : measuresAtSystemEnd) {
        EXPECT_TRUE(mb->isEndOfSystemLock());
    }

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditSystemLocks::addRemoveSystemLocks(tx, score, 4, false); // Add locks every 4 measures
    });

    allLocks = systemLocks->allLocks();
    for (const RangeLock* lock : allLocks) {
        int measureCount = 0;
        for (MeasureBase* mb = lock->startMB(); mb && mb->isBeforeOrEqual(lock->endMB()); mb = mb->next()) {
            ++measureCount;
        }
        EXPECT_EQ(measureCount, 4);
    }

    delete score;
}

TEST_F(Engraving_SystemLocksTests, makeIntoSystem)
{
    MasterScore* score = ScoreRW::readScore(SYSTEM_LOCKS_DATA_DIR + u"system_locks-1.mscx");
    ASSERT_TRUE(score);

    MeasureBase* thirdMeasure = score->first()->next()->next();
    ASSERT_TRUE(thirdMeasure);
    MeasureBase* sixthMeasure = thirdMeasure->next()->next()->next();
    ASSERT_TRUE(sixthMeasure);

    EXPECT_NE(thirdMeasure->system(), sixthMeasure->system());

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditSystemLocks::makeIntoSystem(tx, score, thirdMeasure, sixthMeasure);
    });

    EXPECT_TRUE(thirdMeasure->prev()->isEndOfSystemLock());

    EXPECT_TRUE(thirdMeasure->isStartOfSystemLock());
    EXPECT_TRUE(sixthMeasure->isEndOfSystemLock());

    EXPECT_TRUE(sixthMeasure->next()->isStartOfSystemLock());

    delete score;
}

TEST_F(Engraving_SystemLocksTests, moveToPreviousNext)
{
    MasterScore* score = ScoreRW::readScore(SYSTEM_LOCKS_DATA_DIR + u"system_locks-1.mscx");
    ASSERT_TRUE(score);

    MeasureBase* thirdMeasure = score->first()->next()->next();
    ASSERT_TRUE(thirdMeasure);
    MeasureBase* sixthMeasure = thirdMeasure->next()->next()->next();
    ASSERT_TRUE(sixthMeasure);

    EXPECT_NE(thirdMeasure->system(), sixthMeasure->system());

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditSystemLocks::moveMeasureToPrevSystem(tx, score, sixthMeasure);
    });

    EXPECT_TRUE(sixthMeasure->isEndOfSystemLock());
    EXPECT_TRUE(sixthMeasure->next()->isStartOfSystemLock());

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditSystemLocks::moveMeasureToNextSystem(tx, score, thirdMeasure);
    });

    EXPECT_TRUE(thirdMeasure->prev()->isEndOfSystemLock());
    EXPECT_TRUE(thirdMeasure->isStartOfSystemLock());

    delete score;
}

TEST_F(Engraving_SystemLocksTests, toggleSystemLock)
{
    MasterScore* score = ScoreRW::readScore(SYSTEM_LOCKS_DATA_DIR + u"system_locks-1.mscx");
    ASSERT_TRUE(score);

    EXPECT_TRUE(score->systems().front()->isLocked());

    score->select(score->first(), SelectType::RANGE);

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditSystemLocks::toggleSystemLock(tx, score, score->selection().selectedSystems());
    });

    EXPECT_FALSE(score->systems().front()->isLocked());

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditSystemLocks::toggleSystemLock(tx, score, score->selection().selectedSystems());
    });

    EXPECT_TRUE(score->systems().front()->isLocked());

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditSystemLocks::toggleScoreLock(tx, score);
    });

    for (System* sys : score->systems()) {
        EXPECT_FALSE(sys->isLocked());
    }

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditSystemLocks::toggleScoreLock(tx, score);
    });

    for (System* sys : score->systems()) {
        EXPECT_TRUE(sys->isLocked());
    }

    delete score;
}

// Create a new system with a range starting/ending on frames (boxes)...
TEST_F(Engraving_SystemLocksTests, systemLockFrameRange)
{
    MasterScore* score = ScoreRW::readScore(SYSTEM_LOCKS_DATA_DIR + u"system_locks-frames.mscx");
    EXPECT_TRUE(score);

    HBox* startBox = nullptr;

    //! [GIVEN] A range starting at the second HBox (horizontal frame) in the given score, and
    //! ending at the final HBox...
    int boxesFound = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (mb->isHBox() && !startBox) {
            boxesFound++;
            if (boxesFound > 1) {
                startBox = toHBox(mb);
            }
        }
    }

    MeasureBase* last = score->last();
    HBox* endBox = last && last->isHBox() ? toHBox(last) : nullptr;

    IF_ASSERT_FAILED(startBox && endBox) {
        delete score;
        return;
    }

    score->select(startBox, SelectType::SINGLE);
    score->select(endBox, SelectType::RANGE);

    const Selection& sel = score->selection();
    EXPECT_TRUE(sel.isRange());
    EXPECT_TRUE(sel.startMeasureBase() && sel.startMeasureBase()->isHBox());
    EXPECT_TRUE(sel.endMeasureBase() && sel.endMeasureBase()->isHBox());

    //! [WHEN] Adding a system lock over the current selection...
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving system locks tests"), [&](auto& tx) {
        EditSystemLocks::applyLockToSelection(tx, score);
    });

    //! [THEN] A system lock spans the selected frames...
    EXPECT_TRUE(startBox->isStartOfSystemLock());
    EXPECT_TRUE(endBox->isEndOfSystemLock());
    const RangeLock* lock = score->systemLocks()->lockContaining(startBox);
    ASSERT_TRUE(lock);
    EXPECT_EQ(lock->startMB(), startBox);
    EXPECT_EQ(lock->endMB(), endBox);

    delete score;
}
