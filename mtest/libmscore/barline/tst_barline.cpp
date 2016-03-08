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
//   TestBarline
//---------------------------------------------------------

class TestBarline : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void barline01();
      void barline02();
      void barline03();
      void barline04();
      void barline05();
      void barline06();
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
      for (System* sys : score->systems()) {
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
                  Segment* seg = msr->findSegment(Segment::Type::EndBarLine, msr->tick()+msr->ticks());
                  sprintf(msg, "No SegEndBarLine in measure %d of system %d.", msrNo+1, sysNo+1);
                  QVERIFY2(seg != nullptr, msg);

                  bar = static_cast<BarLine*>(seg->element(0));
                  sprintf(msg, "No barline in measure %d of system %d.", msrNo+1, sysNo+1);
                  QVERIFY2(bar != nullptr, msg);

                  height      = bar->bbox().height() / spatium;
                  sprintf(msg, "Wrong barline height in measure %d of system %d.", msrNo+1, sysNo+1);
                  QVERIFY2(height > heightMin && height < heightMax, msg);
            }
            sysNo++;
      }

//      QVERIFY(saveCompareScore(score, "barline01.mscx", DIR + "barline01-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   barline02
///   add a 3/4 time signature in the second measure and check bar line 'generated' status
//
//    NO REFERENCE SCORE IS USED.
//---------------------------------------------------------

