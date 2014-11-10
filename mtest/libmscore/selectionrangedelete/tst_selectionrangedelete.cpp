//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>

#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/undo.h"
#include "mtest/testutils.h"

#define DIR QString("libmscore/selectionrangedelete/")

using namespace Ms;

//---------------------------------------------------------
//   SelectionRangeDelete
//---------------------------------------------------------

class TestSelectionRangeDelete : public QObject, public MTest
      {
      Q_OBJECT
      void verifyDelete(Score* score, size_t spanners);
      void verifyNoDelete(Score* score, size_t spanners);
      void deleteVoice(int voice, QString idx);

   private slots:
      void initTestCase();
      void deleteSegmentWithSlur();
      void deleteSegmentWithSpanner();
      void deleteVoice1() { deleteVoice(0,"03"); }
      void deleteVoice2() { deleteVoice(1,"04"); }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSelectionRangeDelete::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   verifyDelete
//---------------------------------------------------------

void TestSelectionRangeDelete::verifyDelete(Score* score, size_t spanners)
      {
      score->startCmd();
      score->cmdDeleteSelection();
      score->endCmd();

      QVERIFY(score->spanner().size() == spanners -1);
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(score->spanner().size() == spanners);
      }

//---------------------------------------------------------
//   verifyNoDelete
//---------------------------------------------------------

void TestSelectionRangeDelete::verifyNoDelete(Score* score, size_t spanners)
      {
      score->startCmd();
      score->cmdDeleteSelection();
      score->endCmd();

      QVERIFY(score->spanner().size() == spanners);
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(score->spanner().size() == spanners);
      }

//---------------------------------------------------------
//   chordRestAtBeat
//---------------------------------------------------------

Element* chordRestAtBeat(Score* score, int beat, int half = 0)
      {
      qDebug("Chordrest at beat %i,%i",beat,half);
      return score->tick2segment(beat * 480 + half * 240,false,Segment::Type::ChordRest,false)->element(0);
      }

//---------------------------------------------------------
//   deleteSegmentWithSlur
//---------------------------------------------------------

void TestSelectionRangeDelete::deleteSegmentWithSlur()
      {
      /*
       *  Score looks like this:
       *  ss - start slur, es - end slur, q - quarter note, e - eighth note
       *
       *  ss es ss   es
       *  q  q  q  e e
       */
      Score* score = readScore(DIR + "selectionrangedelete01.mscx");

      score->doLayout();
      QVERIFY(score);
      size_t spanners = score->spanner().size();

      score->select(chordRestAtBeat(score, 0),SelectType::RANGE);
      verifyDelete(score,spanners);
      score->deselectAll();

      score->select(chordRestAtBeat(score, 1),SelectType::RANGE);
      verifyDelete(score,spanners);
      score->deselectAll();

      score->select(chordRestAtBeat(score, 2),SelectType::RANGE);
      verifyDelete(score,spanners);
      score->deselectAll();

      score->select(chordRestAtBeat(score, 3),SelectType::RANGE);
      verifyNoDelete(score,spanners);
      score->deselectAll();

      score->select(chordRestAtBeat(score, 3,1),SelectType::RANGE);
      verifyDelete(score,spanners);
      score->deselectAll();

      delete score;
      }

//---------------------------------------------------------
//   deleteSegmentWithSpanner
//---------------------------------------------------------

void TestSelectionRangeDelete::deleteSegmentWithSpanner()
      {
      /*
       *  Score looks like this:
       *  ss - start spanner, es - end spanner, q - quarter note
       *
       *  ss    es
       *  q  q  q
       */
      Score* score = readScore(DIR + "selectionrangedelete02.mscx");

      score->doLayout();
      QVERIFY(score);
      size_t spanners = score->spanner().size();

      score->select(chordRestAtBeat(score, 0),SelectType::RANGE);
      verifyNoDelete(score,spanners);
//      verifyDelete(score,spanners);
      score->deselectAll();

      score->select(chordRestAtBeat(score, 1),SelectType::RANGE);
      verifyNoDelete(score,spanners);
      score->deselectAll();

      score->select(chordRestAtBeat(score, 2),SelectType::RANGE);
      verifyNoDelete(score,spanners);
      score->deselectAll();

      score->select(chordRestAtBeat(score, 0),SelectType::RANGE);
      score->select(chordRestAtBeat(score, 2),SelectType::RANGE);
      verifyDelete(score,spanners);
      score->deselectAll();

      delete score;
      }

//---------------------------------------------------------
//   deleteVoice
//---------------------------------------------------------

void TestSelectionRangeDelete::deleteVoice(int voice, QString idx)
      {
      Score* score = readScore(DIR + QString("selectionrangedelete%1.mscx").arg(idx));

      Measure* m1 = score->firstMeasure();
      QVERIFY(m1);

      SelectionFilterType voiceFilterType = SelectionFilterType((int)SelectionFilterType::FIRST_VOICE + voice);
      score->selectionFilter().setFiltered(voiceFilterType,false);
      score->select(m1,SelectType::RANGE);

      score->startCmd();
      score->cmdDeleteSelection();
      score->endCmd();

      score->doLayout();

      QVERIFY(saveCompareScore(score, QString("selectionrangedelete%1.mscx").arg(idx),
         DIR + QString("selectionrangedelete%1-ref.mscx").arg(idx)));
      delete score;
      }

QTEST_MAIN(TestSelectionRangeDelete)

#include "tst_selectionrangedelete.moc"
