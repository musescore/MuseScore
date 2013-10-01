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
//   paste
//---------------------------------------------------------

static void paste(Score* _score)
      {
      _score->startCmd();

      const QMimeData* ms = QApplication::clipboard()->mimeData();
      if (ms == 0) {
                  qDebug("no application mime data");
                  return;
                  }
      if (_score->selection().isSingle() && ms->hasFormat(mimeSymbolFormat)) {
                  QByteArray data(ms->data(mimeSymbolFormat));
                  XmlReader e(data);
                  QPointF dragOffset;
                  Fraction duration(1, 4);
                  Element::ElementType type = Element::readType(e, &dragOffset, &duration);
                  if (type != Element::INVALID) {
                              Element* el = Element::create(type, _score);
                              if (el) {
                                          el->read(e);
                                          _score->addRefresh(_score->selection().element()->abbox());   // layout() ?!
                                          DropData ddata;
                                          ddata.view       = 0;
                                          ddata.element    = el;
                                          ddata.duration   = duration;
                                          _score->selection().element()->drop(ddata);
                                          if (_score->selection().element())
                                                      _score->addRefresh(_score->selection().element()->abbox());
                                          }
                              }
                  else
                              qDebug("cannot read type");
                  }
      else if ((_score->selection().state() == SEL_RANGE || _score->selection().state() == SEL_LIST)
               && ms->hasFormat(mimeStaffListFormat)) {
                  ChordRest* cr = 0;
                  if (_score->selection().state() == SEL_RANGE)
                              cr = _score->selection().firstChordRest();
                  else if (_score->selection().isSingle()) {
                              Element* e = _score->selection().element();
                              if (e->type() != Element::NOTE && e->type() != Element::REST) {
                                          qDebug("cannot paste to %s", e->name());
                                          return;
                                          }
                              if (e->type() == Element::NOTE)
                                          e = static_cast<Note*>(e)->chord();
                              cr  = static_cast<ChordRest*>(e);
                              }
                  if (cr == 0)
                              qDebug("no destination to paste");
                  else if (cr->tuplet())
                              qDebug("cannot paste into tuplet");
                  else {
                              QByteArray data(ms->data(mimeStaffListFormat));
                              qDebug("paste <%s>", data.data());
                              XmlReader e(data);
                              _score->pasteStaff(e, cr);
                              }
                  }
      else if (ms->hasFormat(mimeSymbolListFormat) && _score->selection().isSingle())
                  qDebug("cannot paste symbol list to element");
      else {
                  qDebug("cannot paste selState %d staffList %d",
                     _score->selection().state(), ms->hasFormat(mimeStaffListFormat));
                  foreach(const QString& s, ms->formats())
                              qDebug("  format %s", qPrintable(s));
                  }

      _score->endCmd();
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
      paste(score);
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

      score->select(m2, SELECT_RANGE, 0);
      QVERIFY(score->selection().canCopy());
      QString mimeType = score->selection().mimeType();
      QVERIFY(!mimeType.isEmpty());
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, score->selection().mimeData());
      QApplication::clipboard()->setMimeData(mimeData);

      score->deselectAll();

      score->select(m2, SELECT_RANGE, 1);
      paste(score);
      score->doLayout();

      QVERIFY(saveCompareScore(score, QString("copypaste%1.mscx").arg(idx),
         DIR + QString("copypaste%1-ref.mscx").arg(idx)));
      delete score;
      }

void TestCopyPaste::copyPastePartial() {
      Score* score = readScore(DIR + QString("copypaste_partial_01.mscx"));
      score->doLayout();

      Measure* m1 = score->firstMeasure();

      Segment* s = m1->first(Segment::SegChordRest);
      s = s->next(Segment::SegChordRest);
      score->select(s->element(0));
      s = s->next(Segment::SegChordRest);
      score->select(s->element(4), SelectType::SELECT_RANGE);

      QVERIFY(score->selection().canCopy());
      QString mimeType = score->selection().mimeType();
      QVERIFY(!mimeType.isEmpty());
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, score->selection().mimeData());
      QApplication::clipboard()->setMimeData(mimeData);

      score->select(m1->first(Segment::SegChordRest)->element(0));
      paste(score);
      score->doLayout();

      QVERIFY(saveCompareScore(score, QString("copypaste_partial_01.mscx"),
         DIR + QString("copypaste_partial_01-ref.mscx")));
      delete score;
}


QTEST_MAIN(TestCopyPaste)
#include "tst_copypaste.moc"

