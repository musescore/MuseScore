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

#include "dom/chord.h"
#include "dom/chordrest.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/segment.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String JOIN_DATA_DIR("join_data/");

class Engraving_JoinTests : public ::testing::Test
{
public:
    void join(const char* p1, const char* p2, int index = 0);
    void join1(const char* p1);
};

void Engraving_JoinTests::join(const char* p1, const char* p2, int index)
{
    MasterScore* score = ScoreRW::readScore(JOIN_DATA_DIR + String::fromUtf8(p1));
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    EXPECT_TRUE(m1);

    for (int i = 0; i < index; ++i) {
        m1 = m1->nextMeasure();
    }

    Measure* m2 = m1->nextMeasure();
    EXPECT_TRUE(m2);

    EXPECT_NE(m1, m2);

    score->startCmd(TranslatableString::untranslatable("Engraving join tests"));
    score->cmdJoinMeasure(m1, m2);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String::fromUtf8(p1), JOIN_DATA_DIR + String::fromUtf8(p2)));
    delete score;
}

void Engraving_JoinTests::join1(const char* p1)
{
    MasterScore* score = ScoreRW::readScore(JOIN_DATA_DIR + String::fromUtf8(p1));
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    EXPECT_TRUE(m1);

    Measure* m2 = m1->nextMeasure();
    EXPECT_TRUE(m2);

    EXPECT_NE(m1, m2);

    score->cmdJoinMeasure(m1, m2);

    // check if notes are still on line 6
    Segment* s = score->firstSegment(SegmentType::ChordRest);

    for (int i = 0; i < 8; ++i) {
        Note* note = toChord(s->element(0))->upNote();
        EXPECT_EQ(note->line(), 6);
        s = s->next1(SegmentType::ChordRest);
    }

    delete score;
}

TEST_F(Engraving_JoinTests, join01)
{
    join("join01.mscx", "join01-ref.mscx");
}

TEST_F(Engraving_JoinTests, join02)
{
    join("join02.mscx", "join02-ref.mscx");
}

TEST_F(Engraving_JoinTests, join03)
{
    join("join03.mscx", "join03-ref.mscx");
}

TEST_F(Engraving_JoinTests, join04)
{
    join("join04.mscx", "join04-ref.mscx");
}

TEST_F(Engraving_JoinTests, join05)
{
    join("join05.mscx", "join05-ref.mscx");
}

TEST_F(Engraving_JoinTests, join06)
{
    join("join06.mscx", "join06-ref.mscx", 1);
}

TEST_F(Engraving_JoinTests, join07)
{
    join("join07.mscx", "join07-ref.mscx");
}

TEST_F(Engraving_JoinTests, join08)
{
    join1("join08.mscx");
}

TEST_F(Engraving_JoinTests, join09)
{
    join("join09.mscx", "join09-ref.mscx");
}

TEST_F(Engraving_JoinTests, join10)
{
    join("join10.mscx", "join10-ref.mscx", 1);
}

TEST_F(Engraving_JoinTests, joinTieAtStart) {
    // Test splitting a measure when there is a tie ending on the first chord on the split range
    bool use302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    MasterScore* score = ScoreRW::readScore(JOIN_DATA_DIR + u"joinTieAtStart.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    EXPECT_TRUE(m1);

    Segment* s1 = m1->last(SegmentType::ChordRest);
    ChordRest* cr1 = toChordRest(s1->element(0));
    EXPECT_TRUE(cr1 && cr1->isChord());
    Chord* c1 = toChord(cr1);
    Note* n1 = c1->upNote();
    EXPECT_TRUE(n1);

    auto checkTie = [&]() -> Tie* {
        Tie* t = n1->tieFor();
        EXPECT_TRUE(t);

        Note* n2 = t->endNote();
        EXPECT_TRUE(n2);
        EXPECT_EQ(n2->tick(), Fraction(1, 1));
        EXPECT_EQ(n2->chord()->measure(), m1->nextMeasure());

        return t;
    };

    Tie* tie1 = checkTie();

    score->startCmd(TranslatableString::untranslatable("Engraving join tests"));
    Measure* m2 = m1->nextMeasure();
    Measure* m3 = m2->nextMeasure();
    score->cmdJoinMeasure(m2, m3);
    score->endCmd();

    Tie* tie2 = checkTie();
    EXPECT_NE(tie2, tie1);

    score->undoRedo(true, nullptr);

    Tie* tie3 = checkTie();
    EXPECT_EQ(tie3, tie1);

    delete score;
    MScore::useRead302InTestMode = use302;
}