void TestBarline::barline02()
      {
      char msg[256];
      Score* score = readScore(DIR + "barline02.mscx");
      QVERIFY(score);
      Measure* msr = score->firstMeasure()->nextMeasure();
      TimeSig* ts  = new TimeSig(score);
      ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

      score->cmdAddTimeSig(msr, 0, ts, false);
      score->doLayout();

      msr = score->firstMeasure();
      int msrNo = 1;
      while ((msr = msr->nextMeasure())) {
            ++msrNo;
            Segment* seg = msr->findSegment(Segment::Type::EndBarLine, msr->tick()+msr->ticks());
            sprintf(msg, "No SegEndBarLine in measure %d.", msrNo);
            QVERIFY2(seg != nullptr, msg);

            BarLine* bar = static_cast<BarLine*>(seg->element(0));
            sprintf(msg, "No barline in measure %d.", msrNo);
            QVERIFY2(bar != nullptr, msg);

            // bar line should be generated if NORMAL, except the END one at the end
            sprintf(msg, "Barline in measure %d changed into 'non-generated'.", msrNo);
// ws: end barline is also generated
//            bool test = (bar->barLineType() == BarLineType::NORMAL) ? bar->generated() : !bar->generated();
            bool test = bar->generated();
            QVERIFY2(test, msg);
      }
//      QVERIFY(saveCompareScore(score, "barline02.mscx", DIR + "barline02-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   barline03
///   Sets a staff bar line span involving spanFrom and spanTo and
///   check tht it is properly applied to start-repeat
//
//    NO REFERENCE SCORE IS USED.
//---------------------------------------------------------

void TestBarline::barline03()
      {
      Score* score = readScore(DIR + "barline03.mscx");
      QVERIFY(score);
      score->doLayout();
      score->undo(new ChangeBarLineSpan(score->staff(0), 2, 2, 6));
      score->doLayout();

      // 'go' to 5th measure
      Measure* msr = score->firstMeasure();
      for (int i=0; i < 4; i++)
            msr = msr->nextMeasure();
      // check span data of measure-initial start-repeat bar line
      Segment* seg = msr->findSegment(Segment::Type::StartRepeatBarLine, msr->tick());
      QVERIFY2(seg != nullptr, "No SegStartRepeatBarLine segment in measure 5.");

      BarLine* bar = static_cast<BarLine*>(seg->element(0));
      QVERIFY2(bar != nullptr, "No start-repeat barline in measure 5.");

printf("===span %d %d %d\n", bar->span(), bar->spanFrom(), bar->spanTo());

      QVERIFY2(bar->span() == 2 && bar->spanFrom() == 2 && bar->spanTo() == 6,
            "Wrong span data in start-repeat barline of measure 5.");


      // check start-repeat bar ine in second staff is gone
      QVERIFY2(seg->element(1) == nullptr, "Extra start-repeat barline in 2nd staff of measure 5.");

//      QVERIFY(saveCompareScore(score, "barline03.mscx", DIR + "barline03-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   barline04
///   Sets custom span parameters to a system-initial start-repeat bar line and
///   check tht it is properly applied to it and to the start-reapeat bar lines of staves below.
//
//    NO REFERENCE SCORE IS USED.
//---------------------------------------------------------

void TestBarline::barline04()
      {
      Score* score = readScore(DIR + "barline04.mscx");
      QVERIFY(score);
      score->doLayout();

      score->startCmd();
      // 'go' to 5th measure
      Measure* msr = score->firstMeasure();
      for (int i=0; i < 4; i++)
            msr = msr->nextMeasure();
      // check span data of measure-initial start-repeat bar line
      Segment* seg = msr->findSegment(Segment::Type::StartRepeatBarLine, msr->tick());
      QVERIFY2(seg != nullptr, "No SegStartRepeatBarLine segment in measure 5.");

      BarLine* bar = static_cast<BarLine*>(seg->element(0));
      QVERIFY2(bar != nullptr, "No start-repeat barline in measure 5.");

      score->undoChangeSingleBarLineSpan(bar, 2, 2, 6);
      score->endCmd();

      QVERIFY2(bar->span() == 2 && bar->spanFrom() == 2 && bar->spanTo() == 6,
            "Wrong span data in start-repeat barline of measure 5.");

      // check start-repeat bar ine in second staff is gone
      QVERIFY2(seg->element(1) == nullptr, "Extra start-repeat barline in 2nd staff of measure 5.");

//      QVERIFY(saveCompareScore(score, "barline04.mscx", DIR + "barline04-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   barline05
///   Adds a line break in the middle of a end-start-repeat bar line and then checks the two resulting
///   bar lines (an end-repeat and a start-repeat) are not marked as generated.
//
//    NO REFERENCE SCORE IS USED.
//---------------------------------------------------------

void TestBarline::barline05()
      {
      Score* score = readScore(DIR + "barline05.mscx");
      QVERIFY(score);
      score->doLayout();

      // 'go' to 4th measure
      Measure* msr = score->firstMeasure();
      for (int i=0; i < 3; i++)
            msr = msr->nextMeasure();
      // create and add a LineBreak element
      LayoutBreak* lb = new LayoutBreak(score);
      lb->setLayoutBreakType(LayoutBreak::Type::LINE);
      lb->setTrack(-1);             // system-level element
      lb->setParent(msr);
      score->undoAddElement(lb);
      score->doLayout();

      // check an end-repeat bar line has been created at the end of this measure and it is generated
      Segment* seg = msr->findSegment(Segment::Type::EndBarLine, msr->tick()+msr->ticks());
      QVERIFY2(seg != nullptr, "No SegEndBarLine segment in measure 4.");
      BarLine* bar = static_cast<BarLine*>(seg->element(0));
      QVERIFY2(bar != nullptr, "No end-repeat barline in measure 4.");
      QVERIFY2(bar->barLineType() == BarLineType::END_REPEAT, "Barline at measure 4 is not END-REPEAT");
      QVERIFY2(bar->generated(), "End-repeat barline in measure 4 is non-generated.");

      // // check an end-repeat bar line has been created at the beginning of the next measure and it is not generated
      // check an end-repeat bar line has been created at the beginning of the next measure and it is generated
      msr = msr->nextMeasure();
      seg = msr->findSegment(Segment::Type::StartRepeatBarLine, msr->tick());
      QVERIFY2(seg != nullptr, "No SegStartRepeatBarLine segment in measure 5.");
      bar = static_cast<BarLine*>(seg->element(0));
      QVERIFY2(bar != nullptr, "No start-repeat barline in measure 5.");
      QVERIFY2(bar->generated(), "Start-reapeat barline in measure 5 is not generated.");

//      QVERIFY(saveCompareScore(score, "barline05.mscx", DIR + "barline05-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///   barline06
///   Read a score with 3 staves and custom bar line sub-types for staff i-th at measure i-th
///   and check the custom syb-types are applied only to their respective bar lines,
///   rather than to whole measures.
//
//    NO REFERENCE SCORE IS USED.
//---------------------------------------------------------

void TestBarline::barline06()
      {
      char  msg[256];
      Score* score = readScore(DIR + "barline06.mscx");
      QVERIFY(score);
      score->doLayout();

      // scan each measure
      Measure*    msr   = score->firstMeasure();
      int         msrNo = 1;
      for (int i=0; i < 3; i++) {
            // check measure endbarline type
            sprintf(msg, "EndBarLineType not NORMAL in measure %d.", msrNo);
//TODO            QVERIFY2(msr->endBarLineType() == BarLineType::NORMAL, msg);
            // locate end-measure bar line segment
            Segment* seg = msr->findSegment(Segment::Type::EndBarLine, msr->tick()+msr->ticks());
            sprintf(msg, "No SegEndBarLine in measure %d.", msr->no());
            QVERIFY2(seg != nullptr, msg);

            // check only i-th staff has custom bar line type
            for (int j=0; j < 3; j++) {
                  BarLine* bar = static_cast<BarLine*>(seg->element(j*VOICES));
                  // if not the i-th staff, bar should be normal and not custom
                  if (j != i) {
                        sprintf(msg, "barline type NOT NORMAL or CUSTOM TYPE in staff %d of measure %d.", j+1, msrNo);
                        QVERIFY2(bar->barLineType() == BarLineType::NORMAL, msg);
//                        QVERIFY2(bar->customSubtype() == false, msg);
                        }
                  // in the i-th staff, the bar line should be of type DOUBLE and custom type should be true
                  else {
                        sprintf(msg, "No barline for staff %d in measure %d", j+1, msrNo);
                        QVERIFY2(bar != nullptr, msg);
                        sprintf(msg, "barline type NOT DOUBLE or NOT CUSTOM TYPE in staff %d of measure %d.", j+1, msrNo);
                        QVERIFY2(bar->barLineType() == BarLineType::DOUBLE, msg);
//                        QVERIFY2(bar->customSubtype() == true, msg);
                        }
                  }

            msr = msr->nextMeasure();
            msrNo++;
            }
//      QVERIFY(saveCompareScore(score, "barline06.mscx", DIR + "barline06-ref.mscx"));
      delete score;
      }

QTEST_MAIN(TestBarline)
#include "tst_barline.moc"

