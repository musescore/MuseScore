//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2015 Werner Schweer & others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/chord.h"
#include "libmscore/glissando.h"
#include "libmscore/measure.h"
#include "libmscore/score.h"
//#include "libmscore/system.h"
//#include "libmscore/undo.h"

#define DIR QString("libmscore/spanners/")

using namespace Ms;

//---------------------------------------------------------
//   TestClef
//---------------------------------------------------------

class TestSpanners : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void spanners01();            // cross-staff glissando from lower to higher staff
      void spanners02();            // glissando from/to grace notes
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSpanners::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
///  spanners01
///   Check loading of score with a glissando from a lower to a higher staff:
//    A score with:
//          grand staff,
//          glissando from a bass staff note to a treble staff note
//    is loaded and laid out and saved: should be round-trip safe.
//---------------------------------------------------------

void TestSpanners::spanners01()
      {
      Score* score = readScore(DIR + "glissando-crossstaff01.mscx");
      QVERIFY(score);
      score->doLayout();

      QVERIFY(saveCompareScore(score, "glissando-crsossstaff01.mscx", DIR + "glissando-crossstaff01-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///  spanners02
///   Loads a score with before- and after-grace notes and adds several glissandi from/to them.
//---------------------------------------------------------

void TestSpanners::spanners02()
      {
      DropData    dropData;
      Glissando*  gliss;

      Score* score = readScore(DIR + "glissando-graces01.mscx");
      QVERIFY(score);
      score->doLayout();

      // GLISSANDO FROM MAIN NOTE TO AFTER-GRACE
      // go to top note of first chord
      Measure*    msr   = score->firstMeasure();
      QVERIFY(msr);
      Segment*    seg   = msr->findSegment(Segment::Type::ChordRest, 0);
      QVERIFY(seg);
      Chord*      chord = static_cast<Chord*>(seg->element(0));
      QVERIFY(chord && chord->type() == Element::Type::CHORD);
      Note*       note  = chord->upNote();
      QVERIFY(note);
      // drop a glissando on note
      gliss             = new Glissando(score); // create a new element each time, as drop() will eventually delete it
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

      // GLISSANDO FROM AFTER-GRACE TO BEFORE-GRACE OF NEXT CHORD
      // go to last after-grace of chord and drop a glissando on it
      Chord*      grace = chord->graceNotesAfter().last();
      QVERIFY(grace && grace->type() == Element::Type::CHORD);
      note              = grace->upNote();
      QVERIFY(note);
      gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

      // GLISSANDO FROM MAIN NOTE TO BEFORE-GRACE OF NEXT CHORD
      // go to next chord
      seg               = seg->nextCR(0);
      QVERIFY(seg);
      chord             = static_cast<Chord*>(seg->element(0));
      QVERIFY(chord && chord->type() == Element::Type::CHORD);
      note              = chord->upNote();
      QVERIFY(note);
      gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

      // GLISSANDO FROM BEFORE-GRACE TO MAIN NOTE
      // go to next chord
      seg               = seg->nextCR(0);
      QVERIFY(seg);
      chord             = static_cast<Chord*>(seg->element(0));
      QVERIFY(chord && chord->type() == Element::Type::CHORD);
      // go to its last before-grace note
      grace             = chord->graceNotesBefore().last();
      QVERIFY(grace && grace->type() == Element::Type::CHORD);
      note              = grace->upNote();
      QVERIFY(note);
      gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

      QVERIFY(saveCompareScore(score, "glissando-graces01.mscx", DIR + "glissando-graces01-ref.mscx"));
      delete score;
      }


QTEST_MAIN(TestSpanners)
#include "tst_spanners.moc"

