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

#include "dom/clef.h"
#include "dom/factory.h"
#include "dom/masterscore.h"
#include "dom/measure.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String CLEFCOURTESY_DATA_DIR("clef_courtesy_data/");

class Engraving_ClefCourtesyTests : public ::testing::Test
{
};

static Measure* getMeasure(Score* score, int idx)
{
    Measure* m = score->firstMeasure();
    for (int i = 0; i < idx - 1; i++) {
        m = m->nextMeasure();
    }
    return m;
}

static void dropClef(EngravingItem* m, ClefType t)
{
    Clef* clef = Factory::createClef(m->score()->dummy()->segment());   // create a new element, as Measure::drop() will eventually delete it
    clef->setClefType(t);
    EditData dropData(0);
    dropData.pos = m->pagePos();
    dropData.dropElement = clef;
    m->score()->startCmd(TranslatableString::untranslatable("Courtesy clef tests"));
    if (m->isMeasure()) {
        toMeasure(m)->drop(dropData);
    } else {
        m->findMeasure()->drop(dropData);
    }
    m->score()->endCmd();
}

//---------------------------------------------------------
//    add two clefs mid-score at the beginning of systems and look for courtesy clefs
//    the first should be there, the second should not, as it is after a section break
//---------------------------------------------------------
TEST_F(Engraving_ClefCourtesyTests, clef_courtesy01)
{
    MasterScore* score = ScoreRW::readScore(CLEFCOURTESY_DATA_DIR + "clef_courtesy01.mscx");
    EXPECT_TRUE(score);

    // drop G1 clef to 4th measure
    Measure* m1 = getMeasure(score, 4);
    dropClef(m1, ClefType::G8_VA);

    // drop G clef to 7th measure
    Measure* m2 = getMeasure(score, 7);
    dropClef(m2, ClefType::G);

    // check the required courtesy clef is there and it is shown
    Clef* clefCourt = 0;
    Measure* m = m1->prevMeasure();
    Segment* seg = m->findSegment(SegmentType::Clef, m1->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 3.";

    clefCourt = static_cast<Clef*>(seg->element(0));
    EXPECT_TRUE(clefCourt) << "No courtesy clef element in measure 3.";
    EXPECT_GT(clefCourt->ldata()->bbox().width(), 0) << "Courtesy clef in measure 3 is hidden.";

    // check the not required courtesy clef element is there but it is not shown
    clefCourt = nullptr;
    m   = m2->prevMeasure();
    seg = m->findSegment(SegmentType::Clef, m2->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 6.";

    clefCourt = toClef(seg->element(0));
    EXPECT_TRUE(clefCourt) << "No courtesy clef element in measure 6.";
    EXPECT_DOUBLE_EQ(clefCourt->ldata()->bbox().width(), 0.) << "Courtesy clef in measure 6 is NOT hidden.";

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"clef_courtesy01.mscx", CLEFCOURTESY_DATA_DIR + u"clef_courtesy01-ref.mscx"));

    Clef* clef = nullptr;
    seg = m1->findSegment(SegmentType::HeaderClef, m1->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 4.";

    clef = static_cast<Clef*>(seg->element(0));
    score->startCmd(TranslatableString::untranslatable("Courtesy clef tests"));
    clef->undoChangeProperty(Pid::SHOW_COURTESY, false);
    Clef* otherClef = clef->otherClef();
    if (otherClef) {
        otherClef->undoChangeProperty(Pid::SHOW_COURTESY, false);
    }
    score->doLayout();
    score->endCmd();
    // check the required courtesy clef is there but hidden
    m = m1->prevMeasure();
    seg = m->findSegment(SegmentType::Clef, m1->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 3.";

    clefCourt = static_cast<Clef*>(seg->element(0));
    EXPECT_TRUE(clefCourt) << "No courtesy clef element in measure 3.";
    EXPECT_DOUBLE_EQ(clefCourt->ldata()->bbox().width(), 0.) << "Courtesy clef in measure 3 is not hidden when showCourtesy is false.";

    EXPECT_EQ(clef->clefType(), ClefType::G8_VA) << "Clef type in measure 4 is wrong";
    dropClef(clef, ClefType::G15_MA);
    EXPECT_EQ(clef->clefType(), ClefType::G15_MA) << "Clef type in measure 4 is wrong";

    delete score;
}

//---------------------------------------------------------
//    add two clefs mid-score at the beginning of systems and look for courtesy clefs
//    neither should be there, as courtesy clefs are turned off
//---------------------------------------------------------
TEST_F(Engraving_ClefCourtesyTests, clef_courtesy02)
{
    MasterScore* score = ScoreRW::readScore(CLEFCOURTESY_DATA_DIR + "clef_courtesy02.mscx");
    EXPECT_TRUE(score);

    // 'go' to 4th measure
    Measure* m1 = score->firstMeasure();
    for (int i=0; i < 3; i++) {
        m1 = m1->nextMeasure();
    }
    // make a clef-drop object and drop it to the measure
    Clef* clef = Factory::createClef(score->dummy()->segment());   // create a new element, as Measure::drop() will eventually delete it
    clef->setClefType(ClefType::G8_VA);
    EditData dropData(0);
    dropData.pos = m1->pagePos();
    dropData.dropElement = clef;
    m1->drop(dropData);

    // 'go' to 7th measure
    Measure* m2 = m1;
    for (int i=0; i < 3; i++) {
        m2 = m2->nextMeasure();
    }
    // make a clef-drop object and drop it to the measure
    clef = Factory::createClef(score->dummy()->segment());   // create a new element, as Measure::drop() will eventually delete it
    clef->setClefType(ClefType::G);
    dropData.pos = m2->pagePos();
    dropData.dropElement = clef;
    m2->drop(dropData);
    score->doLayout();

    // check both clef elements are there, but none is shown
    Clef* clefCourt = nullptr;
    Measure* m = m1->prevMeasure();
    Segment* seg = m->findSegment(SegmentType::Clef, m1->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 3.";

    clefCourt = toClef(seg->element(0));
    EXPECT_TRUE(clefCourt) << "No courtesy clef element in measure 3.";
    EXPECT_DOUBLE_EQ(clefCourt->ldata()->bbox().width(), 0.) << "Courtesy clef in measure 3 is NOT hidden.";

    clefCourt = nullptr;
    m = m2->prevMeasure();
    seg = m->findSegment(SegmentType::Clef, m2->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 6.";

    clefCourt = static_cast<Clef*>(seg->element(0));
    EXPECT_TRUE(clefCourt) << "No courtesy clef element in measure 6.";
    EXPECT_DOUBLE_EQ(clefCourt->ldata()->bbox().width(), 0.) << "Courtesy clef in measure 6 is NOT hidden.";

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"clef_courtesy02.mscx", CLEFCOURTESY_DATA_DIR + u"clef_courtesy02-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//    tests issue #76006 "if insert clef to measure after single-measure system with section break, then should not display courtesy clef"
//    adds a clef on meas 2, which occurs after a single-measure section
//    the added clef should not be visible, since it is after a section break
//---------------------------------------------------------
TEST_F(Engraving_ClefCourtesyTests, clef_courtesy03)
{
    MasterScore* score = ScoreRW::readScore(CLEFCOURTESY_DATA_DIR + "clef_courtesy03.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    // make a clef-drop object and drop it to the 2nd measure
    Clef* clef = Factory::createClef(score->dummy()->segment());   // create a new element, as Measure::drop() will eventually delete it
    clef->setClefType(ClefType::G8_VA);
    EditData dropData(0);
    dropData.pos = m2->pagePos();
    dropData.dropElement = clef;
    m2->drop(dropData);
    score->doLayout();

    // verify the not required courtesy clef element is on end of m1 but is not shown
    Clef* clefCourt = nullptr;
    Segment* seg = m1->findSegment(SegmentType::Clef, m2->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 1.";

    clefCourt = static_cast<Clef*>(seg->element(0));
    EXPECT_TRUE(clefCourt) << "No courtesy clef element in measure 1.";
    EXPECT_DOUBLE_EQ(clefCourt->ldata()->bbox().width(), 0.) << "Courtesy clef in measure 1 is NOT hidden.";

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"clef_courtesy03.mscx", CLEFCOURTESY_DATA_DIR + u"clef_courtesy03-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//    input score has section breaks on non-measure MeasureBase objects.
//    should not display courtesy clefs at the end of final measure of each section (meas 2, 4, & 6), even if section break occurs on subsequent non-measure frame.
//---------------------------------------------------------
TEST_F(Engraving_ClefCourtesyTests, clef_courtesy_78196)
{
    MasterScore* score = ScoreRW::readScore(CLEFCOURTESY_DATA_DIR + "clef_courtesy_78196.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();
    Measure* m3 = m2->nextMeasure();
    Measure* m4 = m3->nextMeasure();
    Measure* m5 = m4->nextMeasure();
    Measure* m6 = m5->nextMeasure();
    Measure* m7 = m6->nextMeasure();

    // check both clef elements are there, but none is shown
    Clef* clefCourt = nullptr;
    Segment* seg = nullptr;

    // verify clef exists in segment of final tick of m2, but that it is not visible
    seg = m2->findSegment(SegmentType::Clef, m3->tick());
    EXPECT_TRUE(seg) << "No SegClef at end of measure 2.";

    clefCourt = static_cast<Clef*>(seg->element(0));
    EXPECT_TRUE(clefCourt) << "No courtesy clef at end of measure 2.";
    EXPECT_DOUBLE_EQ(clefCourt->ldata()->bbox().width(), 0.) << "Courtesy clef at end of measure 2 is NOT hidden.";

    // verify clef exists in segment of final tick of m4, but that it is not visible
    seg = m4->findSegment(SegmentType::Clef, m5->tick());
    EXPECT_TRUE(seg) << "No SegClef at end of measure 4.";

    clefCourt = static_cast<Clef*>(seg->element(0));
    EXPECT_TRUE(clefCourt) << "No courtesy clef at end of measure 4.";
    EXPECT_DOUBLE_EQ(clefCourt->ldata()->bbox().width(), 0.) << "Courtesy clef at end of measure 4 is NOT hidden.";

    // verify clef exists in segment of final tick of m6, but that it is not visible
    seg = m6->findSegment(SegmentType::Clef, m7->tick());
    EXPECT_TRUE(seg) << "No SegClef at end of measure 6.";

    clefCourt = static_cast<Clef*>(seg->element(0));
    EXPECT_TRUE(clefCourt) << "No courtesy clef at end of measure 6.";
    EXPECT_DOUBLE_EQ(clefCourt->ldata()->bbox().width(), 0.) << "Courtesy clef at end of measure 6 is NOT hidden.";
}

//---------------------------------------------------------
//    add two clefs mid-score at the beginning of systems for different
//    staves and look for courtesy clefs in different "show courtesy" configurations
//    also check the behavior after a horizontal frame
//---------------------------------------------------------
TEST_F(Engraving_ClefCourtesyTests, clef_courtesy04)
{
    MasterScore* score = ScoreRW::readScore(CLEFCOURTESY_DATA_DIR + "clef_courtesy04.mscx");
    EXPECT_TRUE(score);

    Clef* clef = nullptr;
    Segment* seg = nullptr;

    // drop G1 clef to 4th measure, track 0 and track 4
    Measure* m1 = getMeasure(score, 4);
    seg = m1->findSegment(SegmentType::HeaderClef, m1->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 4.";
    clef = toClef(seg->element(0));
    EXPECT_TRUE(clef) << "No Clef in measure 4, track 0.";
    dropClef(clef, ClefType::G8_VA);
    clef = toClef(seg->element(4));
    EXPECT_TRUE(clef) << "No Clef in measure 4, track 4.";
    dropClef(clef, ClefType::G8_VA);

    // drop G clef to 7th measure, track 4
    Measure* m2 = getMeasure(score, 7);
    seg = m2->findSegment(SegmentType::HeaderClef, m2->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 7.";
    clef = toClef(seg->element(4));
    EXPECT_TRUE(clef) << "No Clef in measure 7, track 4.";
    dropClef(clef, ClefType::G);

    // check the required courtesy clefs are there and they are shown
    Measure* m = m1->prevMeasure();
    seg = m->findSegment(SegmentType::Clef, m1->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 3.";
    clef = toClef(seg->element(0));
    EXPECT_TRUE(clef) << "No courtesy clef element in measure 3, track 0.";
    EXPECT_GT(clef->ldata()->bbox().width(), 0) << "Courtesy clef in measure 3 is hidden.";
    clef = toClef(seg->element(4));
    EXPECT_TRUE(clef) << "No courtesy clef element in measure 3, track 4.";
    EXPECT_GT(clef->ldata()->bbox().width(), 0) << "Courtesy clef in measure 3 is hidden.";

    // change "show courtesy" property for the clef in one of the staves
    seg = m1->findSegment(SegmentType::HeaderClef, m1->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 4.";
    clef = toClef(seg->element(0));
    score->startCmd(TranslatableString::untranslatable("Courtesy clef tests"));
    clef->undoChangeProperty(Pid::SHOW_COURTESY, false);
    Clef* otherClef = clef->otherClef();
    if (otherClef) {
        otherClef->undoChangeProperty(Pid::SHOW_COURTESY, false);
    }
    score->doLayout();
    score->endCmd();

    // check the required courtesy clef is there but hidden, and the other one is not hidden
    seg = m->findSegment(SegmentType::Clef, m1->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 3.";
    clef = toClef(seg->element(0));
    EXPECT_TRUE(clef) << "No courtesy clef element in measure 3 for track 0.";
    EXPECT_DOUBLE_EQ(clef->ldata()->bbox().width(),
                     0.) << "Courtesy clef in measure 3, track 0, is not hidden when showCourtesy is false.";
    clef = toClef(seg->element(4));
    EXPECT_TRUE(clef) << "No courtesy clef element in measure 3 for track 4.";
    EXPECT_GT(clef->ldata()->bbox().width(), 0) << "Courtesy clef in measure 3, track 4, is hidden when showCourtesy is true.";

    // drop clefs after a horizontal frame
    Measure* m3 = getMeasure(score, 8);
    seg = m3->findSegment(SegmentType::HeaderClef, m3->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 8.";
    // drop G clef to 8th measure, track 0
    clef = toClef(seg->element(0));
    EXPECT_TRUE(clef) << "No Clef in measure 8, track 0.";
    dropClef(clef, ClefType::G);
    // drop G1 clef to 8th measure, track 4
    clef = toClef(seg->element(4));
    EXPECT_TRUE(clef) << "No Clef in measure 8, track 4.";
    dropClef(clef, ClefType::G8_VA);
    score->startCmd(TranslatableString::untranslatable("Courtesy clef tests"));
    clef->undoChangeProperty(Pid::SHOW_COURTESY, false);
    otherClef = clef->otherClef();
    if (otherClef) {
        otherClef->undoChangeProperty(Pid::SHOW_COURTESY, false);
    }
    score->doLayout();
    score->endCmd();

    // check the required courtesy clef is there but hidden, and the other one is not hidden
    seg = m2->findSegment(SegmentType::Clef, m3->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 7.";
    clef = toClef(seg->element(4));
    EXPECT_TRUE(clef) << "No courtesy clef element in measure 7 for track 4.";
    EXPECT_DOUBLE_EQ(clef->ldata()->bbox().width(),
                     0.) << "Courtesy clef in measure 7, track 4, is not hidden when showCourtesy is false.";
    clef = toClef(seg->element(0));
    EXPECT_TRUE(clef) << "No courtesy clef element in measure 7 for track 0.";
    EXPECT_GT(clef->ldata()->bbox().width(), 0) << "Courtesy clef in measure 7, track 0, is hidden when showCourtesy is true.";

    // for the section break case,
    // check the not required courtesy clef element is there but it is not shown
    m   = m2->prevMeasure();
    seg = m->findSegment(SegmentType::Clef, m2->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 6.";
    clef = toClef(seg->element(4));
    EXPECT_TRUE(clef) << "No courtesy clef element in measure 6, track 4.";
    EXPECT_DOUBLE_EQ(clef->ldata()->bbox().width(), 0.) << "Courtesy clef in measure 6, track 4, is NOT hidden.";

    Measure* m4 = getMeasure(score, 9);
    // drop G1 clef to 9th measure, track 0
    dropClef(m4, ClefType::G8_VA);
    m = m4->prevMeasure();
    seg = m->findSegment(SegmentType::Clef, m4->tick());
    EXPECT_TRUE(seg) << "No SegClef in measure 8.";
    clef = toClef(seg->element(0));
    EXPECT_TRUE(clef) << "No Clef change in measure 8, track 0.";
    EXPECT_GT(clef->ldata()->bbox().width(), 0) << "Clef change in measure 8, track 0, is hidden.";
    score->startCmd(TranslatableString::untranslatable("Courtesy clef tests"));
    clef->undoChangeProperty(Pid::SHOW_COURTESY, false);
    otherClef = clef->otherClef();
    if (otherClef) {
        otherClef->undoChangeProperty(Pid::SHOW_COURTESY, false);
    }
    score->doLayout();
    score->endCmd();
    EXPECT_GT(clef->ldata()->bbox().width(), 0) << "Clef change in measure 8, track 0, is hidden when showCourtesy is true.";

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"clef_courtesy04.mscx", CLEFCOURTESY_DATA_DIR + u"clef_courtesy04-ref.mscx"));

    delete score;
}
