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

#include "engraving/style/style.h"

#include "dom/chord.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/undo.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String EARLYMUSIC_DATA_DIR("earlymusic_data/");

class Engraving_EarlymusicTests : public ::testing::Test
{
};

//---------------------------------------------------------
//   Setting cross-measure value flag and undoing.
//---------------------------------------------------------
TEST_F(Engraving_EarlymusicTests, earlymusic01)
{
    MasterScore* score = ScoreRW::readScore(EARLYMUSIC_DATA_DIR + u"mensurstrich01.mscx");
    EXPECT_TRUE(score);
    score->doLayout();

    // go to first chord and verify crossMeasure values
    Measure* msr   = score->firstMeasure();
    EXPECT_TRUE(msr);
    Segment* seg   = msr->findSegment(SegmentType::ChordRest, Fraction(0, 1));
    EXPECT_TRUE(seg);
    Chord* chord = toChord(seg->element(0));
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
    score->startCmd(TranslatableString::untranslatable("Early music tests"));
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
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"mensurstrich01.mscx", EARLYMUSIC_DATA_DIR + u"mensurstrich01-ref.mscx"));

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
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"mensurstrich01.mscx", EARLYMUSIC_DATA_DIR + u"mensurstrich01.mscx"));
    delete score;
}
