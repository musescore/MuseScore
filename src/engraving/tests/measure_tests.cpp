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

#include "dom/engravingitem.h"
#include "dom/excerpt.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/measurenumber.h"
#include "dom/rest.h"
#include "dom/segment.h"
#include "dom/undo.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"
#include "utils/testutils.h"

using namespace mu;
using namespace mu::engraving;

static const String MEASURE_DATA_DIR("measure_data/");

class Engraving_MeasureTests : public ::testing::Test
{
};

TEST_F(Engraving_MeasureTests, DISABLED_insertMeasureMiddle) //TODO: verify program change, 72 is wrong surely?
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    EXPECT_TRUE(score);

    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    Measure* m = score->firstMeasure()->nextMeasure();
    score->insertMeasure(m);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-1.mscx", MEASURE_DATA_DIR + u"measure-1-ref.mscx"));
    delete score;
}

TEST_F(Engraving_MeasureTests, DISABLED_insertMeasureBegin) // TODO: verify program change, 72 is wrong surely?
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    EXPECT_TRUE(score);

    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    Measure* m = score->firstMeasure();
    score->insertMeasure(m);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-2.mscx", MEASURE_DATA_DIR + u"measure-2-ref.mscx"));
    delete score;
}

TEST_F(Engraving_MeasureTests, DISABLED_insertMeasureEnd) // TODO: verify program change, 72 is wrong surely?
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + "measure-1.mscx");
    EXPECT_TRUE(score);

    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->insertMeasure(0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-3.mscx", MEASURE_DATA_DIR + u"measure-3-ref.mscx"));
    delete score;
}

TEST_F(Engraving_MeasureTests, insertAtBeginning)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-insert_beginning.mscx");
    EXPECT_TRUE(score);
    Measure* m = score->firstMeasure();
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->insertMeasure(m);
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_beginning.mscx",
                                            MEASURE_DATA_DIR + u"measure-insert_beginning-ref.mscx"));
    delete score;
}

TEST_F(Engraving_MeasureTests, insertBfClefChange)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-insert_bf_clef.mscx");
    EXPECT_TRUE(score);

    // 4th measure
    Measure* m = score->firstMeasure()->nextMeasure();
    m = m->nextMeasure()->nextMeasure();
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->insertMeasure(m);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_clef.mscx", MEASURE_DATA_DIR + u"measure-insert_bf_clef-ref.mscx"));

    score->undoRedo(true, 0);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_clef_undo.mscx", MEASURE_DATA_DIR + u"measure-insert_bf_clef.mscx"));

    m = score->firstMeasure()->nextMeasure()->nextMeasure()->nextMeasure()->nextMeasure()->nextMeasure();
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->insertMeasure(m);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_clef-2.mscx",
                                            MEASURE_DATA_DIR + u"measure-insert_bf_clef-2-ref.mscx"));

    score->undoRedo(true, 0);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_clef_undo.mscx", MEASURE_DATA_DIR + u"measure-insert_bf_clef.mscx"));
    delete score;
}

TEST_F(Engraving_MeasureTests, insertBfKeyChange)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-insert_bf_key.mscx");
    EXPECT_TRUE(score);

    // 4th measure
    Measure* m = score->firstMeasure()->nextMeasure();
    m = m->nextMeasure()->nextMeasure();
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->insertMeasure(m);
    score->endCmd();

    EXPECT_TRUE(score->checkKeys());
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_key.mscx", MEASURE_DATA_DIR + u"measure-insert_bf_key-ref.mscx"));

    score->undoRedo(true, 0);

    EXPECT_TRUE(score->checkKeys());
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_key_undo.mscx",
                                            MEASURE_DATA_DIR + u"measure-insert_bf_key_undo-ref.mscx"));

    m = score->firstMeasure()->nextMeasure()->nextMeasure()->nextMeasure()->nextMeasure()->nextMeasure();
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->insertMeasure(m);
    score->endCmd();
    EXPECT_TRUE(score->checkKeys());
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_key-2.mscx",
                                            MEASURE_DATA_DIR + u"measure-insert_bf_key-2-ref.mscx"));

    score->undoRedo(true, 0);

    EXPECT_TRUE(score->checkKeys());
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-insert_bf_key_undo.mscx",
                                            MEASURE_DATA_DIR + u"measure-insert_bf_key_undo-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///   spanner_a
//
//  +----spanner--------+
//    +---add---
//
//---------------------------------------------------------

TEST_F(Engraving_MeasureTests, spanner_a)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-3.mscx");
    EXPECT_TRUE(score);

    Measure* m = score->firstMeasure()->nextMeasure();
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->insertMeasure(m);
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

TEST_F(Engraving_MeasureTests, spanner_b)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-4.mscx");
    EXPECT_TRUE(score);

    Measure* m = score->firstMeasure();
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->insertMeasure(m);
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

