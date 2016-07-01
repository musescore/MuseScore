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
#include "libmscore/soundbank.h"


using namespace Ms;

//---------------------------------------------------------
//   TestInstrumentChange
//---------------------------------------------------------

class TestSoundbank : public QObject, public MTest {
      Q_OBJECT

   private slots:
      void initTestCase();
      void testSoundbank();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSoundbank::initTestCase()
      {
      initMTest();
      }

void TestSoundbank::testSoundbank()
      {
      addToSoundBanks(root + "/libmscore/soundbank/soundbank.xml");
      }

QTEST_MAIN(TestSoundbank)
#include "tst_soundbank.moc"
