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

#include "dom/masterscore.h"
#include "dom/segment.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String RHYTHMICGRP_DATA_DIR("rhythmicGrouping_data/");

class Engraving_RhythmicGroupingTests : public ::testing::Test
{
public:
    void group(const char* p1, const char* p2, size_t staves = 0);
};

void Engraving_RhythmicGroupingTests::group(const char* p1, const char* p2, size_t staves)
{
    MasterScore* score = ScoreRW::readScore(RHYTHMICGRP_DATA_DIR + String::fromUtf8(p1));
    EXPECT_TRUE(score);

    if (!staves) {
        score->cmdSelectAll();
        score->cmdResetNoteAndRestGroupings();
    } else {
        assert(staves < score->nstaves());
        score->startCmd();
        for (size_t track = 0; track < staves * VOICES; track++) {
            score->regroupNotesAndRests(score->firstSegment(SegmentType::All)->tick(),
                                        score->lastSegment()->tick(), static_cast<int>(track));
        }
        score->endCmd();
    }

    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String::fromUtf8(p1), RHYTHMICGRP_DATA_DIR + String::fromUtf8(p2)));
    delete score;
}

TEST_F(Engraving_RhythmicGroupingTests, group8ths44)
{
    group("group8ths4-4.mscx", "group8ths4-4-ref.mscx");
}

TEST_F(Engraving_RhythmicGroupingTests, group8thsSimple)
{
    group("group8thsSimple.mscx", "group8thsSimple-ref.mscx");
}

TEST_F(Engraving_RhythmicGroupingTests, group8thsCompound)
{
    group("group8thsCompound.mscx", "group8thsCompound-ref.mscx");
}

TEST_F(Engraving_RhythmicGroupingTests, groupSubbeats)
{
    group("groupSubbeats.mscx", "groupSubbeats-ref.mscx");
}

TEST_F(Engraving_RhythmicGroupingTests, groupVoices)
{
    group("groupVoices.mscx", "groupVoices-ref.mscx");
}

TEST_F(Engraving_RhythmicGroupingTests, groupConflicts)
{
    group("groupConflicts.mscx", "groupConflicts-ref.mscx", 1);  // only group 1st staff
}

TEST_F(Engraving_RhythmicGroupingTests, groupArticulationsTies)
{
    group("groupArticulationsTies.mscx", "groupArticulationsTies-ref.mscx"); // test for articulations and forward/backward ties
}

TEST_F(Engraving_RhythmicGroupingTests, groupShortenNotes)
{
    group("groupShortenNotes.mscx", "groupShortenNotes-ref.mscx");  // test for regrouping rhythms when notes should be shortened
}
