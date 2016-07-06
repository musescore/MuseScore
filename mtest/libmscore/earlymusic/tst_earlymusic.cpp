//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2015 Werner Schweer & others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/chord.h"
#include "libmscore/measure.h"
#include "libmscore/score.h"
#include "libmscore/style.h"
#include "libmscore/undo.h"

#define DIR QString("libmscore/earlymusic/")

using namespace Ms;

//---------------------------------------------------------
//   TestEarlymusic
//---------------------------------------------------------

class TestEarlymusic : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void earlymusic01();            // setting cross-measure value flag and undoing
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestEarlymusic::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
///  earlymusic01
///   Setting cross-measure value flag and undoing.
//---------------------------------------------------------

void TestEarlymusic::earlymusic01()
      {
      MasterScore* score = readScore(DIR + "mensurstrich01.mscx");
      QVERIFY(score);
      score->doLayout();

      // go to first chord and verify crossMeasure values
      Measure*    msr   = score->firstMeasure();
      QVERIFY(msr);
      Segment*    seg   = msr->findSegment(Segment::Type::ChordRest, 0);
      QVERIFY(seg);
      Ms::Chord*      chord = static_cast<Ms::Chord*>(seg->element(0));
      QVERIFY(chord && chord->type() == Element::Type::CHORD);
      QVERIFY(chord->crossMeasure() == CrossMeasure::UNKNOWN);
      TDuration cmDur   = chord->crossMeasureDurationType();
//      QVERIFY(cmDur.type() == TDuration::DurationType::V_INVALID);    // irrelevant if crossMeasure() == UNKNOWN
      TDuration acDur   = chord->actualDurationType();
      QVERIFY(acDur.type() == TDuration::DurationType::V_BREVE);
      TDuration dur     = chord->durationType();
      QVERIFY(dur.type() == TDuration::DurationType::V_BREVE);

      // set crossMeasureValue flag ON: score should not change
      MStyle newStyle = *score->style();
      newStyle.set(StyleIdx::crossMeasureValues, true);
      score->startCmd();
      score->deselectAll();
      score->undo(new ChangeStyle(score, newStyle));
      score->update();
      score->endCmd();
      // verify crossMeasureDurationType did change
      QVERIFY(chord->crossMeasure() == CrossMeasure::FIRST);
      cmDur = chord->crossMeasureDurationType();
      QVERIFY(cmDur.type() == TDuration::DurationType::V_LONG);
      acDur = chord->actualDurationType();
      QVERIFY(acDur.type() == TDuration::DurationType::V_BREVE);
      dur   = chord->durationType();
      QVERIFY(dur.type() == TDuration::DurationType::V_LONG);
      // verify score file did not change
      QVERIFY(saveCompareScore(score, "mensurstrich01.mscx", DIR + "mensurstrich01-ref.mscx"));

      // UNDO AND VERIFY
      score->undoStack()->undo();
      score->doLayout();
      QVERIFY(chord->crossMeasure() == CrossMeasure::UNKNOWN);
      cmDur = chord->crossMeasureDurationType();
//      QVERIFY(cmDur.type() == TDuration::DurationType::V_LONG);    // irrelevant if crossMeasure() == UNKNOWN
      acDur = chord->actualDurationType();
      QVERIFY(acDur.type() == TDuration::DurationType::V_BREVE);
      dur   = chord->durationType();
      QVERIFY(dur.type() == TDuration::DurationType::V_BREVE);
      QVERIFY(saveCompareScore(score, "mensurstrich01.mscx", DIR + "mensurstrich01.mscx"));
      delete score;
      }


QTEST_MAIN(TestEarlymusic)
#include "tst_earlymusic.moc"

