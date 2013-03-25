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
#include "libmscore/accidental.h"
#include "libmscore/chord.h"

#define DIR QString("libmscore/keysig/")


//---------------------------------------------------------
//   TestKeySig
//---------------------------------------------------------

class TestKeySig : public QObject, public MTest
      {
      Q_OBJECT

      Score* score;
      Measure* m2;

      void checkNotes(int, Accidental::AccidentalType);

   private slots:
      void initTestCase();
      void keysig();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestKeySig::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   checkNotes
//
///   check if notes of measures 2-4 with given
///   pitch have specified accidental or none
//---------------------------------------------------------

void TestKeySig::checkNotes(int pitch, Accidental::AccidentalType type)
      {
      for (Measure* m = m2; m; m = m->nextMeasure()) {
            for (Segment* s = m->first(); s; s = s->next()) {
                  if (s->segmentType() & (Segment::SegChordRest)) {
                        Chord* chord = static_cast<Chord*>(s->element(0));
                        for (int i = 0; i < chord->notes().size(); ++i) {
                              Note* note = chord->notes().at(i);
                              if (note->pitch() == pitch) {
                                    if (type == Accidental::ACC_NONE) {
                                          QVERIFY(note->accidental() == 0);
                                          }
                                    else {
                                          QVERIFY(note->accidental());
                                          QVERIFY(note->accidental()->accidentalType() == type);
                                          }
                                    }
                              }
                        }
                  }
            }
      }
           

//---------------------------------------------------------
//   keysig
//---------------------------------------------------------

void TestKeySig::keysig()
      {
      QString writeFile1("keysig01-test.mscx");
      QString reference1(DIR  + "keysig01-ref.mscx");
      QString writeFile2("keysig02-test.mscx");
      QString reference2(DIR  + "keysig02-ref.mscx");
      QString writeFile3("keysig03-test.mscx");
      QString reference3(DIR  + "keysig.mscx");
      QString writeFile4("keysig04-test.mscx");
      QString reference4(DIR  + "keysig02-ref.mscx");
      QString writeFile5("keysig05-test.mscx");
      QString reference5(DIR  + "keysig01-ref.mscx");
      QString writeFile6("keysig06-test.mscx");
      QString reference6(DIR  + "keysig.mscx");

      // read file
      score = readScore(DIR + "keysig.mscx");
      m2 = score->firstMeasure()->nextMeasure();

      checkNotes(60, Accidental::ACC_NONE); // check C
      checkNotes(66, Accidental::ACC_SHARP); // check F#
      checkNotes(71, Accidental::ACC_NONE); // check B

      // add a key signature in measure 2
      KeySigEvent ke2(2);
      score->startCmd();
      score->undoChangeKeySig(score->staff(0), m2->tick(), ke2);
      score->endCmd();

      checkNotes(60, Accidental::ACC_NATURAL); // check C
      checkNotes(66, Accidental::ACC_NONE); // check F#
      checkNotes(71, Accidental::ACC_NONE); // check B
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // change key signature in measure 2
      KeySigEvent ke_3(-3);
      score->startCmd();
      score->undoChangeKeySig(score->staff(0), m2->tick(), ke_3);
      score->endCmd();

      checkNotes(60, Accidental::ACC_NONE); // check C
      checkNotes(66, Accidental::ACC_SHARP); // check F#
      checkNotes(71, Accidental::ACC_NATURAL); // check B
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      // remove key signature in measure 2
      for (Segment* s = m2->first(); s; s = s->next()) {
            if (s->segmentType() & (Segment::SegKeySig)) {
                  Element* e=s->element(0);
                  score->startCmd();
                  score->undoRemoveElement(e);
                  score->endCmd();
                  break;
                  }
            }
      checkNotes(60, Accidental::ACC_NONE); // check C
      checkNotes(66, Accidental::ACC_SHARP); // check F#
      checkNotes(71, Accidental::ACC_NONE); // check B
      QVERIFY(saveCompareScore(score, writeFile3, reference3));

      // undo remove
      score->undo()->undo();
      checkNotes(60, Accidental::ACC_NONE); // check C
      checkNotes(66, Accidental::ACC_SHARP); // check F#
      checkNotes(71, Accidental::ACC_NATURAL); // check B
      QVERIFY(saveCompareScore(score, writeFile4, reference4));

      // undo change
      score->undo()->undo();
      checkNotes(60, Accidental::ACC_NATURAL); // check C
      checkNotes(66, Accidental::ACC_NONE); // check F#
      checkNotes(71, Accidental::ACC_NONE); // check B
      QVERIFY(saveCompareScore(score, writeFile5, reference5));

      // undo add
      score->undo()->undo();
      checkNotes(60, Accidental::ACC_NONE); // check C
      checkNotes(65, Accidental::ACC_SHARP); // check F#
      checkNotes(71, Accidental::ACC_NONE); // check B
      QVERIFY(saveCompareScore(score, writeFile6, reference6));

      delete score;
      }

QTEST_MAIN(TestKeySig)
#include "tst_keysig.moc"
