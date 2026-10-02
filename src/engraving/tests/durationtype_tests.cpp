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

#include "engraving/dom/chord.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"

#include "engraving/editing/editduration.h"
#include "engraving/editing/noteinput.h"
#include "engraving/editing/transaction/transaction.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

static const String DURATIONTYPE_DATA_DIR("durationtype_data/");

class Engraving_DurationTypeTests : public ::testing::Test
{
private slots:

    void halfDuration();
    void doubleDuration();
    void decDurationDotted();
    void incDurationDotted();
    void halfDoubleDurationLinkedStaves();
};

// Simple tests for command "half-duration" (default shortcut "Q").
// starts with Whole note and repeatedly applies cmdHalfDuration()
TEST_F(Engraving_DurationTypeTests, halfDuration)
{
    MasterScore* score = ScoreRW::readScore(DURATIONTYPE_DATA_DIR + u"empty.mscx");
    EXPECT_TRUE(score);

    score->inputState().setTrack(0);
    score->inputState().setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->inputState().setDuration(DurationType::V_WHOLE);
    score->inputState().setNoteEntryMode(true);

    score->transactionManager()->transaction(TranslatableString::untranslatable("Half duration tests"), [&](Transaction& tx) {
        NoteInput::addPitch(score->transactionManager()->currentOrDummyTransaction(), score, 42, false, false);

        Chord* c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
        EXPECT_EQ(c->ticks(), Fraction(1, 1));

        // repeatedly half-duration from V_WHOLE to V_128
        for (int i = 128; i > 1; i /= 2) {
            EditDuration::halfDuration(tx, score);
            c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
            EXPECT_EQ(c->ticks(), Fraction(i / 2, 128));
        }
    });

    delete score;
}

// Simple tests for command "double-duration" (default shortcut "W").
// Starts with 128th note and repeatedly applies cmdDoubleDuration() up to Whole note.
TEST_F(Engraving_DurationTypeTests, doubleDuration)
{
    MasterScore* score = ScoreRW::readScore(DURATIONTYPE_DATA_DIR + u"empty.mscx");
    EXPECT_TRUE(score);

    score->inputState().setTrack(0);
    score->inputState().setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->inputState().setDuration(DurationType::V_128TH);
    score->inputState().setNoteEntryMode(true);

    score->transactionManager()->transaction(TranslatableString::untranslatable("Double duration tests"), [&](Transaction& tx) {
        NoteInput::addPitch(score->transactionManager()->currentOrDummyTransaction(), score, 42, false, false);

        Chord* c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
        EXPECT_EQ(c->ticks(), Fraction(1, 128));

        // repeatedly double-duration from V_128 to V_WHOLE
        for (int i = 1; i < 128; i *= 2) {
            EditDuration::doubleDuration(tx, score);
            c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
            EXPECT_EQ(c->ticks(), Fraction(2 * i, 128));
        }
    });

    delete score;
}

// Simple tests for command "dec-duration-dotted" (default shortcut "Shift+Q").
// Starts with Whole note and repeatedly applies cmdDecDurationDotted() down to 128th note.
TEST_F(Engraving_DurationTypeTests, decDurationDotted)
{
    MasterScore* score = ScoreRW::readScore(DURATIONTYPE_DATA_DIR + u"empty.mscx");
    EXPECT_TRUE(score);

    score->inputState().setTrack(0);
    score->inputState().setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->inputState().setDuration(DurationType::V_WHOLE);
    score->inputState().setNoteEntryMode(true);

    score->transactionManager()->transaction(TranslatableString::untranslatable("Decrease duration dotted tests"), [&](Transaction& tx) {
        NoteInput::addPitch(score->transactionManager()->currentOrDummyTransaction(), score, 42, false, false);

        Chord* c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
        EXPECT_EQ(c->ticks(), Fraction(1, 1));

        // repeatedly dec-duration-dotted from V_WHOLE to V_128
        for (int i = 128; i > 1; i /= 2) {
            EditDuration::decDurationDotted(tx, score);
            c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
            EXPECT_EQ(c->ticks(), Fraction(i + i / 2, 256));

            EditDuration::decDurationDotted(tx, score);
            c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
            EXPECT_EQ(c->ticks(), Fraction(i / 2, 128));
        }
    });

    delete score;
}

