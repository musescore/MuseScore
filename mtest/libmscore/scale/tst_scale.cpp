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
#include "libmscore/scale.h"

#define DIR QString("libmscore/scale/")

using namespace Ms;

//---------------------------------------------------------
//   TestScale
//---------------------------------------------------------

class TestScale : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void testStandardScale();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestScale::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   testStandardScale
//---------------------------------------------------------

void TestScale::testStandardScale()
      {
      Scale scale;
      int* computedTunings = scale.getComputedTunings();

      for (int index = 0; index <= TPC_NUM_OF; index++)
            QVERIFY(computedTunings[index] == 0);
      }

QTEST_MAIN(TestScale)
#include "tst_scale.moc"