TEST_F(Engraving_MeasureTests, spanner_A)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-6.mscx");
    EXPECT_TRUE(score);

    score->select(score->firstMeasure());
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->cmdTimeDelete();
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

TEST_F(Engraving_MeasureTests, spanner_B)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-7.mscx");
    EXPECT_TRUE(score);

    Measure* m = score->firstMeasure()->nextMeasure();
    score->select(m);
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->cmdTimeDelete();
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

TEST_F(Engraving_MeasureTests, spanner_C)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-8.mscx");
    EXPECT_TRUE(score);

    Measure* m = score->firstMeasure()->nextMeasure();
    score->select(m);
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->cmdTimeDelete();
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

TEST_F(Engraving_MeasureTests, spanner_D)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-9.mscx");
    EXPECT_TRUE(score);

    Measure* m = score->firstMeasure()->nextMeasure();
    score->select(m);
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->cmdTimeDelete();
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-9.mscx", MEASURE_DATA_DIR + u"measure-9-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//    deleteLast
//---------------------------------------------------------

TEST_F(Engraving_MeasureTests, deleteLast)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-10.mscx");
    EXPECT_TRUE(score);

    Measure* m = score->lastMeasure();
    score->select(m);
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->cmdTimeDelete();
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measure-10.mscx", MEASURE_DATA_DIR + u"measure-10-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//    delete rests and check reorganization of lengths
//---------------------------------------------------------

TEST_F(Engraving_MeasureTests, gap)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"gaps.mscx");
    EXPECT_TRUE(score);

    EngravingItem* tst = 0;

    //Select and delete third quarter rest in first Measure (voice 2)
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
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
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
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
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
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

TEST_F(Engraving_MeasureTests, checkMeasure)
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
//      assert(tst);

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

TEST_F(Engraving_MeasureTests, undoDelInitialVBox_269919)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"undoDelInitialVBox_269919.mscx");
    EXPECT_TRUE(score);

    // 1. delete initial VBox
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    MeasureBase* initialVBox = score->measure(0);
    score->select(initialVBox);
    score->cmdDeleteSelection();
    score->endCmd();

    // 2. change duration of first chordrest
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
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

TEST_F(Engraving_MeasureTests, mmrest)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"mmrest.mscx");
    EXPECT_TRUE(score);

    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->undoChangeStyleVal(Sid::createMultiMeasureRests, true);
    score->setLayoutAll();
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"mmrest.mscx", MEASURE_DATA_DIR + u"mmrest-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///   measureNumbers
///    test measure numbers properties
//---------------------------------------------------------

TEST_F(Engraving_MeasureTests, measureNumbers)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measurenumber.mscx");
    EXPECT_TRUE(score);

    // Place measure numbers below
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->undoChangeStyleVal(Sid::measureNumberVPlacement, PlacementV::BELOW);
    score->setLayoutAll();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measurenumber-1.mscx", MEASURE_DATA_DIR + u"measurenumber-1-ref.mscx"));

    // center measure numbers
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->undoChangeStyleVal(Sid::measureNumberHPlacement, AlignH::HCENTER);
    score->setLayoutAll();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measurenumber-2.mscx", MEASURE_DATA_DIR + u"measurenumber-2-ref.mscx"));

    // show on first system too
    score->undoChangeStyleVal(Sid::showMeasureNumberOne, true);
    score->setLayoutAll();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measurenumber-3.mscx", MEASURE_DATA_DIR + u"measurenumber-3-ref.mscx"));

    // every 5 measures (default interval)
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    // to know whether measure numbers are shown at regular intervals or on every system,
    // musescore simply checks if measure numbers are shown at system or not.
    score->undoChangeStyleVal(Sid::measureNumberSystem, false);
    score->setLayoutAll();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measurenumber-4.mscx", MEASURE_DATA_DIR + u"measurenumber-4-ref.mscx"));

    // do not show first measure number. This should shift all measure numbers,
    // because they are still placed at regular intervals.
    // Instead of being at 1-6-11-16-21, etc. They should be at 5-10-15-20-25, etc.
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->undoChangeStyleVal(Sid::showMeasureNumberOne, false);
    score->setLayoutAll();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measurenumber-5.mscx", MEASURE_DATA_DIR + u"measurenumber-5-ref.mscx"));

    // show at every measure (except fist)
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->undoChangeStyleVal(Sid::measureNumberInterval, 1);
    score->setLayoutAll();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measurenumber-6.mscx", MEASURE_DATA_DIR + u"measurenumber-6-ref.mscx"));

    // Disable measure numbers
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    // to know whether measure numbers are shown at regular intervals or on every system,
    // musescore simply checks if measure numbers are shown at system or not.
    score->undoChangeStyleVal(Sid::showMeasureNumber, false);
    score->setLayoutAll();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measurenumber-7.mscx", MEASURE_DATA_DIR + u"measurenumber-7-ref.mscx"));

    delete score;
}

