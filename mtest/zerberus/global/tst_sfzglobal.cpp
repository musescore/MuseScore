
//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>

#include "mtest/testutils.h"

#include "zerberus/instrument.h"
#include "zerberus/zerberus.h"
#include "zerberus/zone.h"
#include "mscore/preferences.h"

using namespace Ms;

//---------------------------------------------------------
//   TestSfzGlobal
//---------------------------------------------------------

class TestSfzGlobal : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void testglobal();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSfzGlobal::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   testglobal
//---------------------------------------------------------

void TestSfzGlobal::testglobal()
      {
      Zerberus* synth = new Zerberus();
      preferences.setPreference(PREF_APP_PATHS_MYSOUNDFONTS, root);
      synth->loadInstrument("globalTest.sfz");
      QCOMPARE(synth->instrument(0)->zones().size(), (size_t) 2);
      QCOMPARE(synth->instrument(0)->zones().front()->keyLo, (char) 40);
      QCOMPARE(synth->instrument(0)->zones().front()->keyHi, (char) 50);
      QCOMPARE(synth->instrument(0)->zones().front()->keyBase, (char) 40);
      QCOMPARE(synth->instrument(0)->zones().back()->keyLo, (char) 60);
      QCOMPARE(synth->instrument(0)->zones().back()->keyHi, (char) 70);
      QCOMPARE(synth->instrument(0)->zones().back()->keyBase, (char) 40);
      }

QTEST_MAIN(TestSfzGlobal)

#include "tst_sfzglobal.moc"


