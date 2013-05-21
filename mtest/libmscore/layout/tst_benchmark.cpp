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

using namespace Ms;

//---------------------------------------------------------
//   TestBechmark
//---------------------------------------------------------

class TestBenchmark : public QObject, public MTest
      {
      Q_OBJECT

      Score* score;
      void beam(const char* path);

   private slots:
      void initTestCase();
      void benchmark3();
//      void benchmark1();
//      void benchmark2();
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

void TestBenchmark::benchmark3()
      {
      QString path = root + "/" + DIR + "goldberg.mscx";
      score = new Score(mscore->baseStyle());
      score->setName(path);
      MScore::testMode = true;
      QBENCHMARK {
            score->loadMsc(path, false);
            }
      }
#if 0
void TestBenchmark::benchmark1()
      {
      score = readScore(DIR + "goldberg.mscx");
      QBENCHMARK {                        // cold run
            score->doLayout();
            }
      }

void TestBenchmark::benchmark2()
      {
      score->doLayout();
      QBENCHMARK {                        // warm run
            score->doLayout();
            }
      }
#endif

QTEST_MAIN(TestBenchmark)
#include "tst_benchmark.moc"