TEST_F(Engraving_MeasureTests, changeMeasureLen) {
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"changeMeasureLen.mscx");
    EXPECT_TRUE(score);

    Measure* m = score->firstMeasure()->nextMeasure();

    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));

    m->adjustToLen(Fraction(2, 4));

    m->adjustToLen(Fraction(6, 4));

    score->setLayoutAll();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"changeMeasureLen.mscx", MEASURE_DATA_DIR + u"changeMeasureLen-ref.mscx"));
}

TEST_F(Engraving_MeasureTests, measureSplit) {
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measureSplit.mscx");
    EXPECT_TRUE(score);

    TestUtils::createParts(score, 2);
    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));

    Measure* m = score->firstMeasure()->nextMeasure();
    EXPECT_TRUE(m);
    ChordRest* cr = m->first(SegmentType::ChordRest)->next()->nextChordRest(0);
    EXPECT_TRUE(cr);

    score->cmdSplitMeasure(cr);

    score->setLayoutAll();
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"measureSplit.mscx", MEASURE_DATA_DIR + u"measureSplit-ref.mscx"));
}

TEST_F(Engraving_MeasureTests, MMRestEndOfMeasureTS) {
    bool use302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"mmrEndOfMeasureTimeSig.mscz");
    EXPECT_TRUE(score);

    Measure* m3 = score->crMeasure(2);
    EXPECT_TRUE(m3 && !m3->isMMRest());
    Segment* tsSeg = m3->findSegmentR(SegmentType::TimeSig, m3->ticks());
    EXPECT_TRUE(tsSeg);
    EngravingItem* tsItem = tsSeg->element(0);
    EXPECT_TRUE(tsItem && tsItem->isTimeSig());

    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->undoChangeStyleVal(Sid::createMultiMeasureRests, true);
    score->setLayoutAll();
    score->endCmd();

    Measure* m3MMR = m3->mmRest();
    EXPECT_TRUE(m3MMR && m3MMR->isMMRest());
    Segment* tsSegMMR = m3MMR->findSegmentR(SegmentType::TimeSig, m3MMR->ticks());
    EXPECT_TRUE(tsSegMMR && tsSegMMR->endOfMeasureChange());
    EngravingItem* tsItemMMR = tsSeg->element(0);
    EXPECT_TRUE(tsItemMMR && tsItemMMR->isTimeSig());

    MScore::useRead302InTestMode = use302;
}

TEST_F(Engraving_MeasureTests, MMRestContinuationCourtesies) {
    bool use302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"mmrContinuationCourtesies.mscz");
    EXPECT_TRUE(score);

    auto checkSegmentsAndItems = [](Measure* m, bool continuationRepeat) {
        Fraction tick = continuationRepeat ? Fraction(0, 0) : m->ticks();

        SegmentType timeSegType = continuationRepeat ? SegmentType::TimeSigStartRepeatAnnounce : SegmentType::TimeSigRepeatAnnounce;
        Segment* tsSeg = m->findSegmentR(timeSegType, tick);
        EXPECT_TRUE(tsSeg);
        EngravingItem* tsItem = tsSeg->element(0);
        EXPECT_TRUE(tsItem && tsItem->isTimeSig());

        SegmentType keySegType = continuationRepeat ? SegmentType::KeySigStartRepeatAnnounce : SegmentType::KeySigRepeatAnnounce;
        Segment* ksSeg = m->findSegmentR(keySegType, tick);
        EXPECT_TRUE(ksSeg);
        EngravingItem* ksItem = ksSeg->element(0);
        EXPECT_TRUE(ksItem && ksItem->isKeySig());

        SegmentType clefSegType = continuationRepeat ? SegmentType::ClefStartRepeatAnnounce : SegmentType::ClefRepeatAnnounce;
        Segment* clefSeg = m->findSegmentR(clefSegType, tick);
        EXPECT_TRUE(clefSeg);
        EngravingItem* clefItem = clefSeg->element(0);
        EXPECT_TRUE(clefItem && clefItem->isClef());
    };

    // Check end of measure courtesies
    Measure* m2 = score->crMeasure(1);
    EXPECT_TRUE(m2 && !m2->isMMRest());
    checkSegmentsAndItems(m2, false);

    // Check continuation courtesies
    Measure* m3 = m2->nextMeasure();
    EXPECT_TRUE(m3 && !m3->isMMRest());
    checkSegmentsAndItems(m3, true);

    score->startCmd(TranslatableString::untranslatable("Engraving measure tests"));
    score->undoChangeStyleVal(Sid::createMultiMeasureRests, true);
    score->setLayoutAll();
    score->endCmd();

    Measure* m2MMR = m2->mmRest();
    EXPECT_TRUE(m2MMR && m2MMR->isMMRest());
    checkSegmentsAndItems(m2MMR, false);

    Measure* m3MMR = m3->mmRest();
    EXPECT_TRUE(m3MMR && m3MMR->isMMRest());
    checkSegmentsAndItems(m3MMR, true);

    MScore::useRead302InTestMode = use302;
}
