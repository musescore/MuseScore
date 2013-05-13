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

using namespace Ms;

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
      void join01() { join("join01.mscx", "join01-ref.mscx"); }
      void join02() { join("join02.mscx", "join02-ref.mscx"); }
      void join03() { join("join03.mscx", "join03-ref.mscx"); }
      void join04() { join("join04.mscx", "join04-ref.mscx"); }
      void join05() { join("join05.mscx", "join05-ref.mscx"); }
      void join06() { join("join06.mscx", "join06-ref.mscx", 1); }
      void join07() { join("join07.mscx", "join07-ref.mscx"); }
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
      score->doLayout();
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
      score->doLayout();
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

