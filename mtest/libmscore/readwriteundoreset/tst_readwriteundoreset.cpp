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

#define DIR QString("libmscore/readwriteundoreset/")

using namespace Ms;

//---------------------------------------------------------
//   TestReadWrite
//---------------------------------------------------------

class TestReadWrite : public QObject, public MTest
      {
      Q_OBJECT
      void readwrite(const QString&);
      void undoreset(const QString&);
      void runtests(const QString& s) { /*readwrite(s); */undoreset(s); }

   private slots:
      void initTestCase();
      void barlines()         { runtests("barlines");          }
      void slurs()            { runtests("slurs");             }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestReadWrite::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   readwrite
//---------------------------------------------------------

void TestReadWrite::readwrite(const QString& file)
      {
      QString readFile(DIR   + file + ".mscx");
      QString writeFile(file + "-readwrite-test.mscx");

      MasterScore* score = readScore(readFile);
      QVERIFY(score);
      QVERIFY(saveCompareScore(score, writeFile, readFile));
      }

//---------------------------------------------------------
//   undoreset
//---------------------------------------------------------

void TestReadWrite::undoreset(const QString& file)
      {
      QString readFile(DIR   + file + ".mscx");
      QString writeFile(file + "-undoreset-test.mscx");

      MasterScore* score = readScore(readFile);
      QVERIFY(score);
      score->cmdResetAllPositions();
      score->undoRedo(/* undo */ true, nullptr);
      QVERIFY(saveCompareScore(score, writeFile, readFile));
      }

QTEST_MAIN(TestReadWrite)
#include "tst_readwriteundoreset.moc"
