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
#include "libmscore/undo.h"
#include "libmscore/mcursor.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/excerpt.h"
#include "libmscore/part.h"

#define DIR QString("libmscore/links/")

using namespace Ms;

//---------------------------------------------------------
//   TestLinks
//---------------------------------------------------------

class TestLinks : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void test3LinkedSameScore_99796();
      void test3LinkedParts_99796();
      void test4LinkedParts_94911();
      void test5LinkedParts_94911();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestLinks::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   test3LinkedSameScore
///  Create an empty 1 staff score
///  Add 2 linked staff
///  Delete first staff, undo, redo
//---------------------------------------------------------

void TestLinks::test3LinkedSameScore_99796()
      {
      MCursor c;
      c.setTimeSig(Fraction(4,4));
      c.createScore("test");
      c.addPart("voice");
      c.move(0, 0);     // move to track 0 tick 0

      c.addKeySig(Key(1));
      c.addTimeSig(Fraction(4,4));
      c.addChord(60, TDuration(TDuration::DurationType::V_WHOLE));

      Score* score = c.score();
      score->doLayout();
      Measure* m = score->firstMeasure();
      Segment* s = m->first(Segment::Type::ChordRest);
      Element* e = s->element(0);
      QVERIFY(e->type() == Element::Type::CHORD);
      
      score->select(e);
      score->cmdDeleteSelection();
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links() == nullptr);
      
      // add a linked staff
      score->startCmd();
      Staff* oStaff = score->staff(0);
      Staff* staff       = new Staff(score);
      staff->setPart(oStaff->part());
      score->undoInsertStaff(staff, 1);
      cloneStaff(oStaff, staff);
      
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 2);
      
      // add a second linked staff
      Staff* staff2       = new Staff(score);
      staff2->setPart(oStaff->part());
      score->undoInsertStaff(staff2, 2);
      cloneStaff(oStaff, staff2);
      score->endCmd();
      
      // we should have now 3 staves and 3 linked rests
      QVERIFY(score->staves().size() == 3);
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 3);
      e = s->element(4);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 3);
      e = s->element(8);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 3);
      
      // delete staff
      score->startCmd();
      score->cmdRemoveStaff(0);
      score->endCmd();
      
      // we have now 2 staves
      QVERIFY(score->staves().size() == 2);
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 2);
      e = s->element(4);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 2);
      
      // undo
      score->undo()->undo();
      // now 3 staves
      QVERIFY(score->staves().size() == 3);
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 3);
      e = s->element(4);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 3);
      e = s->element(8);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 3);
      
      // redo, back to 2 staves
      score->undo()->redo();
      QVERIFY(score->staves().size() == 2);
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 2);
      e = s->element(4);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 2);
      }

//---------------------------------------------------------
//   test3LinkedSameScore
///  Create an empty 1 staff score
///  Create part
///  Add linked staff
///  Delete part
//---------------------------------------------------------

void TestLinks::test3LinkedParts_99796()
      {
      MCursor c;
      c.setTimeSig(Fraction(4,4));
      c.createScore("test");
      c.addPart("voice");
      c.move(0, 0);     // move to track 0 tick 0

      c.addKeySig(Key(1));
      c.addTimeSig(Fraction(4,4));
      c.addChord(60, TDuration(TDuration::DurationType::V_WHOLE));

      Score* score = c.score();
      score->addText("title", "Title");
      score->doLayout();
      // delete chord
      Measure* m = score->firstMeasure();
      Segment* s = m->first(Segment::Type::ChordRest);
      Element* e = s->element(0);
      QVERIFY(e->type() == Element::Type::CHORD);
      score->select(e);
      score->cmdDeleteSelection();
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links() == nullptr);
      
      // create parts//
      score->startCmd();
      QList<Part*> parts;
      parts.append(score->parts().at(0));
      Score* nscore = new Score(score);
      Excerpt ex(score);
      ex.setPartScore(nscore);
      ex.setTitle("voice");
      ex.setParts(parts);
      ::createExcerpt(&ex);
      QVERIFY(nscore);
      nscore->setName(parts.front()->partName());
      score->undo(new AddExcerpt(nscore));
      score->endCmd();
      
      // add a linked staff
      score->startCmd();
      Staff* oStaff = score->staff(0);
      Staff* staff       = new Staff(score);
      staff->setPart(oStaff->part());
      score->undoInsertStaff(staff, 1);
      cloneStaff(oStaff, staff);
      score->endCmd();
      
      // we should have now 2 staves and 3 linked rests
      QVERIFY(score->staves().size() == 2);
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 3);
      e = s->element(4);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 3);
      
      // delete part
      score->startCmd();
      ::deleteExcerpt(&ex);
      score->undo(new RemoveExcerpt(nscore));
      
      // we should have now 2 staves and *2* linked rests
      QVERIFY(score->staves().size() == 2);
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 2);
      e = s->element(4);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 2);
      }

//---------------------------------------------------------
//   test4LinkedParts_94911
///  Create an empty 1 staff score
///  Add linked staff
///  Create part
///  Delete linked staff, undo, redo
//---------------------------------------------------------

