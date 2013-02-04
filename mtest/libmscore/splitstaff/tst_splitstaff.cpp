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
#include "libmscore/segment.h"
#include "libmscore/chordrest.h"

#define DIR QString("libmscore/splitstaff/")

//---------------------------------------------------------
//   TestSplitStaff
//---------------------------------------------------------

class TestSplitStaff : public QObject, public MTest
      {
      Q_OBJECT

      void splitstaff(int);

   private slots:
      void initTestCase();
      void splitstaff01() { splitstaff(1); }
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

void TestSplitStaff::splitstaff(int idx)
      {
      Score* score = readScore(DIR + QString("splitstaff0%1.mscx").arg(idx));
      score->doLayout();
      score->splitStaff(0, 60);
      score->doLayout();

      QVERIFY(saveCompareScore(score, QString("splitstaff0%1.mscx").arg(idx),
         DIR + QString("splitstaff0%1-ref.mscx").arg(idx)));
      delete score;
      }

QTEST_MAIN(TestSplitStaff)
#include "tst_splitstaff.moc"

