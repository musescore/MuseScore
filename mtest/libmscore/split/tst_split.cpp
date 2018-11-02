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
      void split183846()
            {
            split("split183846-irregular-qn-qn-wn.mscx",          "split183846-irregular-qn-qn-wn-ref.mscx", 1);
            split("split183846-irregular-wn-wn.mscx",             "split183846-irregular-wn-wn-ref.mscx", 1);
            split("split183846-irregular-wn-wr-wn-hr-qr.mscx",    "split183846-irregular-wn-wr-wn-hr-qr-ref.mscx", 2);
            split("split183846-irregular-wr-wn-wr-hn-qn.mscx",    "split183846-irregular-wr-wn-wr-hn-qn-ref.mscx", 3);
            split("split183846-irregular-hn-hn-qn-qn-hn-hn.mscx", "split183846-irregular-hn-hn-qn-qn-hn-hn-ref.mscx", 5);
            split("split183846-irregular-verylong.mscx",          "split183846-irregular-verylong-ref.mscx", 7);
            }
       void split184061()
            {
            split("split184061-no-tie.mscx", "split184061-no-tie-ref.mscx", 3);   // splitting on 11/16th the way though measure, but voice 2 has whole note which can't be divided into two durations
            split("split184061-keep-tie.mscx", "split184061-keep-tie-ref.mscx", 3); // same, but this this the split-up whole note has a tie to the next measure...
            split("split184061-keep-tie-before-break-voice-4.mscx", "split184061-keep-tie-before-break-voice-4-ref.mscx", 2); // splitting 1/64th after middle of measure...voice 4 already has a tie that need to be preserved after splitting, and voice 2 has whole note that must be split up with triple-dotted
            split("split184061-other-inst-only-one-tie.mscx", "split184061-other-inst-only-one-tie-ref.mscx", 2); // only the one tied note of the chord in the flute should still be tied over
            }
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
      Segment* s = m->first(SegmentType::ChordRest);
      s = s->next(SegmentType::ChordRest);
      s = s->next(SegmentType::ChordRest);
      ChordRest* cr = toChordRest(s->element(0));

      score->cmdSplitMeasure(cr);

      QVERIFY(saveCompareScore(score, f1, DIR + ref));
      delete score;
      }

void TestSplit::split(const char* f1, const char* ref, int index)
      {
      MasterScore* score = readScore(DIR + f1);
      QVERIFY(score);
      Measure* m = score->firstMeasure();
      Segment* s = m->first(SegmentType::ChordRest);
      for (int i = 0; i < index; ++i)
            s = s->next1(SegmentType::ChordRest);
      ChordRest* cr = toChordRest(s->element(0));

      score->cmdSplitMeasure(cr);

      QVERIFY(saveCompareScore(score, f1, DIR + ref));
      delete score;
      }

QTEST_MAIN(TestSplit)
#include "tst_split.moc"

