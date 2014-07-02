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
#include "libmscore/rest.h"
#include "libmscore/segment.h"
#include "libmscore/image.h"
#include "libmscore/element.h"
#include "libmscore/staff.h"
#include "libmscore/stafftype.h"

#include "libmscore/sym.h"
#include "libmscore/text.h"
#include "libmscore/box.h"
#include "libmscore/arpeggio.h"
#include "libmscore/breath.h"
#include "libmscore/glissando.h"
#include "libmscore/repeat.h"
#include "libmscore/image.h"
#include "libmscore/articulation.h"
#include "libmscore/chordline.h"
#include "libmscore/dynamic.h"
#include "libmscore/fingering.h"
#include "libmscore/stafftext.h"

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

      Score* doAddSymbol();
      Score* doRemoveSymbol();

      Score* doAddText();
      Score* doRemoveText();

      Score* doAddArpeggio();
      Score* doRemoveArpeggio();

      Score* doAddBreath();
      Score* doRemoveBreath();

      Score* doAddGlissando();
      Score* doRemoveGlissando();

      Score* doAddMrepeat();
      Score* doRemoveMrepeat();

      Score* doAddImage();
      Score* doRemoveImage();

      Score* doAddArticulation();
      Score* doRemoveArticulation();

      Score* doAddChordline();
      Score* doRemoveChordline();

      Score* doAddDynamic();
      Score* doRemoveDynamic();

      Score* doAddFingering();
      Score* doRemoveFingering();

      Score* doAddStaffText();
      Score* doRemoveStaffText();

   private slots:
      void initTestCase();

      void createPart1();
      void createPart2();

      void createPartSymbol();
      void addSymbol();
      void undoAddSymbol();
      void undoRedoAddSymbol();
      void removeSymbol();
      void undoRemoveSymbol();
      void undoRedoRemoveSymbol();

      void createPartText();
      void addText();
      void undoAddText();
      void undoRedoAddText();
      void removeText();
      void undoRemoveText();
      void undoRedoRemoveText();

      void createPartArpeggio();
      void addArpeggio();
      void undoAddArpeggio();
      void undoRedoAddArpeggio();
      void removeArpeggio();
      void undoRemoveArpeggio();
      void undoRedoRemoveArpeggio();

      void createPartBreath();
      void addBreath();
      void undoAddBreath();
      void undoRedoAddBreath();
      void removeBreath();
      void undoRemoveBreath();
      void undoRedoRemoveBreath();

      void createPartGlissando();
      void addGlissando();
      void undoAddGlissando();
      void undoRedoAddGlissando();
      void removeGlissando();
      void undoRemoveGlissando();
      void undoRedoRemoveGlissando();

      void createPartMrepeat();
      void addMrepeat();
      void undoAddMrepeat();
      void undoRedoAddMrepeat();
      void removeMrepeat();
      void undoRemoveMrepeat();
      void undoRedoRemoveMrepeat();

      void createPartImage();
      void addImage();
      void undoAddImage();
      void undoRedoAddImage();
      void removeImage();
      void undoRemoveImage();
      void undoRedoRemoveImage();

      void createPartArticulation();
      void addArticulation();
      void undoAddArticulation();
      void undoRedoAddArticulation();
      void removeArticulation();
      void undoRemoveArticulation();
      void undoRedoRemoveArticulation();

      void createPartChordline();
      void addChordline();
      void undoAddChordline();
      void undoRedoAddChordline();
      void removeChordline();
      void undoRemoveChordline();
      void undoRedoRemoveChordline();

      void createPartDynamic();
      void addDynamic();
      void undoAddDynamic();
      void undoRedoAddDynamic();
      void removeDynamic();
      void undoRemoveDynamic();
      void undoRedoRemoveDynamic();

      void createPartFingering();
      void addFingering();
      void undoAddFingering();
      void undoRedoAddFingering();
      void removeFingering();
      void undoRemoveFingering();
      void undoRedoRemoveFingering();

      void createPartStaffText();
      void addStaffText();
      void undoAddStaffText();
      void undoRedoAddStaffText();
      void removeStaffText();
      void undoRemoveStaffText();
      void undoRedoRemoveStaffText();

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
      Score* nscore = ::createExcerpt(parts);
      QVERIFY(nscore);

      nscore->setName(parts.front()->partName());
      score->undo(new AddExcerpt(nscore));
      nscore->style()->set(StyleIdx::createMultiMeasureRests, true);

      //
      // create second part
      //
      parts.clear();
      parts.append(score->parts().at(1));
      nscore = ::createExcerpt(parts);
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

