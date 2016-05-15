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

#define DIR QString("omr/notes/")

using namespace Ms;

//---------------------------------------------------------
//   TestNotes
//---------------------------------------------------------

class TestNotes : public QObject, public MTest
      {
      Q_OBJECT

      void omrFileTest(QString file);

   private slots:
      void initTestCase();
      //void notes2() { omrFileTest("notes2"); }
      //void notes1() { omrFileTest("notes1"); }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestNotes::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   compat_data
//    every "xxx" test requires two *.mscx files:
//          xxx.mscx     is the mscore 1.2 file
//          xxx-ref.mscx is the corresponding (correct)
//                       mscore 2.0 file
//---------------------------------------------------------

void TestNotes::omrFileTest(QString file)
      {
      MasterScore* score = readScore(DIR + file + ".mscx");
      score->doLayout();
      QVERIFY(score);
      savePdf(score, file + ".pdf");
      Score* score1 = readCreatedScore(file + ".pdf");
      QVERIFY(score1);
      score1->doLayout();
      QVERIFY(saveCompareScore(score1, file + ".mscx", DIR + file + "-ref.mscx"));
      }

QTEST_MAIN(TestNotes)
#include "tst_notes.moc"

