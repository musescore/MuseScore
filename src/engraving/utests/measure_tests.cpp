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
#include "libmscore/excerpt.h"
#include "libmscore/part.h"
#include "libmscore/undo.h"
#include "libmscore/measure.h"
#include "libmscore/measurenumber.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/breath.h"
#include "libmscore/segment.h"
#include "libmscore/fingering.h"
#include "libmscore/image.h"
#include "libmscore/engravingitem.h"
#include "libmscore/system.h"
#include "libmscore/durationtype.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String MEASURE_DATA_DIR("measure_data/");

class MeasureTests : public ::testing::Test
{
};

TEST_F(MeasureTests, DISABLED_insertMeasureMiddle) //TODO: verify program change, 72 is wrong surely?
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    EXPECT_TRUE(score);

    score->startCmd();
    Measure* m = score->firstMeasure()->nextMeasure();
    score->insertMeasure(ElementType::MEASURE, m);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-1.mscx", MEASURE_DATA_DIR + u"measure-1-ref.mscx"));
    delete score;
}

TEST_F(MeasureTests, DISABLED_insertMeasureBegin) // TODO: verify program change, 72 is wrong surely?
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    EXPECT_TRUE(score);

    score->startCmd();
    Measure* m = score->firstMeasure();
    score->insertMeasure(ElementType::MEASURE, m);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-2.mscx", MEASURE_DATA_DIR + u"measure-2-ref.mscx"));
    delete score;
}

TEST_F(MeasureTests, DISABLED_insertMeasureEnd) // TODO: verify program change, 72 is wrong surely?
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + "measure-1.mscx");
    EXPECT_TRUE(score);

    score->startCmd();
    score->insertMeasure(ElementType::MEASURE, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-3.mscx", MEASURE_DATA_DIR + u"measure-3-ref.mscx"));
    delete score;
}

TEST_F(MeasureTests, insertBfClefChange)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-insert_bf_clef.mscx");
    EXPECT_TRUE(score);

    // 4th measure
    Measure* m = score->firstMeasure()->nextMeasure();
    m = m->nextMeasure()->nextMeasure();
    score->startCmd();
    score->insertMeasure(ElementType::MEASURE, m);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_clef.mscx", MEASURE_DATA_DIR + u"measure-insert_bf_clef-ref.mscx"));

    score->undoRedo(true, 0);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_clef_undo.mscx", MEASURE_DATA_DIR + u"measure-insert_bf_clef.mscx"));

    m = score->firstMeasure()->nextMeasure()->nextMeasure()->nextMeasure()->nextMeasure()->nextMeasure();
    score->startCmd();
    score->insertMeasure(ElementType::MEASURE, m);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_clef-2.mscx",
                                            MEASURE_DATA_DIR + u"measure-insert_bf_clef-2-ref.mscx"));

    score->undoRedo(true, 0);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_clef_undo.mscx", MEASURE_DATA_DIR + u"measure-insert_bf_clef.mscx"));
    delete score;
}

TEST_F(MeasureTests, insertBfKeyChange)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-insert_bf_key.mscx");
    EXPECT_TRUE(score);

    // 4th measure
    Measure* m = score->firstMeasure()->nextMeasure();
    m = m->nextMeasure()->nextMeasure();
    score->startCmd();
    score->insertMeasure(ElementType::MEASURE, m);
    score->endCmd();

    EXPECT_TRUE(score->checkKeys());
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_key.mscx", MEASURE_DATA_DIR + u"measure-insert_bf_key-ref.mscx"));

    score->undoRedo(true, 0);

    EXPECT_TRUE(score->checkKeys());
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_key_undo.mscx", MEASURE_DATA_DIR + u"measure-insert_bf_key.mscx"));

    m = score->firstMeasure()->nextMeasure()->nextMeasure()->nextMeasure()->nextMeasure()->nextMeasure();
    score->startCmd();
    score->insertMeasure(ElementType::MEASURE, m);
    score->endCmd();
    EXPECT_TRUE(score->checkKeys());
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_key-2.mscx",
                                            MEASURE_DATA_DIR + u"measure-insert_bf_key-2-ref.mscx"));

    score->undoRedo(true, 0);

    EXPECT_TRUE(score->checkKeys());
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_key_undo.mscx", MEASURE_DATA_DIR + u"measure-insert_bf_key.mscx"));
    delete score;
}

