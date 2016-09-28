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
#include "mtest/testutils.h"
#include "libmscore/clef.h"
#include "libmscore/measure.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"

#define DIR QString("libmscore/clef_courtesy/")

using namespace Ms;

//---------------------------------------------------------
//   TestClefCourtesy
//---------------------------------------------------------

class TestClefCourtesy : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void clef_courtesy01();
      void clef_courtesy02();
      void clef_courtesy03();
      void clef_courtesy_78196();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestClefCourtesy::initTestCase()
      {
      initMTest();
      }

static Measure* getMeasure(Score* score, int idx)
      {
      Measure* m = score->firstMeasure();
      for (int i = 0; i < idx-1; i++)
            m = m->nextMeasure();
      return m;
      }

static void dropClef(Measure* m, ClefType t)
      {
      Clef* clef = new Clef(m->score()); // create a new element, as Measure::drop() will eventually delete it
      clef->setClefType(t);
      DropData dropData;
      dropData.pos = m->pagePos();
      dropData.element = clef;
      m->score()->startCmd();
      m->drop(dropData);
      m->score()->endCmd();
      }

//---------------------------------------------------------
//   clef_courtesy01
//    add two clefs mid-score at the begining of systems and look for courtesy clefs
//    the first should be there, the second should not, as it is after a section break
//---------------------------------------------------------

