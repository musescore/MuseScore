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
#include "libmscore/accidental.h"
#include "libmscore/chord.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/tremolo.h"
#include "libmscore/articulation.h"
#include "libmscore/sym.h"
#include "mtest/testutils.h"

#define DIR QString("libmscore/note/")

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
      void grace();
      void tpc();
      void tpcTranspose();
      void tpcTranspose2();
      void noteLimits();
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
      note->setTpc1(22);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->tpc1(), 22);
      delete n;

      note->setTpc1(23);
      note->setTpc2(23);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->tpc2(), 23);
      delete n;

   // small
      note->setSmall(true);
      n = static_cast<Note*>(writeReadElement(note));
      QVERIFY(n->small());
      delete n;

   // mirror
      note->setUserMirror(MScore::DirectionH::LEFT);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), MScore::DirectionH::LEFT);
      delete n;

      note->setUserMirror(MScore::DirectionH::RIGHT);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), MScore::DirectionH::RIGHT);
      delete n;

      note->setUserMirror(MScore::DirectionH::AUTO);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), MScore::DirectionH::AUTO);
      delete n;

   // dot position
      note->setUserDotPosition(Direction::UP);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(int(n->userDotPosition()), int(Direction::UP));
      delete n;

      note->setUserDotPosition(Direction::DOWN);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(int(n->userDotPosition()), int(Direction::DOWN));
      delete n;

      note->setUserDotPosition(Direction::AUTO);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(int(n->userDotPosition()), int(Direction::AUTO));
      delete n;
  // headGroup
      for (int i = 0; i < int (NoteHead::Group::HEAD_GROUPS); ++i) {
            note->setHeadGroup(NoteHead::Group(i));
            n = static_cast<Note*>(writeReadElement(note));
            QCOMPARE(int(n->headGroup()), i);
            delete n;
            }

  // headType
      for (int i = 0; i < int(NoteHead::Type::HEAD_TYPES); ++i) {
            note->setHeadType(NoteHead::Type(i));
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
      note->setVeloType(Note::ValueType::USER_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->veloType(), Note::ValueType::USER_VAL);
      delete n;

      note->setVeloType(Note::ValueType::OFFSET_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->veloType(), Note::ValueType::OFFSET_VAL);
      delete n;

      //================================================
      //   test setProperty(int, QVariant)
      //================================================

   // pitch
      note->setProperty(P_ID::PITCH, 32);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->pitch(), 32);
      delete n;

   // tpc
      note->setProperty(P_ID::TPC1, 21);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->tpc1(), 21);
      delete n;

      note->setProperty(P_ID::TPC1, 22);
      note->setProperty(P_ID::TPC2, 22);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->tpc2(), 22);
      delete n;

   // small
      note->setProperty(P_ID::SMALL, false);
      n = static_cast<Note*>(writeReadElement(note));
      QVERIFY(!n->small());
      delete n;

      note->setProperty(P_ID::SMALL, true);
      n = static_cast<Note*>(writeReadElement(note));
      QVERIFY(n->small());
      delete n;

   // mirror
      note->setProperty(P_ID::MIRROR_HEAD, int(MScore::DirectionH::LEFT));
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), MScore::DirectionH::LEFT);
      delete n;

      note->setProperty(P_ID::MIRROR_HEAD, int(MScore::DirectionH::RIGHT));
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), MScore::DirectionH::RIGHT);
      delete n;

      note->setProperty(P_ID::MIRROR_HEAD, int(MScore::DirectionH::AUTO));
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->userMirror(), MScore::DirectionH::AUTO);
      delete n;

   // dot position
      note->setProperty(P_ID::DOT_POSITION, QVariant::fromValue(Direction(Direction::UP)));
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(int(n->userDotPosition()), int(Direction::UP));
      delete n;

      note->setProperty(P_ID::DOT_POSITION, QVariant::fromValue(Direction(Direction::DOWN)));
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(int(n->userDotPosition()), int(Direction::DOWN));
      delete n;

      note->setProperty(P_ID::DOT_POSITION, QVariant::fromValue(Direction(Direction::AUTO)));
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(int(n->userDotPosition()), int(Direction::AUTO));
      delete n;

  // headGroup
      for (int i = 0; i < int(NoteHead::Group::HEAD_GROUPS); ++i) {
            note->setProperty(P_ID::HEAD_GROUP, i);
            n = static_cast<Note*>(writeReadElement(note));
            QCOMPARE(int(n->headGroup()), i);
            delete n;
            }

  // headType
      for (int i = 0; i < int(NoteHead::Type::HEAD_TYPES); ++i) {
            note->setProperty(P_ID::HEAD_TYPE, i);
            n = static_cast<Note*>(writeReadElement(note));
            QCOMPARE(int(n->headType()), i);
            delete n;
            }

   // velo offset
      note->setProperty(P_ID::VELO_OFFSET, 38);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->veloOffset(), 38);
      delete n;

   // tuning
      note->setProperty(P_ID::TUNING, 2.4);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->tuning(), 2.4);
      delete n;

   // fret
      note->setProperty(P_ID::FRET, 7);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->fret(), 7);
      delete n;

   // string
      note->setProperty(P_ID::STRING, 4);
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->string(), 4);
      delete n;

   // ghost
      note->setProperty(P_ID::GHOST, false);
      n = static_cast<Note*>(writeReadElement(note));
      QVERIFY(!n->ghost());
      delete n;

      note->setProperty(P_ID::GHOST, true);
      n = static_cast<Note*>(writeReadElement(note));
      QVERIFY(n->ghost());
      delete n;

   // velo type
      note->setProperty(P_ID::VELO_TYPE, int(Note::ValueType::USER_VAL));
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->veloType(), Note::ValueType::USER_VAL);
      delete n;

      note->setProperty(P_ID::VELO_TYPE, int(Note::ValueType::OFFSET_VAL));
      n = static_cast<Note*>(writeReadElement(note));
      QCOMPARE(n->veloType(), Note::ValueType::OFFSET_VAL);
      delete n;

      }