void TestParts::createPartSymbol()
      {
      testPartCreation("part-symbol");
      }

void TestParts::createPartText()
      {
      testPartCreation("part-text");
      }

void TestParts::createPartArpeggio()
      {
      testPartCreation("part-arpeggio");
      }

void TestParts::createPartBreath()
      {
      testPartCreation("part-breath");
      }

void TestParts::createPartGlissando()
      {
      testPartCreation("part-glissando");
      }

void TestParts::createPartMrepeat()
      {
      testPartCreation("part-mrepeat");
      }

void TestParts::createPartImage()
      {
      testPartCreation("part-image");
      }

void TestParts::createPartArticulation()
      {
      testPartCreation("part-articulation");
      }

void TestParts::createPartChordline()
      {
      testPartCreation("part-chordline");
      }

void TestParts::createPartDynamic()
      {
      testPartCreation("part-dynamic");
      }

void TestParts::createPartFingering()
      {
      testPartCreation("part-fingering");
      }

void TestParts::createPartStaffText()
      {
      testPartCreation("part-stafftext");
      }

#if 0
void TestParts::createPartImage()
      {
      testPartCreation("part-image");
      }
#endif

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
//   doAddText
//---------------------------------------------------------

Score* TestParts::doAddText()
      {
      Score* score = readScore(DIR + "part-empty-parts.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      VBox* m   = static_cast<VBox*>(score->measures()->first());
      DropData dd;
      dd.view = 0;
      Text* e  = new Text(score);
      e->setText("The Composer");
      e->setTextStyleType(TextStyleType::COMPOSER);
      dd.element = e;

      score->startCmd();
      m->drop(dd);
      score->endCmd();        // does layout
      return score;
      }

//---------------------------------------------------------
//   addText
//---------------------------------------------------------

void TestParts::addText()
      {
      Score* score = doAddText();
      QVERIFY(saveCompareScore(score, "part-text-add.mscx", DIR + "part-text-add.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoAddText
//---------------------------------------------------------

void TestParts::undoAddText()
      {
      Score* score = doAddText();
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-text-uadd.mscx", DIR + "part-text-uadd.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoAddText
//---------------------------------------------------------

void TestParts::undoRedoAddText()
      {
      Score* score = doAddText();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-text-uradd.mscx", DIR + "part-text-uradd.mscx"));
      delete score;
      }


//---------------------------------------------------------
//   doRemoveText
//---------------------------------------------------------

Score* TestParts::doRemoveText()
      {
      Score* score = readScore(DIR + "part-text-parts.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      VBox* m   = static_cast<VBox*>(score->measures()->first());
      Element* se;
      foreach (Element* e, *m->el()) {
            if (e->type() == Element::Type::TEXT) {
                  Text* t = static_cast<Text*>(e);
                  if (t->textStyleType() == TextStyleType::COMPOSER) {
                        se = e;
                        break;
                        }
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
//   removeText
//---------------------------------------------------------

void TestParts::removeText()
      {
      Score* score = doRemoveText();
      QVERIFY(saveCompareScore(score, "part-text-del.mscx", DIR + "part-text-del.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRemoveText
//---------------------------------------------------------

void TestParts::undoRemoveText()
      {
      Score* score = doRemoveText();
      score->undo()->undo();
      score->endUndoRedo();
//      score->doLayout();
      QVERIFY(saveCompareScore(score, "part-text-udel.mscx", DIR + "part-text-udel.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoRemoveText
//---------------------------------------------------------

void TestParts::undoRedoRemoveText()
      {
      Score* score = doRemoveText();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-text-urdel.mscx", DIR + "part-text-urdel.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doAddArpeggio
//---------------------------------------------------------

Score* TestParts::doAddArpeggio()
      {
      Score* score = readScore(DIR + "part-empty-parts.mscx");
      score->doLayout();
      foreach (Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->tick2segment(480);
      Chord* chord = static_cast<Ms::Chord*>(s->element(0));
      Note* note   = chord->upNote();
      score->select(note);
      QList<Note*> nl;
      nl.append(note);
      score->cmdAddInterval(-8, nl);

      DropData dd;
      dd.view = 0;
      Arpeggio* e  = new Arpeggio(score);
      dd.element = e;

      score->startCmd();
      note->drop(dd);
      score->endCmd();        // does layout
      return score;
      }

//---------------------------------------------------------
//   addArpeggio
//---------------------------------------------------------

void TestParts::addArpeggio()
      {
      Score* score = doAddArpeggio();
      QVERIFY(saveCompareScore(score, "part-arpeggio-add.mscx", DIR + "part-arpeggio-add.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoAddArpeggio
//---------------------------------------------------------

void TestParts::undoAddArpeggio()
      {
      Score* score = doAddArpeggio();
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-arpeggio-uadd.mscx", DIR + "part-arpeggio-uadd.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoAddArpeggio
//---------------------------------------------------------

void TestParts::undoRedoAddArpeggio()
      {
      Score* score = doAddArpeggio();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-arpeggio-uradd.mscx", DIR + "part-arpeggio-uradd.mscx"));
      delete score;
      }


//---------------------------------------------------------
//   doRemoveArpeggio
//---------------------------------------------------------

Score* TestParts::doRemoveArpeggio()
      {
      Score* score = readScore(DIR + "part-arpeggio-parts.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->first()->next(Segment::Type::ChordRest);
      Ms::Chord* chord = static_cast<Ms::Chord*>(s->element(0));

      Element* se = chord->arpeggio();
      score->select(se);

      score->startCmd();
      score->cmdDeleteSelection();
      score->setLayoutAll(true);
      score->endCmd();
      return score;
      }

//---------------------------------------------------------
//   removeArpeggio
//---------------------------------------------------------

void TestParts::removeArpeggio()
      {
      Score* score = doRemoveArpeggio();
      QVERIFY(saveCompareScore(score, "part-arpeggio-del.mscx", DIR + "part-arpeggio-del.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRemoveArpeggio
//---------------------------------------------------------

void TestParts::undoRemoveArpeggio()
      {
      Score* score = doRemoveArpeggio();
      score->undo()->undo();
      score->endUndoRedo();
//      score->doLayout();
      QVERIFY(saveCompareScore(score, "part-arpeggio-udel.mscx", DIR + "part-arpeggio-udel.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoRemoveArpeggio
//---------------------------------------------------------

void TestParts::undoRedoRemoveArpeggio()
      {
      Score* score = doRemoveArpeggio();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-arpeggio-urdel.mscx", DIR + "part-arpeggio-urdel.mscx"));
      delete score;
      }

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
//   doAddGlissando
//---------------------------------------------------------

Score* TestParts::doAddGlissando()
      {
      Score* score = readScore(DIR + "part-empty-parts.mscx");
      score->doLayout();
      foreach (Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->tick2segment(480);
      Chord* chord = static_cast<Ms::Chord*>(s->element(0));
      Note* note   = chord->upNote();

      DropData dd;
      dd.view = 0;
      Glissando* e  = new Glissando(score);
      dd.element = e;

      score->startCmd();
      note->drop(dd);
      score->endCmd();        // does layout
      return score;
      }

//---------------------------------------------------------
//   addGlissando
//---------------------------------------------------------

void TestParts::addGlissando()
      {
      Score* score = doAddGlissando();
      QVERIFY(saveCompareScore(score, "part-glissando-add.mscx", DIR + "part-glissando-add.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoAddGlissando
//---------------------------------------------------------

void TestParts::undoAddGlissando()
      {
      Score* score = doAddGlissando();
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-glissando-uadd.mscx", DIR + "part-glissando-uadd.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoAddGlissando
//---------------------------------------------------------

void TestParts::undoRedoAddGlissando()
      {
      Score* score = doAddGlissando();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-glissando-uradd.mscx", DIR + "part-glissando-uradd.mscx"));
      delete score;
      }


//---------------------------------------------------------
//   doRemoveGlissando
//---------------------------------------------------------

Score* TestParts::doRemoveGlissando()
      {
      Score* score = readScore(DIR + "part-glissando-parts.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->first()->next(Segment::Type::ChordRest)->next();
      Ms::Chord* chord = static_cast<Ms::Chord*>(s->element(0));

      Element* se = chord->glissando();
      score->select(se);

      score->startCmd();
      score->cmdDeleteSelection();
      score->setLayoutAll(true);
      score->endCmd();
      return score;
      }

//---------------------------------------------------------
//   removeGlissando
//---------------------------------------------------------

void TestParts::removeGlissando()
      {
      Score* score = doRemoveGlissando();
      QVERIFY(saveCompareScore(score, "part-glissando-del.mscx", DIR + "part-glissando-del.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRemoveGlissando
//---------------------------------------------------------

void TestParts::undoRemoveGlissando()
      {
      Score* score = doRemoveGlissando();
      score->undo()->undo();
      score->endUndoRedo();
//      score->doLayout();
      QVERIFY(saveCompareScore(score, "part-glissando-udel.mscx", DIR + "part-glissando-udel.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoRemoveGlissando
//---------------------------------------------------------

void TestParts::undoRedoRemoveGlissando()
      {
      Score* score = doRemoveGlissando();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-glissando-urdel.mscx", DIR + "part-glissando-urdel.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doAddMrepeat
//---------------------------------------------------------

Score* TestParts::doAddMrepeat()
      {
      Score* score = readScore(DIR + "part-empty-parts.mscx");
      score->doLayout();
      foreach (Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m = score->firstMeasure()->nextMeasure();
      Segment* s = m->first();
      Rest* rest = static_cast<Rest*>(s->element(0));

      DropData dd;
      dd.view = 0;
      RepeatMeasure* e = new RepeatMeasure(score);
      dd.element = e;

      score->startCmd();
      rest->drop(dd);
      score->endCmd();        // does layout
      return score;
      }

//---------------------------------------------------------
//   addMrepeat
//---------------------------------------------------------

void TestParts::addMrepeat()
      {
      Score* score = doAddMrepeat();
      QVERIFY(saveCompareScore(score, "part-mrepeat-add.mscx", DIR + "part-mrepeat-add.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoAddMrepeat
//---------------------------------------------------------

void TestParts::undoAddMrepeat()
      {
      Score* score = doAddMrepeat();
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-mrepeat-uadd.mscx", DIR + "part-mrepeat-uadd.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoAddMrepeat
//---------------------------------------------------------

void TestParts::undoRedoAddMrepeat()
      {
      Score* score = doAddMrepeat();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-mrepeat-uradd.mscx", DIR + "part-mrepeat-uradd.mscx"));
      delete score;
      }


//---------------------------------------------------------
//   doRemoveMrepeat
//---------------------------------------------------------

Score* TestParts::doRemoveMrepeat()
      {
      Score* score = readScore(DIR + "part-mrepeat-parts.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m        = score->firstMeasure()->nextMeasure();
      Segment* s        = m->first(Segment::Type::ChordRest);
      RepeatMeasure* se = static_cast<RepeatMeasure*>(s->element(0));
      score->select(se);

      score->startCmd();
      score->cmdDeleteSelection();
      score->setLayoutAll(true);
      score->endCmd();
      return score;
      }

//---------------------------------------------------------
//   removeMrepeat
//---------------------------------------------------------

void TestParts::removeMrepeat()
      {
      Score* score = doRemoveMrepeat();
      QVERIFY(saveCompareScore(score, "part-mrepeat-del.mscx", DIR + "part-mrepeat-del.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRemoveMrepeat
//---------------------------------------------------------

void TestParts::undoRemoveMrepeat()
      {
      Score* score = doRemoveMrepeat();
      score->undo()->undo();
      score->endUndoRedo();
//      score->doLayout();
      QVERIFY(saveCompareScore(score, "part-mrepeat-udel.mscx", DIR + "part-mrepeat-udel.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoRemoveMrepeat
//---------------------------------------------------------

void TestParts::undoRedoRemoveMrepeat()
      {
      Score* score = doRemoveMrepeat();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-mrepeat-urdel.mscx", DIR + "part-mrepeat-urdel.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doAddImage
//---------------------------------------------------------

Score* TestParts::doAddImage()
      {
      Score* score = readScore(DIR + "part-empty-parts.mscx");
      score->doLayout();
      foreach (Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->first(Segment::Type::ChordRest)->next();
      Chord* chord = static_cast<Chord*>(s->element(0));
      Note* note   = chord->upNote();

      DropData dd;
      dd.view = 0;
      Image* e = new Image(score);
      e->load("schnee.png");
      dd.element = e;

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
      QVERIFY(saveCompareScore(score, "part-image-add.mscx", DIR + "part-image-add.mscx"));
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
      QVERIFY(saveCompareScore(score, "part-image-uadd.mscx", DIR + "part-image-uadd.mscx"));
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
      QVERIFY(saveCompareScore(score, "part-image-uradd.mscx", DIR + "part-image-uradd.mscx"));
      delete score;
      }


//---------------------------------------------------------
//   doRemoveImage
//---------------------------------------------------------

Score* TestParts::doRemoveImage()
      {
      Score* score = readScore(DIR + "part-image-parts.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->first(Segment::Type::ChordRest);
      Chord* chord = static_cast<Chord*>(s->element(0));
      Note* note   = chord->upNote();
      Element* se = 0;
      foreach (Element* e, note->el()) {
            if (e->type() == Element::Type::IMAGE) {
                  se = e;
                  break;
                  }
            }
      if (!se)
            qFatal("no annotation");
      score->select(se);

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
      QVERIFY(saveCompareScore(score, "part-image-del.mscx", DIR + "part-image-del.mscx"));
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
//      score->doLayout();
      QVERIFY(saveCompareScore(score, "part-image-udel.mscx", DIR + "part-image-udel.mscx"));
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
      QVERIFY(saveCompareScore(score, "part-image-urdel.mscx", DIR + "part-image-urdel.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   doAddArticulation
//---------------------------------------------------------

Score* TestParts::doAddArticulation()
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
      Articulation* e  = new Articulation(score);
      e->setArticulationType(ArticulationType::Fermata);
      dd.element = e;

      score->startCmd();
      note->drop(dd);
      score->endCmd();        // does layout

      s = m->tick2segment(480*3);
      Ms::Rest* rest = static_cast<Ms::Rest*>(s->element(0));
      dd.view = 0;
      e  = new Articulation(score);
      e->setArticulationType(ArticulationType::Fermata);
      dd.element = e;

      score->startCmd();
      rest->drop(dd);
      score->endCmd();        // does layout

      return score;
      }

//---------------------------------------------------------
//   addArticulation
//---------------------------------------------------------

void TestParts::addArticulation()
      {
      Score* score = doAddArticulation();
      QVERIFY(saveCompareScore(score, "part-articulation-add.mscx", DIR + "part-articulation-add.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoAddArticulation
//---------------------------------------------------------

void TestParts::undoAddArticulation()
      {
      Score* score = doAddArticulation();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-articulation-uadd.mscx", DIR + "part-articulation-uadd.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoAddArticulation
//---------------------------------------------------------

void TestParts::undoRedoAddArticulation()
      {
      Score* score = doAddArticulation();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-articulation-uradd.mscx", DIR + "part-articulation-uradd.mscx"));
      delete score;
      }


//---------------------------------------------------------
//   doRemoveArticulation
//---------------------------------------------------------

Score* TestParts::doRemoveArticulation()
      {
      Score* score = readScore(DIR + "part-articulation-parts.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->first()->next(Segment::Type::ChordRest);
      Ms::Chord* chord = static_cast<Ms::Chord*>(s->element(0));

      Element* se = 0;
      foreach (Element* e, chord->articulations()) {
            if (e->type() == Element::Type::ARTICULATION) {
                  se = e;
                  break;
                  }
            }
      score->select(se);

      score->startCmd();
      score->cmdDeleteSelection();
      score->setLayoutAll(true);
      score->endCmd();

      s = s->next()->next()->next();
      Ms::Rest* rest = static_cast<Ms::Rest*>(s->element(0));

      se = 0;
      foreach (Element* e, rest->articulations()) {
            if (e->type() == Element::Type::ARTICULATION) {
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
//   removeArticulation
//---------------------------------------------------------

void TestParts::removeArticulation()
      {
      Score* score = doRemoveArticulation();
      QVERIFY(saveCompareScore(score, "part-articulation-del.mscx", DIR + "part-articulation-del.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRemoveArticulation
//---------------------------------------------------------

void TestParts::undoRemoveArticulation()
      {
      Score* score = doRemoveArticulation();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->undo();
      score->endUndoRedo();
//      score->doLayout();
      QVERIFY(saveCompareScore(score, "part-articulation-udel.mscx", DIR + "part-articulation-udel.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoRemoveArticulation
//---------------------------------------------------------

void TestParts::undoRedoRemoveArticulation()
      {
      Score* score = doRemoveArticulation();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-articulation-urdel.mscx", DIR + "part-articulation-urdel.mscx"));
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

//---------------------------------------------------------
//   doAddDynamic
//---------------------------------------------------------

Score* TestParts::doAddDynamic()
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
      Dynamic* e  = new Dynamic(score);
      e->setDynamicType("mf");
      dd.element = e;

      score->startCmd();
      note->drop(dd);
      score->endCmd();        // does layout
      return score;
      }

//---------------------------------------------------------
//   addDynamic
//---------------------------------------------------------

void TestParts::addDynamic()
      {
      Score* score = doAddDynamic();
      QVERIFY(saveCompareScore(score, "part-dynamic-add.mscx", DIR + "part-dynamic-add.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoAddDynamic
//---------------------------------------------------------

void TestParts::undoAddDynamic()
      {
      Score* score = doAddDynamic();
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-dynamic-uadd.mscx", DIR + "part-dynamic-uadd.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoAddDynamic
//---------------------------------------------------------

void TestParts::undoRedoAddDynamic()
      {
      Score* score = doAddDynamic();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-dynamic-uradd.mscx", DIR + "part-dynamic-uradd.mscx"));
      delete score;
      }


//---------------------------------------------------------
//   doRemoveDynamic
//---------------------------------------------------------

Score* TestParts::doRemoveDynamic()
      {
      Score* score = readScore(DIR + "part-dynamic-parts.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->first()->next(Segment::Type::ChordRest);

      Element* se = 0;
      foreach (Element* e, s->annotations()) {
            if (e->type() == Element::Type::DYNAMIC) {
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
//   removeDynamic
//---------------------------------------------------------

void TestParts::removeDynamic()
      {
      Score* score = doRemoveDynamic();
      QVERIFY(saveCompareScore(score, "part-dynamic-del.mscx", DIR + "part-dynamic-del.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRemoveDynamic
//---------------------------------------------------------

void TestParts::undoRemoveDynamic()
      {
      Score* score = doRemoveDynamic();
      score->undo()->undo();
      score->endUndoRedo();
//      score->doLayout();
      QVERIFY(saveCompareScore(score, "part-dynamic-udel.mscx", DIR + "part-dynamic-udel.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoRemoveDynamic
//---------------------------------------------------------

void TestParts::undoRedoRemoveDynamic()
      {
      Score* score = doRemoveDynamic();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-dynamic-urdel.mscx", DIR + "part-dynamic-urdel.mscx"));
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
//   doAddStaffText
//---------------------------------------------------------

Score* TestParts::doAddStaffText()
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
      StaffText* e  = new StaffText(score);
      e->setText("Hello, world");
      dd.element = e;

      score->startCmd();
      note->drop(dd);
      score->endCmd();        // does layout
      return score;
      }

//---------------------------------------------------------
//   addStaffText
//---------------------------------------------------------

void TestParts::addStaffText()
      {
      Score* score = doAddStaffText();
      QVERIFY(saveCompareScore(score, "part-stafftext-add.mscx", DIR + "part-stafftext-add.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoAddStaffText
//---------------------------------------------------------

void TestParts::undoAddStaffText()
      {
      Score* score = doAddStaffText();
      score->undo()->undo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-stafftext-uadd.mscx", DIR + "part-stafftext-uadd.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoAddStaffText
//---------------------------------------------------------

void TestParts::undoRedoAddStaffText()
      {
      Score* score = doAddStaffText();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-stafftext-uradd.mscx", DIR + "part-stafftext-uradd.mscx"));
      delete score;
      }


//---------------------------------------------------------
//   doRemoveStaffText
//---------------------------------------------------------

Score* TestParts::doRemoveStaffText()
      {
      Score* score = readScore(DIR + "part-stafftext-parts.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m   = score->firstMeasure();
      Segment* s   = m->first()->next(Segment::Type::ChordRest);

      Element* se = 0;
      foreach (Element* e, s->annotations()) {
            if (e->type() == Element::Type::STAFF_TEXT) {
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
//   removeStaffText
//---------------------------------------------------------

void TestParts::removeStaffText()
      {
      Score* score = doRemoveStaffText();
      QVERIFY(saveCompareScore(score, "part-stafftext-del.mscx", DIR + "part-stafftext-del.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRemoveStaffText
//---------------------------------------------------------

void TestParts::undoRemoveStaffText()
      {
      Score* score = doRemoveStaffText();
      score->undo()->undo();
      score->endUndoRedo();
//      score->doLayout();
      QVERIFY(saveCompareScore(score, "part-stafftext-udel.mscx", DIR + "part-stafftext-udel.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   undoRedoRemoveStaffText
//---------------------------------------------------------

void TestParts::undoRedoRemoveStaffText()
      {
      Score* score = doRemoveStaffText();
      score->undo()->undo();
      score->endUndoRedo();
      score->undo()->redo();
      score->endUndoRedo();
      QVERIFY(saveCompareScore(score, "part-stafftext-urdel.mscx", DIR + "part-stafftext-urdel.mscx"));
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

