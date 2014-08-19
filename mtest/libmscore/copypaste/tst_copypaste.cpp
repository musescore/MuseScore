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
#include "libmscore/chord.h"
#include "libmscore/xml.h"
#include "libmscore/durationtype.h"

#define DIR QString("libmscore/copypaste/")

using namespace Ms;

//---------------------------------------------------------
//   TestCopyPaste
//---------------------------------------------------------

class TestCopyPaste : public QObject, public MTest
      {
      Q_OBJECT

      void copypaste(const char*);
      void copypastestaff(const char*);
      void copypastevoice(const char*, int);

   private slots:
      void initTestCase();
      void copypaste01() { copypaste("01"); }       // start slur
      void copypaste02() { copypaste("02"); }       // end slur
      void copypaste03() { copypaste("03"); }       // slur
      void copypaste04() { copypaste("04"); }       // start tie
      void copypaste05() { copypaste("05"); }       // end tie
      void copypaste06() { copypaste("06"); }       // tie
      void copypaste07() { copypaste("07"); }       // start ottava
      void copypaste08() { copypaste("08"); }       // end ottava
      void copypaste09() { copypaste("09"); }       // ottava
      void copypaste10() { copypaste("10"); }       // two slurs
      void copypaste11() { copypaste("11"); }       // grace notes
      void copypaste12() { copypaste("12"); }       // voices
      void copyPaste2Voice();                       // voices-partial
      void copyPaste2Voice2() { copypastevoice("14", 0); }
      void copyPaste2Voice3() { copypastevoice("15", 1); }
      void copyPaste2Voice4() { copypastevoice("16",1); } // shorten last cr
      void copyPaste2Voice5();                            // cut and move
      void copyPasteOnlySecondVoice();
      void copypaste19() { copypaste("19"); }       // chord symbols

      void copypastestaff50() { copypastestaff("50"); }       // staff & slurs

      void copyPastePartial();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestCopyPaste::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   copypaste
//    copy measure 2, paste into measure 4
//---------------------------------------------------------

void TestCopyPaste::copypaste(const char* idx)
      {
      Score* score = readScore(DIR + QString("copypaste%1.mscx").arg(idx));
      score->doLayout();
      Measure* m1 = score->firstMeasure();
      Measure* m2 = m1->nextMeasure();    // src
      Measure* m3 = m2->nextMeasure();
      Measure* m4 = m3->nextMeasure();    // dst

      QVERIFY(m1 != 0);
      QVERIFY(m2 != 0);
      QVERIFY(m3 != 0);
      QVERIFY(m4 != 0);

      score->select(m2);
      QVERIFY(score->selection().canCopy());
      QString mimeType = score->selection().mimeType();
      QVERIFY(!mimeType.isEmpty());
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, score->selection().mimeData());
      QApplication::clipboard()->setMimeData(mimeData);
      score->select(m4->first()->element(0));

      score->startCmd();
      score->cmdPaste(mimeData,0);
      score->endCmd();

      score->doLayout();

      QVERIFY(saveCompareScore(score, QString("copypaste%1.mscx").arg(idx),
         DIR + QString("copypaste%1-ref.mscx").arg(idx)));
      delete score;
      }

//---------------------------------------------------------
//   copypaste
//    copy measure 2 from first staff, paste into staff 2
//---------------------------------------------------------

void TestCopyPaste::copypastestaff(const char* idx)
      {
      Score* score = readScore(DIR + QString("copypaste%1.mscx").arg(idx));
      score->doLayout();
      Measure* m1 = score->firstMeasure();
      Measure* m2 = m1->nextMeasure();    // src

      QVERIFY(m1 != 0);
      QVERIFY(m2 != 0);

      score->select(m2, SelectType::RANGE, 0);
      QVERIFY(score->selection().canCopy());
      QString mimeType = score->selection().mimeType();
      QVERIFY(!mimeType.isEmpty());
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, score->selection().mimeData());
      QApplication::clipboard()->setMimeData(mimeData);

      score->deselectAll();

      score->select(m2, SelectType::RANGE, 1);

      score->startCmd();
      score->cmdPaste(mimeData,0);
      score->endCmd();

      score->doLayout();

      QVERIFY(saveCompareScore(score, QString("copypaste%1.mscx").arg(idx),
         DIR + QString("copypaste%1-ref.mscx").arg(idx)));
      delete score;
      }

void TestCopyPaste::copyPastePartial() {
      Score* score = readScore(DIR + QString("copypaste_partial_01.mscx"));
      score->doLayout();

      Measure* m1 = score->firstMeasure();

      Segment* s = m1->first(Segment::Type::ChordRest);
      s = s->next(Segment::Type::ChordRest);
      score->select(s->element(0));
      s = s->next(Segment::Type::ChordRest);
      score->select(s->element(4), SelectType::RANGE);

      QVERIFY(score->selection().canCopy());
      QString mimeType = score->selection().mimeType();
      QVERIFY(!mimeType.isEmpty());
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, score->selection().mimeData());
      QApplication::clipboard()->setMimeData(mimeData);

      score->select(m1->first(Segment::Type::ChordRest)->element(0));

      score->startCmd();
      score->cmdPaste(mimeData,0);
      score->endCmd();

      score->doLayout();

      QVERIFY(saveCompareScore(score, QString("copypaste_partial_01.mscx"),
         DIR + QString("copypaste_partial_01-ref.mscx")));
      delete score;
}