//---------------------------------------------------------
///   spanner_a
//
//  +----spanner--------+
//    +---add---
//
//---------------------------------------------------------

TEST_F(MeasureTests, spanner_a)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-3.mscx");
    EXPECT_TRUE(score);

    Measure* m = score->firstMeasure()->nextMeasure();
    score->startCmd();
    score->insertMeasure(ElementType::MEASURE, m);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-4.mscx", MEASURE_DATA_DIR + u"measure-4-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///   spanner_b
//
//       +----spanner--------
//  +---add---
//
//---------------------------------------------------------

TEST_F(MeasureTests, spanner_b)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-4.mscx");
    EXPECT_TRUE(score);

    Measure* m = score->firstMeasure();
    score->startCmd();
    score->insertMeasure(ElementType::MEASURE, m);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-5.mscx", MEASURE_DATA_DIR + u"measure-5-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///   spanner_A
//
//  +----remove---+ +---spanner---+
//
//---------------------------------------------------------

TEST_F(MeasureTests, spanner_A)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-6.mscx");
    EXPECT_TRUE(score);

    score->select(score->firstMeasure());
    score->startCmd();
    score->localTimeDelete();
    score->endCmd();
    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-6.mscx", MEASURE_DATA_DIR + u"measure-6-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///   spanner_B
//
//  +----spanner--------+
//    +---remove---+
//
//---------------------------------------------------------

TEST_F(MeasureTests, spanner_B)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-7.mscx");
    EXPECT_TRUE(score);

    Measure* m = score->firstMeasure()->nextMeasure();
    score->select(m);
    score->startCmd();
    score->localTimeDelete();
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-7.mscx", MEASURE_DATA_DIR + u"measure-7-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///   spanner_C
//
//    +---spanner---+
//  +----remove--------+
//
//---------------------------------------------------------

TEST_F(MeasureTests, spanner_C)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-8.mscx");
    EXPECT_TRUE(score);

    Measure* m = score->firstMeasure()->nextMeasure();
    score->select(m);
    score->startCmd();
    score->localTimeDelete();
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-8.mscx", MEASURE_DATA_DIR + u"measure-8-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///   spanner_D
//
//       +----spanner--------+
//  +---remove---+
//
//---------------------------------------------------------

TEST_F(MeasureTests, spanner_D)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-9.mscx");
    EXPECT_TRUE(score);

    Measure* m = score->firstMeasure()->nextMeasure();
    score->select(m);
    score->startCmd();
    score->localTimeDelete();
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-9.mscx", MEASURE_DATA_DIR + u"measure-9-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//    deleteLast
//---------------------------------------------------------

TEST_F(MeasureTests, deleteLast)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-10.mscx");
    EXPECT_TRUE(score);

    Measure* m = score->lastMeasure();
    score->select(m);
    score->startCmd();
    score->localTimeDelete();
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-10.mscx", MEASURE_DATA_DIR + u"measure-10-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//    delete rests and check reorganization of lengths
//---------------------------------------------------------

