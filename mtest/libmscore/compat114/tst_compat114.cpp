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

#define DIR QString("libmscore/compat114/")

using namespace Ms;

//---------------------------------------------------------
//   TestCompat114
//---------------------------------------------------------

class TestCompat114 : public QObject, public MTest
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

void TestCompat114::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   compat_data
//    every "xxx" test requires two *.mscx files:
//          xxx.mscx     is the mscore 1.x file
//          xxx-ref.mscx is the corresponding (correct)
//                       mscore 3.0 file
//---------------------------------------------------------

void TestCompat114::compat_data()
      {
      QTest::addColumn<QString>("file");

      QTest::newRow("notes") <<  "notes";       // notes.mscx notes-ref.mscx
      QTest::newRow("noteheads") << "noteheads";
      QTest::newRow("keysig") << "keysig";
      QTest::newRow("hairpin") << "hairpin";
      QTest::newRow("articulations") << "articulations";
      QTest::newRow("textstyles") << "textstyles";
      QTest::newRow("pedal") << "pedal";
      QTest::newRow("textline") << "textline";
      QTest::newRow("ottava") << "ottava";
      QTest::newRow("title") << "title";
      QTest::newRow("fingering") << "fingering";
//TODO      QTest::newRow("notes_useroffset") << "notes_useroffset";
      QTest::newRow("tremolo2notes") << "tremolo2notes";
      QTest::newRow("accidentals") << "accidentals";
      QTest::newRow("slurs") << "slurs";
      QTest::newRow("clefs") << "clefs";
      QTest::newRow("clef_missing_first") << "clef_missing_first";
      QTest::newRow("hor_frame_and_mmrest") << "hor_frame_and_mmrest";
      QTest::newRow("chord_symbol") << "chord_symbol";
      QTest::newRow("style") << "style";
      QTest::newRow("text_scaling") << "text_scaling";
//TODO      QTest::newRow("markers") << "markers";
      QTest::newRow("drumset") << "drumset";
      QTest::newRow("tuplets") << "tuplets";
      QTest::newRow("tuplets_1") << "tuplets_1";
      QTest::newRow("tuplets_2") << "tuplets_2";
      }

//---------------------------------------------------------
//   compat
//---------------------------------------------------------

void TestCompat114::compat()
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

QTEST_MAIN(TestCompat114)
#include "tst_compat114.moc"

