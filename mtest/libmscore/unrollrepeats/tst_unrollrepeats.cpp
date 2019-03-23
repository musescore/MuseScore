//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "libmscore/score.h"
#include "mtest/testutils.h"

#define DIR QString("libmscore/unrollrepeats/")

using namespace Ms;

//---------------------------------------------------------
//   TestUnrollRepeats
//---------------------------------------------------------

class TestUnrollRepeats : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void clefKeyTs();
      void pickupMeasure();
      };


QTEST_MAIN(TestUnrollRepeats)
#include "tst_unrollrepeats.moc"

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestUnrollRepeats::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
///   clefKeyTs
///   unroll a score with a complex system of
///   clef, key, time signature changes.
//---------------------------------------------------------

void TestUnrollRepeats::clefKeyTs()
      {
      MasterScore* score = readScore(DIR + "clef-key-ts-test.mscx");

      MasterScore* unrolled = score->unrollRepeats();

      QVERIFY(saveCompareScore(unrolled, "clef-key-ts-test.mscx", DIR + "clef-key-ts-ref.mscx"));
      }

//---------------------------------------------------------
///   pickupMeasure
///   unroll a score with a pickup measure.
///   pickup measure should get merged to a full bar on repeat
//---------------------------------------------------------

void TestUnrollRepeats::pickupMeasure()
      {
      MasterScore* score = readScore(DIR + "pickup-measure-test.mscx");

      MasterScore* unrolled = score->unrollRepeats();

      QVERIFY(saveCompareScore(unrolled, "pickup-measure-test.mscx", DIR + "pickup-measure-ref.mscx"));
      }