TEST_F(MeasureTests, gap)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"gaps.mscx");
    EXPECT_TRUE(score);

    EngravingItem* tst = 0;

    //Select and delete third quarter rest in first Measure (voice 2)
    score->startCmd();
    Measure* m  = score->firstMeasure();
    Segment* s  = m->undoGetSegment(SegmentType::ChordRest, Fraction::fromTicks(960));
    EngravingItem* el = s->element(1);
    score->select(el);
    score->cmdDeleteSelection();
    score->endCmd();

    tst = s->element(1);
    EXPECT_TRUE(tst);

    EXPECT_TRUE(tst->isRest());
    EXPECT_TRUE(toRest(tst)->isGap());
    /*&& toRest(tst)->durationType() == DurationType::V_QUARTER*/

    //Select and delete second quarter rest in third Measure (voice 4)
    score->startCmd();
    m  = m->nextMeasure()->nextMeasure();
    s  = m->undoGetSegment(SegmentType::ChordRest, Fraction::fromTicks(4320));
    el = s->element(3);
    score->select(el);
    score->cmdDeleteSelection();
    score->endCmd();

    tst = s->element(3);
    EXPECT_TRUE(tst);

    EXPECT_TRUE(tst->isRest());
    EXPECT_TRUE(toRest(tst)->isGap());
    /*&& toRest(tst)->durationType() == DurationType::V_QUARTER*/

    //Select and delete first quarter rest in third Measure (voice 4)
    score->startCmd();
    s  = m->undoGetSegment(SegmentType::ChordRest, Fraction::fromTicks(3840));
    el = s->element(3);
    score->select(el);
    score->cmdDeleteSelection();
    score->endCmd();

    tst = s->element(3);
    EXPECT_TRUE(tst);

    EXPECT_TRUE(tst->isRest());
    EXPECT_TRUE(toRest(tst)->isGap());
    EXPECT_EQ(toRest(tst)->actualTicks(), Fraction::fromTicks(960));
    /*&& toRest(tst)->durationType() == DurationType::V_HALF*/

    delete score;
}

//---------------------------------------------------------
///   checkMeasure
//
//    import a Score with gaps in excerpt and
//
//---------------------------------------------------------

TEST_F(MeasureTests, checkMeasure)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"checkMeasure.mscx");
    EXPECT_TRUE(score);

    EngravingItem* tst = 0;
    Measure* m = score->firstMeasure()->nextMeasure();

    Segment* s = m->undoGetSegment(SegmentType::ChordRest, Fraction::fromTicks(2880));
    tst = s->element(1);
    EXPECT_TRUE(tst);

    EXPECT_TRUE(tst->isRest());
    EXPECT_TRUE(toRest(tst)->isGap());
    EXPECT_EQ(toRest(tst)->actualTicks(), Fraction::fromTicks(480));
    /*&& toRest(tst)->durationType() == DurationType::V_HALF*/

    m = m->nextMeasure();
//      s = m->undoGetSegment(SegmentType::ChordRest, Fraction::fromTicks(3840));
//      tst = s->element(2);
//      Q_ASSERT(tst);

//      QVERIFY(tst->isRest() && toRest(tst)->isGap() && toRest(tst)->actualTicks() == 480/*&& toRest(tst)->durationType() == DurationType::V_HALF*/);

    m = m->nextMeasure();
    s = m->undoGetSegment(SegmentType::ChordRest, Fraction::fromTicks(6240));
    tst = s->element(1);
    EXPECT_TRUE(tst);

    EXPECT_TRUE(tst->isRest());
    EXPECT_TRUE(toRest(tst)->isGap());
    EXPECT_EQ(toRest(tst)->actualTicks(), Fraction::fromTicks(120));
    /*&& toRest(tst)->durationType() == DurationType::V_HALF*/

    s = m->undoGetSegment(SegmentType::ChordRest, Fraction::fromTicks(6480));
    tst = s->element(1);
    EXPECT_TRUE(tst);

    EXPECT_TRUE(tst->isRest());
    EXPECT_TRUE(toRest(tst)->isGap());
    EXPECT_EQ(toRest(tst)->actualTicks(), Fraction::fromTicks(120));
    /*&& toRest(tst)->durationType() == DurationType::V_HALF*/

    delete score;
}

//---------------------------------------------------------
///   undoDelInitialVBox_269919
///    1. Delete first VBox
///    2. Change duration of first chordrest
///    3. Undo to restore first chordrest
///    4. Undo to restore initial VBox results in assert failure crash
//---------------------------------------------------------

