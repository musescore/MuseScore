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

#include "libmscore/cursor.h"
#include "libmscore/score.h"

using namespace Ms;

//---------------------------------------------------------
//   TestCursor
//---------------------------------------------------------

class TestCursor : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void testAddNoteTickUpdate();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestCursor::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
///   testAddTickUpdate
///   Add an element (note) and verify the tick updated with the correct amount
//---------------------------------------------------------

void TestCursor::testAddNoteTickUpdate()
      {
      //test setup
      score = new Score();
      score->appendPart("voice");
      score->appendMeasures(2);
      Cursor c(score);
      //creation of cursor - check default values
      QCOMPARE(c.score(), score);
      QCOMPARE(c.track(), 0);
      QCOMPARE(c.voice(), 0);
      QCOMPARE(c.tick(), 0);
      QVERIFY(c.segment() == nullptr);
      //set the cursor at input position (start of score)
      c.rewind(0);
      QVERIFY(c.segment() != nullptr);

      //actual test
      //add 4 times a 1/8th, totalling a half note
      c.setDuration(1, 8);
      c.addNote(60);
      c.addNote(60);
      c.addNote(60);
      c.addNote(60);
      QCOMPARE(c.tick(), MScore::division * 2); //one division == 1 crotchet
      }

QTEST_MAIN(TestCursor)
#include "tst_cursor.moc"

