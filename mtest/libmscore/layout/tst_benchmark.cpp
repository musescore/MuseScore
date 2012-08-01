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

#define DIR QString("libmscore/layout/")

//---------------------------------------------------------
//   TestBechmark
//---------------------------------------------------------

class TestBenchmark : public QObject, public MTest
      {
      Q_OBJECT

      void beam(const char* path);

   private slots:
      void initTestCase();
      void benchmark();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestBenchmark::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   benchmark
//---------------------------------------------------------

void TestBenchmark::benchmark()
      {
      Score* score = readScore(DIR + "goldberg.mscx");
      QBENCHMARK {
            score->doLayout();
            }
      }


QTEST_MAIN(TestBenchmark)
#include "tst_benchmark.moc"