void TestClefCourtesy::clef_courtesy01()
      {
      MasterScore* score = readScore(DIR + "clef_courtesy01.mscx");

      // drop G1 clef to 4th measure
      Measure* m1 = getMeasure(score, 4);
      dropClef(m1, ClefType::G8_VA);

      // drop G clef to 7th measure
      Measure* m2 = getMeasure(score, 7);
      dropClef(m2, ClefType::G);


      // check the required courtesy clef is there and it is shown
      Clef*    clefCourt = 0;
      Measure* m = m1->prevMeasure();
      Segment* seg = m->findSegment(Segment::Type::Clef, m1->tick());
      QVERIFY2(seg, "No SegClef in measure 3.");
      clefCourt = static_cast<Clef*>(seg->element(0));
      QVERIFY2(clefCourt, "No courtesy clef element in measure 3.");
      QVERIFY2(clefCourt->bbox().width() > 0, "Courtesy clef in measure 3 is hidden.");

      // check the not required courtesy clef element is there but it is not shown
      clefCourt = nullptr;
      m   = m2->prevMeasure();
      seg = m->findSegment(Segment::Type::Clef, m2->tick());
      QVERIFY2(seg, "No SegClef in measure 6.");
      clefCourt = static_cast<Clef*>(seg->element(0));
      QVERIFY2(clefCourt, "No courtesy clef element in measure 6.");
      QVERIFY2(clefCourt->bbox().width() == 0, "Courtesy clef in measure 3 is NOT hidden.");

      QVERIFY(saveCompareScore(score, "clef_courtesy01.mscx", DIR + "clef_courtesy01-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   clef_courtesy02
//    add two clefs mid-score at the begining of systems and look for courtesy clefs
//    neither should be there, as courtesy clefs are turned off
//---------------------------------------------------------

void TestClefCourtesy::clef_courtesy02()
      {
      MasterScore* score = readScore(DIR + "clef_courtesy02.mscx");

      // 'go' to 4th measure
      Measure* m1 = score->firstMeasure();
      for (int i=0; i < 3; i++)
            m1 = m1->nextMeasure();
      // make a clef-drop object and drop it to the measure
      Clef* clef = new Clef(score); // create a new element, as Measure::drop() will eventually delete it
      clef->setClefType(ClefType::G8_VA);
      DropData dropData;
      dropData.pos = m1->pagePos();
      dropData.element = clef;
      m1->drop(dropData);

      // 'go' to 7th measure
      Measure* m2 = m1;
      for (int i=0; i < 3; i++)
            m2 = m2->nextMeasure();
      // make a clef-drop object and drop it to the measure
      clef = new Clef(score); // create a new element, as Measure::drop() will eventually delete it
      clef->setClefType(ClefType::G);
      dropData.pos = m2->pagePos();
      dropData.element = clef;
      m2->drop(dropData);
      score->doLayout();

      // check both clef elements are there, but none is shown
      Clef*    clefCourt = nullptr;
      Measure* m = m1->prevMeasure();
      Segment* seg = m->findSegment(Segment::Type::Clef, m1->tick());
      QVERIFY2(seg != nullptr, "No SegClef in measure 3.");
      clefCourt = static_cast<Clef*>(seg->element(0));
      QVERIFY2(clefCourt != nullptr, "No courtesy clef element in measure 3.");
      QVERIFY2(clefCourt->bbox().width() == 0, "Courtesy clef in measure 3 is NOT hidden.");

      clefCourt = nullptr;
      m = m2->prevMeasure();
      seg = m->findSegment(Segment::Type::Clef, m2->tick());
      QVERIFY2(seg != nullptr, "No SegClef in measure 6.");
      clefCourt = static_cast<Clef*>(seg->element(0));
      QVERIFY2(clefCourt != nullptr, "No courtesy clef element in measure 6.");
      QVERIFY2(clefCourt->bbox().width() == 0, "Courtesy clef in measure 3 is NOT hidden.");

      QVERIFY(saveCompareScore(score, "clef_courtesy02.mscx", DIR + "clef_courtesy02-ref.mscx"));
      delete score;
      }


//---------------------------------------------------------
//   clef_courtesy03
//    tests issue #76006 "if insert clef to measure after single-measure system with section break, then should not display courtesy clef"
//    adds a clef on meas 2, which occurs after a single-measure section
//    the added clef should not be visible, since it is after a section break
//---------------------------------------------------------

void TestClefCourtesy::clef_courtesy03()
      {
      MasterScore* score = readScore(DIR + "clef_courtesy03.mscx");

      Measure* m1 = score->firstMeasure();
      Measure* m2 = m1->nextMeasure();

      // make a clef-drop object and drop it to the 2nd measure
      Clef* clef = new Clef(score); // create a new element, as Measure::drop() will eventually delete it
      clef->setClefType(ClefType::G8_VA);
      DropData dropData;
      dropData.pos = m2->pagePos();
      dropData.element = clef;
      m2->drop(dropData);
      score->doLayout();

      // verify the not required courtesy clef element is on end of m1 but is not shown
      Clef *clefCourt = nullptr;
      Segment *seg = m1->findSegment(Segment::Type::Clef, m2->tick());
      QVERIFY2(seg != nullptr, "No SegClef in measure 1.");
      clefCourt = static_cast<Clef*>(seg->element(0));
      QVERIFY2(clefCourt != nullptr, "No courtesy clef element in measure 1.");
      QVERIFY2(clefCourt->bbox().width() == 0, "Courtesy clef in measure 1 is NOT hidden.");

      QVERIFY(saveCompareScore(score, "clef_courtesy03.mscx", DIR + "clef_courtesy03-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   clef_courtesy_78196
//    input score has section breaks on non-measure MeasureBase objects.
//    should not display courtesy clefs at the end of final measure of each section (meas 2, 4, & 6), even if section break occurs on subsequent non-measure frame.
//---------------------------------------------------------

void TestClefCourtesy::clef_courtesy_78196()
      {
      MasterScore* score = readScore(DIR + "clef_courtesy_78196.mscx");

      Measure* m1 = score->firstMeasure();
      Measure* m2 = m1->nextMeasure();
      Measure* m3 = m2->nextMeasure();
      Measure* m4 = m3->nextMeasure();
      Measure* m5 = m4->nextMeasure();
      Measure* m6 = m5->nextMeasure();
      Measure* m7 = m6->nextMeasure();

      // check both clef elements are there, but none is shown
      Clef*    clefCourt = nullptr;
      Segment* seg = nullptr;

      // verify clef exists in segment of final tick of m2, but that it is not visible
      seg = m2->findSegment(Segment::Type::Clef, m3->tick());
      QVERIFY2(seg != nullptr, "No SegClef at end of measure 2.");
      clefCourt = static_cast<Clef*>(seg->element(0));
      QVERIFY2(clefCourt != nullptr, "No courtesy clef at end of measure 2.");
      QVERIFY2(clefCourt->bbox().width() == 0, "Courtesy clef at end of measure 2 is NOT hidden.");

      // verify clef exists in segment of final tick of m4, but that it is not visible
      seg = m4->findSegment(Segment::Type::Clef, m5->tick());
      QVERIFY2(seg != nullptr, "No SegClef at end of measure 4.");
      clefCourt = static_cast<Clef*>(seg->element(0));
      QVERIFY2(clefCourt != nullptr, "No courtesy clef at end of measure 4.");
      QVERIFY2(clefCourt->bbox().width() == 0, "Courtesy clef at end of measure 4 is NOT hidden.");

      // verify clef exists in segment of final tick of m6, but that it is not visible
      seg = m6->findSegment(Segment::Type::Clef, m7->tick());
      QVERIFY2(seg != nullptr, "No SegClef at end of measure 6.");
      clefCourt = static_cast<Clef*>(seg->element(0));
      QVERIFY2(clefCourt != nullptr, "No courtesy clef at end of measure 6.");
      QVERIFY2(clefCourt->bbox().width() == 0, "Courtesy clef at end of measure 6 is NOT hidden.");
      }

QTEST_MAIN(TestClefCourtesy)
#include "tst_clef_courtesy.moc"

