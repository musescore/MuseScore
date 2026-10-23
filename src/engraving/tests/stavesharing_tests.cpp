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

#include "engraving/dom/beam.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/sharedpart.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/stavesharingchange.h"
#include "engraving/dom/system.h"
#include "engraving/dom/timesig.h"

#include "engraving/editing/editclef.h"
#include "engraving/editing/editstavesharing.h"
#include "engraving/editing/edittimesig.h"
#include "engraving/editing/flip.h"
#include "engraving/editing/noteinput.h"
#include "engraving/editing/splitjoinmeasure.h"
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
        } else if (part->sharedPart()) {
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

static std::vector<Fraction> userModifiedStaveSharingTicks(const Score* score, const SharedPart* sharedPart)
{
    std::vector<Fraction> ticks;
    for (const auto& [tick, state] : sharedPart->staveSharingStatesBetweenTicks(Fraction(0, 1), score->endTick())) {
        if (state.isUserModified()) {
            ticks.push_back(tick);
        }
    }
    return ticks;
}

void checkSharedKeysMatchOrigins(const SharedPart* sharedPart, const Fraction& tick)
{
    // Each shared staff in use should show the same written key as the origin part mapped to its first voice
    size_t checkedStaves = 0;
    for (const Staff* sharedStaff : sharedPart->staves()) {
        const Part* originPart = sharedPart->originPartForStaff(sharedStaff->idx(), tick);
        if (!originPart) {
            continue;
        }

        ++checkedStaves;
        EXPECT_EQ(sharedStaff->key(tick), originPart->staff(0)->key(tick))
            << "shared staff " << sharedStaff->idx() << " at tick " << tick.ticks();
    }

    EXPECT_GT(checkedStaves, 0);
}

void toggleConcertPitch(MasterScore* score)
{
    for (bool concertPitch : { true, false }) {
        score->startCmd(muse::TranslatableString::untranslatable("Toggle concert pitch"));
        score->cmdConcertPitchChanged(concertPitch);
        score->endCmd();
    }
}

TEST_F(Engraving_StaveSharingTests, testCreateSharedPart)
{
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_00.mscz");
    ASSERT_TRUE(score);

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
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
    ASSERT_TRUE(score);

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
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
    ASSERT_TRUE(score);

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);

    EXPECT_TRUE(checkSharedPartExist(sharedPart, originParts));

    score->startCmd(muse::TranslatableString::untranslatable("Remove shared part"));
    score->cmdRemovePart(sharedPart);
    score->endCmd();

    collectSharedAndOriginParts(score, &sharedPart, originParts);
    EXPECT_TRUE(checkSharedPartNotExist(sharedPart, originParts));

    score->undoRedo(true, nullptr);

    collectSharedAndOriginParts(score, &sharedPart, originParts);
    EXPECT_TRUE(checkSharedPartExist(sharedPart, originParts));

    Part* partToRemove = originParts.front();
    score->startCmd(muse::TranslatableString::untranslatable("Remove origin part"));
    score->cmdRemovePart(partToRemove);
    score->endCmd();

    collectSharedAndOriginParts(score, &sharedPart, originParts);
    EXPECT_FALSE(partToRemove->sharedPart());
    EXPECT_FALSE(muse::contains(sharedPart->originParts(), partToRemove));

    partToRemove = originParts.front();
    score->startCmd(muse::TranslatableString::untranslatable("Remove origin part"));
    score->cmdRemovePart(partToRemove);
    score->endCmd();

    collectSharedAndOriginParts(score, &sharedPart, originParts);
    EXPECT_FALSE(sharedPart);
    EXPECT_TRUE(originParts.empty());
}

TEST_F(Engraving_StaveSharingTests, testSaveReloadStaveSharing)
{
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_00.mscz");
    ASSERT_TRUE(score);

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"staveSharing", STAVE_SHARING_DIR + u"staveSharing_00_ref.mscx"));

    delete score;

    score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_00_ref.mscx");
    ASSERT_TRUE(score);

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
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });

    // [THEN] Rest should have a shared item now
    ASSERT_TRUE(cr1->sharedItem());

    // [WHEN] Stave sharing is disabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Disable stave sharing"), [&](Transaction& tx) {
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
    score->startCmd(muse::TranslatableString::untranslatable("Change rest duration"));
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
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });

    // [THEN] Notes and articulations should have shared items now, but chords should not
    for (Chord* chord : chords) {
        ASSERT_FALSE(chord->sharedItem());
        ASSERT_TRUE(chord->notes().front()->sharedItem());
        ASSERT_TRUE(chord->articulations().front()->sharedItem());
    }

    // [WHEN] Stave sharing is disabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Disable stave sharing"), [&](Transaction& tx) {
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
    score->startCmd(muse::TranslatableString::untranslatable("Change chord duration"));
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
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });

    // [WHEN] Stave sharing is disabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Disable stave sharing"), [&](Transaction& tx) {
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
        score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Flip tie direction"), [&](Transaction& tx) {
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

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
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

