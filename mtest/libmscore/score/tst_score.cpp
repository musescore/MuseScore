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
#include "libmscore/key.h"
#include "libmscore/pitchspelling.h"
#include "mtest/testutils.h"

#define DIR QString("libmscore/score/")

using namespace Ms;

//---------------------------------------------------------
//   TestScore
//---------------------------------------------------------

class TestScore : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void makeGap_184156();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestScore::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   makeGap
//    change first 128th rest before a long note into a 64th note...this will cause makeGap to adjust the long note to make space to accomdate the 64th rest.
//    verifies that the result duration is a bunch of tied notes that are shorter by 1/128th.
//---------------------------------------------------------

void TestScore::makeGap_184156()
      {
      score = readScore(DIR + "score_makeGap_184156.mscx");
      score->doLayout();

      // change first 128th rest into a 64th note
      score->inputState().setTrack(0);
      score->inputState().setSegment(score->tick2segment(0, false, Segment::Type::ChordRest));
      score->inputState().setDuration(TDuration::DurationType::V_64TH);
      score->inputState().setNoteEntryMode(true);
      score->cmdAddPitch(42, false);
      QVERIFY(score->firstMeasure()->findChord(0, 0)->duration() == Fraction(1, 64)); // check that sucessfully inputted 64th note

      Chord* c = score->firstMeasure()->findChord(30, 0); // the double-dotted note which has been shortenend
      for (Note* n : c->notes()) {
            if (n->pitch() == 42)
                  QVERIFY(n->playTicks() == 1680 - 15); // check that the double-dotted original note was shortened by a 128th
            else
                  QVERIFY(n->playTicks() == 1920 - 30); // check that the double-dotted original note kept its tie to the next note
            }
      }


QTEST_MAIN(TestScore)

#include "tst_score.moc"

