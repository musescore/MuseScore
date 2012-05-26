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

#define DIR QString("libmscore/join/")

//---------------------------------------------------------
//   TestJoin
//---------------------------------------------------------

class TestJoin : public QObject, public MTest
      {
      Q_OBJECT

      void join(const char* p1, const char* p2);
      void join(const char* p1, const char* p2, int);

   private slots:
      void initTestCase();
      void join1() { join("join.mscx",  "join-ref.mscx"); }
      void join2() { join("join1.mscx", "join1-ref.mscx"); }
      void join3() { join("join2.mscx", "join2-ref.mscx"); }
      void join4() { join("join3.mscx", "join3-ref.mscx"); }
      void join5() { join("join4.mscx", "join4-ref.mscx"); }
      void join6() { join("join5.mscx", "join5-ref.mscx", 1); }
      void join7() { join("join6.mscx", "join6-ref.mscx"); }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestJoin::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   beam
//---------------------------------------------------------

void TestJoin::join(const char* p1, const char* p2)
      {
      Score* score = readScore(DIR + p1);
      QVERIFY(score);
      Measure* m1 = score->firstMeasure();
      Measure* m2 = m1->nextMeasure();

      QVERIFY(m1 != 0);
      QVERIFY(m2 != 0);
      QVERIFY(m1 != m2);

      score->cmdJoinMeasure(m1, m2);

      QVERIFY(saveCompareScore(score, p1, DIR + p2));
      delete score;
      }

void TestJoin::join(const char* p1, const char* p2, int index)
      {
      Score* score = readScore(DIR + p1);
      QVERIFY(score);
      Measure* m1 = score->firstMeasure();
      for (int i = 0; i < index; ++i)
            m1 = m1->nextMeasure();
      Measure* m2 = m1->nextMeasure();

      QVERIFY(m1 != 0);
      QVERIFY(m2 != 0);
      QVERIFY(m1 != m2);

      score->cmdJoinMeasure(m1, m2);

      QVERIFY(saveCompareScore(score, p1, DIR + p2));
      delete score;
      }

QTEST_MAIN(TestJoin)
#include "tst_join.moc"

