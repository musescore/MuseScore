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
#include "libmscore/timesig.h"

#define DIR QString("libmscore/timesig/")

using namespace Ms;

//---------------------------------------------------------
//   TestTimesig
//---------------------------------------------------------

class TestTimesig : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase() { initMTest(); }
      void timesig01();
      void timesig02();
      void timesig03();
      void timesig04();
      void timesig05();
      void timesig_78216();
      };

//---------------------------------------------------------
///   timesig01
///   add a 3/4 time signature in the second measure
//---------------------------------------------------------

void TestTimesig::timesig01()
      {
      MasterScore* score = readScore(DIR + "timesig01.mscx");
      QVERIFY(score);
      Measure* m  = score->firstMeasure()->nextMeasure();
      TimeSig* ts = new TimeSig(score);
      ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

      score->startCmd();
      int staffIdx = 0;
      bool local   = false;
      score->cmdAddTimeSig(m, staffIdx, ts, local);
      score->endCmd();

      QVERIFY(saveCompareScore(score, "timesig01.mscx", DIR + "timesig01-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   timesig02
///   Attempt to change a 4/4 measure containing a triplet of minims to a 3/4 time signature
///   The attempt should fail, the score left unchanged
//---------------------------------------------------------

void TestTimesig::timesig02()
      {
      MasterScore* score = readScore(DIR + "timesig-02.mscx");
      QVERIFY(score);
      Measure* m = score->firstMeasure();
      TimeSig* ts = new TimeSig(score);
      ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

      score->startCmd();
      score->cmdAddTimeSig(m, 0, ts, false);
      score->doLayout();
      score->endCmd();

      QVERIFY(saveCompareScore(score, "timesig-02.mscx", DIR + "timesig-02-ref.mscx"));
      delete score;

      }

//---------------------------------------------------------
///   timesig03
///   add a 3/4 time signature in the second measure
///   rewrite notes
///   be sure that annotations and spanners are preserved
///   even annotations in otherwise empty segments
///   also measure repeats and non-default barlines
//---------------------------------------------------------

void TestTimesig::timesig03()
      {
      MasterScore* score = readScore(DIR + "timesig-03.mscx");
      QVERIFY(score);
      Measure* m = score->firstMeasure()->nextMeasure();
      TimeSig* ts = new TimeSig(score);
      ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

      score->cmdAddTimeSig(m, 0, ts, false);
      score->doLayout();

      QVERIFY(saveCompareScore(score, "timesig-03.mscx", DIR + "timesig-03-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   timesig04
///   add a 6/4 time signature in the second measure
///   which already contains a quarter note
//---------------------------------------------------------

void TestTimesig::timesig04()
      {
      MasterScore* score = readScore(DIR + "timesig-04.mscx");
      QVERIFY(score);
      Measure* m = score->firstMeasure()->nextMeasure();
      TimeSig* ts = new TimeSig(score);
      ts->setSig(Fraction(6, 4), TimeSigType::NORMAL);

      score->cmdAddTimeSig(m, 0, ts, false);
      score->doLayout();

      QVERIFY(saveCompareScore(score, "timesig-04.mscx", DIR + "timesig-04-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   timesig05
///   Add a 3/4 time signature to the first measure.
///   Test that spanners are preserved, especially those
///   that span across time signature change border.
///   Inspired by the issue #279593 where such spanners
///   caused crashes.
//---------------------------------------------------------

void TestTimesig::timesig05()
      {
      MasterScore* score = readScore(DIR + "timesig-05.mscx");
      QVERIFY(score);
      Measure* m = score->firstMeasure();
      TimeSig* ts = new TimeSig(score);
      ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

      score->cmdAddTimeSig(m, 0, ts, false);
      score->doLayout();

      QVERIFY(saveCompareScore(score, "timesig-05.mscx", DIR + "timesig-05-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   timesig_78216
//    input score has section breaks on non-measure MeasureBase objects.
//    should not display courtesy timesig at the end of final measure of each section (meas 1, 2, & 3), even if section break occurs on subsequent non-measure frame.
//---------------------------------------------------------

void TestTimesig::timesig_78216()
      {
      MasterScore* score = readScore(DIR + "timesig_78216.mscx");
      score->doLayout();

      Measure* m1 = score->firstMeasure();
      Measure* m2 = m1->nextMeasure();
      Measure* m3 = m2->nextMeasure();

      // verify no timesig exists in segment of final tick of m1, m2, m3
      QVERIFY2(m1->findSegment(SegmentType::TimeSig, m1->endTick()) == nullptr, "Should be no timesig at end of measure 1.");
      QVERIFY2(m2->findSegment(SegmentType::TimeSig, m2->endTick()) == nullptr, "Should be no timesig at end of measure 2.");
      QVERIFY2(m3->findSegment(SegmentType::TimeSig, m3->endTick()) == nullptr, "Should be no timesig at end of measure 3.");
      }

QTEST_MAIN(TestTimesig)
#include "tst_timesig.moc"