TEST_F(Engraving_StaveSharingTests, testTimeShiftMovesSharedTupletsOnce)
{
    // [GIVEN] Two flutes, the second with an eighth triplet on beat 2, and stave sharing enabled
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_05.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = score->firstMeasure();
    score->transactionManager()->transaction(muse::TranslatableString("staveSharingTest", "Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();
    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    ASSERT_EQ(originParts.size(), 2);
    const track_idx_t flute1Track = originParts.front()->staff(0)->idx() * VOICES;

    // [THEN] The second flute and the shared staff each have the triplet on beat 2
    std::set<Tuplet*> tuplets;
    for (Segment* s = m1->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        for (EngravingItem* e : s->elist()) {
            if (e && toChordRest(e)->tuplet()) {
                tuplets.insert(toChordRest(e)->tuplet());
            }
        }
    }
    ASSERT_EQ(tuplets.size(), 2);
    for (Tuplet* t : tuplets) {
        ASSERT_EQ(t->tick(), Fraction(1, 4));
    }

    // [WHEN] A quarter is inserted on the first flute's beat 1
    InputState& is = score->inputState();
    is.setDuration(DurationType::V_QUARTER);
    is.setNoteEntryMode(true);
    is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    is.setTrack(flute1Track);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->select(toChord(is.cr())->upNote());
    is.setTrack(flute1Track);
    score->transactionManager()->transaction(muse::TranslatableString("staveSharingTest", "Insert note"), [&](Transaction& tx) {
        NoteInput::addPitch(tx, score, NoteInputParams { 0 }, false, false);
    });

    // [THEN] Both triplets move to beat 3, once
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(m1->ticks(), Fraction(5, 4));
    for (Tuplet* t : tuplets) {
        EXPECT_EQ(t->tick(), Fraction(2, 4));
        EXPECT_EQ(t->elements().front()->tick(), Fraction(2, 4));
    }

    // [WHEN] Undo
    score->undoRedo(true, nullptr);

    // [THEN] Both triplets are back on beat 2
    for (Tuplet* t : tuplets) {
        EXPECT_EQ(t->tick(), Fraction(1, 4));
    }

    // [WHEN] Redo, then remove the inserted quarter
    score->undoRedo(false, nullptr);
    is.setNoteEntryMode(false);
    score->select(toChord(m1->findChordRest(m1->tick(), flute1Track))->upNote());
    score->transactionManager()->transaction(muse::TranslatableString("staveSharingTest", "Remove time"), [&](Transaction&) {
        score->cmdTimeDelete();
    });

    // [THEN] Both triplets move back to beat 2, once
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(m1->ticks(), Fraction(4, 4));
    for (Tuplet* t : tuplets) {
        EXPECT_EQ(t->tick(), Fraction(1, 4));
        EXPECT_EQ(t->elements().front()->tick(), Fraction(1, 4));
    }
    delete score;
}

TEST_F(Engraving_StaveSharingTests, testDefualtStaveSharingPatterns)
{
    // [GIVEN] A score of 5 measures with 2 flute staves
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_15.mscx");
    ASSERT_TRUE(score);

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();
    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    ASSERT_EQ(originParts.size(), 2);

    const track_idx_t firstOriginTrack = originParts.at(0)->staff(0)->idx() * VOICES;
    const track_idx_t secondOriginTrack = originParts.at(1)->staff(0)->idx() * VOICES;
    const track_idx_t sharedVoice1Track = sharedPart->staff(0)->idx() * VOICES;
    const track_idx_t sharedVoice2Track = sharedVoice1Track + 1;

    EXPECT_EQ(firstOriginTrack,  8);
    EXPECT_EQ(secondOriginTrack, 12);
    EXPECT_EQ(sharedVoice1Track, 0);
    EXPECT_EQ(sharedVoice2Track, 1);

    // [THEN] Each measure should have the expected default track mapping
    const std::vector<SharedTrackMap> expectedTrackMaps = {
        { { firstOriginTrack, sharedVoice1Track }, { secondOriginTrack, sharedVoice1Track } }, // m1: unison
        { { firstOriginTrack, sharedVoice1Track }, { secondOriginTrack, sharedVoice2Track } }, // m2: different rhythms, 2
        { { firstOriginTrack, sharedVoice1Track }, { secondOriginTrack, sharedVoice1Track } }, // m3: unison
        { { firstOriginTrack, sharedVoice1Track }, { secondOriginTrack, sharedVoice2Track } }, // m4: 2nd staff has bar rest
        { { firstOriginTrack, sharedVoice1Track }, { secondOriginTrack, sharedVoice2Track } }, // m5: 1st staff has bar rest
    };

    for (size_t i = 0; i < expectedTrackMaps.size(); ++i) {
        Measure* m = score->crMeasure(static_cast<int>(i));
        ASSERT_TRUE(m);
        const SharedTrackMap& trackMap = sharedPart->staveSharingStateAtTick(m->tick()).sharedTrackMap();
        EXPECT_EQ(trackMap, expectedTrackMaps.at(i)) << "Measure " << i + 1 << ": " << dump(trackMap);
    }

    // [THEN] The score should not be corrupt
    EXPECT_TRUE(score->sanityCheck());
    delete score;
}

TEST_F(Engraving_StaveSharingTests, testSharedNestedTupletAddedOnce)
{
    // [GIVEN] Two flutes, each with an eighth triplet nested in a quarter triplet on beat 2, and stave sharing enabled
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_20.mscx");
    ASSERT_TRUE(score);
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();
    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    const track_idx_t sharedTrack = staff2track(sharedPart->staff(0)->idx());

    // [WHEN] The layout is updated twice more
    score->doLayout();
    score->doLayout();

    // [THEN] Each shared tuplet holds its three elements once
    ChordRest* cr = score->firstMeasure()->findChordRest(Fraction(1, 4), sharedTrack);
    ASSERT_TRUE(cr);
    Tuplet* innerTuplet = cr->tuplet();
    ASSERT_TRUE(innerTuplet);
    Tuplet* outerTuplet = innerTuplet->tuplet();
    ASSERT_TRUE(outerTuplet);
    EXPECT_EQ(innerTuplet->elements().size(), 3);
    EXPECT_EQ(outerTuplet->elements().size(), 3);

    delete score;
}
TEST_F(Engraving_StaveSharingTests, testInvalidUserTrackMaps)
{
    // [GIVEN] A score with 3 staves
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_06.mscx");
    ASSERT_TRUE(score);

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    ASSERT_EQ(originParts.size(), 3);
    ASSERT_GE(sharedPart->nstaves(), 2);

    // Track of the first voice of each stave
    const track_idx_t origin1 = originParts.at(0)->staff(0)->idx() * VOICES;
    const track_idx_t origin2 = originParts.at(1)->staff(0)->idx() * VOICES;
    const track_idx_t origin3 = originParts.at(2)->staff(0)->idx() * VOICES;
    const track_idx_t shared1 = sharedPart->staff(0)->idx() * VOICES;
    const track_idx_t shared2 = sharedPart->staff(1)->idx() * VOICES;

    EXPECT_EQ(origin1, 12);
    EXPECT_EQ(origin2, 16);
    EXPECT_EQ(origin3, 20);
    EXPECT_EQ(shared1, 0);
    EXPECT_EQ(shared2, 4);

    const std::vector<SharedTrackMap> userTrackMaps = {
        { { origin1, shared1 }, { origin2, shared2 + 1 }, { origin3, shared1 } }, // m1
        { { origin1, shared1 }, { origin2, shared1 }, { origin3, shared2 } },     // m2
        { { origin1, shared1 }, { origin2, shared1 + 1 }, { origin3, shared1 } }, // m3
    };

    // [WHEN] The invalid user track maps are applied to each measure
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Set track mapping"), [&](Transaction& tx) {
        for (size_t i = 0; i < userTrackMaps.size(); ++i) {
            Measure* m = score->crMeasure(static_cast<int>(i));
            ASSERT_TRUE(m);
            Segment* seg = m->first(SegmentType::ChordRest);
            ASSERT_TRUE(seg);
            for (const auto& [originTrack, sharedTrack] : userTrackMaps.at(i)) {
                EditStaveSharing::setTrackMapping(tx, seg, sharedPart, originTrack, sharedTrack);
            }
        }
    });
    score->doLayout();

    // [THEN] Each user track map should be stored but flagged as invalid
    for (size_t i = 0; i < userTrackMaps.size(); ++i) {
        Measure* m = score->crMeasure(static_cast<int>(i));
        ASSERT_TRUE(m);
        std::optional<StaveSharingState> state = sharedPart->exactStaveSharingStateAtTick(m->tick());
        ASSERT_TRUE(state) << "Measure " << i + 1;
        ASSERT_TRUE(state->userTrackMap()) << "Measure " << i + 1;
        EXPECT_EQ(state->userTrackMap().value(), userTrackMaps.at(i)) << "Measure " << i + 1;
        EXPECT_FALSE(state->isUserMapValid()) << "Measure " << i + 1;
    }

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testUserTrackMapInvalidInLaterSystem)
{
    // [GIVEN] A score of 3 measures, each on its own system, with 3 staves in unison in measure 1 only
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_06.mscx");
    ASSERT_TRUE(score);

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Stave sharing tests"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    ASSERT_EQ(originParts.size(), 3);

    Measure* m1 = score->crMeasure(0);
    Measure* m2 = score->crMeasure(1);
    ASSERT_TRUE(m1 && m2);

    // [WHEN] A stave sharing change is added at the start of the score, with no reset after it
    const SharedTrackMap m1TrackMap = sharedPart->staveSharingStateAtTick(m1->tick()).sharedTrackMap();
    const track_idx_t origin1 = staff2track(originParts.at(0)->staff(0)->idx());
    ASSERT_TRUE(muse::contains(m1TrackMap, origin1)) << dump(m1TrackMap);

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Stave sharing tests"), [&](Transaction& tx) {
        EditStaveSharing::setTrackMapping(tx, m1->first(SegmentType::ChordRest), sharedPart, origin1, m1TrackMap.at(origin1));
    });
    score->doLayout();

    // [THEN] The change should be the only user change in the score, so it applies to the whole score
    ASSERT_EQ(userModifiedStaveSharingTicks(score, sharedPart), std::vector<Fraction>({ Fraction(0, 1) }));

    // [THEN] The change should be flagged as invalid, as the staves are not in unison in measure 2
    std::optional<StaveSharingState> state = sharedPart->exactStaveSharingStateAtTick(m1->tick());
    ASSERT_TRUE(state && state->userTrackMap());
    EXPECT_EQ(state->userTrackMap().value(), m1TrackMap);
    EXPECT_FALSE(state->isUserMapValid());

    // [THEN] Measure 2 should use a computed track map rather than the invalid user one
    const StaveSharingState& m2State = sharedPart->staveSharingStateAtTick(m2->tick());
    EXPECT_FALSE(m2State.isUserModified());
    const SharedTrackMap& m2TrackMap = m2State.sharedTrackMap();
    EXPECT_NE(m2TrackMap, m1TrackMap) << dump(m2TrackMap);

    // [THEN] Measure 2 should not be corrupt - each origin chord/rest should be mirrored on its mapped shared track
    for (const auto& [originTrack, sharedTrack] : m2TrackMap) {
        for (Segment* seg = m2->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
            const EngravingItem* originItem = seg->element(originTrack);
            if (!originItem) {
                continue;
            }
            const EngravingItem* sharedItem = seg->element(sharedTrack);
            ASSERT_TRUE(sharedItem && sharedItem->isChordRest())
                << "Origin track " << originTrack << ", tick " << seg->tick().toString().toStdString();
            EXPECT_EQ(toChordRest(sharedItem)->ticks(), toChordRest(originItem)->ticks())
                << "Origin track " << originTrack << ", tick " << seg->tick().toString().toStdString();
            EXPECT_EQ(sharedItem->type(), originItem->type())
                << "Origin track " << originTrack << ", tick " << seg->tick().toString().toStdString();
        }
    }

    // [THEN] The score should not be corrupt
    EXPECT_TRUE(score->sanityCheck());

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testFillVoice1WithRests)
{
    // [GIVEN] A score of 5 measures with 4 staves, 3 of which are grouped for stave sharing
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_07.mscx");
    ASSERT_TRUE(score);

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Stave sharing tests"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(sharedPart);
    ASSERT_EQ(sharedPart->nstaves(), 3);

    // [WHEN] A stave sharing change is added at beat 3 of measure 3, moving the 3rd origin stave onto the 2nd shared stave
    Segment* seg = score->tick2segment(Fraction(5, 2), true, SegmentType::ChordRest);
    ASSERT_TRUE(seg);

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Stave sharing tests"), [&](Transaction& tx) {
        EditStaveSharing::addStaveSharingChange(tx, seg, sharedPart, false);
    });
    score->doLayout();

    // [THEN] The score should not be corrupt
    EXPECT_TRUE(score->sanityCheck());

    const track_idx_t sharedStave2Track = staff2track(sharedPart->staff(1)->idx());

    // [THEN] Voice 1 of the 2nd shared stave should have measure rests in measures 1 and 2
    for (const Fraction& tick : { Fraction(0, 1), Fraction(1, 1) }) {
        ChordRest* measureRest = score->findCR(tick, sharedStave2Track);
        ASSERT_TRUE(measureRest) << "Tick " << tick.toString().toStdString();
        EXPECT_EQ(measureRest->tick(), tick) << "Tick " << tick.toString().toStdString();
        EXPECT_TRUE(measureRest->isFullMeasureRest()) << "Tick " << tick.toString().toStdString();
    }

    // [THEN] Voice 1 of the 2nd shared stave should be filled with a minim rest up to the stave sharing change
    ChordRest* cr = score->findCR(Fraction(2, 1), sharedStave2Track);
    ASSERT_TRUE(cr);
    EXPECT_EQ(cr->tick(), Fraction(2, 1));
    EXPECT_TRUE(cr->isRest());
    EXPECT_EQ(cr->durationType().type(), DurationType::V_HALF);
    EXPECT_EQ(cr->ticks(), Fraction(1, 2));

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testFillVoice1WithRests2)
{
    // [GIVEN] A score of 5 measures with 4 staves, 3 of which are grouped for stave sharing
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_07.mscx");
    ASSERT_TRUE(score);

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Stave sharing tests"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(sharedPart);
    ASSERT_EQ(sharedPart->nstaves(), 3);

    // [WHEN] A stave sharing change is added at beat 3 of measure 3, moving the 3rd origin stave onto the 2nd shared stave
    Segment* seg = score->tick2segment(Fraction(5, 2), true, SegmentType::ChordRest);
    ASSERT_TRUE(seg);

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Stave sharing tests"), [&](Transaction& tx) {
        EditStaveSharing::addStaveSharingChange(tx, seg, sharedPart, false);
    });
    score->doLayout();

    // [WHEN] The added stave sharing change is edited to move the 2nd origin stave (flute 2) onto the 3rd shared stave
    const TrackRange sharedTracks = sharedPart->trackRange();
    const StaveSharingChange* change = toStaveSharingChange(seg->findAnnotation(ElementType::STAVE_SHARING_CHANGE,
                                                                                sharedTracks.startTrack, sharedTracks.endTrack - 1));
    ASSERT_TRUE(change);
    ASSERT_EQ(change->tick(), Fraction(5, 2));

    const track_idx_t origin2Track = staff2track(originParts.at(1)->staff(0)->idx());
    const track_idx_t sharedStave3Track = staff2track(sharedPart->staff(2)->idx());

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Stave sharing tests"), [&](Transaction& tx) {
        EditStaveSharing::setTrackMapping(tx, change->segment(), sharedPart, origin2Track, sharedStave3Track);
    });
    score->doLayout();

    // [THEN] The score should not be corrupt
    EXPECT_TRUE(score->sanityCheck());

    // [THEN] Voice 1 of the 3rd shared stave should be filled with a minim rest up to the stave sharing change
    ChordRest* cr = score->findCR(Fraction(2, 1), sharedStave3Track);
    ASSERT_TRUE(cr);
    EXPECT_EQ(cr->tick(), Fraction(2, 1));
    EXPECT_TRUE(cr->isRest());
    EXPECT_EQ(cr->durationType().type(), DurationType::V_HALF);
    EXPECT_EQ(cr->ticks(), Fraction(1, 2));

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testStaveSharingChangeMovesWithMeasure)
{
    // [GIVEN] A score of 5 measures of 4/4 with 2 staves
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_15.mscx");
    ASSERT_TRUE(score);

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));

    // [WHEN] A stave sharing change is added at the start of measure 3
    Measure* m3 = score->crMeasure(2);
    ASSERT_TRUE(m3);
    ASSERT_EQ(m3->tick(), Fraction(2, 1));

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Add stave sharing change"), [&](Transaction& tx) {
        EditStaveSharing::addStaveSharingChange(tx, m3->first(SegmentType::ChordRest), sharedPart, false);
    });
    score->doLayout();

    // [THEN] The change should be at the start of measure 3, with a reset at the start of the next system (measure 4)
    const std::vector<Fraction> originalTicks = { Fraction(2, 1), Fraction(3, 1) };
    ASSERT_EQ(userModifiedStaveSharingTicks(score, sharedPart), originalTicks);

    // [WHEN] A measure is inserted before measure 2
    score->startCmd(muse::TranslatableString::untranslatable("Insert measure"));
    score->insertMeasure(score->crMeasure(1));
    score->endCmd();

    // [THEN] The changes should move with their measures
    EXPECT_EQ(m3->tick(), Fraction(3, 1));
    EXPECT_EQ(userModifiedStaveSharingTicks(score, sharedPart), std::vector<Fraction>({ Fraction(3, 1), Fraction(4, 1) }));

    // [WHEN] The insertion is undone
    score->undoRedo(true, nullptr);

    // [THEN] The change should be back at the start of measure 3
    EXPECT_EQ(userModifiedStaveSharingTicks(score, sharedPart), originalTicks);

    // [WHEN] A measure is inserted after measure 4
    score->startCmd(muse::TranslatableString::untranslatable("Insert measure"));
    score->insertMeasure(score->crMeasure(4));
    score->endCmd();

    // [THEN] The changes should not move
    EXPECT_EQ(userModifiedStaveSharingTicks(score, sharedPart), originalTicks);

    score->undoRedo(true, nullptr);

    // [WHEN] Measure 1 is deleted
    score->startCmd(muse::TranslatableString::untranslatable("Delete measure"));
    score->deleteMeasures(score->crMeasure(0), score->crMeasure(0));
    score->endCmd();

    // [THEN] The changes should move with their measures
    EXPECT_EQ(m3->tick(), Fraction(1, 1));
    EXPECT_EQ(userModifiedStaveSharingTicks(score, sharedPart), std::vector<Fraction>({ Fraction(1, 1), Fraction(2, 1) }));

    // [WHEN] The deletion is undone
    score->undoRedo(true, nullptr);

    // [THEN] The change should be back at the start of measure 3
    EXPECT_EQ(userModifiedStaveSharingTicks(score, sharedPart), originalTicks);

    // [WHEN] Measure 3 itself is deleted
    score->startCmd(muse::TranslatableString::untranslatable("Delete measure"));
    score->deleteMeasures(m3, m3);
    score->endCmd();

    // [THEN] The change should be removed along with the measure, and the reset should move back into its place
    EXPECT_EQ(userModifiedStaveSharingTicks(score, sharedPart), std::vector<Fraction>({ Fraction(2, 1) }));

    // [WHEN] The deletion is undone
    score->undoRedo(true, nullptr);

    // [THEN] The change should be restored at the start of measure 3
    EXPECT_EQ(userModifiedStaveSharingTicks(score, sharedPart), originalTicks);

    // [WHEN] A 2/4 time signature is added to measure 2, rewriting the measures containing the changes
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Add time signature"), [&](Transaction& tx) {
        TimeSig* ts = Factory::createTimeSig(score->dummy());
        ts->setSig(Fraction(2, 4));
        EditTimeSig::addTimeSig(tx, score, score->crMeasure(1), 0, ts, false);
    });

    // [THEN] The changes should keep their ticks
    EXPECT_EQ(score->tick2measure(Fraction(2, 1))->timesig(), Fraction(2, 4));
    EXPECT_EQ(userModifiedStaveSharingTicks(score, sharedPart), originalTicks);

    // [WHEN] The time signature change is undone
    score->undoRedo(true, nullptr);

    // [THEN] The changes should still be in place
    EXPECT_EQ(userModifiedStaveSharingTicks(score, sharedPart), originalTicks);

    // [WHEN] Measure 3 is split on beat 3
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Split measure"), [&](Transaction& tx) {
        SplitJoinMeasure::splitMeasure(tx, score, Fraction(5, 2));
    });

    // [THEN] The changes should keep their ticks
    EXPECT_EQ(score->tick2measure(Fraction(5, 2))->tick(), Fraction(5, 2));
    EXPECT_EQ(userModifiedStaveSharingTicks(score, sharedPart), originalTicks);

    // [WHEN] The split is undone
    score->undoRedo(true, nullptr);

    // [THEN] The changes should still be in place
    EXPECT_EQ(userModifiedStaveSharingTicks(score, sharedPart), originalTicks);

    // [WHEN] Measures 3 and 4 are joined
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Join measures"), [&](Transaction& tx) {
        SplitJoinMeasure::joinMeasures(tx, score, Fraction(2, 1), Fraction(3, 1));
    });

    // [THEN] The changes should keep their ticks
    EXPECT_EQ(score->tick2measure(Fraction(3, 1))->tick(), Fraction(2, 1));
    EXPECT_EQ(userModifiedStaveSharingTicks(score, sharedPart), originalTicks);

    // [WHEN] The join is undone
    score->undoRedo(true, nullptr);

    // [THEN] The changes should still be in place
    EXPECT_EQ(userModifiedStaveSharingTicks(score, sharedPart), originalTicks);

    // [THEN] The score should not be corrupt
    EXPECT_TRUE(score->sanityCheck());

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testTieBetweenVoicesAfterStaveSharingChange)
{
    // Test stale ties don't break links between notes and new ties upon removal

    // [GIVEN] A score of 2 measures with 2 shared staves and 2 origin staves, where origin stave 2 has a tie across the barline
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_08.mscx");
    ASSERT_TRUE(score);

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Stave sharing tests"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    ASSERT_EQ(originParts.size(), 2);
    ASSERT_EQ(sharedPart->nstaves(), 2);

    const track_idx_t origin1 = staff2track(originParts.at(0)->staff(0)->idx());
    const track_idx_t origin2 = staff2track(originParts.at(1)->staff(0)->idx());
    const track_idx_t shared1 = staff2track(sharedPart->staff(0)->idx());
    const track_idx_t shared2 = staff2track(sharedPart->staff(1)->idx());

    // [WHEN] A stave sharing change is added at the start of measure 2, mapping origin stave 1 to shared stave 2
    // and origin stave 2 to voice 1 of shared stave 1
    Measure* m2 = score->crMeasure(1);
    ASSERT_TRUE(m2);
    ASSERT_EQ(m2->tick(), Fraction(1, 1));
    Segment* seg = m2->first(SegmentType::ChordRest);
    ASSERT_TRUE(seg);

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Stave sharing tests"), [&](Transaction& tx) {
        EditStaveSharing::addStaveSharingChange(tx, seg, sharedPart, false);
        EditStaveSharing::setTrackMapping(tx, seg, sharedPart, origin1, shared2);
        EditStaveSharing::setTrackMapping(tx, seg, sharedPart, origin2, shared1);
    });
    score->doLayout();

    const SharedTrackMap& m2TrackMap = sharedPart->staveSharingStateAtTick(m2->tick()).sharedTrackMap();
    ASSERT_EQ(m2TrackMap, SharedTrackMap({ { origin1, shared2 }, { origin2, shared1 } })) << dump(m2TrackMap);

    // [THEN] There should be a tie from the note on voice 2 of shared stave 1 at beat 4 of measure 1
    // to the note on voice 1 of shared stave 1 at the start of measure 2
    ChordRest* startCR = score->findCR(Fraction(3, 4), shared1 + 1);
    ASSERT_TRUE(startCR && startCR->isChord());
    ASSERT_EQ(startCR->tick(), Fraction(3, 4));
    ChordRest* endCR = score->findCR(Fraction(1, 1), shared1);
    ASSERT_TRUE(endCR && endCR->isChord());
    ASSERT_EQ(endCR->tick(), Fraction(1, 1));

    ASSERT_EQ(toChord(startCR)->notes().size(), 1);
    ASSERT_EQ(toChord(endCR)->notes().size(), 1);
    Note* startNote = toChord(startCR)->notes().front();
    Note* endNote = toChord(endCR)->notes().front();

    Tie* tie = startNote->tieFor();
    ASSERT_TRUE(tie);
    EXPECT_FALSE(tie->isPartialTie());
    EXPECT_EQ(tie->startNote(), startNote);
    EXPECT_EQ(tie->endNote(), endNote);
    EXPECT_EQ(endNote->tieBack(), tie);

    // [THEN] The score should not be corrupt
    EXPECT_TRUE(score->sanityCheck());

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testMultipleSharedParts)
{
    // Test stave sharing doesn't corrupt the score when it creates more than one shared part

    // [GIVEN] A score of 5 measures with 2 flute staves and 2 oboe staves
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_16.mscx");
    ASSERT_TRUE(score);

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Stave sharing tests"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();

    // [THEN] A shared part should be created for each instrument
    size_t sharedPartCount = 0;
    for (const Part* part : score->parts()) {
        if (part->isSharedPart()) {
            ++sharedPartCount;
        }
    }
    EXPECT_EQ(sharedPartCount, 2);

    // [THEN] The score should not be corrupt
    EXPECT_TRUE(score->sanityCheck());

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testUserTrackMapWithVoiceCrossing)
{
    // Test a user track map is valid when it causes voice crossing, even if voice crossing is disabled in the style

    // [GIVEN] A score of 4 measures with 2 flute staves, in unison in measures 1 and 2, with flute 1 above flute 2 in measures 3 and 4
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_17.mscz");
    ASSERT_TRUE(score);
    ASSERT_FALSE(score->style().styleB(Sid::allowVoiceCrossing));

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Stave sharing tests"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    ASSERT_EQ(originParts.size(), 2);

    const track_idx_t origin1 = staff2track(originParts.at(0)->staff(0)->idx());
    const track_idx_t origin2 = staff2track(originParts.at(1)->staff(0)->idx());
    const track_idx_t shared1 = staff2track(sharedPart->staff(0)->idx());

    // [WHEN] A stave sharing change is added at the start of measure 2, mapping origin stave 1 to voice 2 of shared stave 1
    // and origin stave 2 to voice 1 of shared stave 1
    Measure* m2 = score->crMeasure(1);
    ASSERT_TRUE(m2);
    ASSERT_EQ(m2->tick(), Fraction(1, 1));
    Segment* seg = m2->first(SegmentType::ChordRest);
    ASSERT_TRUE(seg);

    const SharedTrackMap userTrackMap = { { origin1, shared1 + 1 }, { origin2, shared1 } };

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Stave sharing tests"), [&](Transaction& tx) {
        EditStaveSharing::addStaveSharingChange(tx, seg, sharedPart, false);
        for (const auto& [originTrack, sharedTrack] : userTrackMap) {
            EditStaveSharing::setTrackMapping(tx, seg, sharedPart, originTrack, sharedTrack);
        }
    });
    score->doLayout();

    // [THEN] The user track map should be stored and flagged as valid
    std::optional<StaveSharingState> state = sharedPart->exactStaveSharingStateAtTick(m2->tick());
    ASSERT_TRUE(state && state->userTrackMap());
    EXPECT_EQ(state->userTrackMap().value(), userTrackMap) << dump(state->userTrackMap().value());
    EXPECT_TRUE(state->isUserMapValid());

    // [THEN] The user track map should be used from measure 2
    const SharedTrackMap& m2TrackMap = sharedPart->staveSharingStateAtTick(m2->tick()).sharedTrackMap();
    EXPECT_EQ(m2TrackMap, userTrackMap) << dump(m2TrackMap);

    // [THEN] The score should not be corrupt
    EXPECT_TRUE(score->sanityCheck());

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testTimeDeleteAtStaveSharingChangeUndo)
{
    // [GIVEN] A score of 5 measures of 4/4 with 2 staves
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_15.mscx");
    ASSERT_TRUE(score);

    // [GIVEN] Stave sharing is enabled, with a stave sharing change at the start of measure 2
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));

    Measure* m2 = score->crMeasure(1);
    ASSERT_TRUE(m2);
    ASSERT_EQ(m2->tick(), Fraction(1, 1));

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Add stave sharing change"), [&](Transaction& tx) {
        EditStaveSharing::addStaveSharingChange(tx, m2->first(SegmentType::ChordRest), sharedPart, false);
    });
    score->doLayout();

    using SegmentInfo = std::pair<SegmentType, Fraction>;
    auto collectSegments = [m2]() {
        std::vector<SegmentInfo> segments;
        for (Segment* s = m2->first(); s; s = s->next()) {
            segments.emplace_back(s->segmentType(), s->tick());
        }
        return segments;
    };

    auto expectTicksInOrder = [](const std::vector<SegmentInfo>& segments) {
        for (size_t i = 1; i < segments.size(); ++i) {
            EXPECT_LE(segments[i - 1].second, segments[i].second);
        }
    };

    const std::vector<SegmentInfo> segmentsBefore = collectSegments();
    expectTicksInOrder(segmentsBefore);

    // [WHEN] The time of the first note in measure 2 is deleted
    const track_idx_t originTrack = staff2track(originParts.front()->staff(0)->idx());
    ChordRest* cr = m2->findChordRest(m2->tick(), originTrack);
    ASSERT_TRUE(cr && cr->isChord());
    const Fraction deletedTicks = cr->actualTicks();
    score->select(toChord(cr)->upNote());

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Remove time"), [&](Transaction&) {
        score->cmdTimeDelete();
    });
    score->doLayout();

    // [THEN] Measure 2 is shortened, and its segment ticks are in order
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(m2->ticks(), Fraction(4, 4) - deletedTicks);
    expectTicksInOrder(collectSegments());

    // [WHEN] The deletion is undone
    score->undoRedo(true, nullptr);
    score->doLayout();

    // [THEN] The segments are back in their original order
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(m2->ticks(), Fraction(4, 4));
    const std::vector<SegmentInfo> segmentsAfter = collectSegments();
    EXPECT_EQ(segmentsAfter, segmentsBefore);
    expectTicksInOrder(segmentsAfter);

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testHeaderClefsOnSharedStaves)
{
    // [GIVEN] A score with 2 staves, both changing to bass clef at the end of the first measure
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_11.mscx");
    ASSERT_TRUE(score);

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();

    // [THEN] The final system should have 2 visible staves, both with bass header clefs
    Measure* lastMeasure = score->lastMeasure();
    ASSERT_TRUE(lastMeasure);
    System* lastSystem = lastMeasure->system();
    ASSERT_TRUE(lastSystem);
    Measure* firstMeasure = lastSystem->firstMeasure();
    ASSERT_TRUE(firstMeasure);
    Segment* headerClefSeg = firstMeasure->findSegment(SegmentType::HeaderClef, firstMeasure->tick());
    ASSERT_TRUE(headerClefSeg);

    std::vector<staff_idx_t> visibleStaves;
    for (staff_idx_t staffIdx = 0; staffIdx < lastSystem->staves().size(); ++staffIdx) {
        if (lastSystem->staff(staffIdx)->show()) {
            visibleStaves.push_back(staffIdx);
        }
    }
    ASSERT_EQ(visibleStaves.size(), 2);

    for (staff_idx_t staffIdx : visibleStaves) {
        EngravingItem* item = headerClefSeg->element(staff2track(staffIdx));
        ASSERT_TRUE(item && item->isClef());
        EXPECT_EQ(toClef(item)->clefType(), ClefType::F);
    }

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testHeaderAndCourtesyKeySigsOnSharedStaves)
{
    // [GIVEN] A score with 2 staves, the second changing instrument (and key) at the start of the final system
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_12.mscx");
    ASSERT_TRUE(score);

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    ASSERT_EQ(sharedPart->nstaves(), 2);

    const staff_idx_t secondSharedStaff = sharedPart->staff(1)->idx();
    const track_idx_t secondSharedTrack = staff2track(secondSharedStaff);

    Measure* m2 = score->crMeasure(1);
    Measure* m3 = score->crMeasure(2);
    ASSERT_TRUE(m2 && m3);

    // [THEN] The second shared staff should be visible on the final system
    System* lastSystem = score->lastMeasure()->system();
    ASSERT_TRUE(lastSystem);
    ASSERT_EQ(lastSystem->firstMeasure(), m3);
    EXPECT_TRUE(lastSystem->staff(secondSharedStaff)->show());

    // [THEN] It should have a header key signature of A major at the start of bar 3
    Segment* headerKeySigSeg = m3->findSegment(SegmentType::KeySig, m3->tick());
    ASSERT_TRUE(headerKeySigSeg);
    EngravingItem* headerKeySig = headerKeySigSeg->element(secondSharedTrack);
    ASSERT_TRUE(headerKeySig && headerKeySig->isKeySig());
    EXPECT_EQ(toKeySig(headerKeySig)->key(), Key::A);

    // [THEN] There should be a courtesy key signature in the trailer of bar 2
    Segment* courtesyKeySigSeg = m2->findSegment(SegmentType::KeySigAnnounce, m2->endTick());
    ASSERT_TRUE(courtesyKeySigSeg);
    EngravingItem* courtesyKeySig = courtesyKeySigSeg->element(secondSharedTrack);
    ASSERT_TRUE(courtesyKeySig && courtesyKeySig->isKeySig());
    EXPECT_EQ(toKeySig(courtesyKeySig)->key(), Key::A);

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testUserSetFirstClefOnSharedStaves)
{
    // [GIVEN] A score with 2 staves, the second with a user-set header clef at the start of the score
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_13.mscx");
    ASSERT_TRUE(score);

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    ASSERT_EQ(originParts.size(), 2);

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);
    Segment* headerClefSeg = firstMeasure->findSegment(SegmentType::HeaderClef, firstMeasure->tick());
    ASSERT_TRUE(headerClefSeg);

    const Staff* secondOriginStaff = originParts.at(1)->staff(0);
    EngravingItem* originClef = headerClefSeg->element(staff2track(secondOriginStaff->idx()));
    ASSERT_TRUE(originClef && originClef->isClef());
    ASSERT_FALSE(originClef->generated());

    // [THEN] The shared staff showing the second origin staff should have the same header clef
    const Staff* sharedStaff = nullptr;
    for (const Staff* staff : sharedPart->staves()) {
        if (sharedPart->originStaffForStaff(staff->idx(), firstMeasure->tick()) == secondOriginStaff) {
            sharedStaff = staff;
            break;
        }
    }
    ASSERT_TRUE(sharedStaff);

    EngravingItem* sharedClef = headerClefSeg->element(staff2track(sharedStaff->idx()));
    ASSERT_TRUE(sharedClef && sharedClef->isClef());
    EXPECT_EQ(toClef(sharedClef)->clefTypeList(), toClef(originClef)->clefTypeList());
    EXPECT_EQ(toClef(sharedClef)->clefTypeList(), ClefTypeList(ClefType::G, ClefType::F));

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testNoCourtesyKeySigWhenMappingChanges)
{
    // [GIVEN] A score with 3 staves, the third in a different key, where the origin mapped to the
    // second shared staff changes from the third staff to the second staff at the start of the second system
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_14.mscx");
    ASSERT_TRUE(score);

    // [WHEN] Stave sharing is enabled and the score is laid out again
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->setLayoutAll();
    score->doLayout();

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    ASSERT_EQ(sharedPart->nstaves(), 3);
    ASSERT_EQ(originParts.size(), 3);

    Measure* m2 = score->crMeasure(1);
    Measure* m3 = score->crMeasure(2);
    ASSERT_TRUE(m2 && m3);
    ASSERT_TRUE(m2->system() && m3->system());
    ASSERT_NE(m2->system(), m3->system());

    const staff_idx_t secondSharedStaff = sharedPart->staff(1)->idx();
    ASSERT_EQ(sharedPart->originStaffForStaff(secondSharedStaff, m2->tick()), originParts.at(2)->staff(0));
    ASSERT_EQ(sharedPart->originStaffForStaff(secondSharedStaff, m3->tick()), originParts.at(1)->staff(0));

    // [THEN] There should be no courtesy key signature at the end of the first system
    EXPECT_FALSE(m2->hasCourtesyKeySig());
    Segment* courtesyKeySigSeg = m2->findSegment(SegmentType::KeySigAnnounce, m2->endTick());
    if (courtesyKeySigSeg) {
        EXPECT_FALSE(courtesyKeySigSeg->enabled() && courtesyKeySigSeg->element(staff2track(secondSharedStaff)));
    }

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testTimeSigsFollowOriginWhenMappingChanges)
{
    // [GIVEN] A score with 3 staves, the first two in 3/4 and the third in 6/8, where the origins mapped to the
    // second and third shared staves swap at the start of the second system without any time signature being present
    // 1st system   2nd system
    // 1 -> 1       1 -> 1
    // 2 -> 1       2 -> 2
    // 3 -> 2       3 -> 3
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_21.mscx");
    ASSERT_TRUE(score);

    // [WHEN] Stave sharing is enabled and the score is laid out again
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->setLayoutAll();
    score->doLayout();

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    ASSERT_EQ(sharedPart->nstaves(), 3);
    ASSERT_EQ(originParts.size(), 3);

    Measure* m2 = score->crMeasure(1);
    Measure* m3 = score->crMeasure(2);
    ASSERT_TRUE(m2 && m3);
    ASSERT_TRUE(m2->system() && m3->system());
    ASSERT_NE(m2->system(), m3->system());

    const staff_idx_t secondSharedStaff = sharedPart->staff(1)->idx();
    const staff_idx_t thirdSharedStaff = sharedPart->staff(2)->idx();
    ASSERT_EQ(sharedPart->originStaffForStaff(secondSharedStaff, m2->tick()), originParts.at(2)->staff(0));
    ASSERT_EQ(sharedPart->originStaffForStaff(secondSharedStaff, m3->tick()), originParts.at(1)->staff(0));
    ASSERT_EQ(sharedPart->originStaffForStaff(thirdSharedStaff, m3->tick()), originParts.at(2)->staff(0));

    // [THEN] The shared staves should use the time signature of the origin mapped to them in the second system
    for (staff_idx_t sharedStaffIdx : { secondSharedStaff, thirdSharedStaff }) {
        const Staff* originStaff = sharedPart->originStaffForStaff(sharedStaffIdx, m3->tick());
        const TimeSig* sharedTimeSig = score->staff(sharedStaffIdx)->timeSig(m3->tick());
        const TimeSig* originTimeSig = originStaff->timeSig(m3->tick());
        ASSERT_TRUE(sharedTimeSig && originTimeSig);
        // == compares values, which would treat 6/8 and 3/4 as equal
        EXPECT_TRUE(sharedTimeSig->sig().identical(originTimeSig->sig()));
    }

    // [THEN] The 6/8 eighths on the third shared staff should be beamed in groups of 3, like on the origin staff
    const Segment* firstCRSeg = m3->first(SegmentType::ChordRest);
    ASSERT_TRUE(firstCRSeg);
    const ChordRest* sharedCR = toChordRest(firstCRSeg->element(staff2track(thirdSharedStaff)));
    const ChordRest* originCR = toChordRest(firstCRSeg->element(staff2track(originParts.at(2)->staff(0)->idx())));
    ASSERT_TRUE(sharedCR && originCR);
    ASSERT_TRUE(sharedCR->beam() && originCR->beam());
    EXPECT_EQ(originCR->beam()->elements().size(), 3);
    EXPECT_EQ(sharedCR->beam()->elements().size(), originCR->beam()->elements().size());

    delete score;
}

