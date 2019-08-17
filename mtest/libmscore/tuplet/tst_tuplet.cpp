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
#include "libmscore/staff.h"
#include "libmscore/score.h"
#include "libmscore/tuplet.h"
#include "libmscore/chord.h"
#include "libmscore/measure.h"
#include "libmscore/timesig.h"

#define DIR QString("libmscore/tuplet/")

using namespace Ms;

//---------------------------------------------------------
//   TestTuplet
//---------------------------------------------------------

class TestTuplet : public QObject, public MTest
      {
      Q_OBJECT

      bool createTuplet(int n, ChordRest* cr);
      void tuplet(const char* p1, const char* p2);
      void split(const char* p1, const char* p2);

   private slots:
      void initTestCase();
      void join1()  { tuplet("tuplet1.mscx", "tuplet1-ref.mscx"); }
      void split1() { split("split1.mscx",   "split1-ref.mscx");  }
      void split2() { split("split2.mscx",   "split2-ref.mscx");  }
      void split3() { split("split3.mscx",   "split3-ref.mscx");  }
      void split4() { split("split4.mscx",   "split4-ref.mscx");  }
      void addStaff();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestTuplet::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   createTuplet
//---------------------------------------------------------

bool TestTuplet::createTuplet(int n, ChordRest* cr)
      {
      if (cr->durationType() < TDuration(TDuration::DurationType::V_128TH))
            return false;

      Fraction f(cr->ticks());
      Fraction tick = cr->tick();
      Tuplet* ot    = cr->tuplet();

      f.reduce();       //measure duration might not be reduced
      Fraction ratio(n, f.numerator());
      Fraction fr(1, f.denominator());
      while (ratio.numerator() >= ratio.denominator() * 2) {
            ratio *= Fraction(1,2);
            fr    *= Fraction(1,2);
            }

      Tuplet* tuplet = new Tuplet(cr->score());
      tuplet->setRatio(ratio);

      //
      // "fr" is the fraction value of one tuple element
      //
      // "tuplet time" is "normal time" / tuplet->ratio()
      //    Example: an 1/8 has 240 midi ticks, in an 1/8 triplet the note
      //             has a tick duration of 240 / (3/2) = 160 ticks
      //             (assume tpq = 480)
      //

      tuplet->setTicks(f);
      TDuration baseLen(fr);
      tuplet->setBaseLen(baseLen);

      tuplet->setTrack(cr->track());
      tuplet->setTick(tick);
      Measure* measure = cr->measure();
      tuplet->setParent(measure);

      if (ot)
            tuplet->setTuplet(ot);
      cr->score()->startCmd();
      cr->score()->cmdCreateTuplet(cr, tuplet);
      cr->score()->endCmd();
      return true;
      }

//---------------------------------------------------------
//   tuplet
//---------------------------------------------------------

void TestTuplet::tuplet(const char* p1, const char* p2)
      {
      MasterScore* score = readScore(DIR + p1);
      Measure* m1 = score->firstMeasure();
      Measure* m2 = m1->nextMeasure();

      QVERIFY(m1 != 0);
      QVERIFY(m2 != 0);
      QVERIFY(m1 != m2);

      Segment* s = m2->first(SegmentType::ChordRest);
      QVERIFY(s != 0);
      Chord* c = toChord(s->element(0));
      QVERIFY(c != 0);

      QVERIFY(createTuplet(3, c));

      QVERIFY(saveCompareScore(score, p1, DIR + p2));
      delete score;
      }

//---------------------------------------------------------
//   split
//---------------------------------------------------------

void TestTuplet::split(const char* p1, const char* p2)
      {
      MasterScore* score = readScore(DIR + p1);
      Measure* m         = score->firstMeasure();
      TimeSig* ts        = new TimeSig(score);
      ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

      score->startCmd();
      EditData dd(0);
      dd.dropElement = ts;
      dd.modifiers = 0;
      dd.dragOffset = QPointF();
      dd.pos = m->pagePos();
      m->drop(dd);
      score->endCmd();

      QVERIFY(saveCompareScore(score, p1, DIR + p2));
      delete score;
      }

//---------------------------------------------------------
//   addStaff
//    Checks that after adding a staff the resulting
//    score is equal to the reference score
//---------------------------------------------------------

void TestTuplet::addStaff()
      {
      MasterScore* score = readScore(DIR + "nestedTuplets_addStaff.mscx");
      QVERIFY(score);

      // add a staff to the existing staff
      // (copied and adapted from void MuseScore::editInstrList() in mscore/instrdialog.cpp)
      Staff* oldStaff   = score->staff(0);
      Staff* newStaff   = new Staff(score);
      newStaff->setPart(oldStaff->part());
      newStaff->initFromStaffType(oldStaff->staffType(Fraction(0,1)));
      newStaff->setDefaultClefType(ClefTypeList(ClefType::F));
      KeySigEvent ke = oldStaff->keySigEvent(Fraction(0,1));
      newStaff->setKey(Fraction(0,1), ke);
      score->undoInsertStaff(newStaff, 0, true);

      QVERIFY(saveCompareScore(score, "nestedTuplets_addStaff.mscx", DIR + "nestedTuplets_addStaff-ref.mscx"));
      delete score;
      }

QTEST_MAIN(TestTuplet)
#include "tst_tuplet.moc"

