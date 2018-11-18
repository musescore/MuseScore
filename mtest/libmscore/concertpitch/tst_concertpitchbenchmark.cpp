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

#define DIR QString("libmscore/concertpitch/")

using namespace Ms;

//---------------------------------------------------------
//   TestConcertPitchBenchmark
//---------------------------------------------------------

class TestConcertPitchBenchmark : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void benchmark();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestConcertPitchBenchmark::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   benchmark
//---------------------------------------------------------

void TestConcertPitchBenchmark::benchmark()
      {
      Score* score = readScore(DIR + "concertpitchbenchmark.mscx");
      QBENCHMARK {
            // switch to concert pitch
            score->cmdConcertPitchChanged(true,true);
            score->doLayout();
            // switch back
            // TODO: this should be UNDO, but UNDO doesn't work with transpose!
            score->cmdConcertPitchChanged(false,true);
            score->doLayout();
            }
      }

QTEST_MAIN(TestConcertPitchBenchmark)
#include "tst_concertpitchbenchmark.moc"
