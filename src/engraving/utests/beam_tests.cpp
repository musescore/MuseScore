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
#include "libmscore/chordrest.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

static const QString BEAM_DATA_DIR("beam_data/");

using namespace mu::engraving;
using namespace Ms;

//---------------------------------------------------------
//   TestBeam
//---------------------------------------------------------

class BeamTests : public ::testing::Test
{
public:
    void beam(const char* path);
};

//---------------------------------------------------------
//   beam
//---------------------------------------------------------
void BeamTests::beam(const char* path)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + path);
    EXPECT_TRUE(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, path, BEAM_DATA_DIR + path));
    delete score;
}

TEST_F(BeamTests, beamA)
{
    beam("Beam-A.mscx");
}

TEST_F(BeamTests, beamB)
{
    beam("Beam-B.mscx");
}

TEST_F(BeamTests, beamC)
{
    beam("Beam-C.mscx");
}

TEST_F(BeamTests, beamD)
{
    beam("Beam-D.mscx");
}

TEST_F(BeamTests, beamE)
{
    beam("Beam-E.mscx");
}

TEST_F(BeamTests, beamF)
{
    beam("Beam-F.mscx");
}

TEST_F(BeamTests, beamG)
{
    beam("Beam-G.mscx");
}

TEST_F(BeamTests, beam2)
{
    beam("Beam-2.mscx");
}

TEST_F(BeamTests, beam23)
{
    beam("Beam-23.mscx");
}

TEST_F(BeamTests, beamS0)
{
    beam("Beam-S0.mscx");
}

TEST_F(BeamTests, beamDir)
{
    beam("Beam-dir.mscx");
}

TEST_F(BeamTests, beamCrossMeasure2)
{
    beam("Beam-CrossM2.mscx");
}

TEST_F(BeamTests, beamCrossMeasure3)
{
    beam("Beam-CrossM3.mscx");
}

TEST_F(BeamTests, beamCrossMeasure4)
{
    beam("Beam-CrossM4.mscx");
}

//---------------------------------------------------------
//   beamCrossMeasure1
//   This method simulates following operations:
//   - Update the score
//   - Check if the beam has been recreated. If yes, this is wrong behaviour
//---------------------------------------------------------
TEST_F(BeamTests, beamCrossMeasure1)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + "Beam-CrossM1.mscx");
    EXPECT_TRUE(score);

    Measure* first_measure = score->firstMeasure();
    EXPECT_TRUE(first_measure);

    // find the first segment that has a chord
    Segment* s = first_measure->first(SegmentType::ChordRest);
    while (s && !s->element(0)->isChord()) {
        s = s->next(SegmentType::ChordRest);
    }
    EXPECT_TRUE(s);

    // locate the first beam
    ChordRest* first_note = toChordRest(s->element(0));
    EXPECT_TRUE(first_note);

    Beam* b = first_note->beam();
    score->update();
    // locate the beam again, and check if it is still b
    Beam* new_b = first_note->beam();

    EXPECT_EQ(new_b, b);

    delete score;
}