TEST_F(MeasureTests, undoDelInitialVBox_269919)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"undoDelInitialVBox_269919.mscx");
    EXPECT_TRUE(score);

    // 1. delete initial VBox
    score->startCmd();
    MeasureBase* initialVBox = score->measure(0);
    score->select(initialVBox);
    score->cmdDeleteSelection();
    score->endCmd();

    // 2. change duration of first chordrest
    score->startCmd();
    Measure* m = score->firstMeasure();
    ChordRest* cr = m->findChordRest(Fraction(0, 1), 0);
    Fraction quarter(4, 1);
    score->changeCRlen(cr, quarter);
    score->endCmd();

    // 3. Undo to restore first chordrest
    score->undoRedo(true, 0);

    // 4. Undo to restore initial VBox resulted in assert failure crash
    score->undoRedo(true, 0);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"undoDelInitialVBox_269919.mscx",
                                            MEASURE_DATA_DIR + u"undoDelInitialVBox_269919-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///   mmrest
///    mmrest creation
//---------------------------------------------------------

TEST_F(MeasureTests, mmrest)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"mmrest.mscx");
    EXPECT_TRUE(score);

    score->startCmd();
    score->undo(new ChangeStyleVal(score, Sid::createMultiMeasureRests, true));
    score->setLayoutAll();
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"mmrest.mscx", MEASURE_DATA_DIR + u"mmrest-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///   measureNumbers
///    test measure numbers properties
//---------------------------------------------------------

TEST_F(MeasureTests, measureNumbers)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measurenumber.mscx");
    EXPECT_TRUE(score);

    MeasureNumber* measureNumber = new MeasureNumber(score->dummy()->measure());

    // horizontal placement
    measureNumber->setHPlacement(PlacementH::CENTER);
    measureNumber->setPropertyFlags(Pid::HPLACEMENT, PropertyFlags::UNSTYLED);
    MeasureNumber* mn = static_cast<MeasureNumber*>(ScoreRW::writeReadElement(measureNumber));
    EXPECT_EQ(mn->hPlacement(), PlacementH::CENTER);
    delete mn;

    // Place measure numbers below
    score->startCmd();
    score->undo(new ChangeStyleVal(score, Sid::measureNumberVPlacement, PlacementV::BELOW));
    score->setLayoutAll();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measurenumber-1.mscx", MEASURE_DATA_DIR + u"measurenumber-1-ref.mscx"));

    // center measure numbers
    score->startCmd();
    score->undo(new ChangeStyleVal(score, Sid::measureNumberHPlacement, PlacementH::CENTER));
    score->setLayoutAll();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measurenumber-2.mscx", MEASURE_DATA_DIR + u"measurenumber-2-ref.mscx"));

    // show on first system too
    score->undo(new ChangeStyleVal(score, Sid::showMeasureNumberOne, true));
    score->setLayoutAll();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measurenumber-3.mscx", MEASURE_DATA_DIR + u"measurenumber-3-ref.mscx"));

    // every 5 measures (default interval)
    score->startCmd();
    // to know whether measure numbers are shown at regular intervals or on every system,
    // musescore simply checks if measure numbers are shown at system or not.
    score->undo(new ChangeStyleVal(score, Sid::measureNumberSystem, false));
    score->setLayoutAll();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measurenumber-4.mscx", MEASURE_DATA_DIR + u"measurenumber-4-ref.mscx"));

    // do not show first measure number. This should shift all measure numbers,
    // because they are still placed at regular intervals.
    // Instead of being at 1-6-11-16-21, etc. They should be at 5-10-15-20-25, etc.
    score->startCmd();
    score->undo(new ChangeStyleVal(score, Sid::showMeasureNumberOne, false));
    score->setLayoutAll();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measurenumber-5.mscx", MEASURE_DATA_DIR + u"measurenumber-5-ref.mscx"));

    // show at every measure (except fist)
    score->startCmd();
    score->undo(new ChangeStyleVal(score, Sid::measureNumberInterval, 1));
    score->setLayoutAll();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measurenumber-6.mscx", MEASURE_DATA_DIR + u"measurenumber-6-ref.mscx"));

    // Disable measure numbers
    score->startCmd();
    // to know whether measure numbers are shown at regular intervals or on every system,
    // musescore simply checks if measure numbers are shown at system or not.
    score->undo(new ChangeStyleVal(score, Sid::showMeasureNumber, false));
    score->setLayoutAll();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measurenumber-7.mscx", MEASURE_DATA_DIR + u"measurenumber-7-ref.mscx"));

    delete score;
}
