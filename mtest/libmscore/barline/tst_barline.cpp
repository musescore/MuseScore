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
#include "libmscore/barline.h"
#include "libmscore/measure.h"
#include "libmscore/score.h"
#include "libmscore/system.h"
#include "libmscore/undo.h"

#define DIR QString("libmscore/barline/")

using namespace Ms;

//---------------------------------------------------------
//   TestClef
//---------------------------------------------------------

class TestBarline : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void barline01();
      void barline02();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestBarline::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
///  barline01
///   Check bar line and brackets presence and length with hidden empty staves:
//    A score with:
//          3 staves,
//          bracket across all 3 staves
//          bar lines across all 3 staves
//          systems with each staff hidden in turn because empty
//    is loaded, laid out and bracket/bar line sizes are checked.
//
//    NO REFERENCE SCORE IS USED: the test has to do with layout/formatting,
//    not with edit or read/save operations.
//---------------------------------------------------------

// actual 3-staff bracket should be high 28.6 SP ca.: allow for some layout margin
static const qreal      BRACKET0_HEIGHT_MIN     = 27;
static const qreal      BRACKET0_HEIGHT_MAX     = 30;
// actual 2-staff bracket should be high 18.1 SP ca.
static const qreal      BRACKET_HEIGHT_MIN      = 17;
static const qreal      BRACKET_HEIGHT_MAX      = 20;
// actual 3-staff bar line should be high 25 SP
static const qreal      BARLINE0_HEIGHT_MIN     = 24;
static const qreal      BARLINE0_HEIGHT_MAX     = 26;
// actual 2-staff bar line should be high 14.5 SP
static const qreal      BARLINE_HEIGHT_MIN      = 14;
static const qreal      BARLINE_HEIGHT_MAX      = 15;

void TestBarline::barline01()
      {
      char msg[256];
      Score* score = readScore(DIR + "barline01.mscx");
      score->doLayout();

      qreal height, heightMin, heightMax;
      qreal spatium = score->spatium();
      int sysNo = 0;
      foreach(System* sys , *score->systems()) {
            // check number of the brackets of each system
            sprintf(msg, "Wrong number of brackets in system %d.", sysNo+1);
            QVERIFY2(sys->brackets().count() == 1, msg);

            // check height of the bracket of each system
            // (bracket height is different between first system (3 staves) and other systems (2 staves) )
            Bracket* bracket = sys->brackets().at(0);
            height      = bracket->bbox().height() / spatium;
            heightMin   = (sysNo == 0) ? BRACKET0_HEIGHT_MIN : BRACKET_HEIGHT_MIN;
            heightMax   = (sysNo == 0) ? BRACKET0_HEIGHT_MAX : BRACKET_HEIGHT_MAX;
            sprintf(msg, "Wrong bracket height in system %d.", sysNo+1);
            QVERIFY2(height > heightMin && height < heightMax, msg);

            // check presence and height of the bar line of each measure of each system
            // (2 measure for each system)
            heightMin = (sysNo == 0) ? BARLINE0_HEIGHT_MIN : BARLINE_HEIGHT_MIN;
            heightMax = (sysNo == 0) ? BARLINE0_HEIGHT_MAX : BARLINE_HEIGHT_MAX;
            for (int msrNo=0; msrNo < 2; ++msrNo) {
                  BarLine* bar = nullptr;
                  Measure* msr = static_cast<Measure*>(sys->measure(msrNo));
                  Segment* seg = msr->findSegment(Segment::SegEndBarLine, msr->tick()+msr->ticks());
                  sprintf(msg, "No SegEndBarLine in measure %d of system %d.", msrNo+1, sysNo+1);
                  QVERIFY2(seg != nullptr, msg);

                  bar = static_cast<BarLine*>(seg->element(0));
                  sprintf(msg, "No bar line in measure %d of system %d.", msrNo+1, sysNo+1);
                  QVERIFY2(bar != nullptr, msg);

                  height      = bar->bbox().height() / spatium;
                  sprintf(msg, "Wrong bar line height in measure %d of system %d.", msrNo+1, sysNo+1);
                  QVERIFY2(height > heightMin && height < heightMax, msg);
            }
            sysNo++;
      }

//      QVERIFY(saveCompareScore(score, "barline01.mscx", DIR + "barline01-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   barline02
///   add a 3/4 time signature in the second measure and chech bar line 'generated' status
//
//    NO REFERENCE SCORE IS USED.
//---------------------------------------------------------

void TestBarline::barline02()
      {
      char msg[256];
      Score* score = readScore(DIR + "barline02.mscx");
      QVERIFY(score);
      Measure* msr = score->firstMeasure()->nextMeasure();
      TimeSig* ts = new TimeSig(score);
      ts->setSig(Fraction(3, 4), TSIG_NORMAL);

      score->cmdAddTimeSig(msr, 0, ts, false);
      score->doLayout();

      msr = score->firstMeasure();
      int msrNo = 1;
      while ( (msr=msr->nextMeasure()) != nullptr ) {
            ++msrNo;
            Segment* seg = msr->findSegment(Segment::SegEndBarLine, msr->tick()+msr->ticks());
            sprintf(msg, "No SegEndBarLine in measure %d.", msrNo);
            QVERIFY2(seg != nullptr, msg);

            BarLine* bar = static_cast<BarLine*>(seg->element(0));
            sprintf(msg, "No bar line in measure %d.", msrNo);
            QVERIFY2(bar != nullptr, msg);

            sprintf(msg, "Bar line  in measure %d changed into 'generated'.", msrNo);
            QVERIFY2(!bar->generated(), msg);
      }
//      QVERIFY(saveCompareScore(score, "barline02.mscx", DIR + "barline02-ref.mscx"));
      delete score;
      }

QTEST_MAIN(TestBarline)
#include "tst_barline.moc"