static const Staff* sharedStaffForOrigin(const SharedPart* sharedPart, const Staff* originStaff, const Fraction& tick)
{
    const StaveSharingState& state = sharedPart->staveSharingStateAtTick(tick);
    const track_idx_t sharedTrack = muse::value(state.sharedTrackMap(), staff2track(originStaff->idx()), muse::nidx);
    return sharedTrack != muse::nidx ? originStaff->score()->staff(track2staff(sharedTrack)) : nullptr;
}

TEST_F(Engraving_StaveSharingTests, testSingleClefChangeAtEndOfSystemOnSharedStave)
{
    // [GIVEN] A score with 2 unison bass clef staves and a line break after bar 2
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_22.mscx");
    ASSERT_TRUE(score);

    Measure* m2 = score->crMeasure(1);
    Measure* m3 = score->crMeasure(2);
    ASSERT_TRUE(m2 && m3);

    // [WHEN] Stave sharing is enabled and the first staff changes to treble clef at the start of the second system
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    ASSERT_EQ(originParts.size(), 2);

    Staff* firstOriginStaff = originParts.at(0)->staff(0);
    Staff* secondOriginStaff = originParts.at(1)->staff(0);

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Change clef"), [&](Transaction& tx) {
        EditClef::undoChangeClef(tx, score, firstOriginStaff, m3, ClefType::G);
    });
    score->setLayoutAll();
    score->doLayout();
    ASSERT_NE(m2->system(), m3->system());

    // [THEN] Both origin staves share a stave on the first system, and are split on the second
    const Staff* sharedStaffInFirstSystem = sharedStaffForOrigin(sharedPart, firstOriginStaff, m2->tick());
    ASSERT_TRUE(sharedStaffInFirstSystem);
    EXPECT_EQ(sharedStaffForOrigin(sharedPart, secondOriginStaff, m2->tick()), sharedStaffInFirstSystem);

    const Staff* firstSharedStaff = sharedStaffForOrigin(sharedPart, firstOriginStaff, m3->tick());
    const Staff* secondSharedStaff = sharedStaffForOrigin(sharedPart, secondOriginStaff, m3->tick());
    ASSERT_TRUE(firstSharedStaff && secondSharedStaff);
    ASSERT_NE(firstSharedStaff, secondSharedStaff);

    // [THEN] The shared stave on the first system shows the treble clef change at the end of the system
    Segment* clefSeg = m2->findSegment(SegmentType::Clef, m2->endTick());
    ASSERT_TRUE(clefSeg);
    EngravingItem* courtesyClef = clefSeg->element(staff2track(sharedStaffInFirstSystem->idx()));
    ASSERT_TRUE(courtesyClef && courtesyClef->isClef());
    EXPECT_EQ(toClef(courtesyClef)->clefType(), ClefType::G);

    // [THEN] The split staves on the second system have the header clefs of their origin staves
    Segment* headerClefSeg = m3->findSegment(SegmentType::HeaderClef, m3->tick());
    ASSERT_TRUE(headerClefSeg);
    EngravingItem* firstHeaderClef = headerClefSeg->element(staff2track(firstSharedStaff->idx()));
    EngravingItem* secondHeaderClef = headerClefSeg->element(staff2track(secondSharedStaff->idx()));
    ASSERT_TRUE(firstHeaderClef && firstHeaderClef->isClef());
    ASSERT_TRUE(secondHeaderClef && secondHeaderClef->isClef());
    EXPECT_EQ(toClef(firstHeaderClef)->clefType(), ClefType::G);
    EXPECT_EQ(toClef(secondHeaderClef)->clefType(), ClefType::F);

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testMultipleClefChangesAtEndOfSystemOnSharedStave)
{
    // [GIVEN] A score with 2 unison bass clef staves and a line break after bar 2
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_22.mscx");
    ASSERT_TRUE(score);

    Measure* m2 = score->crMeasure(1);
    Measure* m3 = score->crMeasure(2);
    ASSERT_TRUE(m2 && m3);

    // [WHEN] Stave sharing is enabled and both staves change to different clefs at the start of the second system
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    ASSERT_EQ(originParts.size(), 2);

    Staff* firstOriginStaff = originParts.at(0)->staff(0);
    Staff* secondOriginStaff = originParts.at(1)->staff(0);

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Change clefs"), [&](Transaction& tx) {
        EditClef::undoChangeClef(tx, score, firstOriginStaff, m3, ClefType::G);
        EditClef::undoChangeClef(tx, score, secondOriginStaff, m3, ClefType::C3);
    });
    score->setLayoutAll();
    score->doLayout();
    ASSERT_NE(m2->system(), m3->system());

    // [THEN] Both origin staves share a stave on the first system, and are split on the second
    const Staff* sharedStaffInFirstSystem = sharedStaffForOrigin(sharedPart, firstOriginStaff, m2->tick());
    ASSERT_TRUE(sharedStaffInFirstSystem);
    EXPECT_EQ(sharedStaffForOrigin(sharedPart, secondOriginStaff, m2->tick()), sharedStaffInFirstSystem);

    const Staff* firstSharedStaff = sharedStaffForOrigin(sharedPart, firstOriginStaff, m3->tick());
    const Staff* secondSharedStaff = sharedStaffForOrigin(sharedPart, secondOriginStaff, m3->tick());
    ASSERT_TRUE(firstSharedStaff && secondSharedStaff);
    ASSERT_NE(firstSharedStaff, secondSharedStaff);

    // [THEN] The shared stave on the first system shows no clef change at the end of the system
    Segment* clefSeg = m2->findSegment(SegmentType::Clef, m2->endTick());
    ASSERT_TRUE(clefSeg);
    EXPECT_FALSE(clefSeg->element(staff2track(sharedStaffInFirstSystem->idx())));

    // [THEN] The split staves on the second system have the header clefs of their origin staves
    Segment* headerClefSeg = m3->findSegment(SegmentType::HeaderClef, m3->tick());
    ASSERT_TRUE(headerClefSeg);
    EngravingItem* firstHeaderClef = headerClefSeg->element(staff2track(firstSharedStaff->idx()));
    EngravingItem* secondHeaderClef = headerClefSeg->element(staff2track(secondSharedStaff->idx()));
    ASSERT_TRUE(firstHeaderClef && firstHeaderClef->isClef());
    ASSERT_TRUE(secondHeaderClef && secondHeaderClef->isClef());
    EXPECT_EQ(toClef(firstHeaderClef)->clefType(), ClefType::G);
    EXPECT_EQ(toClef(secondHeaderClef)->clefType(), ClefType::C3);

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testPreferFlatsOnSharedStaves)
{
    // [GIVEN] Flute and two clarinets preferring flats, with a key change to E major (concert) for all staves in m.2
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_09.mscx");
    ASSERT_TRUE(score);

    const Fraction keyChangeTick = score->crMeasure(1)->tick();

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    ASSERT_EQ(sharedPart->nstaves(), 2);

    // [THEN] The shared staves show G♭ major, like the origin staves
    for (const Part* originPart : sharedPart->originParts()) {
        EXPECT_EQ(originPart->staff(0)->key(keyChangeTick), Key::G_B);
    }
    for (const Staff* sharedStaff : sharedPart->staves()) {
        if (sharedPart->originPartForStaff(sharedStaff->idx(), keyChangeTick)) {
            EXPECT_EQ(sharedStaff->key(keyChangeTick), Key::G_B);
        }
    }
    checkSharedKeysMatchOrigins(sharedPart, keyChangeTick);

    // [WHEN] Concert pitch is toggled on and off
    toggleConcertPitch(score);
    score->doLayout();

    // [THEN] The shared staves still match the origin staves
    checkSharedKeysMatchOrigins(sharedPart, keyChangeTick);

    delete score;
}

