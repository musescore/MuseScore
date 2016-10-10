
//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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
//   TestSfzComments
//---------------------------------------------------------

class TestSfzComments : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void testcomments();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSfzComments::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   testcomments
//---------------------------------------------------------

void TestSfzComments::testcomments()
      {
      Zerberus* synth = new Zerberus();
      Ms::preferences.mySoundfontsPath += ";" + root;
      synth->loadInstrument("commentTest.sfz");

      QCOMPARE(synth->instrument(0)->zones().size(), (size_t) 3);

      std::list<Zone *>::iterator curZone = synth->instrument(0)->zones().begin();
      QCOMPARE((*curZone)->keyLo, (char) 60);
      QCOMPARE((*curZone)->keyHi, (char) 70);
      QCOMPARE((*curZone)->keyBase, (char) 40);
      curZone++;
      QCOMPARE((*curZone)->keyLo, (char) 23);
      QCOMPARE((*curZone)->keyHi, (char) 42);
      QCOMPARE((*curZone)->keyBase, (char) 40);
      curZone++;
      QCOMPARE((*curZone)->keyLo, (char) 42);
      QCOMPARE((*curZone)->keyHi, (char) 23);
      QCOMPARE((*curZone)->keyBase, (char) 40);
      }

QTEST_MAIN(TestSfzComments)

#include "tst_sfzcomments.moc"


