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
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "mtest/testutils.h"

//---------------------------------------------------------
//   TestNote
//---------------------------------------------------------

class TestNote : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void note();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestNote::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   note
//    read write test of note
//---------------------------------------------------------

void TestNote::note()
      {
      Chord* chord = new Chord(score);
      Note* note = new Note(score);
      chord->add(note);

   // pitch
      note->setPitch(33);
      note->setTpcFromPitch();
      Note* n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->pitch(), 33);
      delete n;

   // tpc
      note->setTpc(22);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->tpc(), 22);
      delete n;

   // small
      note->setSmall(true);
      n = static_cast<Note*>(writeReadElement(note));
      QVERIFY(n->small());
      delete n;

   // mirror
      note->setUserMirror(DH_LEFT);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), DH_LEFT);
      delete n;

      note->setUserMirror(DH_RIGHT);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), DH_RIGHT);
      delete n;

      note->setUserMirror(DH_AUTO);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), DH_AUTO);
      delete n;

   // dot position
      note->setDotPosition(UP);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->dotPosition(), UP);
      delete n;

      note->setDotPosition(DOWN);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->dotPosition(), DOWN);
      delete n;

      note->setDotPosition(AUTO);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->dotPosition(), AUTO);
      delete n;

  // onTimeUserOffset
      note->setOnTimeUserOffset(12);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->onTimeUserOffset(), 12);
      delete n;

  // offTimeUserOffset
      note->setOffTimeUserOffset(21);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->offTimeUserOffset(), 21);
      delete n;

  // headGroup
      for (int i = 0; i < HEAD_GROUPS; ++i) {
            note->setHeadGroup(NoteHeadGroup(i));
            n = static_cast<Note*>(writeReadElement(note));
            QCOMPARE(int(n->headGroup()), i);
            delete n;
            }

  // headType
      for (int i = 0; i < 5; ++i) {
            note->setHeadType(NoteHeadType(i));
            n = static_cast<Note*>(writeReadElement(note));
            QCOMPARE(int(n->headType()), i);
            delete n;
            }

   // velo offset
      note->setVeloOffset(71);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->veloOffset(), 71);
      delete n;

   // tuning
      note->setTuning(1.3);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->tuning(), 1.3);
      delete n;

   // fret
      note->setFret(9);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->fret(), 9);
      delete n;

   // string
      note->setString(3);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->string(), 3);
      delete n;

   // ghost
      note->setGhost(true);
      n = static_cast<Note*>(writeReadElement(note));
      QVERIFY(n->ghost());
      delete n;

   // velo type
      note->setVeloType(USER_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->veloType(), USER_VAL);
      delete n;

      note->setVeloType(OFFSET_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->veloType(), OFFSET_VAL);
      delete n;

      //================================================
      //   test setProperty(int, QVariant)
      //================================================

   // pitch
      note->setProperty(P_PITCH, 32);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->pitch(), 32);
      delete n;

   // tpc
      note->setProperty(P_TPC, 21);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->tpc(), 21);
      delete n;

   // small
      note->setProperty(P_SMALL, false);
      n = static_cast<Note*>(writeReadElement(note));
      QVERIFY(!n->small());
      delete n;

      note->setProperty(P_SMALL, true);
      n = static_cast<Note*>(writeReadElement(note));
      QVERIFY(n->small());
      delete n;

   // mirror
      note->setProperty(P_MIRROR_HEAD, int(DH_LEFT));
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), DH_LEFT);
      delete n;

      note->setProperty(P_MIRROR_HEAD, int(DH_RIGHT));
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), DH_RIGHT);
      delete n;

      note->setProperty(P_MIRROR_HEAD, DH_AUTO);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), DH_AUTO);
      delete n;

   // dot position
      note->setProperty(P_DOT_POSITION, UP);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->dotPosition(), UP);
      delete n;

      note->setProperty(P_DOT_POSITION, DOWN);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->dotPosition(), DOWN);
      delete n;

      note->setProperty(P_DOT_POSITION, AUTO);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->dotPosition(), AUTO);
      delete n;

  // onTimeUserOffset
      note->setProperty(P_ONTIME_OFFSET, 9);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->onTimeUserOffset(), 9);
      delete n;

  // offTimeUserOffset
      note->setProperty(P_OFFTIME_OFFSET, 19);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->offTimeUserOffset(), 19);
      delete n;

  // headGroup
      for (int i = 0; i < HEAD_GROUPS; ++i) {
            note->setProperty(P_HEAD_GROUP, int(i));
            n = static_cast<Note*>(writeReadElement(note));
            QCOMPARE(int(n->headGroup()), i);
            delete n;
            }

  // headType
      for (int i = 0; i < 5; ++i) {
            note->setProperty(P_HEAD_TYPE, i);
            n = static_cast<Note*>(writeReadElement(note));
            QCOMPARE(int(n->headType()), i);
            delete n;
            }

   // velo offset
      note->setProperty(P_VELO_OFFSET, 38);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->veloOffset(), 38);
      delete n;

   // tuning
      note->setProperty(P_TUNING, 2.4);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->tuning(), 2.4);
      delete n;

   // fret
      note->setProperty(P_FRET, 7);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->fret(), 7);
      delete n;

   // string
      note->setProperty(P_STRING, 4);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->string(), 4);
      delete n;

   // ghost
      note->setProperty(P_GHOST, false);
      n = static_cast<Note*>(writeReadElement(note));
      QVERIFY(!n->ghost());
      delete n;

      note->setProperty(P_GHOST, true);
      n = static_cast<Note*>(writeReadElement(note));
      QVERIFY(n->ghost());
      delete n;

   // velo type
      note->setProperty(P_VELO_TYPE, USER_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->veloType(), USER_VAL);
      delete n;

      note->setProperty(P_VELO_TYPE, OFFSET_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->veloType(), OFFSET_VAL);
      delete n;
      }

QTEST_MAIN(TestNote)

#include "tst_note.moc"

