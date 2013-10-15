//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/score.h"
#include "mscore/preferences.h"

#define DIR QString("guitarpro/")

using namespace Ms;

//---------------------------------------------------------
//   TestGuitarPro
//---------------------------------------------------------

class TestGuitarPro : public QObject, public MTest
      {
      Q_OBJECT

      void gpReadTest(const char* file,  const char* ext);
      
private slots:
      void initTestCase();
      void gpTestIrrTuplet() { gpReadTest("testIrrTuplet", "gp4"); }
      void gpSlur() { gpReadTest("slur", "gp4"); }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestGuitarPro::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   gpReadTest
//   read a Capella file, write to a MuseScore file and verify against reference
//---------------------------------------------------------

void TestGuitarPro::gpReadTest(const char* file, const char* ext)
      {
      Score* score = readScore(DIR + file + "." + ext);
      QVERIFY(score);
      score->doLayout();
      QVERIFY(saveCompareScore(score, QString("%1.%2.mscx").arg(file).arg(ext),
                               DIR + QString("%1.%2-ref.mscx").arg(file).arg(ext)));
      delete score;
      }

QTEST_MAIN(TestGuitarPro)
#include "tst_guitarpro.moc"
