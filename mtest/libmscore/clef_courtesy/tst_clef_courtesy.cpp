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
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestClefCourtesy::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   clef_courtesy01
//    add two clefs mid-score at the begining of systems and look for courtesy clefs
//    the first should be there, the second should not, as it is after a section break
//---------------------------------------------------------

void TestClefCourtesy::clef_courtesy01()
      {
      Score* score = readScore(DIR + "clef_courtesy01.mscx");
      score->doLayout();

      // 'go' to 4th measure
      Measure* m1 = score->firstMeasure();
      for (int i=0; i < 3; i++)
            m1 = m1->nextMeasure();
      // make a clef-drop object and drop it to the measure
      Clef* clef = new Clef(score); // create a new element, as Measure::drop() will eventually delete it
      clef->setClefType(ClefType::G1);
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

      // check the required courtesy clef is there and it is shown
      Clef*    clefCourt = nullptr;
      Measure* m = m1->prevMeasure();
      Segment* seg = m->findSegment(Segment::Type::Clef, m1->tick());
      QVERIFY2(seg != nullptr, "No SegClef in measure 3.");
      clefCourt = static_cast<Clef*>(seg->element(0));
      QVERIFY2(clefCourt != nullptr, "No courtesy clef element in measure 3.");
      QVERIFY2(clefCourt->bbox().width() > 0, "Courtesy clef in measure 3 is hidden.");

      // check the not required courtesy clef element is there but it is not shown
      clefCourt = nullptr;
      m = m2->prevMeasure();
      seg = m->findSegment(Segment::Type::Clef, m2->tick());
      QVERIFY2(seg != nullptr, "No SegClef in measure 6.");
      clefCourt = static_cast<Clef*>(seg->element(0));
      QVERIFY2(clefCourt != nullptr, "No courtesy clef element in measure 6.");
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
      Score* score = readScore(DIR + "clef_courtesy02.mscx");
      score->doLayout();

      // 'go' to 4th measure
      Measure* m1 = score->firstMeasure();
      for (int i=0; i < 3; i++)
            m1 = m1->nextMeasure();
      // make a clef-drop object and drop it to the measure
      Clef* clef = new Clef(score); // create a new element, as Measure::drop() will eventually delete it
      clef->setClefType(ClefType::G1);
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

QTEST_MAIN(TestClefCourtesy)
#include "tst_clef_courtesy.moc"

