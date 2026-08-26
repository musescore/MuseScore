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

#include <algorithm>

#include <gtest/gtest.h>

#include "types/translatablestring.h"

#include "engraving/dom/chordrest.h"
#include "engraving/dom/durationline.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/segment.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

static const String JIANPU_DURATION_LINES_DATA_DIR("beam_data/");

namespace {
struct ChordRestLocator {
    Fraction tick;
    track_idx_t track = 0;
};

ChordRest* findJianpuChordRestWithDurationLines(MasterScore* score)
{
    for (Segment* seg = score->firstSegment(SegmentType::ChordRest); seg; seg = seg->next1(SegmentType::ChordRest)) {
        for (track_idx_t track = 0; track < score->ntracks(); ++track) {
            EngravingItem* item = seg->element(track);
            if (!item || !item->isChordRest()) {
                continue;
            }

            ChordRest* chordRest = toChordRest(item);
            if (!chordRest->isJianpuStaff() || chordRest->durationLines().empty()) {
                continue;
            }

            return chordRest;
        }
    }

    return nullptr;
}

ChordRest* findChordRestByLocator(MasterScore* score, const ChordRestLocator& locator)
{
    for (Segment* seg = score->firstSegment(SegmentType::ChordRest); seg; seg = seg->next1(SegmentType::ChordRest)) {
        for (track_idx_t track = 0; track < score->ntracks(); ++track) {
            EngravingItem* item = seg->element(track);
            if (!item || !item->isChordRest()) {
                continue;
            }

            ChordRest* chordRest = toChordRest(item);
            if (chordRest->track() == locator.track && chordRest->tick() == locator.tick) {
                return chordRest;
            }
        }
    }

    return nullptr;
}

void expectDurationLinesOwnedByChordRest(const ChordRest* chordRest)
{
    ASSERT_FALSE(chordRest->durationLines().empty());

    for (DurationLine* durationLine : chordRest->durationLines()) {
        ASSERT_NE(durationLine, nullptr);
        EXPECT_EQ(durationLine->explicitParent(), chordRest);
        EXPECT_EQ(durationLine->track(), chordRest->track());
    }
}
}

class Engraving_DurationLineTests : public ::testing::Test
{
};

TEST_F(Engraving_DurationLineTests, propertiesAndSpatiumScaling)
{
    MasterScore* score = ScoreRW::readScore(JIANPU_DURATION_LINES_DATA_DIR + u"jianpuBeam.mscx");
    ASSERT_TRUE(score);

    ChordRest* chordRest = findJianpuChordRestWithDurationLines(score);
    ASSERT_TRUE(chordRest);
    ASSERT_FALSE(chordRest->durationLines().empty());

    DurationLine* durationLine = chordRest->durationLines().front();
    ASSERT_TRUE(durationLine);
    EXPECT_EQ(durationLine->chordRest(), chordRest);
    EXPECT_DOUBLE_EQ(durationLine->mag(), chordRest->mag());

    durationLine->setLen(2.0);
    EXPECT_DOUBLE_EQ(durationLine->len(), 2.0);

    durationLine->spatiumChanged(2.0, 3.0);
    EXPECT_DOUBLE_EQ(durationLine->len(), 3.0);

    durationLine->spatiumChanged(0.0, 3.0);
    EXPECT_DOUBLE_EQ(durationLine->len(), 3.0);

    delete score;
}

TEST_F(Engraving_DurationLineTests, detachedDurationLineUsesLocalPosition)
{
    MasterScore* score = ScoreRW::readScore(JIANPU_DURATION_LINES_DATA_DIR + u"jianpuBeam.mscx");
    ASSERT_TRUE(score);

    ChordRest* chordRest = findJianpuChordRestWithDurationLines(score);
    ASSERT_TRUE(chordRest);

    {
        // EngravingObject requires a non-null constructor parent, so detach it afterwards.
        // Scoped so it is destroyed (and unlinked from chordRest) before the score is deleted.
        DurationLine durationLine(chordRest);
        durationLine.resetExplicitParent();
        durationLine.setPos(PointF(2.0, 3.0));

        EXPECT_EQ(durationLine.pagePos(), PointF(2.0, 3.0));
    }

    delete score;
}

