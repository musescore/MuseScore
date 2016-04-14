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
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/undo.h"

#define DIR QString("libmscore/clef/")

using namespace Ms;

//---------------------------------------------------------
//   TestClef
//---------------------------------------------------------

class TestClef : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void clef1();
      void clef2();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestClef::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   clef1
//    two clefs at tick position zero
//---------------------------------------------------------

void TestClef::clef1()
      {
      MasterScore* score = readScore(DIR + "clef-1.mscx");
      QVERIFY(saveCompareScore(score, "clef-1.mscx", DIR + "clef-1-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   clef2
//    change timesig -> rewrite measures ->insertTime
//---------------------------------------------------------

void TestClef::clef2()
      {
      MasterScore* score = readScore(DIR + "clef-2.mscx");
      Measure* m = score->firstMeasure();
      m = m->nextMeasure();
      m = m->nextMeasure();
      TimeSig* ts = new TimeSig(score);
      ts->setSig(2, 4);
      score->cmdAddTimeSig(m, 0, ts, false);

      score->doLayout();
      QVERIFY(saveCompareScore(score, "clef-2.mscx", DIR + "clef-2-ref.mscx"));
      delete score;
      }

QTEST_MAIN(TestClef)
#include "tst_clef.moc"

