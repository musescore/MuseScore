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

#define DIR QString("libmscore/compat/")


//---------------------------------------------------------
//   TestCompat
//---------------------------------------------------------

class TestCompat : public QObject, public MTest
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

void TestCompat::initTestCase()
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

void TestCompat::compat_data()
      {
      QTest::addColumn<QString>("file");

      QTest::newRow("notes") <<  "notes";       // notes.mscx notes-ref.mscx
      QTest::newRow("keysig") << "keysig";
      QTest::newRow("articulations") << "articulations";
      }

//---------------------------------------------------------
//   compat
//---------------------------------------------------------

void TestCompat::compat()
      {
      QFETCH(QString, file);

      QString readFile(DIR   + file + ".mscx");
      QString writeFile(file + "-test.mscx");
      QString reference(DIR  + file + "-ref.mscx");

      Score* score = readScore(readFile);
      QVERIFY(score);
      QVERIFY(saveCompareScore(score, writeFile, reference));
      }

QTEST_MAIN(TestCompat)
#include "tst_compat.moc"

