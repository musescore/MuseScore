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

#include "dom/beam.h"
#include "dom/chord.h"
#include "dom/chordrest.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/tremolotwochord.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String BEAM_DATA_DIR("beam_data/");

//---------------------------------------------------------
//   TestBeam
//---------------------------------------------------------

class Engraving_BeamTests : public ::testing::Test
{
public:
    void beam(const char* path);
};

//---------------------------------------------------------
//   beam
//---------------------------------------------------------
void Engraving_BeamTests::beam(const char* path)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + String::fromUtf8(path));
    EXPECT_TRUE(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String::fromUtf8(path), BEAM_DATA_DIR + String::fromUtf8(path)));
    delete score;
}

TEST_F(Engraving_BeamTests, beamA)
{
    beam("Beam-A.mscx");
}

TEST_F(Engraving_BeamTests, beamB)
{
    beam("Beam-B.mscx");
}

TEST_F(Engraving_BeamTests, beamC)
{
    beam("Beam-C.mscx");
}

TEST_F(Engraving_BeamTests, beamD)
{
    beam("Beam-D.mscx");
}

TEST_F(Engraving_BeamTests, beamE)
{
    beam("Beam-E.mscx");
}

TEST_F(Engraving_BeamTests, beamF)
{
    beam("Beam-F.mscx");
}

TEST_F(Engraving_BeamTests, beamG)
{
    beam("Beam-G.mscx");
}

// make sure the beam end positions are correct for 2+ beams
TEST_F(Engraving_BeamTests, beamPositions)
{
    beam("beamPositions.mscx");
}

// if the beamNoSlope style parameter is true, all beams are flat
TEST_F(Engraving_BeamTests, beamNoSlope)
{
    beam("beamNoSlope.mscx");
}

TEST_F(Engraving_BeamTests, flippedDirection)
{
    beam("flippedDirection.mscx");
}

TEST_F(Engraving_BeamTests, wideBeams)
{
    beam("wideBeams.mscx");
}

TEST_F(Engraving_BeamTests, flatBeams)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + u"flatBeams.mscx");
    EXPECT_TRUE(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"flatBeams.mscx", BEAM_DATA_DIR + u"flatBeams-ref.mscx"));
    delete score;
}

// cross staff beaming is not yet supported
// in the new beams code
TEST_F(Engraving_BeamTests, DISABLED_beamCrossMeasure2)
{
    beam("Beam-CrossM2.mscx");
}

TEST_F(Engraving_BeamTests, DISABLED_beamCrossMeasure3)
{
    beam("Beam-CrossM3.mscx");
}

TEST_F(Engraving_BeamTests, DISABLED_beamCrossMeasure4)
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
TEST_F(Engraving_BeamTests, DISABLED_beamCrossMeasure1)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + u"Beam-CrossM1.mscx");
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

TEST_F(Engraving_BeamTests, beamStemDir)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + u"beamStemDir.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ChordRest* cr = toChordRest(m1->findSegment(SegmentType::ChordRest, m1->tick())->element(0));

    cr->beam()->setDirection(DirectionV::UP);

    score->update();
    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"beamStemDir-01.mscx", BEAM_DATA_DIR + u"beamStemDir-01-ref.mscx"));

    delete score;
}

//---------------------------------------------------------
//   flipBeamStemDir
//   This method tests if a beam's stem direction will be set to
//   all its chords and will not affect other beams in score
//   after using the flip command
//---------------------------------------------------------

TEST_F(Engraving_BeamTests, flipBeamStemDir)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + u"flipBeamStemDir.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ChordRest* cr = toChordRest(m1->findSegment(SegmentType::ChordRest, m1->tick())->element(0));
    Chord* c2 = toChord(cr->beam()->elements()[1]);

    score->select(c2);
    score->startCmd(TranslatableString::untranslatable("Engraving beam tests"));
    score->cmdFlip();
    score->endCmd();
    cr->beam()->setDirection(DirectionV::DOWN);

    score->update();
    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"flipBeamStemDir-01.mscx", BEAM_DATA_DIR + u"flipBeamStemDir-01-ref.mscx"));

    delete score;
}

//---------------------------------------------------------
//   flipTremoloStemDir
//   This method tests if a tremolo's stem direction will be set to
//   all its chords and will not affect other tremolos in score
//   after using the flip command
//---------------------------------------------------------

TEST_F(Engraving_BeamTests, flipTremoloStemDir)
{
    MasterScore* score = ScoreRW::readScore(BEAM_DATA_DIR + "flipTremoloStemDir.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ChordRest* cr = toChordRest(m1->findSegment(SegmentType::ChordRest, m1->tick())->element(0));
    TremoloTwoChord* t = toChord(cr)->tremoloTwoChord();
    Chord* c1 = t->chord1();
    Chord* c2 = t->chord2();
    EXPECT_TRUE(t->up() && c1->up() && c2->up());

    score->select(c1->upNote());
    score->startCmd(TranslatableString::untranslatable("Engraving beam tests"));
    score->cmdFlip();
    score->endCmd();

    score->update();
    score->doLayout();
    EXPECT_FALSE(t->up() || c1->up() || c2->up());

    delete score;
}
