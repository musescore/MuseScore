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
#include "libmscore/chord.h"

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
      void join1(const char* p1);

   private slots:
      void initTestCase();
      void join01() { join("join01.mscx", "join01-ref.mscx"); }
      void join02() { join("join02.mscx", "join02-ref.mscx"); }
      void join03() { join("join03.mscx", "join03-ref.mscx"); }
      void join04() { join("join04.mscx", "join04-ref.mscx"); }
      void join05() { join("join05.mscx", "join05-ref.mscx"); }
      void join06() { join("join06.mscx", "join06-ref.mscx", 1); }
      void join07() { join("join07.mscx", "join07-ref.mscx"); }
      void join08() { join1("join08.mscx"); }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestJoin::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   join
//---------------------------------------------------------

void TestJoin::join(const char* p1, const char* p2)
      {
      MasterScore* score = readScore(DIR + p1);
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
      MasterScore* score = readScore(DIR + p1);
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

void TestJoin::join1(const char* p1)
      {
      MasterScore* score = readScore(DIR + p1);
      score->doLayout();
      Measure* m1 = score->firstMeasure();
      Measure* m2 = m1->nextMeasure();

      QVERIFY(m1 != 0);
      QVERIFY(m2 != 0);
      QVERIFY(m1 != m2);

      score->cmdJoinMeasure(m1, m2);

      // check if notes are still on line 6
      Segment* s = score->firstSegment(Segment::Type::ChordRest);

      for (int i = 0; i < 8; ++i) {
            Note* note = static_cast<Ms::Chord*>(s->element(0))->upNote();
            QVERIFY(note->line() == 6);
            s = s->next1(Segment::Type::ChordRest);
            }

      delete score;
      }

QTEST_MAIN(TestJoin)
#include "tst_join.moc"

