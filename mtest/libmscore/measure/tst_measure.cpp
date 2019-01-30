//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "libmscore/score.h"
#include "libmscore/excerpt.h"
#include "libmscore/part.h"
#include "libmscore/undo.h"
#include "libmscore/measure.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/breath.h"
#include "libmscore/segment.h"
#include "libmscore/fingering.h"
#include "libmscore/image.h"
#include "libmscore/element.h"
#include "libmscore/system.h"
#include "libmscore/durationtype.h"
#include "mtest/testutils.h"

#define DIR QString("libmscore/measure/")

using namespace Ms;

//---------------------------------------------------------
//   TestMeasure
//---------------------------------------------------------

class TestMeasure : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();

      void insertMeasureMiddle();
      void insertMeasureBegin();
      void insertMeasureEnd();
      void insertBfClefChange();
      void insertBfKeyChange();
      void spanner_a();
      void spanner_b();
      void spanner_A();
      void spanner_B();
      void spanner_C();
      void spanner_D();
      void deleteLast();
//      void minWidth();
      void undoDelInitialVBox_269919();

      void gap();
      void checkMeasure();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestMeasure::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
///   insertMeasureMiddle
//---------------------------------------------------------

void TestMeasure::insertMeasureMiddle()
      {
      MasterScore* score = readScore(DIR + "measure-1.mscx");

      score->startCmd();
      Measure* m = score->firstMeasure()->nextMeasure();
      score->insertMeasure(ElementType::MEASURE, m);
      score->endCmd();

      QVERIFY(saveCompareScore(score, "measure-1.mscx", DIR + "measure-1-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   insertMeasureBegin
//---------------------------------------------------------

void TestMeasure::insertMeasureBegin()
      {
      MasterScore* score = readScore(DIR + "measure-1.mscx");

      score->startCmd();
      Measure* m = score->firstMeasure();
      score->insertMeasure(ElementType::MEASURE, m);
      score->endCmd();
      QVERIFY(saveCompareScore(score, "measure-2.mscx", DIR + "measure-2-ref.mscx"));

      delete score;
      }

//---------------------------------------------------------
///   insertMeasureEnd
//---------------------------------------------------------

void TestMeasure::insertMeasureEnd()
      {
      MasterScore* score = readScore(DIR + "measure-1.mscx");

      score->startCmd();
      score->insertMeasure(ElementType::MEASURE, 0);
      score->endCmd();

      QVERIFY(saveCompareScore(score, "measure-3.mscx", DIR + "measure-3-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   insertBfClefChange
//---------------------------------------------------------

void TestMeasure::insertBfClefChange()
      {
      MasterScore* score = readScore(DIR + "measure-insert_bf_clef.mscx");
      // 4th measure
      Measure* m = score->firstMeasure()->nextMeasure();
      m = m->nextMeasure()->nextMeasure();
      score->startCmd();
      score->insertMeasure(ElementType::MEASURE, m);
      score->endCmd();
      QVERIFY(score->checkClefs());
      QVERIFY(saveCompareScore(score, "measure-insert_bf_clef.mscx", DIR + "measure-insert_bf_clef-ref.mscx"));
      score->undoRedo(true, 0);
      QVERIFY(score->checkClefs());
      QVERIFY(saveCompareScore(score, "measure-insert_bf_clef_undo.mscx", DIR + "measure-insert_bf_clef.mscx"));
      m = score->firstMeasure()->nextMeasure()->nextMeasure()->nextMeasure()->nextMeasure()->nextMeasure();
      score->startCmd();
      score->insertMeasure(ElementType::MEASURE, m);
      score->endCmd();
      QVERIFY(score->checkClefs());
      QVERIFY(saveCompareScore(score, "measure-insert_bf_clef-2.mscx", DIR + "measure-insert_bf_clef-2-ref.mscx"));
      score->undoRedo(true, 0);
      QVERIFY(score->checkClefs());
      QVERIFY(saveCompareScore(score, "measure-insert_bf_clef_undo.mscx", DIR + "measure-insert_bf_clef.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   insertBfKeyChange
//---------------------------------------------------------

void TestMeasure::insertBfKeyChange()
      {
      MasterScore* score = readScore(DIR + "measure-insert_bf_key.mscx");
      // 4th measure
      Measure* m = score->firstMeasure()->nextMeasure();
      m = m->nextMeasure()->nextMeasure();
      score->startCmd();
      score->insertMeasure(ElementType::MEASURE, m);
      score->endCmd();
      QVERIFY(score->checkKeys());
      QVERIFY(saveCompareScore(score, "measure-insert_bf_key.mscx", DIR + "measure-insert_bf_key-ref.mscx"));
      score->undoRedo(true, 0);
      QVERIFY(score->checkKeys());
      QVERIFY(saveCompareScore(score, "measure-insert_bf_key_undo.mscx", DIR + "measure-insert_bf_key.mscx"));
      m = score->firstMeasure()->nextMeasure()->nextMeasure()->nextMeasure()->nextMeasure()->nextMeasure();
      score->startCmd();
      score->insertMeasure(ElementType::MEASURE, m);
      score->endCmd();
      QVERIFY(score->checkKeys());
      QVERIFY(saveCompareScore(score, "measure-insert_bf_key-2.mscx", DIR + "measure-insert_bf_key-2-ref.mscx"));
      score->undoRedo(true, 0);
      QVERIFY(score->checkKeys());
      QVERIFY(saveCompareScore(score, "measure-insert_bf_key_undo.mscx", DIR + "measure-insert_bf_key.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   minWidth
//---------------------------------------------------------

#if 0
void TestMeasure::minWidth()
      {
      MasterScore* score = readScore(DIR + "measure-2.mscx");

      int n = score->systems().size();
      int measuresSystem[n];
      for (int i = 0; i < n; ++i)
            measuresSystem[i] = score->systems().at(i)->measures().size();

      score->doLayout();

      Measure* m1 = score->systems().at(1)->lastMeasure();
      Measure* m2 = score->systems().at(2)->firstMeasure();
      qreal mw1   = m1->minWidth1();
      qreal mw2   = m2->minWidth1();

      score->doLayout();

      QCOMPARE(mw1, m1->minWidth1());
      QCOMPARE(mw2, m2->minWidth1());

      // after second layout nothing should be changed:
      for (int i = 0; i < n; ++i) {
            QCOMPARE(measuresSystem[i], int(score->systems().at(i)->measures().size()));
            }
      }
#endif

//---------------------------------------------------------
///   spanner_a
//
//  +----spanner--------+
//    +---add---
//
//---------------------------------------------------------

void TestMeasure::spanner_a()
      {
      MasterScore* score = readScore(DIR + "measure-3.mscx");

      Measure* m = score->firstMeasure()->nextMeasure();
      score->startCmd();
      score->insertMeasure(ElementType::MEASURE, m);
      score->endCmd();
      QVERIFY(saveCompareScore(score, "measure-4.mscx", DIR + "measure-4-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   spanner_b
//
//       +----spanner--------
//  +---add---
//
//---------------------------------------------------------

void TestMeasure::spanner_b()
      {
      MasterScore* score = readScore(DIR + "measure-4.mscx");

      Measure* m = score->firstMeasure();
      score->startCmd();
      score->insertMeasure(ElementType::MEASURE, m);
      score->endCmd();
      QVERIFY(saveCompareScore(score, "measure-5.mscx", DIR + "measure-5-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   spanner_A
//
//  +----remove---+ +---spanner---+
//
//---------------------------------------------------------

void TestMeasure::spanner_A()
      {
      MasterScore* score = readScore(DIR + "measure-6.mscx");

      score->select(score->firstMeasure());
      score->startCmd();
      score->localTimeDelete();
      score->endCmd();
      score->doLayout();
      QVERIFY(saveCompareScore(score, "measure-6.mscx", DIR + "measure-6-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   spanner_B
//
//  +----spanner--------+
//    +---remove---+
//
//---------------------------------------------------------

void TestMeasure::spanner_B()
      {
      MasterScore* score = readScore(DIR + "measure-7.mscx");

      Measure* m = score->firstMeasure()->nextMeasure();
      score->select(m);
      score->startCmd();
      score->localTimeDelete();
      score->endCmd();

      QVERIFY(saveCompareScore(score, "measure-7.mscx", DIR + "measure-7-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   spanner_C
//
//    +---spanner---+
//  +----remove--------+
//
//---------------------------------------------------------

void TestMeasure::spanner_C()
      {
      MasterScore* score = readScore(DIR + "measure-8.mscx");

      Measure* m = score->firstMeasure()->nextMeasure();
      score->select(m);
      score->startCmd();
      score->localTimeDelete();
      score->endCmd();

      QVERIFY(saveCompareScore(score, "measure-8.mscx", DIR + "measure-8-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   spanner_D
//
//       +----spanner--------+
//  +---remove---+
//
//---------------------------------------------------------

void TestMeasure::spanner_D()
      {
      MasterScore* score = readScore(DIR + "measure-9.mscx");

      Measure* m = score->firstMeasure()->nextMeasure();
      score->select(m);

      score->startCmd();
      score->localTimeDelete();
      score->endCmd();

      QVERIFY(saveCompareScore(score, "measure-9.mscx", DIR + "measure-9-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
//    deleteLast
//---------------------------------------------------------

void TestMeasure::deleteLast()
      {
      MasterScore* score = readScore(DIR + "measure-10.mscx");

      Measure* m = score->lastMeasure();
      score->select(m);

      score->startCmd();
      score->localTimeDelete();
      score->endCmd();

      QVERIFY(saveCompareScore(score, "measure-10.mscx", DIR + "measure-10-ref.mscx"));
      delete score;
      }


//---------------------------------------------------------
///   gaps
//
//    delete rests and check reorganization of lengths
//
//---------------------------------------------------------

void TestMeasure::gap()
      {
      MasterScore* score = readScore(DIR + "gaps.mscx");
      Element* tst       = 0;

      //Select and delete third quarter rest in first Measure (voice 2)
      score->startCmd();
      Measure* m  = score->firstMeasure();
      Segment* s  = m->undoGetSegment(SegmentType::ChordRest, Fraction::fromTicks(960));
      Element* el = s->element(1);
      score->select(el);
      score->cmdDeleteSelection();
      score->endCmd();

      tst = s->element(1);
      Q_ASSERT(tst);

      QVERIFY(tst->isRest() && toRest(tst)->isGap() /*&& toRest(tst)->durationType() == TDuration::DurationType::V_QUARTER*/);

      //Select and delete second quarter rest in third Measure (voice 4)
      score->startCmd();
      m  = m->nextMeasure()->nextMeasure();
      s  = m->undoGetSegment(SegmentType::ChordRest, Fraction::fromTicks(4320));
      el = s->element(3);
      score->select(el);
      score->cmdDeleteSelection();
      score->endCmd();

      tst = s->element(3);
      Q_ASSERT(tst);

      QVERIFY(tst->isRest() && toRest(tst)->isGap() /*&& toRest(tst)->durationType() == TDuration::DurationType::V_QUARTER*/);

      //Select and delete first quarter rest in third Measure (voice 4)
      score->startCmd();
      s  = m->undoGetSegment(SegmentType::ChordRest, Fraction::fromTicks(3840));
      el = s->element(3);
      score->select(el);
      score->cmdDeleteSelection();
      score->endCmd();

      tst = s->element(3);
      Q_ASSERT(tst);

      QVERIFY(tst->isRest() && toRest(tst)->isGap()
         && toRest(tst)->actualTicks() == Fraction::fromTicks(960)
         /*&& toRest(tst)->durationType() == TDuration::DurationType::V_HALF*/
         );


      delete score;
      }

//---------------------------------------------------------
///   checkMeasure
//
//    import a Score with gaps in excerpt and
//
//---------------------------------------------------------

void TestMeasure::checkMeasure()
      {
      MasterScore* score = readScore(DIR + "checkMeasure.mscx");
      Element* tst       = 0;
      Measure* m         = score->firstMeasure()->nextMeasure();

      Segment* s = m->undoGetSegment(SegmentType::ChordRest, Fraction::fromTicks(2880));
      tst = s->element(1);
      Q_ASSERT(tst);

      QVERIFY(tst->isRest() && toRest(tst)->isGap() && toRest(tst)->actualTicks() == Fraction::fromTicks(480)
         /*&& toRest(tst)->durationType() == TDuration::DurationType::V_HALF*/
         );

      m = m->nextMeasure();
//      s = m->undoGetSegment(SegmentType::ChordRest, Fraction::fromTicks(3840));
//      tst = s->element(2);
//      Q_ASSERT(tst);

//      QVERIFY(tst->isRest() && toRest(tst)->isGap() && toRest(tst)->actualTicks() == 480/*&& toRest(tst)->durationType() == TDuration::DurationType::V_HALF*/);

      m = m->nextMeasure();
      s = m->undoGetSegment(SegmentType::ChordRest, Fraction::fromTicks(6240));
      tst = s->element(1);
      Q_ASSERT(tst);

      QVERIFY(tst->isRest() && toRest(tst)->isGap() && toRest(tst)->actualTicks() == Fraction::fromTicks(120)
         /*&& toRest(tst)->durationType() == TDuration::DurationType::V_HALF*/
         );

      s = m->undoGetSegment(SegmentType::ChordRest, Fraction::fromTicks(6480));
      tst = s->element(1);
      Q_ASSERT(tst);

      QVERIFY(tst->isRest() && toRest(tst)->isGap() && toRest(tst)->actualTicks() == Fraction::fromTicks(120)
         /*&& toRest(tst)->durationType() == TDuration::DurationType::V_HALF*/
         );

      delete score;
      }

//---------------------------------------------------------
///   undoDelInitialVBox_269919
///    1. Delete first VBox
///    2. Change duration of first chordrest
///    3. Undo to restore first chordrest
///    4. Undo to restore initial VBox results in assert failure crash
//---------------------------------------------------------

void TestMeasure::undoDelInitialVBox_269919()
      {
      MasterScore* score = readScore(DIR + "undoDelInitialVBox_269919.mscx");

      // 1. delete initial VBox
      score->startCmd();
      MeasureBase* initialVBox = score->measure(0);
      score->select(initialVBox);
      score->cmdDeleteSelection();
      score->endCmd();

      // 2. change duration of first chordrest
      score->startCmd();
      Measure* m = score->firstMeasure();
      ChordRest* cr = m->findChordRest(Fraction(0,1), 0);
      Fraction quarter(4, 1);
      score->changeCRlen(cr, quarter);
      score->endCmd();

      // 3. Undo to restore first chordrest
      score->undoRedo(true, 0);

      // 4. Undo to restore initial VBox resulted in assert failure crash
      score->undoRedo(true, 0);

      QVERIFY(saveCompareScore(score, "undoDelInitialVBox_269919.mscx", DIR + "undoDelInitialVBox_269919-ref.mscx"));
      delete score;
      }


QTEST_MAIN(TestMeasure)

#include "tst_measure.moc"

