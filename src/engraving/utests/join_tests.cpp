/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/chordrest.h"
#include "libmscore/chord.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

static const QString JOIN_DATA_DIR("join_data/");

using namespace mu::engraving;
using namespace Ms;

class JoinTests : public ::testing::Test
{
public:
    void join(const char* p1, const char* p2, int index = 0);
    void join1(const char* p1);
};

void JoinTests::join(const char* p1, const char* p2, int index)
{
    MasterScore* score = ScoreRW::readScore(JOIN_DATA_DIR + p1);
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    EXPECT_TRUE(m1);

    for (int i = 0; i < index; ++i) {
        m1 = m1->nextMeasure();
    }

    Measure* m2 = m1->nextMeasure();
    EXPECT_TRUE(m2);

    EXPECT_NE(m1, m2);

    score->cmdJoinMeasure(m1, m2);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, p1, JOIN_DATA_DIR + p2));
    delete score;
}

void JoinTests::join1(const char* p1)
{
    MasterScore* score = ScoreRW::readScore(JOIN_DATA_DIR + p1);
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
        Note* note = static_cast<Ms::Chord*>(s->element(0))->upNote();
        EXPECT_EQ(note->line(), 6);
        s = s->next1(SegmentType::ChordRest);
    }

    delete score;
}

TEST_F(JoinTests, join01)
{
    join("join01.mscx", "join01-ref.mscx");
}

TEST_F(JoinTests, join02)
{
    join("join02.mscx", "join02-ref.mscx");
}

TEST_F(JoinTests, join03)
{
    join("join03.mscx", "join03-ref.mscx");
}

TEST_F(JoinTests, join04)
{
    join("join04.mscx", "join04-ref.mscx");
}

TEST_F(JoinTests, join05)
{
    join("join05.mscx", "join05-ref.mscx");
}

TEST_F(JoinTests, join06)
{
    join("join06.mscx", "join06-ref.mscx", 1);
}

TEST_F(JoinTests, join07)
{
    join("join07.mscx", "join07-ref.mscx");
}

TEST_F(JoinTests, join08)
{
    join1("join08.mscx");
}
