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
#include "libmscore/chord.h"

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

// make sure the beam end positions are correct for 2+ beams
TEST_F(BeamTests, beamPositions)
{
    beam("beamPositions.mscx");
}

// if the beamNoSlope style parameter is true, all beams are flat
TEST_F(BeamTests, beamNoSlope)
{
    beam("beamNoSlope.mscx");
}

TEST_F(BeamTests, flippedDirection)
{
    beam("flippedDirection.mscx");
}

TEST_F(BeamTests, wideBeams)
{
    beam("wideBeams.mscx");
}

TEST_F(BeamTests, flatBeams)
{
    beam("flatBeams.mscx");
}

// cross staff beaming is not yet supported
// in the new beams code
TEST_F(BeamTests, DISABLED_beamCrossMeasure2)
{
    beam("Beam-CrossM2.mscx");
}

TEST_F(BeamTests, DISABLED_beamCrossMeasure3)
{
    beam("Beam-CrossM3.mscx");
}

TEST_F(BeamTests, DISABLED_beamCrossMeasure4)
{
    beam("Beam-CrossM4.mscx");
}

//---------------------------------------------------------
//   beamCrossMeasure1
//   This method simulates following operations:
//   - Update the score
//   - Check if the beam has been recreated. If yes, this is wrong behaviour
//---------------------------------------------------------

// cross measure beams are not yet supported
// in the refactored beams code
TEST_F(BeamTests, DISABLED_beamCrossMeasure1)
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

//---------------------------------------------------------
//   beamStemDir
//   This method tests if a beam's stem direction will be set to
//   all its chords and will not affect other beams in score
//---------------------------------------------------------

TEST_F(BeamTests, beamStemDir)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + "beamStemDir.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ChordRest* cr = toChordRest(m1->findSegment(SegmentType::ChordRest, m1->tick())->element(0));

    cr->beam()->setBeamDirection(DirectionV::UP);

    score->update();
    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, "beamStemDir-01.mscx", BEAM_DATA_DIR + "beamStemDir-01-ref.mscx"));

    delete score;
}

//---------------------------------------------------------
//   flipBeamStemDir
//   This method tests if a beam's stem direction will be set to
//   all its chords and will not affect other beams in score
//   after using the flip command
//---------------------------------------------------------

TEST_F(BeamTests, flipBeamStemDir)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + "flipBeamStemDir.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ChordRest* cr = toChordRest(m1->findSegment(SegmentType::ChordRest, m1->tick())->element(0));
    Chord* c2 = toChord(cr->beam()->elements()[1]);

    score->select(c2);
    score->startCmd();
    score->cmdFlip();
    score->endCmd();
    cr->beam()->setBeamDirection(DirectionV::DOWN);

    score->update();
    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, "flipBeamStemDir-01.mscx", BEAM_DATA_DIR + "flipBeamStemDir-01-ref.mscx"));

    delete score;
}
