//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/score.h"
#include "libmscore/system.h"
#include "libmscore/measure.h"
#include "libmscore/chord.h"
#include "libmscore/xml.h"

#define DIR QString("libmscore/copypastesymbollist/")

using namespace Ms;

//---------------------------------------------------------
//   TestCopyPaste
//---------------------------------------------------------

class TestCopyPasteSymbolList : public QObject, public MTest
      {
      Q_OBJECT

      void copypaste(const char*, Element::Type);

   private slots:
      void initTestCase();
      void copypasteArticulation()  { copypaste("articulation", Element::Type::ARTICULATION); }
      void copypasteChordNames()    { copypaste("chordnames", Element::Type::HARMONY); }
      void copypasteChordNames1()    { copypaste("chordnames-01", Element::Type::HARMONY); }
      void copypasteFiguredBass()   { copypaste("figuredbass", Element::Type::FIGURED_BASS); }
      void copypasteLyrics()        { copypaste("lyrics", Element::Type::LYRICS); }

      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestCopyPasteSymbolList::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   copypaste
//    copy measure 2, paste into measure 4
//---------------------------------------------------------

void TestCopyPasteSymbolList::copypaste(const char* name, Element::Type type)
      {
      Score* score = readScore(DIR + QString("copypastesymbollist-%1.mscx").arg(name));
      score->doLayout();

      Element* el = Element::create(type,score);
      score->selectSimilar(el,false);
      delete el;

      // copy selection to clipboard
      QVERIFY(score->selection().canCopy());
      QString mimeType = score->selection().mimeType();
      QVERIFY(!mimeType.isEmpty());
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, score->selection().mimeData());
      QApplication::clipboard()->setMimeData(mimeData);

      // select first chord in 4th measure
      Measure* m = score->firstMeasure();
      for (int i=0; i < 4; i++)
            m = m->nextMeasure();
      score->select(m->first()->element(0));

      score->startCmd();
      const QMimeData* ms = QApplication::clipboard()->mimeData();
      if (!ms->hasFormat(mimeSymbolListFormat)) {
            qDebug("wrong type mime data");
            return;
            }

      PasteStatus status = score->cmdPaste(ms,0);
      switch (status) {
            case PasteStatus::NO_DEST:
                  qDebug("no destination chord"); return;
            case PasteStatus::DEST_TUPLET:
                  qDebug("cannot paste mid-tuplet"); return;
            default: ;
      }

      score->endCmd();
      score->doLayout();

      QVERIFY(saveCompareScore(score, QString("copypastesymbollist-%1.mscx").arg(name),
         DIR + QString("copypastesymbollist-%1-ref.mscx").arg(name)));
      delete score;
      }


QTEST_MAIN(TestCopyPasteSymbolList)
#include "tst_copypastesymbollist.moc"

