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

using namespace Ms;

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
      void split01() { split("split01.mscx", "split01-ref.mscx"); }
      void split02() { split("split02.mscx", "split02-ref.mscx"); }
      void split03() { split("split03.mscx", "split03-ref.mscx"); }
      void split04() { split("split04.mscx", "split04-ref.mscx"); }
      void split05() { split("split05.mscx", "split05-ref.mscx"); }
      void split06() { split("split06.mscx", "split06-ref.mscx", 6); }
      void split07() { split("split07.mscx", "split07-ref.mscx"); }
      void split08() { split("split08.mscx", "split08-ref.mscx"); }
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
      MasterScore* score = readScore(DIR + f1);
      QVERIFY(score);
      Measure* m = score->firstMeasure();
      Segment* s = m->first(Segment::Type::ChordRest);
      s = s->next(Segment::Type::ChordRest);
      s = s->next(Segment::Type::ChordRest);
      ChordRest* cr = static_cast<ChordRest*>(s->element(0));

      score->cmdSplitMeasure(cr);

      QVERIFY(saveCompareScore(score, f1, DIR + ref));
      delete score;
      }

void TestSplit::split(const char* f1, const char* ref, int index)
      {
      MasterScore* score = readScore(DIR + f1);
      QVERIFY(score);
      Measure* m = score->firstMeasure();
      Segment* s = m->first(Segment::Type::ChordRest);
      for (int i = 0; i < index; ++i)
            s = s->next1(Segment::Type::ChordRest);
      ChordRest* cr = static_cast<ChordRest*>(s->element(0));

      score->cmdSplitMeasure(cr);

      QVERIFY(saveCompareScore(score, f1, DIR + ref));
      delete score;
      }

QTEST_MAIN(TestSplit)
#include "tst_split.moc"

