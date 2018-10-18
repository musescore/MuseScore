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
#include "libmscore/dynamic.h"
#include "mtest/testutils.h"

using namespace Ms;

//---------------------------------------------------------
//   TestDynamic
//---------------------------------------------------------

class TestDynamic : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void test1();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestDynamic::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   test1
//    read write test
//---------------------------------------------------------

void TestDynamic::test1()
      {
      Dynamic* dynamic = new Dynamic(score);
      dynamic->setDynamicType(Dynamic::Type(1));

      Dynamic* d;

      dynamic->setPlacement(Placement::ABOVE);
      dynamic->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
      d = toDynamic(writeReadElement(dynamic));
      QCOMPARE(d->placement(), Placement::ABOVE);
      delete d;

      dynamic->setPlacement(Placement::BELOW);
      dynamic->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->placement(), Placement::BELOW);
      delete d;

      dynamic->setProperty(Pid::PLACEMENT, int(Placement::ABOVE));
      dynamic->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->placement(), Placement::ABOVE);
      delete d;

      dynamic->setProperty(Pid::PLACEMENT, int(Placement::BELOW));
      dynamic->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->placement(), Placement::BELOW);
      delete d;

      dynamic->setVelocity(23);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->velocity(), 23);
      delete d;

      dynamic->setVelocity(57);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->velocity(), 57);
      delete d;

      dynamic->setProperty(Pid::VELOCITY, 23);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->velocity(), 23);
      delete d;

      dynamic->setProperty(Pid::VELOCITY, 57);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->velocity(), 57);
      delete d;

      dynamic->setDynRange(Dynamic::Range::STAFF);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->dynRange(), Dynamic::Range::STAFF);
      delete d;

      dynamic->setDynRange(Dynamic::Range::PART);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->dynRange(), Dynamic::Range::PART);
      delete d;

      dynamic->setDynRange(Dynamic::Range::SYSTEM);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->dynRange(), Dynamic::Range::SYSTEM);
      delete d;

      dynamic->setProperty(Pid::DYNAMIC_RANGE, int(Dynamic::Range::STAFF));
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->dynRange(), Dynamic::Range::STAFF);
      delete d;

      dynamic->setProperty(Pid::DYNAMIC_RANGE, int(Dynamic::Range::PART));
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->dynRange(), Dynamic::Range::PART);
      delete d;

      dynamic->setDynRange(Dynamic::Range::SYSTEM);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->dynRange(), Dynamic::Range::SYSTEM);
      delete d;


      }

QTEST_MAIN(TestDynamic)

#include "tst_dynamic.moc"

