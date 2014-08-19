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
#include "libmscore/staff.h"
#include "libmscore/stafftype.h"
#include "libmscore/sym.h"
#include "libmscore/chordline.h"
#include "mtest/testutils.h"

#define DIR QString("libmscore/parts/")

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
      Score* doAddChordline();
      Score* doRemoveChordline();
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

      void createPartChordline();
      void addChordline();
      void undoAddChordline();
      void undoRedoAddChordline();
      void removeChordline();
      void undoRemoveChordline();
      void undoRedoRemoveChordline();

//      void createPartImage();
//      void addImage();
//      void undoAddImage();
//      void undoRedoAddImage();
//      void removeImage();
//      void undoRemoveImage();
//      void undoRedoRemoveImage();

      void appendMeasure();
      void insertMeasure();
      void styleScore();
      void styleScoreReload();
//      void stylePartDefault();
//      void styleScoreDefault();
//      void staffStyles();

      void measureProperties();
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
      Score* nscore = new Score(score);
      ::createExcerpt(nscore, parts);
      QVERIFY(nscore);

      nscore->setName(parts.front()->partName());
      score->undo(new AddExcerpt(nscore));
      nscore->style()->set(StyleIdx::createMultiMeasureRests, true);

      //
      // create second part
      //
      parts.clear();
      parts.append(score->parts().at(1));
      nscore = new Score(score);
      ::createExcerpt(nscore, parts);
      QVERIFY(nscore);

      nscore->setName(parts.front()->partName());
      score->undo(new AddExcerpt(nscore));
      nscore->style()->set(StyleIdx::createMultiMeasureRests, true);
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
      QVERIFY(saveCompareScore(score, test + "-parts.mscx", DIR + test + "-parts.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   testAppendMeasure
//---------------------------------------------------------

void TestParts::appendMeasure()
      {
      Score* score = readScore(DIR + "part-all.mscx");
      score->doLayout();

      QVERIFY(score);
      createParts(score);

      score->startCmd();
      score->insertMeasure(Element::Type::MEASURE, 0);
      score->endCmd();

      QVERIFY(saveCompareScore(score, "part-all-appendmeasures.mscx", DIR + "part-all-appendmeasures.mscx"));

      score->undo()->undo();
      score->endUndoRedo();

      QVERIFY(saveCompareScore(score, "part-all-uappendmeasures.mscx", DIR + "part-all-uappendmeasures.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   testInsertMeasure
//---------------------------------------------------------

void TestParts::insertMeasure()
      {
      Score* score = readScore(DIR + "part-all.mscx");
      score->doLayout();
      QVERIFY(score);
      createParts(score);

      score->startCmd();
      Measure* m = score->firstMeasure();
      score->insertMeasure(Element::Type::MEASURE, m);
      score->endCmd();

      // QVERIFY(saveCompareScore(score, "part-all-insertmeasures.mscx", DIR + "part-all-insertmeasures.mscx"));

      score->undo()->undo();
      score->endUndoRedo();

      QVERIFY(saveCompareScore(score, "part-all-uinsertmeasures.mscx", DIR + "part-all-uinsertmeasures.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   styleScore
//---------------------------------------------------------

void TestParts::styleScore()
      {
      Score* score = readScore(DIR + "partStyle.mscx");
      score->doLayout();
      QVERIFY(score);
      createParts(score);
      score->style()->set(StyleIdx::clefLeftMargin, 4.0);
      QVERIFY(saveCompareScore(score, "partStyle-score-test.mscx", DIR + "partStyle-score-ref.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   styleScoreReload
//---------------------------------------------------------

void TestParts::styleScoreReload()
      {
      Score* partScore = readScore(DIR + "partStyle-score-reload.mscx");
      QVERIFY(saveCompareScore(partScore, "partStyle-score-reload-test.mscx", DIR + "partStyle-score-reload-ref.mscx"));
      delete partScore;
      }

//---------------------------------------------------------
//   stylePartDefault
//---------------------------------------------------------

#if 0
void TestParts::stylePartDefault()
      {
      Score* score = readScore(DIR + "partStyle.mscx");
      score->doLayout();
      QVERIFY(score);
      // TODO: set defaultStyleForParts
      MScore::_defaultStyleForParts = new MStyle();
      QFile f(DIR + "style_test.mss");
      QVERIFY(f.open(QIODevice::ReadOnly));
      MStyle* s = new MStyle(*defaultStyle());
      QVERIFY(s->load(&f));
      MScore::_defaultStyleForParts = s;
      createParts(score);
      QVERIFY(saveCompareScore(score, "partStyle-part-default-test.mscx", DIR + "partStyle-part-default-ref.mscx"));
      }

//---------------------------------------------------------
//   styleScoreDefault
//---------------------------------------------------------

void TestParts::styleScoreDefault()
      {
      Score* score = readScore(DIR + "partStyle.mscx");
      score->doLayout();
      QVERIFY(score);
      // TODO: set defaultStyle
      createParts(score);
      QVERIFY(saveCompareScore(score, "partStyle-score-default-test.mscx", DIR + "partStyle-score-default-ref.mscx"));
      }
#endif

//---------------------------------------------------------
//   test part creation
//---------------------------------------------------------

void TestParts::createPart1()
      {
      testPartCreation("part-empty");
      }

void TestParts::createPart2()
      {
      testPartCreation("part-all");
      }

void TestParts::createPartBreath()
      {
      testPartCreation("part-breath");
      }

void TestParts::createPartFingering()
      {
      testPartCreation("part-fingering");
      }

void TestParts::createPartSymbol()
      {
      testPartCreation("part-symbol");
      }

void TestParts::createPartChordline()
      {
      testPartCreation("part-chordline");
      }

#if 0
void TestParts::createPartImage()
      {
      testPartCreation("part-image");
      }
#endif
//---------------------------------------------------------
//    doAddBreath
//---------------------------------------------------------

Score* TestParts::doAddBreath()
      {
      Score* score = readScore(DIR + "part-empty-parts.mscx");
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
      QVERIFY(saveCompareScore(score, "part-breath-add.mscx", DIR + "part-breath-add.mscx"));
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

      QVERIFY(saveCompareScore(score, "part-breath-uadd.mscx", DIR + "part-breath-uadd.mscx"));
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
      score->doLayout();

      score->undo()->redo();
      score->endUndoRedo();

      QVERIFY(saveCompareScore(score, "part-breath-uradd.mscx", DIR + "part-breath-uradd.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doRemoveBreath
//---------------------------------------------------------

Score* TestParts::doRemoveBreath()
      {
      Score* score = readScore(DIR + "part-breath-add.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->first()->next(Segment::Type::Breath);
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
      QVERIFY(saveCompareScore(score, "part-breath-del.mscx", DIR + "part-breath-del.mscx"));
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
      QVERIFY(saveCompareScore(score, "part-breath-udel.mscx", DIR + "part-breath-udel.mscx"));
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
      score->doLayout();

      score->undo()->redo();
      score->endUndoRedo();

      QVERIFY(saveCompareScore(score, "part-breath-urdel.mscx", DIR + "part-breath-urdel.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doAddFingering
//---------------------------------------------------------

Score* TestParts::doAddFingering()
      {
      Score* score = readScore(DIR + "part-empty-parts.mscx");
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
      QVERIFY(saveCompareScore(score, "part-fingering-add.mscx", DIR + "part-fingering-add.mscx"));
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
      QVERIFY(saveCompareScore(score, "part-fingering-uadd.mscx", DIR + "part-fingering-uadd.mscx"));
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
      QVERIFY(saveCompareScore(score, "part-fingering-uradd.mscx", DIR + "part-fingering-uradd.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doRemoveFingering
//---------------------------------------------------------

Score* TestParts::doRemoveFingering()
      {
      Score* score = readScore(DIR + "part-fingering-parts.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->first()->next(Segment::Type::ChordRest);
      Ms::Chord* chord = static_cast<Ms::Chord*>(s->element(0));
      Note* note   = chord->upNote();
      Element* fingering = 0;
      foreach(Element* e, note->el()) {
            if (e->type() == Element::Type::FINGERING) {
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
      QVERIFY(saveCompareScore(score, "part-fingering-del.mscx", DIR + "part-fingering-del.mscx"));
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
      QVERIFY(saveCompareScore(score, "part-fingering-udel.mscx", DIR + "part-fingering-udel.mscx"));
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
      QVERIFY(saveCompareScore(score, "part-fingering-urdel.mscx", DIR + "part-fingering-urdel.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doAddSymbol
//---------------------------------------------------------

Score* TestParts::doAddSymbol()
      {
      Score* score = readScore(DIR + "part-empty-parts.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->tick2segment(480);
      Ms::Chord* chord = static_cast<Ms::Chord*>(s->element(0));
      Note* note   = chord->upNote();
      DropData dd;
      dd.view = 0;
      Symbol* b  = new Symbol(score);
      b->setSym(SymId::gClef);
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
      QVERIFY(saveCompareScore(score, "part-symbol-add.mscx", DIR + "part-symbol-add.mscx"));
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
      QVERIFY(saveCompareScore(score, "part-symbol-uadd.mscx", DIR + "part-symbol-uadd.mscx"));
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
      QVERIFY(saveCompareScore(score, "part-symbol-uradd.mscx", DIR + "part-symbol-uradd.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doRemoveSymbol
//---------------------------------------------------------

Score* TestParts::doRemoveSymbol()
      {
      Score* score = readScore(DIR + "part-symbol-parts.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->first()->next(Segment::Type::ChordRest);
      Ms::Chord* chord = static_cast<Ms::Chord*>(s->element(0));
      Note* note   = chord->upNote();
      Element* se = 0;
      foreach(Element* e, note->el()) {
            if (e->type() == Element::Type::SYMBOL) {
                  se = e;
                  break;
                  }
            }
      score->select(se);

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
      QVERIFY(saveCompareScore(score, "part-symbol-del.mscx", DIR + "part-symbol-del.mscx"));
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
      QVERIFY(saveCompareScore(score, "part-symbol-udel.mscx", DIR + "part-symbol-udel.mscx"));
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
      QVERIFY(saveCompareScore(score, "part-symbol-urdel.mscx", DIR + "part-symbol-urdel.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doAddChordline
//---------------------------------------------------------

Score* TestParts::doAddChordline()
      {
      Score* score = readScore(DIR + "part-empty-parts.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->tick2segment(480);
      Ms::Chord* chord = static_cast<Ms::Chord*>(s->element(0));
      Note* note   = chord->upNote();
      DropData dd;
      dd.view = 0;
      ChordLine* b  = new ChordLine(score);
      b->setChordLineType(ChordLineType::FALL);
      dd.element = b;

      score->startCmd();
      note->drop(dd);
      score->endCmd();        // does layout
      return score;
      }

//---------------------------------------------------------
//   addChordline
//---------------------------------------------------------

void TestParts::addChordline()
      {
      Score* score = doAddChordline();
      QVERIFY(saveCompareScore(score, "part-chordline-add.mscx", DIR + "part-chordline-add.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoAddChordline
//---------------------------------------------------------

void TestParts::undoAddChordline()
      {
      Score* score = doAddChordline();
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-chordline-uadd.mscx", DIR + "part-chordline-uadd.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoAddChordline
//---------------------------------------------------------

void TestParts::undoRedoAddChordline()
      {
      Score* score = doAddChordline();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-chordline-uradd.mscx", DIR + "part-chordline-uradd.mscx"));
      delete score;
      }


//---------------------------------------------------------
//   doRemoveChordline
//---------------------------------------------------------

Score* TestParts::doRemoveChordline()
      {
      Score* score = readScore(DIR + "part-chordline-parts.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->first()->next(Segment::Type::ChordRest);
      Ms::Chord* chord = static_cast<Ms::Chord*>(s->element(0));

      Element* se = 0;
      foreach(Element* e, chord->el()) {
            if (e->type() == Element::Type::CHORDLINE) {
                  se = e;
                  break;
                  }
            }
      score->select(se);

      score->startCmd();
      score->cmdDeleteSelection();
      score->setLayoutAll(true);
      score->endCmd();
      return score;
      }

//---------------------------------------------------------
//   removeChordline
//---------------------------------------------------------

void TestParts::removeChordline()
      {
      Score* score = doRemoveChordline();
      QVERIFY(saveCompareScore(score, "part-chordline-del.mscx", DIR + "part-chordline-del.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRemoveChordline
//---------------------------------------------------------

void TestParts::undoRemoveChordline()
      {
      Score* score = doRemoveChordline();
      score->undo()->undo();
      score->endUndoRedo();
//      score->doLayout();
      QVERIFY(saveCompareScore(score, "part-chordline-udel.mscx", DIR + "part-chordline-udel.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoRemoveChordline
//---------------------------------------------------------

void TestParts::undoRedoRemoveChordline()
      {
      Score* score = doRemoveChordline();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-chordline-urdel.mscx", DIR + "part-chordline-urdel.mscx"));
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

//---------------------------------------------------------
//   undoRedoRemoveImage
//---------------------------------------------------------

#if 0
void TestParts::staffStyles()
      {
      Score* score = readScore(DIR + "part1.mscx");
      score->doLayout();
      QVERIFY(score);
//      int numOfStaffTypes = score->staffTypes().count();
      createParts(score);
      // check the number of staff styles did not change
//      QVERIFY(numOfStaffTypes == score->staffTypes().count());
      // modify a staff type
      int numOfLines = score->staffType(0)->lines() - 1;
      StaffType* newStaffType = score->staffType(0)->clone();
      newStaffType->setLines(numOfLines);
      score->addStaffType(0, newStaffType);
      // check the number of staff lines is correctly updated in root score and in parts
      QVERIFY(score->staff(0)->lines() == numOfLines);
      Excerpt* part = score->excerpts().at(0);
      QVERIFY(part->score()->staff(0)->lines() == numOfLines);
      part = score->excerpts().at(1);
      QVERIFY(part->score()->staff(0)->lines() == numOfLines);
      delete score;
      }
#endif


//---------------------------------------------------------
//   measureProperties
//---------------------------------------------------------

void TestParts::measureProperties()
      {
      }


QTEST_MAIN(TestParts)

#include "tst_parts.moc"

