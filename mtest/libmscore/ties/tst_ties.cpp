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
#include "libmscore/segment.h"
#include "libmscore/chord.h"
#include "libmscore/undo.h"
#include "libmscore/mscore.h"

#define DIR QString("libmscore/ties/")

//---------------------------------------------------------
//   TestTies
//---------------------------------------------------------

class TestTies : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void undoChangePitchTies();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestTies::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   keysig
//---------------------------------------------------------

void TestTies::undoChangePitchTies()
      {
      // check if undo of change pitch leaves tied notes on the same line

      // read file
      Score* score = readScore(DIR + "undoChangePitchTies.mscx");

      // find first note
      Segment* s = score->firstMeasure()->first();
      while (!(s->segmentType() & (Segment::SegChordRest)))
            s = s->next();
      Chord* chord = static_cast<Chord*>(s->element(0));
      Note*  note  = chord->notes().at(0);

      // remember original pitch and line
      int origLine  = note->line();
      int origPitch = note->pitch();

      // change pitch via calling upDown()
      score->startCmd();
      score->select(note);
      score->upDown(false, UP_DOWN_CHROMATIC);
      score->endCmd();

      // undo
      score->undo()->undo();

      // check if pitch and line of all notes are reverted
      for (; s; s = s->next1()) {
            if (s->segmentType() & (Segment::SegChordRest)) {
                  Chord* c = static_cast<Chord*>(s->element(0));
                  if (c->type() == Chord::REST) continue;
                  Note*  n = c->notes().at(0);
                  QVERIFY(n->pitch() == origPitch);
                  QVERIFY(n->line()  == origLine);
                  }
            }

      // clean up
      delete score;
      }

QTEST_MAIN(TestTies)
#include "tst_ties.moc"
