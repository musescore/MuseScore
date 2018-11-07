//=============================================================================
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
#include "libmscore/breath.h"
#include "libmscore/sym.h"

#define DIR QString("libmscore/breath/")

using namespace Ms;

//---------------------------------------------------------
//   TestBreath
//---------------------------------------------------------

class TestBreath : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void breath();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestBreath::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   breath
//---------------------------------------------------------

void TestBreath::breath()
      {
      QString readFile(DIR + "breath.mscx");
      QString writeFile1("breath01-test.mscx");
      QString reference1(DIR  + "breath01-ref.mscx");
      QString writeFile2("breath02-test.mscx");
      QString reference2(DIR  + "breath02-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();

      // do
      score->startCmd();
      score->cmdSelectAll();
      for (Element* e : score->selection().elements()) {
            EditData dd(0);
            dd.view = 0;
            Breath* b = new Breath(score);
            b->setSymId(SymId::breathMarkComma);
            dd.element = b;
            if (e->acceptDrop(dd))
                  e->drop(dd);
          }
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo(0);
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

QTEST_MAIN(TestBreath)
#include "tst_breath.moc"

