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

#define DIR QString("libmscore/tools/")

using namespace Ms;

//---------------------------------------------------------
//   TestTools
//---------------------------------------------------------

class TestTools : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void undoExplode();
      void undoImplode();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestTools::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   undoExplode
//---------------------------------------------------------

void TestTools::undoExplode()
      {
      QString readFile(DIR + "undoExplode.mscx");
      QString writeFile1("undoExplode01-test.mscx");
      QString reference1(DIR  + "undoExplode01-ref.mscx");
      QString writeFile2("undoExplode02-test.mscx");
      QString reference2(DIR  + "undoExplode02-ref.mscx");

      Score* score = readScore(readFile);

      // select all
      score->cmdSelectAll();

      score->startCmd();
      score->cmdExplode();
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undo()->undo();
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

void TestTools::undoImplode()
      {
      QString readFile(DIR + "undoImplode.mscx");
      QString writeFile1("undoImplode01-test.mscx");
      QString reference1(DIR  + "undoImplode01-ref.mscx");
      QString writeFile2("undoImplode02-test.mscx");
      QString reference2(DIR  + "undoImplode02-ref.mscx");

      Score* score = readScore(readFile);

      // select all
      score->cmdSelectAll();

      score->startCmd();
      score->cmdImplode();
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undo()->undo();
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

QTEST_MAIN(TestTools)
#include "tst_tools.moc"

