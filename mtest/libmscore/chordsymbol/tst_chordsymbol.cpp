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
#include "libmscore/undo.h"
#include "libmscore/excerpt.h"
#include "libmscore/part.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/chordrest.h"
#include "libmscore/harmony.h"
#include "libmscore/duration.h"
#include "libmscore/durationtype.h"

#define DIR QString("libmscore/chordsymbol/")

using namespace Ms;

//---------------------------------------------------------
//   TestChordSymbol
//---------------------------------------------------------

class TestChordSymbol : public QObject, public MTest {
      Q_OBJECT

      MasterScore* test_pre(const char* p);
      void test_post(MasterScore* score, const char* p);

   private slots:
      void initTestCase();
      void testExtend();
      void testClear();
      void testAddLink();
      void testAddPart();
      void testNoSystem();
      void testTranspose();
      void testTransposePart();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestChordSymbol::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   chordsymbol
//---------------------------------------------------------

MasterScore* TestChordSymbol::test_pre(const char* p)
      {
      QString p1 = DIR + p + ".mscx";
      MasterScore* score = readScore(p1);
      score->doLayout();
      return score;
      }

void TestChordSymbol::test_post(MasterScore* score, const char* p)
      {
      QString p1 = p;
      p1 += "-test.mscx";
      QString p2 = DIR + p + "-ref.mscx";
      QVERIFY(saveCompareScore(score, p1, p2));
      delete score;
      }

void TestChordSymbol::testExtend()
      {
      MasterScore* score = test_pre("extend");
      Measure* m = score->firstMeasure();
      Segment* s = m->first(SegmentType::ChordRest);
      ChordRest* cr = s->cr(0);
      score->changeCRlen(cr, TDuration::DurationType::V_WHOLE);
      score->doLayout();
      test_post(score, "extend");
      }

void TestChordSymbol::testClear()
      {
      MasterScore* score = test_pre("clear");
      Measure* m = score->firstMeasure();
      score->select(m, SelectType::SINGLE, 0);
      score->cmdDeleteSelection();
      score->doLayout();
      test_post(score, "clear");
      }

void TestChordSymbol::testAddLink()
      {
      MasterScore* score = test_pre("add-link");
      Segment* seg = score->firstSegment(SegmentType::ChordRest);
      ChordRest* cr = seg->cr(0);
      Harmony* harmony = new Harmony(score);
      harmony->setHarmony("C7");
      harmony->setTrack(cr->track());
      harmony->setParent(cr->segment());
      score->undoAddElement(harmony);
      score->doLayout();
      test_post(score, "add-link");
      }

void TestChordSymbol::testAddPart()
      {
      MasterScore* score = test_pre("add-part");
      Segment* seg = score->firstSegment(SegmentType::ChordRest);
      ChordRest* cr = seg->cr(0);
      Harmony* harmony = new Harmony(score);
      harmony->setHarmony("C7");
      harmony->setTrack(cr->track());
      harmony->setParent(cr->segment());
      score->undoAddElement(harmony);
      score->doLayout();
      test_post(score, "add-part");
      }

void TestChordSymbol::testNoSystem()
      {
      MasterScore* score = test_pre("no-system");

      //
      // create first part
      //
      QList<Part*> parts;
      parts.append(score->parts().at(0));
      Score* nscore = new Score(score);

      Excerpt* ex = new Excerpt(score);
      ex->setPartScore(nscore);
      nscore->setExcerpt(ex);
      score->excerpts().append(ex);
      ex->setTitle(parts.front()->longName());
      ex->setParts(parts);
      Excerpt::createExcerpt(ex);
      QVERIFY(nscore);

//      nscore->setTitle(parts.front()->partName());
      nscore->style().set(Sid::createMultiMeasureRests, true);

      //
      // create second part
      //
      parts.clear();
      parts.append(score->parts().at(1));
      nscore = new Score(score);

      ex = new Excerpt(score);
      ex->setPartScore(nscore);
      nscore->setExcerpt(ex);
      score->excerpts().append(ex);
      ex->setTitle(parts.front()->longName());
      ex->setParts(parts);
      Excerpt::createExcerpt(ex);
      QVERIFY(nscore);

//      nscore->setTitle(parts.front()->partName());
      nscore->style().set(Sid::createMultiMeasureRests, true);

      score->setExcerptsChanged(true);
      score->doLayout();
      test_post(score, "no-system");
      }

void TestChordSymbol::testTranspose()
      {
      MasterScore* score = test_pre("transpose");
      score->startCmd();
      score->cmdSelectAll();
      score->transpose(TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 4, false, true, true);
      score->endCmd();
      test_post(score, "transpose");
      }

void TestChordSymbol::testTransposePart()
      {
      MasterScore* score = test_pre("transpose-part");
      score->startCmd();
      score->cmdSelectAll();
      score->transpose(TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 4, false, true, true);
      score->endCmd();
      test_post(score, "transpose-part");
      }

QTEST_MAIN(TestChordSymbol)
#include "tst_chordsymbol.moc"