TEST_F(Engraving_StaveSharingTests, testInstrumentChangeForAllOriginParts)
{
    // [GIVEN] Flute and two clarinets, both changing to alto saxophone in m.2, with a key change to G major (concert) for all staves in m.4
    MasterScore* score = ScoreRW::readScore(STAVE_SHARING_DIR + u"staveSharing_10.mscx");
    ASSERT_TRUE(score);

    const Fraction instrumentChangeTick = score->crMeasure(1)->tick();
    const Fraction keyChangeTick = score->crMeasure(3)->tick();

    // [WHEN] Stave sharing is enabled
    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Enable stave sharing"), [&](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });
    score->doLayout();

    SharedPart* sharedPart = nullptr;
    std::vector<Part*> originParts;
    collectSharedAndOriginParts(score, &sharedPart, originParts);
    ASSERT_TRUE(checkSharedPartExist(sharedPart, originParts));
    ASSERT_EQ(sharedPart->nstaves(), 2);

    // [THEN] The shared staves show E major from m.4, like the origin staves
    for (const Part* originPart : sharedPart->originParts()) {
        EXPECT_EQ(originPart->staff(0)->key(keyChangeTick), Key::E);
    }
    for (const Staff* sharedStaff : sharedPart->staves()) {
        if (sharedPart->originPartForStaff(sharedStaff->idx(), keyChangeTick)) {
            EXPECT_EQ(sharedStaff->key(keyChangeTick), Key::E);
        }
    }
    checkSharedKeysMatchOrigins(sharedPart, instrumentChangeTick);
    checkSharedKeysMatchOrigins(sharedPart, keyChangeTick);

    delete score;
}
