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
#include "libmscore/excerpt.h"
#include "libmscore/part.h"
#include "libmscore/undo.h"
#include "libmscore/measure.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/breath.h"
#include "libmscore/segment.h"
#include "libmscore/fingering.h"
#include "libmscore/image.h"
#include "libmscore/element.h"
#include "mtest/testutils.h"

#define DIR QString("libmscore/link/")

using namespace Ms;

//---------------------------------------------------------
//   TestParts
//---------------------------------------------------------

class TestParts : public QObject, public MTest
      {
      Q_OBJECT

      void createParts(Score* score);
      void testPartCreation(const QString& test);

      Score* doAddBreath();
      Score* doRemoveBreath();
      Score* doAddFingering();
      Score* doRemoveFingering();
      Score* doAddSymbol();
      Score* doRemoveSymbol();
//      Score* doAddImage();
//      Score* doRemoveImage();

   private slots:
      void initTestCase();

      void createPart1();
      void createPart2();

      void createPartBreath();
      void addBreath();
      void undoAddBreath();
      void undoRedoAddBreath();
      void removeBreath();
      void undoRemoveBreath();
      void undoRedoRemoveBreath();

      void createPartFingering();
      void addFingering();
      void undoAddFingering();
      void undoRedoAddFingering();
      void removeFingering();
      void undoRemoveFingering();
      void undoRedoRemoveFingering();

      void createPartSymbol();
      void addSymbol();
      void undoAddSymbol();
      void undoRedoAddSymbol();
      void removeSymbol();
      void undoRemoveSymbol();
      void undoRedoRemoveSymbol();

//      void createPartImage();
//      void addImage();
//      void undoAddImage();
//      void undoRedoAddImage();
//      void removeImage();
//      void undoRemoveImage();
//      void undoRedoRemoveImage();

      void appendMeasure();
      void insertMeasure();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestParts::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   createParts
//---------------------------------------------------------

void TestParts::createParts(Score* score)
      {
      //
      // create first part
      //
      QList<Part*> parts;
      parts.append(score->parts().at(0));
      Score* nscore = ::createExcerpt(parts);
      QVERIFY(nscore);

      nscore->setName(parts.front()->partName());
      score->undo(new AddExcerpt(nscore));
      nscore->style()->set(ST_createMultiMeasureRests, true);

      //
      // create second part
      //
      parts.clear();
      parts.append(score->parts().at(1));
      nscore = ::createExcerpt(parts);
      QVERIFY(nscore);

      nscore->setName(parts.front()->partName());
      score->undo(new AddExcerpt(nscore));
      nscore->style()->set(ST_createMultiMeasureRests, true);
      }

//---------------------------------------------------------
//   testPartCreation
//---------------------------------------------------------

void TestParts::testPartCreation(const QString& test)
      {
      Score* score = readScore(DIR + test + ".mscx");
      score->doLayout();
      QVERIFY(score);
      QVERIFY(saveCompareScore(score, test + "-1.mscx", DIR + test + ".mscx"));
      createParts(score);
      QVERIFY(saveCompareScore(score, test + "-2.mscx", DIR + test + "-2o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   testAppendMeasure
//---------------------------------------------------------

void TestParts::appendMeasure()
      {
      Score* score = readScore(DIR + "part2.mscx");
      score->doLayout();

      QVERIFY(score);
      createParts(score);

      score->startCmd();
      score->insertMeasure(Element::MEASURE, 0);
      score->endCmd();

      QVERIFY(saveCompareScore(score, "part2-3.mscx", DIR + "part2-3o.mscx"));

      score->undo()->undo();
      score->endUndoRedo();

      QVERIFY(saveCompareScore(score, "part2-4.mscx", DIR + "part2-4o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   testInsertMeasure
//---------------------------------------------------------

void TestParts::insertMeasure()
      {
      Score* score = readScore(DIR + "part2.mscx");
      score->doLayout();
      QVERIFY(score);
      createParts(score);

      score->startCmd();
      Measure* m = score->firstMeasure();
      score->insertMeasure(Element::MEASURE, m);
      score->endCmd();

      // QVERIFY(saveCompareScore(score, "part2-5.mscx", DIR + "part2-5o.mscx"));

      score->undo()->undo();
      score->endUndoRedo();

      QVERIFY(saveCompareScore(score, "part2-6.mscx", DIR + "part2-6o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   test part creation
//---------------------------------------------------------

void TestParts::createPart1()
      {
      testPartCreation("part1");
      }

void TestParts::createPart2()
      {
      testPartCreation("part2");
      }

void TestParts::createPartBreath()
      {
      testPartCreation("part3");
      }

void TestParts::createPartFingering()
      {
      testPartCreation("part10");
      }

void TestParts::createPartSymbol()
      {
      testPartCreation("part11");
      }

#if 0
void TestParts::createPartImage()
      {
      testPartCreation("part12");
      }
#endif

//---------------------------------------------------------
//    doAddBreath
//---------------------------------------------------------

Score* TestParts::doAddBreath()
      {
      Score* score = readScore(DIR + "part1-2o.mscx");
      score->doLayout();

      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->tick2segment(480);
      Ms::Chord* chord = static_cast<Ms::Chord*>(s->element(0));
      Note* note   = chord->upNote();
      DropData dd;
      dd.view = 0;
      Breath* b = new Breath(score);
      b->setBreathType(0);
      dd.element = b;

      score->startCmd();
      note->drop(dd);
      score->endCmd();        // does layout

      return score;
      }

//---------------------------------------------------------
//   addBreath
//---------------------------------------------------------

void TestParts::addBreath()
      {
      Score* score = doAddBreath();
      QVERIFY(saveCompareScore(score, "part4.mscx", DIR + "part4o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoAddBreath
//---------------------------------------------------------

void TestParts::undoAddBreath()
      {
      Score* score = doAddBreath();

      score->undo()->undo();
      score->endUndoRedo();

      QVERIFY(saveCompareScore(score, "part5.mscx", DIR + "part5o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoAddBreath
//---------------------------------------------------------

void TestParts::undoRedoAddBreath()
      {
      Score* score = doAddBreath();

      score->undo()->undo();
      score->endUndoRedo();

      score->undo()->redo();
      score->endUndoRedo();

      QVERIFY(saveCompareScore(score, "part6.mscx", DIR + "part6o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doRemoveBreath
//---------------------------------------------------------

Score* TestParts::doRemoveBreath()
      {
      Score* score = readScore(DIR + "part4o.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->first()->next(Segment::SegBreath);
      Breath*  b   = static_cast<Breath*>(s->element(0));

      score->select(b);
      score->startCmd();
      score->cmdDeleteSelection();
      score->setLayoutAll(true);
      score->endCmd();
      return score;
      }

//---------------------------------------------------------
//   removeBreath
//---------------------------------------------------------

void TestParts::removeBreath()
      {
      Score* score = doRemoveBreath();
      QVERIFY(saveCompareScore(score, "part7.mscx", DIR + "part7o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRemoveBreath
//---------------------------------------------------------

void TestParts::undoRemoveBreath()
      {
      Score* score = doRemoveBreath();
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part8.mscx", DIR + "part8o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoRemoveBreath
//---------------------------------------------------------

void TestParts::undoRedoRemoveBreath()
      {
      Score* score = doRemoveBreath();
      score->undo()->undo();
      score->endUndoRedo();

      score->undo()->redo();
      score->endUndoRedo();

      QVERIFY(saveCompareScore(score, "part9.mscx", DIR + "part9o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doAddFingering
//---------------------------------------------------------

Score* TestParts::doAddFingering()
      {
      Score* score = readScore(DIR + "part1-2o.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->tick2segment(480);
      Ms::Chord* chord = static_cast<Ms::Chord*>(s->element(0));
      Note* note   = chord->upNote();
      DropData dd;
      dd.view = 0;
      Fingering* b = new Fingering(score);
      b->setText("3");
      dd.element = b;

      score->startCmd();
      note->drop(dd);
      score->endCmd();        // does layout
      return score;
      }

//---------------------------------------------------------
//   addFingering
//---------------------------------------------------------

void TestParts::addFingering()
      {
      Score* score = doAddFingering();
      QVERIFY(saveCompareScore(score, "part13.mscx", DIR + "part13o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoAddFingering
//---------------------------------------------------------

void TestParts::undoAddFingering()
      {
      Score* score = doAddFingering();
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part14.mscx", DIR + "part14o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoAddFingering
//---------------------------------------------------------

void TestParts::undoRedoAddFingering()
      {
      Score* score = doAddFingering();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part15.mscx", DIR + "part15o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doRemoveFingering
//---------------------------------------------------------

Score* TestParts::doRemoveFingering()
      {
      Score* score = readScore(DIR + "part10-2o.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->first()->next(Segment::SegChordRest);
      Ms::Chord* chord = static_cast<Ms::Chord*>(s->element(0));
      Note* note   = chord->upNote();
      Element* fingering = 0;
      foreach(Element* e, note->el()) {
            if (e->type() == Element::FINGERING) {
                  fingering = e;
                  break;
                  }
            }
      score->select(fingering);

      score->startCmd();
      score->cmdDeleteSelection();
      score->setLayoutAll(true);
      score->endCmd();
      return score;
      }

//---------------------------------------------------------
//   removeFingering
//---------------------------------------------------------

void TestParts::removeFingering()
      {
      Score* score = doRemoveFingering();
      QVERIFY(saveCompareScore(score, "part16.mscx", DIR + "part16o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRemoveFingering
//---------------------------------------------------------

void TestParts::undoRemoveFingering()
      {
      Score* score = doRemoveFingering();
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part17.mscx", DIR + "part17o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoRemoveFingering
//---------------------------------------------------------

void TestParts::undoRedoRemoveFingering()
      {
      Score* score = doRemoveFingering();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part18.mscx", DIR + "part18o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doAddSymbol
//---------------------------------------------------------

Score* TestParts::doAddSymbol()
      {
      Score* score = readScore(DIR + "part1-2o.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->tick2segment(480);
      Ms::Chord* chord = static_cast<Ms::Chord*>(s->element(0));
      Note* note   = chord->upNote();
      DropData dd;
      dd.view = 0;
      Symbol* b = new Symbol(score, accDiscantSym);
      dd.element = b;

      score->startCmd();
      note->drop(dd);
      score->endCmd();        // does layout
      return score;
      }

//---------------------------------------------------------
//   addSymbol
//---------------------------------------------------------

void TestParts::addSymbol()
      {
      Score* score = doAddSymbol();
      QVERIFY(saveCompareScore(score, "part19.mscx", DIR + "part19o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoAddSymbol
//---------------------------------------------------------

void TestParts::undoAddSymbol()
      {
      Score* score = doAddSymbol();
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part20.mscx", DIR + "part20o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoAddSymbol
//---------------------------------------------------------

void TestParts::undoRedoAddSymbol()
      {
      Score* score = doAddSymbol();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part21.mscx", DIR + "part21o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doRemoveSymbol
//---------------------------------------------------------

Score* TestParts::doRemoveSymbol()
      {
      Score* score = readScore(DIR + "part11-2o.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->first()->next(Segment::SegChordRest);
      Ms::Chord* chord = static_cast<Ms::Chord*>(s->element(0));
      Note* note   = chord->upNote();
      Element* fingering = 0;
      foreach(Element* e, note->el()) {
            if (e->type() == Element::SYMBOL) {
                  fingering = e;
                  break;
                  }
            }
      score->select(fingering);

      score->startCmd();
      score->cmdDeleteSelection();
      score->setLayoutAll(true);
      score->endCmd();
      return score;
      }

//---------------------------------------------------------
//   removeSymbol
//---------------------------------------------------------

void TestParts::removeSymbol()
      {
      Score* score = doRemoveSymbol();
      QVERIFY(saveCompareScore(score, "part22.mscx", DIR + "part22o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRemoveSymbol
//---------------------------------------------------------

void TestParts::undoRemoveSymbol()
      {
      Score* score = doRemoveSymbol();
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part23.mscx", DIR + "part23o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoRemoveSymbol
//---------------------------------------------------------

void TestParts::undoRedoRemoveSymbol()
      {
      Score* score = doRemoveSymbol();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part24.mscx", DIR + "part24o.mscx"));
      delete score;
      }

#if 0
//---------------------------------------------------------
//   doAddImage
//---------------------------------------------------------

Score* TestParts::doAddImage()
      {
      Score* score = readScore(DIR + "part1-2o.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->tick2segment(480);
      Chord* chord = static_cast<Chord*>(s->element(0));
      Note* note   = chord->upNote();
      DropData dd;
      dd.view = 0;
      RasterImage* b = new RasterImage(score);
      b->load(DIR + "schnee.png");
      dd.element = b;

      score->startCmd();
      note->drop(dd);
      score->endCmd();        // does layout
      return score;
      }

//---------------------------------------------------------
//   addImage
//---------------------------------------------------------

void TestParts::addImage()
      {
      Score* score = doAddImage();
      QVERIFY(saveCompareScore(score, "part25.mscx", DIR + "part25o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoAddImage
//---------------------------------------------------------

void TestParts::undoAddImage()
      {
      Score* score = doAddImage();
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part26.mscx", DIR + "part26o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoAddImage
//---------------------------------------------------------

void TestParts::undoRedoAddImage()
      {
      Score* score = doAddImage();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part27.mscx", DIR + "part27o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doRemoveImage
//---------------------------------------------------------

Score* TestParts::doRemoveImage()
      {
      Score* score = readScore(DIR + "part12o.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->first()->next(SegChordRest);
      Ms::Chord* chord = static_cast<Ms::Chord*>(s->element(0));
      Note* note   = chord->upNote();
      Element* fingering = 0;
      foreach(Element* e, note->el()) {
            if (e->type() == IMAGE) {
                  fingering = e;
                  break;
                  }
            }
      score->select(fingering);

      score->startCmd();
      score->cmdDeleteSelection();
      score->setLayoutAll(true);
      score->endCmd();
      return score;
      }

//---------------------------------------------------------
//   removeImage
//---------------------------------------------------------

void TestParts::removeImage()
      {
      Score* score = doRemoveImage();
      QVERIFY(saveCompareScore(score, "part28.mscx", DIR + "part28o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRemoveImage
//---------------------------------------------------------

void TestParts::undoRemoveImage()
      {
      Score* score = doRemoveImage();
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part29.mscx", DIR + "part29o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoRemoveImage
//---------------------------------------------------------

void TestParts::undoRedoRemoveImage()
      {
      Score* score = doRemoveImage();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part30.mscx", DIR + "part30o.mscx"));
      delete score;
      }
#endif

QTEST_MAIN(TestParts)

#include "tst_parts.moc"