//---------------------------------------------------------
///   grace
///   read/write test of grace notes
//---------------------------------------------------------

void TestNote::grace()
      {
      MasterScore* score = readScore(DIR + "grace.mscx");
      score->doLayout();
      Ms::Chord* chord = score->firstMeasure()->findChord(0, 0);
      Note* note = chord->upNote();

      // create
      score->setGraceNote(chord, note->pitch(), NoteType::APPOGGIATURA, MScore::division/2);
      Ms::Chord* gc = chord->graceNotes().first();
      Note* gn = gc->notes().front();
//      Note* n = static_cast<Note*>(writeReadElement(gn));
//      QCOMPARE(n->noteType(), NoteType::APPOGGIATURA);
//      delete n;

      // tie
      score->select(gn);
      score->cmdAddTie();
//      n = static_cast<Note*>(writeReadElement(gn));
//      QVERIFY(n->tieFor() != 0);
//      delete n;

      // tremolo
      score->startCmd();
      Tremolo* tr = new Tremolo(score);
      tr->setTremoloType(TremoloType::R16);
      tr->setParent(gc);
      tr->setTrack(gc->track());
      score->undoAddElement(tr);
      score->endCmd();
//      Ms::Chord* c = static_cast<Ms::Chord*>(writeReadElement(gc));
//      QVERIFY(c->tremolo() != 0);
//      delete c;

      // articulation
      score->startCmd();
      Articulation* ar = new Articulation(SymId::articAccentAbove, score);
      ar->setParent(gc);
      ar->setTrack(gc->track());
      score->undoAddElement(ar);
      score->endCmd();
//      c = static_cast<Ms::Chord*>(writeReadElement(gc));
//      QVERIFY(c->articulations().size() == 1);
//      delete c;

      QVERIFY(saveCompareScore(score, "grace-test.mscx", DIR + "grace-ref.mscx"));
      }

//---------------------------------------------------------
///   tpc
///   test of note tpc values
//---------------------------------------------------------

