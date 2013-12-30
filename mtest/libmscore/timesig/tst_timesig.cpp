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
#include "mtest/testutils.h"
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/timesig.h"

#define DIR QString("libmscore/timesig/")

using namespace Ms;

//---------------------------------------------------------
//   TestTimesig
//---------------------------------------------------------

class TestTimesig : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase() { initMTest(); }
      void timesig01();
      void timesig02();
      };

//---------------------------------------------------------
///   timesig01
///   add a 3/4 time signature in the second measure
//---------------------------------------------------------

void TestTimesig::timesig01()
      {
      Score* score = readScore(DIR + "timesig01.mscx");
      QVERIFY(score);
      Measure* m = score->firstMeasure()->nextMeasure();
      TimeSig* ts = new TimeSig(score);
      ts->setSig(Fraction(3, 4), TSIG_NORMAL);

      score->cmdAddTimeSig(m, 0, ts, false);
      score->doLayout();

      QVERIFY(saveCompareScore(score, "timesig01.mscx", DIR + "timesig01-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   timesig02
///   Attempt to change a 4/4 measure containing a triplet of minims to a 3/4 time signature
///   The attempt should fail, the score left unchanged
//---------------------------------------------------------

void TestTimesig::timesig02()
      {
      Score* score = readScore(DIR + "timesig-02.mscx");
      QVERIFY(score);
      Measure* m = score->firstMeasure();
      TimeSig* ts = new TimeSig(score);
      ts->setSig(Fraction(3, 4), TSIG_NORMAL);

      score->cmdAddTimeSig(m, 0, ts, false);
      score->doLayout();

      QVERIFY(saveCompareScore(score, "timesig-02a.mscx", DIR + "timesig-02-ref.mscx"));
      delete score;

      }

QTEST_MAIN(TestTimesig)
#include "tst_timesig.moc"

