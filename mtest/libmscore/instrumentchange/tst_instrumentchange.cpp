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
#include "libmscore/instrchange.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/undo.h"
#include "synthesizer/midipatch.h"

#define DIR QString("libmscore/instrumentchange/")

using namespace Ms;

//---------------------------------------------------------
//   TestChordSymbol
//---------------------------------------------------------

class TestInstrumentChange : public QObject, public MTest {
      Q_OBJECT

      Score* test_pre(const char* p);
      void test_post(Score* score, const char* p);

   private slots:
      void initTestCase();
      void testAdd();
      void testDelete();
      void testChange();
      void testMixer();
      void testCopy();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestInstrumentChange::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   chordsymbol
//---------------------------------------------------------

Score* TestInstrumentChange::test_pre(const char* p)
      {
      QString p1 = DIR + p + ".mscx";
      Score* score = readScore(p1);
      score->doLayout();
      return score;
      }

void TestInstrumentChange::test_post(Score* score, const char* p)
      {
      QString p1 = p;
      p1 += "-test.mscx";
      QString p2 = DIR + p + "-ref.mscx";
      QVERIFY(saveCompareScore(score, p1, p2));
      delete score;
      }

void TestInstrumentChange::testAdd()
      {
      Score* score = test_pre("add");
      Measure* m = score->firstMeasure()->nextMeasure();
      Segment* s = m->first(Segment::Type::ChordRest);
      InstrumentChange* ic = new InstrumentChange(score);
      ic->setParent(s);
      ic->setTrack(0);
      ic->setText("Instrument");
      score->undoAddElement(ic);
      score->doLayout();
      test_post(score, "add");
      }

void TestInstrumentChange::testDelete()
      {
      Score* score = test_pre("delete");
      Measure* m = score->firstMeasure()->nextMeasure();
      Segment* s = m->first(Segment::Type::ChordRest);
      InstrumentChange* ic = static_cast<InstrumentChange*>(s->annotations()[0]);
      score->deleteItem(ic);
      score->doLayout();
      test_post(score, "delete");
      }

void TestInstrumentChange::testChange()
      {
      Score* score = test_pre("change");
      Measure* m = score->firstMeasure()->nextMeasure();
      Segment* s = m->first(Segment::Type::ChordRest);
      InstrumentChange* ic = static_cast<InstrumentChange*>(s->annotations()[0]);
      Instrument* ni = score->staff(1)->part()->instr();
      ic->setInstrument(*ni);
      score->startCmd();
      ic->setText("Instrument Oboe");
      score->undo(new ChangeInstrument(ic, ic->instrument()));
      score->endCmd();
      score->doLayout();
      test_post(score, "change");
      }

void TestInstrumentChange::testMixer()
      {
      Score* score = test_pre("mixer");
      Measure* m = score->firstMeasure()->nextMeasure();
      Segment* s = m->first(Segment::Type::ChordRest);
      InstrumentChange* ic = static_cast<InstrumentChange*>(s->annotations()[0]);
      int idx = score->staff(0)->channel(s->tick(), 0);
      Channel* c = score->staff(0)->part()->instr(s->tick())->channel(idx);
      MidiPatch* mp = new MidiPatch;
      mp->bank = 0;
      mp->drum = false;
      mp->name = "Viola";
      mp->prog = 41;
      mp->synti = "Fluid";
      score->startCmd();
      ic->setText("Mixer Viola");
      score->undo(new ChangePatch(score, c, mp));
      score->endCmd();
      score->doLayout();
      test_post(score, "mixer");
      }

void TestInstrumentChange::testCopy()
      {
      Score* score = test_pre("copy");
      Measure* m = score->firstMeasure()->nextMeasure();
      Segment* s = m->first(Segment::Type::ChordRest);
      InstrumentChange* ic = static_cast<InstrumentChange*>(s->annotations()[0]);
      m = m->nextMeasure();
      s = m->first(Segment::Type::ChordRest);
      InstrumentChange* nic = new InstrumentChange(*ic);
      nic->setParent(s);
      nic->setTrack(4);
      score->undoAddElement(nic);
      score->doLayout();
      test_post(score, "copy");
      }

QTEST_MAIN(TestInstrumentChange)
#include "tst_instrumentchange.moc"
