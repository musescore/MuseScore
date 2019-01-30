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
#include "libmscore/score.h"
#include "libmscore/undo.h"
#include "libmscore/measure.h"
#include "libmscore/chord.h"

#define DIR QString("libmscore/tools/")

using namespace Ms;

// Q_LOGGING_CATEGORY(undoRedo, "undoRedo")

//---------------------------------------------------------
//   TestTools
//---------------------------------------------------------

class TestTools : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void undoAddLineBreaks();
//      void undoLockLineBreaks();        // makes assumptions on layout for empty measures
//      void undoRemoveLineBreaks();
      void undoSlashFill();
      void undoSlashRhythm();
      void undoResequenceAlpha();
      void undoResequenceNumeric();
      void undoResequenceMeasure();
      void undoResequencePart();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestTools::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   undoAddLineBreaks
//---------------------------------------------------------

void TestTools::undoAddLineBreaks()
      {
      QString readFile(DIR + "undoAddLineBreaks.mscx");
      QString writeFile1("undoAddLineBreaks01-test.mscx");
      QString reference1(DIR  + "undoAddLineBreaks01-ref.mscx");
      QString writeFile2("undoAddLineBreaks02-test.mscx");
      QString reference2(DIR  + "undoAddLineBreaks02-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();

      // select all
      score->startCmd();
      score->cmdSelectAll();
      score->endCmd();

      // do
      score->startCmd();
      score->addRemoveBreaks(4, false);
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo(&ed);
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

//---------------------------------------------------------
//   undoAddLineBreaks
//---------------------------------------------------------

#if 0
void TestTools::undoLockLineBreaks()
      {
      QString readFile(DIR + "undoLockLineBreaks.mscx");
      QString writeFile1("undoLockLineBreaks01-test.mscx");
      QString reference1(DIR  + "undoLockLineBreaks01-ref.mscx");
      QString writeFile2("undoLockLineBreaks02-test.mscx");
      QString reference2(DIR  + "undoLockLineBreaks02-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();

      // select all
      score->startCmd();
      score->cmdSelectAll();
      score->endCmd();

      // do
      score->startCmd();
      score->addRemoveBreaks(0, true);
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo();
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

//---------------------------------------------------------
//   undoRemoveLineBreaks
//---------------------------------------------------------

void TestTools::undoRemoveLineBreaks()
      {
      QString readFile(DIR + "undoRemoveLineBreaks.mscx");
      QString writeFile1("undoRemoveLineBreaks01-test.mscx");
      QString reference1(DIR  + "undoRemoveLineBreaks01-ref.mscx");
      QString writeFile2("undoRemoveLineBreaks02-test.mscx");
      QString reference2(DIR  + "undoRemoveLineBreaks02-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();

      // select all
      score->startCmd();
      score->cmdSelectAll();
      score->endCmd();

      // do
      score->startCmd();
      score->addRemoveBreaks(0, false);
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo();
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }
#endif

void TestTools::undoSlashFill()
      {
      QString readFile(DIR + "undoSlashFill.mscx");
      QString writeFile1("undoSlashFill01-test.mscx");
      QString reference1(DIR  + "undoSlashFill01-ref.mscx");
      QString writeFile2("undoSlashFill02-test.mscx");
      QString reference2(DIR  + "undoSlashFill02-ref.mscx");

      MasterScore* score = readScore(readFile);

      // select
      Segment* s = score->firstMeasure()->findSegment(SegmentType::ChordRest, Fraction(2,4));
      score->selection().setRange(s, score->lastSegment(), 0, 2);

      // do
      score->startCmd();
      score->cmdSlashFill();
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo(&ed);
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

void TestTools::undoSlashRhythm()
      {
      QString readFile(DIR + "undoSlashRhythm.mscx");
      QString writeFile1("undoSlashRhythm01-test.mscx");
      QString reference1(DIR  + "undoSlashRhythm01-ref.mscx");
      QString writeFile2("undoSlashRhythm02-test.mscx");
      QString reference2(DIR  + "undoSlashRhythm02-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();

      // select all
      score->startCmd();
      score->cmdSelectAll();
      score->endCmd();

      // do
      score->startCmd();
      score->cmdSlashRhythm();
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo(&ed);
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

void TestTools::undoResequenceAlpha()
      {
      QString readFile(DIR + "undoResequenceAlpha.mscx");
      QString writeFile1("undoResequenceAlpha01-test.mscx");
      QString reference1(DIR  + "undoResequenceAlpha01-ref.mscx");
      QString writeFile2("undoResequenceAlpha02-test.mscx");
      QString reference2(DIR  + "undoResequenceAlpha02-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();

      // do
      score->startCmd();
      score->cmdResequenceRehearsalMarks();
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo(&ed);
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

void TestTools::undoResequenceNumeric()
      {
      QString readFile(DIR + "undoResequenceNumeric.mscx");
      QString writeFile1("undoResequenceNumeric01-test.mscx");
      QString reference1(DIR  + "undoResequenceNumeric01-ref.mscx");
      QString writeFile2("undoResequenceNumeric02-test.mscx");
      QString reference2(DIR  + "undoResequenceNumeric02-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();

      // do
      score->startCmd();
      score->cmdResequenceRehearsalMarks();
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo(&ed);
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

void TestTools::undoResequenceMeasure()
      {
      QString readFile(DIR + "undoResequenceMeasure.mscx");
      QString writeFile1("undoResequenceMeasure01-test.mscx");
      QString reference1(DIR  + "undoResequenceMeasure01-ref.mscx");
      QString writeFile2("undoResequenceMeasure02-test.mscx");
      QString reference2(DIR  + "undoResequenceMeasure02-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();

      // do
      score->startCmd();
      score->cmdResequenceRehearsalMarks();
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo(&ed);
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

void TestTools::undoResequencePart()
      {
      QString readFile(DIR + "undoResequencePart.mscx");
      QString writeFile1("undoResequencePart01-test.mscx");
      QString reference1(DIR  + "undoResequencePart01-ref.mscx");
      QString writeFile2("undoResequencePart02-test.mscx");
      QString reference2(DIR  + "undoResequencePart02-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();

      // do
      score->startCmd();
      score->cmdResequenceRehearsalMarks();
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo(&ed);
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

QTEST_MAIN(TestTools)
#include "tst_tools.moc"

