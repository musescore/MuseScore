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

#define DIR QString("libmscore/compat206/")

using namespace Ms;

//---------------------------------------------------------
//   TestCompat206
//---------------------------------------------------------

class TestCompat206 : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void compat_data();
      void compat();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestCompat206::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   compat_data
//    every "xxx" test requires two *.mscx files:
//          xxx.mscx     is the mscore 2.x file
//          xxx-ref.mscx is the corresponding (correct)
//                       mscore 3.0 file
//---------------------------------------------------------

void TestCompat206::compat_data()
      {
      QTest::addColumn<QString>("file");
      QTest::newRow("accidentals") <<  "accidentals";
      QTest::newRow("articulations") <<  "articulations";
      QTest::newRow("breath") <<  "breath";
      QTest::newRow("clefs") <<  "clefs";
      QTest::newRow("markers") <<  "markers";
      QTest::newRow("textstyles") <<  "textstyles";
      }

//---------------------------------------------------------
//   compat
//---------------------------------------------------------

void TestCompat206::compat()
      {
      QFETCH(QString, file);

      QString readFile(DIR   + file + ".mscx");
      QString writeFile(file + "-test.mscx");
      QString reference(DIR  + file + "-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();
      QVERIFY(score);
      QVERIFY(saveCompareScore(score, writeFile, reference));
      }

QTEST_MAIN(TestCompat206)
#include "tst_compat206.moc"

