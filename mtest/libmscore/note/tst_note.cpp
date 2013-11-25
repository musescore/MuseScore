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

#include "libmscore/mscore.h"
#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "mtest/testutils.h"

using namespace Ms;

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
///   note
///   read/write test of note
//---------------------------------------------------------

void TestNote::note()
      {
      Ms::Chord* chord = new Ms::Chord(score);
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
      note->setUserMirror(MScore::DH_LEFT);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), MScore::DH_LEFT);
      delete n;

      note->setUserMirror(MScore::DH_RIGHT);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), MScore::DH_RIGHT);
      delete n;

      note->setUserMirror(MScore::DH_AUTO);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), MScore::DH_AUTO);
      delete n;

   // dot position
      note->setDotPosition(MScore::UP);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->dotPosition(), MScore::UP);
      delete n;

      note->setDotPosition(MScore::DOWN);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->dotPosition(), MScore::DOWN);
      delete n;

      note->setDotPosition(MScore::AUTO);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->dotPosition(), MScore::AUTO);
      delete n;
  // headGroup
      for (int i = 0; i < int (NoteHeadGroup::HEAD_GROUPS); ++i) {
            note->setHeadGroup(NoteHeadGroup(i));
            n = static_cast<Note*>(writeReadElement(note));
            QCOMPARE(int(n->headGroup()), i);
            delete n;
            }

  // headType
      for (int i = 0; i < int(NoteHeadType::HEAD_TYPES); ++i) {
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
      note->setVeloType(MScore::USER_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->veloType(), MScore::USER_VAL);
      delete n;

      note->setVeloType(MScore::OFFSET_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->veloType(), MScore::OFFSET_VAL);
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
      note->setProperty(P_MIRROR_HEAD, int(MScore::DH_LEFT));
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), MScore::DH_LEFT);
      delete n;

      note->setProperty(P_MIRROR_HEAD, int(MScore::DH_RIGHT));
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), MScore::DH_RIGHT);
      delete n;

      note->setProperty(P_MIRROR_HEAD, MScore::DH_AUTO);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), MScore::DH_AUTO);
      delete n;

   // dot position
      note->setProperty(P_DOT_POSITION, MScore::UP);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->dotPosition(), MScore::UP);
      delete n;

      note->setProperty(P_DOT_POSITION, MScore::DOWN);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->dotPosition(), MScore::DOWN);
      delete n;

      note->setProperty(P_DOT_POSITION, MScore::AUTO);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->dotPosition(), MScore::AUTO);
      delete n;

  // headGroup
      for (int i = 0; i < int(NoteHeadGroup::HEAD_GROUPS); ++i) {
            note->setProperty(P_HEAD_GROUP, i);
            n = static_cast<Note*>(writeReadElement(note));
            QCOMPARE(int(n->headGroup()), i);
            delete n;
            }

  // headType
      for (int i = 0; i < int(NoteHeadType::HEAD_TYPES); ++i) {
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
      note->setProperty(P_VELO_TYPE, MScore::USER_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->veloType(), MScore::USER_VAL);
      delete n;

      note->setProperty(P_VELO_TYPE, MScore::OFFSET_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->veloType(), MScore::OFFSET_VAL);
      delete n;
      }

QTEST_MAIN(TestNote)

#include "tst_note.moc"

