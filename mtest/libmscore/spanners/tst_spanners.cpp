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
#include "libmscore/excerpt.h"
#include "libmscore/glissando.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/score.h"
//#include "libmscore/system.h"
#include "libmscore/undo.h"

#define DIR QString("libmscore/spanners/")

using namespace Ms;

//---------------------------------------------------------
//   TestSpanners
//---------------------------------------------------------

class TestSpanners : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void spanners01();            // adding glissandos in several contexts
      void spanners02();            // loading back existing cross-staff glissando from lower to higher staff
      void spanners03();            // adding glissandos from/to grace notes
      void spanners04();            // linking a staff to a staff containing a glissando
      void spanners05();            // creating part from an existing staff containing a glissando
      void spanners06();            // add a glissando to a staff with a linked staff
      void spanners07();            // add a glissando to a satff with an excerpt attached
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
///   Adds glissandi in several contexts.
//---------------------------------------------------------

void TestSpanners::spanners01()
      {
      DropData    dropData;
      Glissando*  gliss;

      Score* score = readScore(DIR + "glissando01.mscx");
      QVERIFY(score);
      score->doLayout();

      // SIMPLE CASE: GLISSANDO FROM A NOTE TO THE FOLLOWING
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

      // GLISSANDO FROM TOP STAFF TO BOTTOM STAFF
      // go to top note of first chord of next measure
      msr   = msr->nextMeasure();
      QVERIFY(msr);
      seg   = msr->first();
      QVERIFY(seg);
      chord = static_cast<Chord*>(seg->element(0));   // voice 0 of staff 0
      QVERIFY(chord && chord->type() == Element::Type::CHORD);
      note  = chord->upNote();
      QVERIFY(note);
      // drop a glissando on note
      gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

      // GLISSANDO FROM BOTTOM STAFF TO TOP STAFF
      // go to bottom note of first chord of next measure
      msr   = msr->nextMeasure();
      QVERIFY(msr);
      seg   = msr->first();
      QVERIFY(seg);
      chord = static_cast<Chord*>(seg->element(4));   // voice 0 of staff 1
      QVERIFY(chord && chord->type() == Element::Type::CHORD);
      note  = chord->upNote();
      QVERIFY(note);
      // drop a glissando on note
      gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

      // GLISSANDO OVER INTERVENING NOTES IN ANOTHER VOICE
      // go to top note of first chord of next measure
      msr   = msr->nextMeasure();
      QVERIFY(msr);
      seg   = msr->first();
      QVERIFY(seg);
      chord = static_cast<Chord*>(seg->element(0));   // voice 0 of staff 0
      QVERIFY(chord && chord->type() == Element::Type::CHORD);
      note  = chord->upNote();
      QVERIFY(note);
      // drop a glissando on note
      gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

      // GLISSANDO OVER INTERVENING NOTES IN ANOTHER STAFF
      // go to top note of first chord of next measure
      msr   = msr->nextMeasure()->nextMeasure();
      QVERIFY(msr);
      seg   = msr->first();
      QVERIFY(seg);
      chord = static_cast<Chord*>(seg->element(0));   // voice 0 of staff 0
      QVERIFY(chord && chord->type() == Element::Type::CHORD);
      note  = chord->upNote();
      QVERIFY(note);
      // drop a glissando on note
      gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

      QVERIFY(saveCompareScore(score, "glissando01.mscx", DIR + "glissando01-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///  spanners02
///   Check loading of score with a glissando from a lower to a higher staff:
//    A score with:
//          grand staff,
//          glissando from a bass staff note to a treble staff note
//    is loaded and laid out and saved: should be round-trip safe.
//---------------------------------------------------------

void TestSpanners::spanners02()
      {
      Score* score = readScore(DIR + "glissando-crossstaff01.mscx");
      QVERIFY(score);
      score->doLayout();

      QVERIFY(saveCompareScore(score, "glissando-crsossstaff01.mscx", DIR + "glissando-crossstaff01-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///  spanners03
///   Loads a score with before- and after-grace notes and adds several glissandi from/to them.
//---------------------------------------------------------

void TestSpanners::spanners03()
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

//---------------------------------------------------------
///  spanners04
///   Linking a staff to an existing staff containing a glissando
//---------------------------------------------------------

void TestSpanners::spanners04()
      {

      Score* score = readScore(DIR + "glissando-cloning01.mscx");
      QVERIFY(score);
      score->doLayout();

      // add a linked staff to the existing staff
      // (copied and adapted from void MuseScore::editInstrList() in mscore/instrdialog.cpp)
      Staff* oldStaff   = score->staff(0);
      Staff* newStaff   = new Staff(score);
      newStaff->setPart(oldStaff->part());
      newStaff->initFromStaffType(oldStaff->staffType());
      newStaff->setDefaultClefType(ClefTypeList(ClefType::G));

      KeySigEvent ke;
      ke.setKey(Key::C);
      newStaff->setKey(0, ke);

      score->undoInsertStaff(newStaff, 1, false);
      cloneStaff(oldStaff, newStaff);

      QVERIFY(saveCompareScore(score, "glissando-cloning01.mscx", DIR + "glissando-cloning01-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///  spanners05
///   Creating part from an existing staff containing a glissando
//---------------------------------------------------------

void TestSpanners::spanners05()
      {

      Score* score = readScore(DIR + "glissando-cloning02.mscx");
      QVERIFY(score);
      score->doLayout();

      // create parts
      // (copied and adapted from void TestParts::createParts() in mtest/libmscore/parts/tst_parts.cpp)
      QList<Part*> parts;
      parts.append(score->parts().at(0));
      Score* nscore = new Score(score);

      Excerpt ex(score);
      ex.setPartScore(nscore);
      ex.setTitle(parts.front()->longName());
      ex.setParts(parts);
      ::createExcerpt(&ex);
      QVERIFY(nscore);

      nscore->setName(parts.front()->partName());
      score->undo(new AddExcerpt(nscore));

      QVERIFY(saveCompareScore(score, "glissando-cloning02.mscx", DIR + "glissando-cloning02-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///  spanners06
///   Drop a glissando on a staff with a linked staff
//---------------------------------------------------------

void TestSpanners::spanners06()
      {
      DropData    dropData;
      Glissando*  gliss;

      Score* score = readScore(DIR + "glissando-cloning03.mscx");
      QVERIFY(score);
      score->doLayout();

      // DROP A GLISSANDO ON FIRST NOTE
      Measure*    msr   = score->firstMeasure();
      QVERIFY(msr);
      Segment*    seg   = msr->findSegment(Segment::Type::ChordRest, 0);
      QVERIFY(seg);
      Chord*      chord = static_cast<Chord*>(seg->element(0));
      QVERIFY(chord && chord->type() == Element::Type::CHORD);
      Note*       note  = chord->upNote();
      QVERIFY(note);
      // drop a glissando on note
      gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

      QVERIFY(saveCompareScore(score, "glissando-cloning03.mscx", DIR + "glissando-cloning03-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
///  spanners07
///   Drop a glissando on a staff with an excerpt
//---------------------------------------------------------

void TestSpanners::spanners07()
      {
      DropData    dropData;
      Glissando*  gliss;

      Score* score = readScore(DIR + "glissando-cloning04.mscx");
      QVERIFY(score);
      score->doLayout();

      // DROP A GLISSANDO ON FIRST NOTE
      Measure*    msr   = score->firstMeasure();
      QVERIFY(msr);
      Segment*    seg   = msr->findSegment(Segment::Type::ChordRest, 0);
      QVERIFY(seg);
      Chord*      chord = static_cast<Chord*>(seg->element(0));
      QVERIFY(chord && chord->type() == Element::Type::CHORD);
      Note*       note  = chord->upNote();
      QVERIFY(note);
      // drop a glissando on note
      gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

      QVERIFY(saveCompareScore(score, "glissando-cloning04.mscx", DIR + "glissando-cloning04-ref.mscx"));
      delete score;
      }


QTEST_MAIN(TestSpanners)
#include "tst_spanners.moc"

