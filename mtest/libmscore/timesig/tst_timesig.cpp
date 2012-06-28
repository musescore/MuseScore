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

//---------------------------------------------------------
//   TestTimesig
//---------------------------------------------------------

class TestTimesig : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase() { initMTest(); }
      void timesig1();
      };

//---------------------------------------------------------
//   timesig1
//---------------------------------------------------------

void TestTimesig::timesig1()
      {
      Score* score = readScore(DIR + "timesig1.mscx");
      QVERIFY(score);
      Measure* m = score->firstMeasure()->nextMeasure();
      TimeSig* ts = new TimeSig(score, 3, 4);

      score->cmdAddTimeSig(m, 0, ts);
      score->doLayout();

      QVERIFY(saveCompareScore(score, "timesig1.mscx", DIR + "timesig1-ref.mscx"));
      delete score;
      }

QTEST_MAIN(TestTimesig)
#include "tst_timesig.moc"