void TestCopyPaste::copyPaste2Voice()
      {
      Score* score = readScore(DIR + QString("copypaste13.mscx"));
      score->doLayout();
      Measure* m1 = score->firstMeasure();
      Measure* m2 = m1->nextMeasure();

      QVERIFY(m1 != 0);
      QVERIFY(m2 != 0);

      // select 2 chord rests at the start of the first measure
      Segment* s = m1->first(Segment::Type::ChordRest);
      score->select(static_cast<Chord*>(s->element(0))->notes().at(0));
      s = s->next(Segment::Type::ChordRest);
      score->select(s->element(0), SelectType::RANGE);

      QVERIFY(score->selection().canCopy());
      QString mimeType = score->selection().mimeType();
      QVERIFY(!mimeType.isEmpty());
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, score->selection().mimeData());
      QApplication::clipboard()->setMimeData(mimeData);

      // paste into the second CR of second measure
      Segment* secondCRSeg = m2->first()->next1(Segment::Type::ChordRest);
      score->select(secondCRSeg->element(0));

      score->startCmd();
      score->cmdPaste(mimeData,0);
      score->endCmd();

      score->doLayout();

      QVERIFY(saveCompareScore(score, QString("copypaste13.mscx"),
         DIR + QString("copypaste13-ref.mscx")));
      delete score;
      }

//---------------------------------------------------------
//   copypaste
//    copy measure 2 from first staff, paste into staff 2
//---------------------------------------------------------

void TestCopyPaste::copypastevoice(const char* idx, int voice)
      {
      Score* score = readScore(DIR + QString("copypaste%1.mscx").arg(idx));
      score->doLayout();
      Measure* m1 = score->firstMeasure();
      Measure* m2 = m1->nextMeasure();

      QVERIFY(m1 != 0);
      QVERIFY(m2 != 0);

      // create a range selection on 2 and 3 beat of first measure
      Segment::Type segTypeCR = Segment::Type::ChordRest;
      Segment* s = m1->first(segTypeCR)->next1(segTypeCR);
      score->select(static_cast<Chord*>(s->element(voice))->notes().at(0));
      s = s->next(Segment::Type::ChordRest);
      score->select(s->element(voice), SelectType::RANGE);

      QVERIFY(score->selection().canCopy());
      QString mimeType = score->selection().mimeType();
      QVERIFY(!mimeType.isEmpty());
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, score->selection().mimeData());
      QApplication::clipboard()->setMimeData(mimeData);

      //paste to second measure
      score->select(m2->first()->element(0));

      score->startCmd();
      score->cmdPaste(mimeData,0);
      score->endCmd();

      score->doLayout();

      QVERIFY(saveCompareScore(score, QString("copypaste%1.mscx").arg(idx),
         DIR + QString("copypaste%1-ref.mscx").arg(idx)));
      delete score;
      }

void TestCopyPaste::copyPaste2Voice5()
      {
      Score* score = readScore(DIR + QString("copypaste17.mscx"));
      score->doLayout();
      Measure* m1 = score->firstMeasure();

      QVERIFY(m1 != 0);

      // create a range selection from 2 eighth note to the end of first measure
      Segment::Type segTypeCR = Segment::Type::ChordRest;
      Segment* s = m1->first(segTypeCR)->next1(segTypeCR);
      score->select(static_cast<Chord*>(s->element(0))->notes().at(0));

      s = m1->last()->prev(Segment::Type::ChordRest);
      score->select(s->element(0), SelectType::RANGE);

      QVERIFY(score->selection().canCopy());
      QString mimeType = score->selection().mimeType();
      QVERIFY(!mimeType.isEmpty());
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, score->selection().mimeData());
      QApplication::clipboard()->setMimeData(mimeData);

      score->cmdDeleteSelection(); //cut

      //paste to quarter rest
      Element* dest = m1->first()->next(segTypeCR)->next(segTypeCR)->next(segTypeCR)->element(0);
      QVERIFY(dest->type() == Element::Type::REST
              && static_cast<ChordRest*>(dest)->durationType() == TDuration::DurationType::V_QUARTER);
      score->select(dest);

      score->startCmd();
      score->cmdPaste(mimeData,0);
      score->endCmd();

      score->doLayout();

      QVERIFY(saveCompareScore(score, QString("copypaste17.mscx"),
         DIR + QString("copypaste17-ref.mscx")));
      delete score;
      }


void TestCopyPaste::copyPasteOnlySecondVoice()
      {
      Score* score = readScore(DIR + QString("copypaste18.mscx"));
      score->doLayout();
      Measure* m1 = score->firstMeasure();
      Measure* m2 = m1->nextMeasure();

      QVERIFY(m1 != 0);
      QVERIFY(m2 != 0);

      score->select(m1, SelectType::RANGE, 0);

      score->selectionFilter().setFiltered(SelectionFilterType::FIRST_VOICE,false);

      QVERIFY(score->selection().canCopy());
      QString mimeType = score->selection().mimeType();
      QVERIFY(!mimeType.isEmpty());
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, score->selection().mimeData());
      QApplication::clipboard()->setMimeData(mimeData);


      //paste to second measure
      score->deselectAll();
      score->selectionFilter().setFiltered(SelectionFilterType::FIRST_VOICE,true);
      score->select(m2,SelectType::RANGE);

      score->startCmd();
      score->cmdPaste(mimeData,0);
      score->endCmd();

      score->doLayout();

      QVERIFY(saveCompareScore(score, QString("copypaste18.mscx"),
         DIR + QString("copypaste18-ref.mscx")));
      delete score;
      }

QTEST_MAIN(TestCopyPaste)
#include "tst_copypaste.moc"

