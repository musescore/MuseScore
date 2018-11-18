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
#include "libmscore/segment.h"
#include "libmscore/chordrest.h"

#define DIR QString("libmscore/splitstaff/")

using namespace Ms;

//---------------------------------------------------------
//   TestSplitStaff
//---------------------------------------------------------

class TestSplitStaff : public QObject, public MTest
      {
      Q_OBJECT

      void splitstaff(int, int);

   private slots:
      void initTestCase();
      void splitstaff01() { splitstaff(1, 0); } //single notes
      void splitstaff02() { splitstaff(2, 0); } //chord
      void splitstaff03() { splitstaff(3, 1); } //non-top staff
      void splitstaff04() { splitstaff(4, 0); } //slur up
      void splitstaff05() { splitstaff(5, 0); } //slur down
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSplitStaff::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
///   splitstaff
//---------------------------------------------------------

void TestSplitStaff::splitstaff(int idx, int staffIdx)
      {
      MasterScore* score = readScore(DIR + QString("splitstaff0%1.mscx").arg(idx));
      score->startCmd();
      score->splitStaff(staffIdx, 60);
      score->endCmd();

      QVERIFY(saveCompareScore(score, QString("splitstaff0%1.mscx").arg(idx),
         DIR + QString("splitstaff0%1-ref.mscx").arg(idx)));
      delete score;
      }

QTEST_MAIN(TestSplitStaff)
#include "tst_splitstaff.moc"

