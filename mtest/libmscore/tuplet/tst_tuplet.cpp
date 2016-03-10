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
#include "libmscore/tuplet.h"
#include "libmscore/chord.h"
#include "libmscore/measure.h"

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

   private slots:
      void initTestCase();
      void join1() { tuplet("tuplet1.mscx",  "tuplet1-ref.mscx"); }
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

      Fraction f(cr->duration());
      int tick    = cr->tick();
      Tuplet* ot  = cr->tuplet();

      f.reduce();       //measure duration might not be reduced
      Fraction ratio(n, f.numerator());
      Fraction fr(1, f.denominator());
      while (ratio.numerator() >= ratio.denominator()*2) {
            ratio /= 2;
            fr    /= 2;
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

      tuplet->setDuration(f);
      TDuration baseLen(fr);
      tuplet->setBaseLen(baseLen);

      tuplet->setTrack(cr->track());
      tuplet->setTick(tick);
      Measure* measure = cr->measure();
      tuplet->setParent(measure);

      if (ot)
            tuplet->setTuplet(ot);
      cr->score()->cmdCreateTuplet(cr, tuplet);
      return true;
      }

//---------------------------------------------------------
//   tuplet
//---------------------------------------------------------

void TestTuplet::tuplet(const char* p1, const char* p2)
      {
      MasterScore* score = readScore(DIR + p1);
      score->doLayout();
      Measure* m1 = score->firstMeasure();
      Measure* m2 = m1->nextMeasure();

      QVERIFY(m1 != 0);
      QVERIFY(m2 != 0);
      QVERIFY(m1 != m2);

      Segment* s = m2->first(Segment::Type::ChordRest);
      QVERIFY(s != 0);
      Ms::Chord* c = static_cast<Ms::Chord*>(s->element(0));
      QVERIFY(c != 0);

      QVERIFY(createTuplet(3, c));

      QVERIFY(saveCompareScore(score, p1, DIR + p2));
      delete score;
      }

QTEST_MAIN(TestTuplet)
#include "tst_tuplet.moc"

