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
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/keysig.h"
#include "libmscore/undo.h"

#define DIR QString("libmscore/keysig/")

using namespace Ms;

//---------------------------------------------------------
//   TestKeySig
//---------------------------------------------------------

class TestKeySig : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void keysig();
      void concertPitch();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestKeySig::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   keysig
//---------------------------------------------------------

void TestKeySig::keysig()
      {
      QString writeFile1("keysig01-test.mscx");
      QString reference1(DIR  + "keysig01-ref.mscx");  // with D maj
      QString writeFile2("keysig02-test.mscx");
      QString reference2(DIR  + "keysig02-ref.mscx");  // with Eb maj
      QString writeFile3("keysig03-test.mscx");
      QString reference3(DIR  + "keysig.mscx");        // orig
      QString writeFile4("keysig04-test.mscx");
      QString reference4(DIR  + "keysig02-ref.mscx");  // with Eb maj
      QString writeFile5("keysig05-test.mscx");
      QString reference5(DIR  + "keysig01-ref.mscx");  // with D maj
      QString writeFile6("keysig06-test.mscx");
      QString reference6(DIR  + "keysig.mscx");        // orig

      // read file
      Score* score = readScore(DIR + "keysig.mscx");
      Measure* m2 = score->firstMeasure()->nextMeasure();

      // add a key signature (D major) in measure 2
      KeySigEvent ke2;
      ke2.setKey(Key::D);
      score->startCmd();
      score->undoChangeKeySig(score->staff(0), m2->tick(), ke2);
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // change key signature in measure 2 to E flat major
      KeySigEvent ke_3;
      ke_3.setKey(Key(-3));
      score->startCmd();
      score->undoChangeKeySig(score->staff(0), m2->tick(), ke_3);
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      // remove key signature in measure 2
      Segment* s = m2->first();
      while (!(s->segmentType() & (Segment::Type::KeySig)))
            s = s->next();
      Element* e=s->element(0);
      score->startCmd();
      score->undoRemoveElement(e);
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile3, reference3));

      // undo remove
      score->undo()->undo();
      QVERIFY(saveCompareScore(score, writeFile4, reference4));

      // undo change
      score->undo()->undo();
      QVERIFY(saveCompareScore(score, writeFile5, reference5));

      // undo add
      score->undo()->undo();
      QVERIFY(saveCompareScore(score, writeFile6, reference6));

      delete score;
      }

void TestKeySig::concertPitch()
      {
      Score* score = readScore(DIR + "concert-pitch.mscx");
      score->doLayout();
      score->cmdConcertPitchChanged(true, true);
      QVERIFY(saveCompareScore(score, "concert-pitch-01-test.mscx", DIR + "concert-pitch-01-ref.mscx"));
      score->cmdConcertPitchChanged(false, true);
      QVERIFY(saveCompareScore(score, "concert-pitch-02-test.mscx", DIR + "concert-pitch-02-ref.mscx"));
      }

QTEST_MAIN(TestKeySig)
#include "tst_keysig.moc"
