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
      score->insertMeasure(MEASURE, m);
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
      score->insertMeasure(MEASURE, m);
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
      score->insertMeasure(MEASURE, 0);
      score->endCmd();
      QVERIFY(saveCompareScore(score, "measure1-3.mscx", DIR + "measure1-3o.mscx"));
      delete score;
      }

QTEST_MAIN(TestMeasure)

#include "tst_measure.moc"