void TestLinks::test4LinkedParts_94911()
      {
      MCursor c;
      c.setTimeSig(Fraction(4,4));
      c.createScore("test");
      c.addPart("electric-guitar");
      c.move(0, 0);     // move to track 0 tick 0

      c.addKeySig(Key(1));
      c.addTimeSig(Fraction(4,4));
      c.addChord(60, TDuration(TDuration::DurationType::V_WHOLE));

      Score* score = c.score();
      score->addText("title", "Title");
      score->doLayout();
      // delete chord
      Measure* m = score->firstMeasure();
      Segment* s = m->first(Segment::Type::ChordRest);
      Element* e = s->element(0);
      QVERIFY(e->type() == Element::Type::CHORD);
      score->select(e);
      score->cmdDeleteSelection();
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links() == nullptr);

      // add a linked staff
      score->startCmd();
      Staff* oStaff = score->staff(0);
      Staff* staff       = new Staff(score);
      staff->setPart(oStaff->part());
      score->undoInsertStaff(staff, 1);
      cloneStaff(oStaff, staff);
      score->endCmd();

      // we should have now 2 staves and 2 linked rests
      QVERIFY(score->staves().size() == 2);
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 2);
      e = s->element(4);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 2);

      // create parts//
      score->startCmd();
      QList<Part*> parts;
      parts.append(score->parts().at(0));
      Score* nscore = new Score(score);
      Excerpt ex(score);
      ex.setPartScore(nscore);
      ex.setTitle("Guitar");
      ex.setParts(parts);
      ::createExcerpt(&ex);
      QVERIFY(nscore);
      nscore->setName(parts.front()->partName());
      score->undo(new AddExcerpt(nscore));
      score->endCmd();

      // we should have now 2 staves and 4 linked rests
      QVERIFY(score->staves().size() == 2);
      QVERIFY(score->staves()[0]->linkedStaves()->staves().size() == 4);
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 4);
      e = s->element(4);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 4);
      QVERIFY(score->excerpts().size() == 1);

      // delete second staff
      score->startCmd();
      score->cmdRemoveStaff(1);
      score->endCmd();

      // we should have now 2 staves and *4* linked rest
      // no excerpt
      QVERIFY(score->staves().size() == 1);
      QVERIFY(score->staves()[0]->linkedStaves() == nullptr);
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links() == nullptr);
      qDebug() << score->excerpts().size();

      // undo
      score->undo()->undo();
      // we should have now 2 staves and 4 linked rests
      QVERIFY(score->staves().size() == 2);
      QVERIFY(score->staves()[0]->linkedStaves()->staves().size() == 4);
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 4);
      e = s->element(4);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 4);
      QVERIFY(score->excerpts().size() == 1);

      // redo
      score->undo()->redo();
      // we should have now 2 staves and *4* linked rest
      // no excerpt
      QVERIFY(score->staves().size() == 1);
      QVERIFY(score->staves()[0]->linkedStaves() == nullptr);
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links() == nullptr);
      }

//---------------------------------------------------------
//   test5LinkedParts_94911
///  Create an empty 1 staff score
///  Create part
///  Add linked staff, undo, redo
//---------------------------------------------------------

void TestLinks::test5LinkedParts_94911()
      {
      MCursor c;
      c.setTimeSig(Fraction(4,4));
      c.createScore("test");
      c.addPart("electric-guitar");
      c.move(0, 0);     // move to track 0 tick 0

      c.addKeySig(Key(1));
      c.addTimeSig(Fraction(4,4));
      c.addChord(60, TDuration(TDuration::DurationType::V_WHOLE));

      Score* score = c.score();
      score->addText("title", "Title");
      score->doLayout();
      // delete chord
      Measure* m = score->firstMeasure();
      Segment* s = m->first(Segment::Type::ChordRest);
      Element* e = s->element(0);
      QVERIFY(e->type() == Element::Type::CHORD);
      score->select(e);
      score->cmdDeleteSelection();
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links() == nullptr);

      // create parts//
      score->startCmd();
      QList<Part*> parts;
      parts.append(score->parts().at(0));
      Score* nscore = new Score(score);
      Excerpt ex(score);
      ex.setPartScore(nscore);
      ex.setTitle("Guitar");
      ex.setParts(parts);
      ::createExcerpt(&ex);
      QVERIFY(nscore);
      nscore->setName(parts.front()->partName());
      score->undo(new AddExcerpt(nscore));
      score->endCmd();

      // we should have now 1 staff and 2 linked rests
      QVERIFY(score->staves().size() == 1);
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 2);

      // add a linked staff
      score->startCmd();
      Staff* oStaff = score->staff(0);
      Staff* staff       = new Staff(score);
      staff->setPart(oStaff->part());
      score->undoInsertStaff(staff, 1);
      cloneStaff(oStaff, staff);
      score->endCmd();

      // we should have now 2 staves and 3 linked rests
      QVERIFY(score->staves().size() == 2);
      QVERIFY(score->staves()[0]->linkedStaves()->staves().size() == 3);
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 3);
      e = s->element(4);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 3);
      QVERIFY(score->excerpts().size() == 1);

      // undo
      score->undo()->undo();
      // we should have now 1 staves and 2 linked rests
      QVERIFY(score->staves().size() == 1);
      QVERIFY(score->staves()[0]->linkedStaves()->staves().size() == 2);
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 2);
      QVERIFY(score->excerpts().size() == 1);

      // redo
      score->undo()->redo();
      // we should have now 2 staves and 3 linked rests
      QVERIFY(score->staves().size() == 2);
      QVERIFY(score->staves()[0]->linkedStaves()->staves().size() == 3);
      e = s->element(0);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 3);
      e = s->element(4);
      QVERIFY(e->type() == Element::Type::REST);
      QVERIFY(e->links()->size() == 3);
      QVERIFY(score->excerpts().size() == 1);
      }


QTEST_MAIN(TestLinks)
#include "tst_links.moc"