// Simple tests for command "inc-duration-dotted" (default shortcut "Shift+W").
// Starts with 128th note and repeatedly applies cmdIncDurationDotted() up to Whole note.
TEST_F(Engraving_DurationTypeTests, incDurationDotted)
{
    MasterScore* score = ScoreRW::readScore(DURATIONTYPE_DATA_DIR + u"empty.mscx");
    EXPECT_TRUE(score);

    score->inputState().setTrack(0);
    score->inputState().setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->inputState().setDuration(DurationType::V_128TH);
    score->inputState().setNoteEntryMode(true);

    score->transactionManager()->transaction(TranslatableString::untranslatable("Increase duration dotted tests"), [&](Transaction& tx) {
        NoteInput::addPitch(score->transactionManager()->currentOrDummyTransaction(), score, 42, false, false);

        Chord* c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
        EXPECT_EQ(c->ticks(), Fraction(1, 128));

        // repeatedly inc-duration-dotted from V_128 to V_WHOLE
        for (int i = 1; i < 128; i *= 2) {
            EditDuration::incDurationDotted(tx, score);
            c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
            EXPECT_EQ(c->ticks(), Fraction(3 * i, 256));

            EditDuration::incDurationDotted(tx, score);
            c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
            EXPECT_EQ(c->ticks(), Fraction(i, 64));
        }
    });

    delete score;
}

// "half-duration" and "double-duration" on a note selected together with its linked copy
// should change the duration only once.
TEST_F(Engraving_DurationTypeTests, halfDoubleDurationLinkedStaves)
{
    MasterScore* score = ScoreRW::readScore(DURATIONTYPE_DATA_DIR + u"empty.mscx");
    EXPECT_TRUE(score);

    score->inputState().setTrack(0);
    score->inputState().setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->inputState().setDuration(DurationType::V_QUARTER);
    score->inputState().setNoteEntryMode(true);

    score->transactionManager()->transaction(TranslatableString::untranslatable("Linked staves duration tests"), [&](Transaction&) {
        NoteInput::addPitch(score->transactionManager()->currentOrDummyTransaction(), score, 42, false, false);

        Staff* staff = score->staff(0);
        Staff* linkedStaff = Factory::createStaff(staff->part());
        linkedStaff->setPart(staff->part());
        score->undoInsertStaff(linkedStaff, 1, false);
        Excerpt::cloneStaff(staff, linkedStaff);
    });

    score->inputState().setNoteEntryMode(false);

    auto applyToBothCopies = [score](void (*func)(Transaction&, Score*)) {
        Chord* c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
        Chord* linkedChord = score->firstMeasure()->findChord(Fraction(0, 1), VOICES);
        ASSERT_TRUE(c && linkedChord);
        EXPECT_TRUE(c->isLinked(linkedChord));

        score->select(c->upNote(), SelectType::SINGLE);
        score->select(linkedChord->upNote(), SelectType::ADD);
        score->transactionManager()->transaction(TranslatableString::untranslatable("Linked staves duration tests"), [&](Transaction& tx) {
            func(tx, score);
        });
    };

    auto expectTicks = [score](const Fraction& ticks) {
        EXPECT_EQ(score->firstMeasure()->findChord(Fraction(0, 1), 0)->ticks(), ticks);
        EXPECT_EQ(score->firstMeasure()->findChord(Fraction(0, 1), VOICES)->ticks(), ticks);
    };

    applyToBothCopies(EditDuration::doubleDuration);
    expectTicks(Fraction(1, 2));

    score->undoRedo(true, nullptr);
    expectTicks(Fraction(1, 4));

    applyToBothCopies(EditDuration::halfDuration);
    expectTicks(Fraction(1, 8));

    delete score;
}
