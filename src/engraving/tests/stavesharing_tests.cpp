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
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/sharedpart.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/part.h"
#include "engraving/dom/sharedpart.h"
#include "engraving/dom/staff.h"

#include "engraving/editing/editstavesharing.h"
#include "engraving/editing/flip.h"
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
    ASSERT_TRUE(score);

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

TEST_F(Engraving_StaveSharingTests, testChangeChordAfterUndoingStaveSharing)
{
    // [GIVEN] A score of 1 measure with 2 staves, each starting with a chord with a staccato, and stave sharing disabled
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_02.mscx");
    ASSERT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    std::vector<Chord*> chords;
    for (track_idx_t track : { track_idx_t(0), track_idx_t(VOICES) }) {
        ChordRest* cr = m1->findChordRest(Fraction(0, 1), track);
        ASSERT_TRUE(cr && cr->isChord());
        chords.push_back(toChord(cr));
    }

    for (Chord* chord : chords) {
        ASSERT_EQ(chord->notes().size(), 1);
        ASSERT_EQ(chord->articulations().size(), 1);
    }

    // [THEN] Check the chord's children have no shared items yet
    for (Chord* chord : chords) {
        ASSERT_FALSE(chord->notes().front()->sharedItem());
        ASSERT_FALSE(chord->articulations().front()->sharedItem());
    }

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString("staveSharingTest", "Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });

    // [THEN] Notes and articulations should have shared items now, but chords should not
    for (Chord* chord : chords) {
        ASSERT_FALSE(chord->sharedItem());
        ASSERT_TRUE(chord->notes().front()->sharedItem());
        ASSERT_TRUE(chord->articulations().front()->sharedItem());
    }

    // [WHEN] Stave sharing is disabled
    score->transactionManager()->transaction(muse::TranslatableString("staveSharingTest", "Disable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, false);
    });

    // [THEN] Notes and articulations should still have shared items, as the shared parts still exist
    for (Chord* chord : chords) {
        ASSERT_TRUE(chord->notes().front()->sharedItem());
        ASSERT_TRUE(chord->articulations().front()->sharedItem());
    }

    // [WHEN] Undo twice
    // Undo "Disable stave sharing"
    score->undoRedo(true, nullptr);
    // Undo "Enable stave sharing" - creation of the stave sharing groups is now undone
    score->undoRedo(true, nullptr);

    // [THEN] The shared chord's children should have been removed from the score and disconnected from the origin items
    for (Chord* chord : chords) {
        EXPECT_FALSE(chord->notes().front()->sharedItem());
        EXPECT_FALSE(chord->articulations().front()->sharedItem());
    }

    // [THEN] Shared parts should have been removed
    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    EXPECT_TRUE(checkSharedPartNotExist(sharedPart, originParts));

    // [WHEN] The chord's duration is changed
    score->startCmd(muse::TranslatableString("staveSharingTest", "Change chord duration"));
    score->changeCRlen(chords.front(), TDuration(DurationType::V_HALF));
    score->endCmd();

    // [THEN] We should not crash

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testFlipTieAfterUndoingStaveSharing)
{
    // [GIVEN] A score of 1 measure with 2 staves, the first containing a tie, and stave sharing disabled
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_03.mscx");
    ASSERT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ChordRest* cr = m1->findChordRest(Fraction(0, 1), 0);
    ASSERT_TRUE(cr && cr->isChord());
    Tie* tie = toChord(cr)->notes().front()->tieFor();
    ASSERT_TRUE(tie);

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString("staveSharingTest", "Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });

    // [WHEN] Stave sharing is disabled
    score->transactionManager()->transaction(muse::TranslatableString("staveSharingTest", "Disable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, false);
    });

    // [WHEN] Undo twice
    // Undo "Disable stave sharing"
    score->undoRedo(true, nullptr);
    // Undo "Enable stave sharing" - creation of the stave sharing groups is now undone
    score->undoRedo(true, nullptr);

    // [THEN] Shared parts should have been removed
    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    EXPECT_TRUE(checkSharedPartNotExist(sharedPart, originParts));

    // [WHEN] The tie is selected and its direction flipped twice
    ASSERT_FALSE(tie->segmentsEmpty());
    for (int i = 0; i < 2; ++i) {
        score->select(tie->frontSegment());
        score->transactionManager()->transaction(muse::TranslatableString("staveSharingTest", "Flip tie direction"), [&](Transaction& tx) {
            Flip::flip(tx, score);
        });
    }

    // [THEN] We should not crash

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testLocalClefKeyTimeSigsCopiedToSharedStaves)
{
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_04.mscx");
    ASSERT_TRUE(score);

    score->transactionManager()->transaction(muse::TranslatableString("staveSharingTest", "Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    ASSERT_EQ(sharedPart->nstaves(), 2);

    const track_idx_t firstSharedTrack = sharedPart->staff(0)->idx() * VOICES;
    const track_idx_t secondSharedTrack = sharedPart->staff(1)->idx() * VOICES;

    Measure* m2 = score->crMeasure(1);
    Measure* m3 = score->crMeasure(2);
    Measure* m4 = score->crMeasure(3);
    ASSERT_TRUE(m2 && m3 && m4);

    Segment* clefSeg = m3->findSegmentR(SegmentType::Clef, m3->ticks());
    ASSERT_TRUE(clefSeg);
    EXPECT_TRUE(clefSeg->element(firstSharedTrack) && clefSeg->element(firstSharedTrack)->isClef());
    EXPECT_FALSE(clefSeg->element(secondSharedTrack));

    Segment* timeSigSeg = m4->findSegment(SegmentType::TimeSig, m4->tick());
    ASSERT_TRUE(timeSigSeg);
    EXPECT_TRUE(timeSigSeg->element(firstSharedTrack) && timeSigSeg->element(firstSharedTrack)->isTimeSig());
    EXPECT_FALSE(timeSigSeg->element(secondSharedTrack));

    Segment* keySigSeg = m2->findSegment(SegmentType::KeySig, m2->tick());
    ASSERT_TRUE(keySigSeg);
    EXPECT_TRUE(keySigSeg->element(secondSharedTrack) && keySigSeg->element(secondSharedTrack)->isKeySig());
    EXPECT_FALSE(keySigSeg->element(firstSharedTrack));

    delete score;
}
