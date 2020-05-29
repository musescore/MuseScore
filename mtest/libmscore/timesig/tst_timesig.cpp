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
#include "libmscore/undo.h"

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
      void timesig06();
      void timesig07();
      void timesig08();
      void timesig09();
      void timesig10();
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
//   timesig06
//    Change timesig with a tremolo that doesn't end up across a barline
//---------------------------------------------------------

void TestTimesig::timesig06()
      {
      MasterScore* score = readScore(DIR + "timesig-06.mscx");
      QVERIFY(score);
      Measure* m = score->firstMeasure();
      TimeSig* ts = new TimeSig(score);
      ts->setSig(Fraction(5, 4), TimeSigType::NORMAL);

      score->startCmd();
      score->cmdAddTimeSig(m, 0, ts, false);
      score->doLayout();
      QVERIFY(saveCompareScore(score, "timesig-06.mscx", DIR + "timesig-06-ref.mscx"));
      score->endCmd();

      // Now undo the change, if it crashes, it will fail
      score->undoStack()->undo(0);
      score->doLayout();
      delete score;
      }

//---------------------------------------------------------
//   timesig07
//    Change timesig with a tremolo that _does_ end up across a barline
//    The tremolo should end up removed.
//---------------------------------------------------------

void TestTimesig::timesig07()
      {
      MasterScore* score = readScore(DIR + "timesig-07.mscx");
      QVERIFY(score);
      Measure* m = score->firstMeasure();
      TimeSig* ts = new TimeSig(score);
      ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

      score->startCmd();
      score->cmdAddTimeSig(m, 0, ts, false);
      score->doLayout();
      QVERIFY(saveCompareScore(score, "timesig-07.mscx", DIR + "timesig-07-ref.mscx"));
      score->endCmd();

      // Now undo the change, if there is a crash the test will fail
      score->undoStack()->undo(0);
      score->doLayout();
      delete score;
      }

//---------------------------------------------------------
//   timesig08
//    Check if a courtesy time signature is created along with
//    a local time signature in the next system, no matter which staff the local time signature is in
//    (in this particular case, stave no.2)
//---------------------------------------------------------

void TestTimesig::timesig08()
      {
      MasterScore* score = readScore(DIR + "timesig-08.mscx");
      score->doLayout();

      Measure* m1 = score->firstMeasure();
      Segment* seg = m1->findSegment(SegmentType::TimeSigAnnounce, m1->endTick());
      Element* el = seg->element(staff2track(1));

      QVERIFY2(el, "Should be a courtesy signature in the second staff at the end of measure 1.");
      delete score;
      }

//---------------------------------------------------------
//   timesig09
//    Change timesig with tremolos on notes that end up across barlines
//---------------------------------------------------------

void TestTimesig::timesig09()
      {
      MasterScore* score = readScore(DIR + "timesig-09.mscx");
      QVERIFY(score);
      Measure* m = score->firstMeasure();
      TimeSig* ts = new TimeSig(score);
      ts->setSig(Fraction(9, 8), TimeSigType::NORMAL);

      score->startCmd();
      score->cmdAddTimeSig(m, 0, ts, false);
      score->doLayout();
      QVERIFY(saveCompareScore(score, "timesig-09-1.mscx", DIR + "timesig-09-ref.mscx"));
      score->endCmd();

      // Now undo the change
      score->undoStack()->undo(0);
      score->doLayout();
      QVERIFY(saveCompareScore(score, "timesig-09-2.mscx", DIR + "timesig-09.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   timesig10
//    Check if 4/4 is correctly changed to alla breve when commanded to do so
//    Same for 2/2 to common time
//---------------------------------------------------------

void TestTimesig::timesig10()
      {
      MasterScore* score = readScore(DIR + "timesig-10.mscx");

      Measure* m1 = score->firstMeasure();
      TimeSig* ts1 = new TimeSig(score);
      ts1->setSig(Fraction(2, 2), TimeSigType::ALLA_BREVE);

      score->startCmd();
      score->cmdAddTimeSig(m1, 0, ts1, false);

      Measure* m2 = m1->nextMeasure();
      TimeSig* ts2 = new TimeSig(score);
      ts2->setSig(Fraction(2, 2), TimeSigType::NORMAL);
      TimeSig* ts3 = new TimeSig(score);
      ts3->setSig(Fraction(4, 4), TimeSigType::FOUR_FOUR);
      //TimeSig* ts4 = new TimeSig(score);
      //ts4->setSig(Fraction(2, 2), TimeSigType::CUT_BACH);

      score->cmdAddTimeSig(m2, 0, ts2, false);
      m2 = score->firstMeasure()->nextMeasure();
      score->cmdAddTimeSig(m2, 0, ts3, false);
      //m2 = score->firstMeasure()->nextMeasure();
      //score->cmdAddTimeSig(m2, 0, ts4, false);
      //m2 = score->firstMeasure()->nextMeasure();
      //score->cmdAddTimeSig(m2, 0, ts5, false);

      score->doLayout();
      QVERIFY(saveCompareScore(score, "timesig-10.mscx", DIR + "timesig-10-ref.mscx"));
      score->endCmd();
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
      //Measure* m4 = m3->nextMeasure();
      //Measure* m5 = m4->nextMeasure();

      // verify no timesig exists in segment of final tick of m1, m2, m3
      QVERIFY2(!m1->findSegment(SegmentType::TimeSig, m1->endTick()), "Should be no timesig at the end of measure 1.");
      QVERIFY2(!m2->findSegment(SegmentType::TimeSig, m2->endTick()), "Should be no timesig at the end of measure 2.");
      QVERIFY2(!m3->findSegment(SegmentType::TimeSig, m3->endTick()), "Should be no timesig at the end of measure 3.");
      //QVERIFY2(!m4->findSegment(SegmentType::TimeSig, m4->endTick()), "Should be no timesig at the end of measure 4.");
      //QVERIFY2(!m5->findSegment(SegmentType::TimeSig, m5->endTick()), "Should be no timesig at the end of measure 5.");
      delete score;
      }

QTEST_MAIN(TestTimesig)
#include "tst_timesig.moc"