TEST_F(Engraving_DurationLineTests, jianpuUndoRedoRemoveChordRestKeepsDurationLinesOwnership)
{
    MasterScore* score = ScoreRW::readScore(JIANPU_DURATION_LINES_DATA_DIR + u"jianpuBeam.mscx");
    ASSERT_TRUE(score);

    ChordRest* original = findJianpuChordRestWithDurationLines(score);
    ASSERT_TRUE(original);
    expectDurationLinesOwnedByChordRest(original);

    const ChordRestLocator locator { original->tick(), original->track() };

    score->startCmd(muse::TranslatableString::untranslatable("Jianpu duration lines tests"));
    score->undoRemoveElement(original);
    score->endCmd();

    EXPECT_EQ(findChordRestByLocator(score, locator), nullptr);

    score->undoRedo(true, nullptr);

    ChordRest* restored = findChordRestByLocator(score, locator);
    ASSERT_TRUE(restored);
    expectDurationLinesOwnedByChordRest(restored);

    score->undoRedo(false, nullptr);
    EXPECT_EQ(findChordRestByLocator(score, locator), nullptr);

    delete score;
}

TEST_F(Engraving_DurationLineTests, jianpuCloneResizeDurationLinesUsesCloneAsOwner)
{
    MasterScore* score = ScoreRW::readScore(JIANPU_DURATION_LINES_DATA_DIR + u"jianpuBeam.mscx");
    ASSERT_TRUE(score);

    ChordRest* source = findJianpuChordRestWithDurationLines(score);
    ASSERT_TRUE(source);
    ASSERT_FALSE(source->durationLines().empty());

    DurationLine* sourceDurationLine = source->durationLines().front();

    EngravingItem* clonedItem = source->clone();
    ASSERT_TRUE(clonedItem);
    ASSERT_TRUE(clonedItem->isChordRest());

    ChordRest* cloned = toChordRest(clonedItem);
    const size_t lineCount = std::max<size_t>(1, source->durationLines().size());
    cloned->resizeDurationLinesTo(lineCount);
    expectDurationLinesOwnedByChordRest(cloned);
    ASSERT_FALSE(cloned->durationLines().empty());

    DurationLine* clonedDurationLine = cloned->durationLines().front();
    clonedDurationLine->setLen(2.5);
    EXPECT_DOUBLE_EQ(clonedDurationLine->len(), 2.5);
    EXPECT_NE(clonedDurationLine, sourceDurationLine);

    delete cloned;
    delete score;
}

TEST_F(Engraving_DurationLineTests, jianpuSaveReloadKeepsDurationLinesOwnership)
{
    String tempFile(u"jianpu-durationlines-save-reload-test.mscx");

    MasterScore* score = ScoreRW::readScore(JIANPU_DURATION_LINES_DATA_DIR + u"jianpuBeam.mscx");
    ASSERT_TRUE(score);

    ChordRest* beforeSave = findJianpuChordRestWithDurationLines(score);
    ASSERT_TRUE(beforeSave);
    expectDurationLinesOwnedByChordRest(beforeSave);

    const ChordRestLocator locator { beforeSave->tick(), beforeSave->track() };

    ASSERT_TRUE(ScoreRW::saveScore(score, tempFile));
    delete score;

    MasterScore* reloaded = ScoreRW::readScore(tempFile, true);
    ASSERT_TRUE(reloaded);

    ChordRest* afterReload = findChordRestByLocator(reloaded, locator);
    ASSERT_TRUE(afterReload);
    expectDurationLinesOwnedByChordRest(afterReload);

    delete reloaded;
}