void TestNote::tpc()
      {
      MasterScore* score = readScore(DIR + "tpc.mscx");

      score->inputState().setTrack(0);
      score->inputState().setSegment(score->tick2segment(0, false, Segment::Type::ChordRest));
      score->inputState().setDuration(TDuration::DurationType::V_QUARTER);
      score->inputState().setNoteEntryMode(true);
      int octave = 5 * 7;
      score->cmdAddPitch(octave + 1, false, false);
      score->cmdAddPitch(octave + 2, false, false);
      score->cmdAddPitch(octave + 3, false, false);
      score->cmdAddPitch(octave + 4, false, false);
      score->cmdAddPitch(octave + 5, false, false);
      score->cmdAddPitch(octave + 6, false, false);
      score->cmdAddPitch(octave + 7, false, false);
      score->cmdAddPitch(octave + 8, false, false);

      score->cmdConcertPitchChanged(true, true);

      QVERIFY(saveCompareScore(score, "tpc-test.mscx", DIR + "tpc-ref.mscx"));
      }

//---------------------------------------------------------
///   tpcTranspose
///   test of note tpc values & transposition
//---------------------------------------------------------

void TestNote::tpcTranspose()
      {
      MasterScore* score = readScore(DIR + "tpc-transpose.mscx");

      score->startCmd();
      Measure* m = score->firstMeasure();
      score->select(m, SelectType::SINGLE, 0);
      score->changeAccidental(AccidentalType::FLAT);
      score->endCmd();

      score->startCmd();
      m = m->nextMeasure();
      score->select(m, SelectType::SINGLE, 0);
      score->upDown(false, UpDownMode::CHROMATIC);
      score->endCmd();

      score->startCmd();
      score->cmdConcertPitchChanged(true, true);
      score->endCmd();

      QVERIFY(saveCompareScore(score, "tpc-transpose-test.mscx", DIR + "tpc-transpose-ref.mscx"));

      }

//---------------------------------------------------------
///   tpcTranspose2
///   more tests of note tpc values & transposition
//---------------------------------------------------------

void TestNote::tpcTranspose2()
      {
      MasterScore* score = readScore(DIR + "tpc-transpose2.mscx");

      score->inputState().setTrack(0);
      score->inputState().setSegment(score->tick2segment(0, false, Segment::Type::ChordRest));
      score->inputState().setDuration(TDuration::DurationType::V_QUARTER);
      score->inputState().setNoteEntryMode(true);
      int octave = 5 * 7;
      score->cmdAddPitch(octave + 3, false, false);

      score->cmdConcertPitchChanged(true, true);

      QVERIFY(saveCompareScore(score, "tpc-transpose2-test.mscx", DIR + "tpc-transpose2-ref.mscx"));
      }

//---------------------------------------------------------
///   noteLimits
//---------------------------------------------------------

void TestNote::noteLimits()
      {
      MasterScore* score = readScore(DIR + "empty.mscx");

      score->inputState().setTrack(0);
      score->inputState().setSegment(score->tick2segment(0, false, Segment::Type::ChordRest));
      score->inputState().setDuration(TDuration::DurationType::V_QUARTER);
      score->inputState().setNoteEntryMode(true);

      // over 127 shouldn't crash
      score->cmdAddPitch(140, false, false);
      // below 0 shouldn't crash
      score->cmdAddPitch(-40, false, false);

      // stack chords
      score->cmdAddPitch(42, false, false);
      for (int i = 1; i < 20; i++)
            score->cmdAddPitch(42 + i * 7, true, false);

      // interval below
      score->cmdAddPitch(42, false, false);
      for (int i = 0; i < 20; i++) {
            std::vector<Note*> nl = score->selection().noteList();
            score->cmdAddInterval(-8, nl);
            }

      // interval above
      score->cmdAddPitch(42, false, false);
      for (int i = 0; i < 20; i++) {
            std::vector<Note*> nl = score->selection().noteList();
            score->cmdAddInterval(8, nl);
            }
      QVERIFY(saveCompareScore(score, "notelimits-test.mscx", DIR + "notelimits-ref.mscx"));
      }

QTEST_MAIN(TestNote)

#include "tst_note.moc"

