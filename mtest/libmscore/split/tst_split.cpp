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

#define DIR QString("libmscore/split/")

//---------------------------------------------------------
//   TestSplit
//---------------------------------------------------------

class TestSplit : public QObject, public MTest
      {
      Q_OBJECT

      void split(const char* file, const char* reference);
      void split(const char* file, const char* reference, int index);

   private slots:
      void initTestCase();
      void split1() { split("split.mscx",  "split-ref.mscx"); }
      void split2() { split("split1.mscx", "split1-ref.mscx"); }
      void split3() { split("split2.mscx", "split2-ref.mscx"); }
      void split4() { split("split3.mscx", "split3-ref.mscx"); }
      void split5() { split("split4.mscx", "split4-ref.mscx"); }
      void split6() { split("split5.mscx", "split5-ref.mscx", 6); }
      void split7() { split("split6.mscx", "split6-ref.mscx"); }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSplit::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   split
//---------------------------------------------------------

void TestSplit::split(const char* f1, const char* ref)
      {
      Score* score = readScore(DIR + f1);
      QVERIFY(score);
      Measure* m = score->firstMeasure();
      Segment* s = m->first(SegChordRest);
      s = s->next(SegChordRest);
      s = s->next(SegChordRest);
      ChordRest* cr = static_cast<ChordRest*>(s->element(0));

      score->cmdSplitMeasure(cr);

      QVERIFY(saveCompareScore(score, f1, DIR + ref));
      delete score;
      }

void TestSplit::split(const char* f1, const char* ref, int index)
      {
      Score* score = readScore(DIR + f1);
      QVERIFY(score);
      Measure* m = score->firstMeasure();
      Segment* s = m->first(SegChordRest);
      for (int i = 0; i < index; ++i)
            s = s->next1(SegChordRest);
      ChordRest* cr = static_cast<ChordRest*>(s->element(0));

      score->cmdSplitMeasure(cr);

      QVERIFY(saveCompareScore(score, f1, DIR + ref));
      delete score;
      }

QTEST_MAIN(TestSplit)
#include "tst_split.moc"

