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
#include "libmscore/measure.h"
#include "libmscore/chord.h"

#define DIR QString("libmscore/implode_explode/")

using namespace Ms;

//---------------------------------------------------------
//   TestImplodeExplode
//---------------------------------------------------------

class TestImplodeExplode : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void undoExplode();
      void undoExplodeVoices();
      void undoExplode1();
      void undoImplode();
      void undoImplodeVoice();
      void implode1();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestImplodeExplode::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   undoExplode
//---------------------------------------------------------

void TestImplodeExplode::undoExplode()
      {
      QString readFile(DIR + "undoExplode.mscx");
      QString writeFile1("undoExplode01-test.mscx");
      QString reference1(DIR  + "undoExplode01-ref.mscx");
      QString writeFile2("undoExplode02-test.mscx");
      QString reference2(DIR  + "undoExplode02-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();

      // select all
      score->startCmd();
      score->cmdSelectAll();
      score->endCmd();

      // do
      score->startCmd();
      score->cmdExplode();
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo();
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

//---------------------------------------------------------
//   undoExplodeVoices
//---------------------------------------------------------

void TestImplodeExplode::undoExplodeVoices()
      {
      QString readFile(DIR + "undoExplode.mscx");
      QString writeFile1("undoExplode01-test.mscx");
      QString reference1(DIR  + "undoExplode01-ref.mscx");
      QString writeFile2("undoExplode02-test.mscx");
      QString reference2(DIR  + "undoExplode02-ref.mscx");
    }

//---------------------------------------------------------
//   undoImplode
//---------------------------------------------------------

void TestImplodeExplode::undoImplode()
      {
      QString readFile(DIR + "undoImplode.mscx");
      QString writeFile1("undoImplode01-test.mscx");
      QString reference1(DIR  + "undoImplode01-ref.mscx");
      QString writeFile2("undoImplode02-test.mscx");
      QString reference2(DIR  + "undoImplode02-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();

      // select all
      score->startCmd();
      score->cmdSelectAll();
      score->endCmd();

      // do
      score->startCmd();
      score->cmdImplode();
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo();
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

//---------------------------------------------------------
//   undoExplode1
//---------------------------------------------------------

void TestImplodeExplode::undoExplode1()
      {
      QString readFile(DIR + "explode1.mscx");
      QString writeFile1("explode1-test.mscx");
      QString reference1(DIR  + "explode1-ref.mscx");
      QString writeFile2("explode1-test2.mscx");
      QString reference2(DIR  + "explode1-ref2.mscx");
    }

//---------------------------------------------------------
//   undoImplodeVoice
//---------------------------------------------------------

void TestImplodeExplode::undoImplodeVoice()
      {
      QString readFile(DIR + "undoImplodeVoice.mscx");
      QString writeFile1("undoImplodeVoice01-test.mscx");
      QString reference1(DIR  + "undoImplodeVoice01-ref.mscx");
      QString writeFile2("undoImplodeVoice02-test.mscx");
      QString reference2(DIR  + "undoImplodeVoice02-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();

      // select all
      score->startCmd();
      score->cmdSelectAll();
      score->endCmd();

      // do
      score->startCmd();
      score->cmdImplode();
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo();
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

//---------------------------------------------------------
//   implode1
//---------------------------------------------------------

void TestImplodeExplode::implode1()
      {
      QString readFile(DIR + "implode1.mscx");
      QString writeFile1("implode1-test1.mscx");
      QString writeFile2("implode1-test2.mscx");
      QString reference(DIR + "implode1-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();

      // select all
      score->startCmd();
      score->cmdSelectAll();
      score->endCmd();

      // do
      score->startCmd();
      score->cmdImplode();
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference));

      // undo
      score->undoStack()->undo();
      QVERIFY(saveCompareScore(score, writeFile2, readFile));

      delete score;
      }

QTEST_MAIN(TestImplodeExplode)

#include "tst_implodeExplode.moc"

