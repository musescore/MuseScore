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

#define DIR QString("libmscore/pitchsplit/")

//---------------------------------------------------------
//   TestPitchSplit
//---------------------------------------------------------

class TestPitchSplit : public QObject, public MTest
      {
      Q_OBJECT

      void pitchsplit(int);

   private slots:
      void initTestCase();
      void pitchsplit1() { pitchsplit(1); }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestPitchSplit::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   pitchsplit
//---------------------------------------------------------

void TestPitchSplit::pitchsplit(int idx)
      {
      Score* score = readScore(DIR + QString("pitchsplit%1.mscx").arg(idx));
      score->doLayout();
      score->splitStaff(0, 60);
      score->doLayout();

      QVERIFY(saveCompareScore(score, QString("pitchsplit%1.mscx").arg(idx),
         DIR + QString("pitchsplit%1-ref.mscx").arg(idx)));
      delete score;
      }

QTEST_MAIN(TestPitchSplit)
#include "tst_pitchsplit.moc"

