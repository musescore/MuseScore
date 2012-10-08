//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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
#include "mtest/testutils.h"

#define DIR QString("libmscore/measure/")

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
      void minWidth();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestMeasure::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   insertMeasureMiddle
//---------------------------------------------------------

void TestMeasure::insertMeasureMiddle()
      {
      Score* score = readScore(DIR + "measure1.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m = score->firstMeasure()->nextMeasure();
      score->startCmd();
      score->insertMeasure(Element::MEASURE, m);
      score->endCmd();
      QVERIFY(saveCompareScore(score, "measure1-1.mscx", DIR + "measure1-1o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   insertMeasureBegin
//---------------------------------------------------------

void TestMeasure::insertMeasureBegin()
      {
      Score* score = readScore(DIR + "measure1.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m = score->firstMeasure();
      score->startCmd();
      score->insertMeasure(Element::MEASURE, m);
      score->endCmd();
      QVERIFY(saveCompareScore(score, "measure1-2.mscx", DIR + "measure1-2o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   insertMeasureEnd
//---------------------------------------------------------

void TestMeasure::insertMeasureEnd()
      {
      Score* score = readScore(DIR + "measure1.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      score->startCmd();
      score->insertMeasure(Element::MEASURE, 0);
      score->endCmd();
      QVERIFY(saveCompareScore(score, "measure1-3.mscx", DIR + "measure1-3o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   minWidth
//---------------------------------------------------------

void TestMeasure::minWidth()
      {
      Score* score = readScore(DIR + "measure2.mscx");
      score->doLayout();
      int n = score->systems()->size();
      int measuresSystem[n];
      for (int i = 0; i < n; ++i)
            measuresSystem[i] = score->systems()->at(i)->measures().size();

      Measure* m1 = score->systems()->at(1)->lastMeasure();
      Measure* m2 = score->systems()->at(2)->firstMeasure();
      qreal mw1 = m1->minWidth1();
      qreal mw2 = m2->minWidth1();

      score->doLayout();

      QCOMPARE(mw1, m1->minWidth1());
      QCOMPARE(mw2, m2->minWidth1());

      // after second layout nothing should be changed:
      for (int i = 0; i < n; ++i)
            QCOMPARE(measuresSystem[i], score->systems()->at(i)->measures().size());
      }

QTEST_MAIN(TestMeasure)

#include "tst_measure.moc"

