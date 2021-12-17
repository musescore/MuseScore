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

#include "engraving/style/style.h"

#include "libmscore/chord.h"
#include "libmscore/measure.h"
#include "libmscore/masterscore.h"
#include "libmscore/undo.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

static const QString EARLYMUSIC_DATA_DIR("earlymusic_data/");

using namespace mu::engraving;
using namespace Ms;

class EarlymusicTests : public ::testing::Test
{
};

//---------------------------------------------------------
//   Setting cross-measure value flag and undoing.
//---------------------------------------------------------
TEST_F(EarlymusicTests, earlymusic01)
{
    MasterScore* score = ScoreRW::readScore(EARLYMUSIC_DATA_DIR + "mensurstrich01.mscx");
    EXPECT_TRUE(score);
    score->doLayout();

    // go to first chord and verify crossMeasure values
    Measure* msr   = score->firstMeasure();
    EXPECT_TRUE(msr);
    Segment* seg   = msr->findSegment(SegmentType::ChordRest, Fraction(0, 1));
    EXPECT_TRUE(seg);
    Ms::Chord* chord = static_cast<Ms::Chord*>(seg->element(0));
    EXPECT_TRUE(chord);
    EXPECT_EQ(chord->type(), ElementType::CHORD);
    EXPECT_EQ(chord->crossMeasure(), CrossMeasure::UNKNOWN);
    TDuration cmDur   = chord->crossMeasureDurationType();
//    EXPECT_EQ(cmDur.type(), DurationType::V_INVALID);    // irrelevant if crossMeasure() == UNKNOWN
    TDuration acDur   = chord->actualDurationType();
    EXPECT_EQ(acDur.type(), DurationType::V_BREVE);
    TDuration dur     = chord->durationType();
    EXPECT_EQ(dur.type(), DurationType::V_BREVE);

    // set crossMeasureValue flag ON: score should not change
    MStyle newStyle = score->style();
    newStyle.set(Sid::crossMeasureValues, true);
    score->startCmd();
    score->deselectAll();
    score->undo(new ChangeStyle(score, newStyle));
    score->update();
    score->endCmd();
    // verify crossMeasureDurationType did change
    EXPECT_EQ(chord->crossMeasure(), CrossMeasure::FIRST);
    cmDur = chord->crossMeasureDurationType();
    EXPECT_EQ(cmDur.type(), DurationType::V_LONG);
    acDur = chord->actualDurationType();
    EXPECT_EQ(acDur.type(), DurationType::V_BREVE);
    dur   = chord->durationType();
    EXPECT_EQ(dur.type(), DurationType::V_LONG);
    // verify score file did not change
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, "mensurstrich01.mscx", EARLYMUSIC_DATA_DIR + "mensurstrich01-ref.mscx"));

    // UNDO AND VERIFY
    EditData ed;
    score->undoStack()->undo(&ed);
    score->doLayout();
    EXPECT_EQ(chord->crossMeasure(), CrossMeasure::UNKNOWN);
    cmDur = chord->crossMeasureDurationType();
//      QVERIFY(cmDur.type() == DurationType::V_LONG);    // irrelevant if crossMeasure() == UNKNOWN
    acDur = chord->actualDurationType();
    EXPECT_EQ(acDur.type(), DurationType::V_BREVE);
    dur   = chord->durationType();
    EXPECT_EQ(dur.type(), DurationType::V_BREVE);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, "mensurstrich01.mscx", EARLYMUSIC_DATA_DIR + "mensurstrich01.mscx"));
    delete score;
}
