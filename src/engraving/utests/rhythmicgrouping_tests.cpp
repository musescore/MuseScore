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
#include "libmscore/segment.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

static const QString RHYTHMICGRP_DATA_DIR("rhythmicGrouping_data/");

using namespace mu::engraving;
using namespace Ms;

class RhythmicGroupingTests : public ::testing::Test
{
public:
    void group(const char* p1, const char* p2, int staves = 0);
};

void RhythmicGroupingTests::group(const char* p1, const char* p2, int staves)
{
    MasterScore* score = ScoreRW::readScore(RHYTHMICGRP_DATA_DIR + p1);
    EXPECT_TRUE(score);

    if (!staves) {
        score->cmdSelectAll();
        score->cmdResetNoteAndRestGroupings();
    } else {
        Q_ASSERT(staves < score->nstaves());
        score->startCmd();
        for (int track = 0; track < staves * VOICES; track++) {
            score->regroupNotesAndRests(score->firstSegment(SegmentType::All)->tick(),
                                        score->lastSegment()->tick(), track);
        }
        score->endCmd();
    }

    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, p1, RHYTHMICGRP_DATA_DIR + p2));
    delete score;
}

TEST_F(RhythmicGroupingTests, group8ths44)
{
    group("group8ths4-4.mscx", "group8ths4-4-ref.mscx");
}

TEST_F(RhythmicGroupingTests, group8thsSimple)
{
    group("group8thsSimple.mscx", "group8thsSimple-ref.mscx");
}

TEST_F(RhythmicGroupingTests, group8thsCompound)
{
    group("group8thsCompound.mscx", "group8thsCompound-ref.mscx");
}

TEST_F(RhythmicGroupingTests, groupSubbeats)
{
    group("groupSubbeats.mscx", "groupSubbeats-ref.mscx");
}

TEST_F(RhythmicGroupingTests, groupVoices)
{
    group("groupVoices.mscx", "groupVoices-ref.mscx");
}

TEST_F(RhythmicGroupingTests, groupConflicts)
{
    group("groupConflicts.mscx", "groupConflicts-ref.mscx", 1);  // only group 1st staff
}

TEST_F(RhythmicGroupingTests, groupArticulationsTies)
{
    group("groupArticulationsTies.mscx", "groupArticulationsTies-ref.mscx"); // test for articulations and forward/backward ties
}

TEST_F(RhythmicGroupingTests, groupShortenNotes)
{
    group("groupShortenNotes.mscx", "groupShortenNotes-ref.mscx");  // test for regrouping rhythms when notes should be shortened
}
