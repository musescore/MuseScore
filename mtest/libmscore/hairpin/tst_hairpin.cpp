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

#include "libmscore/score.h"
#include "libmscore/hairpin.h"
#include "mtest/testutils.h"

using namespace Ms;

//---------------------------------------------------------
//   TestHairpin
//---------------------------------------------------------

class TestHairpin : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void hairpin();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestHairpin::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   hairpin
//---------------------------------------------------------

void TestHairpin::hairpin()
      {
      Hairpin* hp = new Hairpin(score);

   // subtype
      hp->setHairpinType(Hairpin::DECRESCENDO);
      Hairpin* hp2 = static_cast<Hairpin*>(writeReadElement(hp));
      QCOMPARE(hp2->hairpinType(), Hairpin::DECRESCENDO);
      delete hp2;

      hp->setHairpinType(Hairpin::CRESCENDO);
      hp2 = static_cast<Hairpin*>(writeReadElement(hp));
      QCOMPARE(hp2->hairpinType(), Hairpin::CRESCENDO);
      delete hp2;
      }

QTEST_MAIN(TestHairpin)

#include "tst_hairpin.moc"

