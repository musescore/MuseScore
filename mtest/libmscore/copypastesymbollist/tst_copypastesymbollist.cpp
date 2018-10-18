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
//   TestCopyPasteSymbolList
//---------------------------------------------------------

class TestCopyPasteSymbolList : public QObject, public MTest
      {
      Q_OBJECT

      void copypastecommon(MasterScore*, const char*);
      void copypaste(const char*, ElementType);
      void copypastepart(const char*, ElementType);
      void copypastedifferentvoice(const char*, ElementType);

   private slots:
      void initTestCase();
      void copypasteArticulation()  { copypaste("articulation", ElementType::ARTICULATION); }
      void copypasteChordNames()    { copypaste("chordnames", ElementType::HARMONY); }
      void copypasteChordNames1()   { copypaste("chordnames-01", ElementType::HARMONY); }
      void copypasteFiguredBass() {} //   { copypaste("figuredbass", ElementType::FIGURED_BASS); }
      void copypasteLyrics()        { copypaste("lyrics", ElementType::LYRICS); }

      void copypasteRange()         { copypastepart("range", ElementType::ARTICULATION); }
      void copypasteRange1()        { copypastedifferentvoice("range-01", ElementType::ARTICULATION); }

      void copypasteArticulationRest()   { copypaste("articulation-rest", ElementType::ARTICULATION); }
//      void copypasteFermataRest()        { copypaste("fermata-rest", ElementType::ARTICULATION); }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestCopyPasteSymbolList::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   copypastecommon
//   copy and paste to first chord in measure 4
//---------------------------------------------------------

void TestCopyPasteSymbolList::copypastecommon(MasterScore* score, const char* name)
      {
      // copy selection to clipboard
      QVERIFY(score->selection().canCopy());
      QString mimeType = score->selection().mimeType();
      QVERIFY(!mimeType.isEmpty());
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, score->selection().mimeData());
      QApplication::clipboard()->setMimeData(mimeData);

      // select first chord in 5th measure
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
      score->cmdPaste(ms,0);
      score->endCmd();
      score->doLayout();

      QVERIFY(saveCompareScore(score, QString("copypastesymbollist-%1.mscx").arg(name),
         DIR + QString("copypastesymbollist-%1-ref.mscx").arg(name)));
      delete score;
      }

//---------------------------------------------------------
//   copypaste
//    select all elements of type and copy paste
//---------------------------------------------------------

void TestCopyPasteSymbolList::copypaste(const char* name, ElementType type)
      {
      MasterScore* score = readScore(DIR + QString("copypastesymbollist-%1.mscx").arg(name));
      // score->doLayout();

      Element* el = Element::create(type, score);
      score->selectSimilar(el, false);
      delete el;

      copypastecommon(score, name);
      }

//---------------------------------------------------------
//   copypastepart
//    select all elements of type in 2 first measures
//    in the first staff and copy paste
//---------------------------------------------------------

void TestCopyPasteSymbolList::copypastepart(const char* name, ElementType type)
      {
      MasterScore* score = readScore(DIR + QString("copypastesymbollist-%1.mscx").arg(name));
      score->doLayout();

      //select all
      score->select(score->firstMeasure());
      score->select(score->firstMeasure()->nextMeasure(),SelectType::RANGE);


      Element* el = Element::create(type,score);
      score->selectSimilarInRange(el);
      delete el;

      copypastecommon(score,name);
      }

//---------------------------------------------------------
//   copypastedifferentvoice
//    select all elements of type in 2 first measures
//    in both staves and copy paste
//---------------------------------------------------------

void TestCopyPasteSymbolList::copypastedifferentvoice(const char* name, ElementType type)
      {
      MasterScore* score = readScore(DIR + QString("copypastesymbollist-%1.mscx").arg(name));
      score->doLayout();

      //select all
      score->select(score->firstMeasure());
      score->select(score->firstMeasure()->nextMeasure(),SelectType::RANGE, 1);

      Element* el = Element::create(type,score);
      score->selectSimilarInRange(el);
      delete el;

      copypastecommon(score,name);

      }

QTEST_MAIN(TestCopyPasteSymbolList)
#include "tst_copypastesymbollist.moc"

