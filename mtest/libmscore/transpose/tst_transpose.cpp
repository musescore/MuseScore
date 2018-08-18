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

#define DIR QString("libmscore/transpose/")

using namespace Ms;

//---------------------------------------------------------
//   TestTranspose
//---------------------------------------------------------

class TestTranspose : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void undoTranspose();
      void undoDiatonicTranspose();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestTranspose::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   undoTranspose
//---------------------------------------------------------

void TestTranspose::undoTranspose()
      {
      QString readFile(DIR + "undoTranspose.mscx");
      QString writeFile1("undoTranspose01-test.mscx");
      QString reference1(DIR  + "undoTranspose01-ref.mscx");
      QString writeFile2("undoTranspose02-test.mscx");
      QString reference2(DIR  + "undoTranspose02-ref.mscx");

      MasterScore* score = readScore(readFile);

      // select all
      score->cmdSelectAll();

      // transpose major second up
      score->startCmd();
      score->transpose(TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 4,
                       true, true, true, false);
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo(&ed);
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

//---------------------------------------------------------
//   undoDiatonicTranspose
//---------------------------------------------------------

void TestTranspose::undoDiatonicTranspose()
      {
      QString readFile(DIR + "undoDiatonicTranspose.mscx");
      QString writeFile1("undoDiatonicTranspose01-test.mscx");
      QString reference1(DIR  + "undoDiatonicTranspose01-ref.mscx");
      QString writeFile2("undoDiatonicTranspose02-test.mscx");
      QString reference2(DIR  + "undoDiatonicTranspose02-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();

      // select all
      score->cmdSelectAll();

      // transpose diatonic fourth down
      score->startCmd();
      score->transpose(TransposeMode::DIATONICALLY, TransposeDirection::DOWN, Key::C, 3,
                       true, false, false, false);
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo(&ed);
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

QTEST_MAIN(TestTranspose)
#include "tst_transpose.moc"

