//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/score.h"
#include "libmscore/system.h"
#include "libmscore/measure.h"
//#include "libmscore/segment.h"
//#include "libmscore/chordrest.h"
#include "libmscore/chord.h"

#define DIR QString("libmscore/copypastesymbollist/")

using namespace Ms;

//---------------------------------------------------------
//   TestCopyPaste
//---------------------------------------------------------

class TestCopyPasteSymbolList : public QObject, public MTest
      {
      Q_OBJECT

      void copypaste(const char*, const char*);

   private slots:
      void initTestCase();
      void copypasteArticulation()  { copypaste("articulation", "articulation"); }
      void copypasteChordNames()    { copypaste("chordnames", "chordnames"); }
      void copypasteChordNames1()    { copypaste("chordnames-01", "chordnames"); }
      void copypasteFiguredBass()   { copypaste("figuredbass", "figuredbass"); }
      void copypasteLyrics()        { copypaste("lyrics", "lyrics"); }

      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestCopyPasteSymbolList::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   collectMatch
//
//    (copied from mscore/musescore.cpp)
//---------------------------------------------------------

static void collectMatch(void* data, Element* e)
      {
      ElementPattern* p = static_cast<ElementPattern*>(data);
/*      if (p->type == e->type() && p->subtype != e->subtype())
            qDebug("%s subtype %d does not match", e->name(), e->subtype());
      */
//TODO      if ((p->type != e->type()) || (p->subtypeValid && p->subtype != e->subtype()))
      if (p->type != e->type())
            return;
      if ((p->staff != -1) && (p->staff != e->staffIdx()))
            return;
      if (e->type() == Element::CHORD || e->type() == Element::REST || e->type() == Element::NOTE || e->type() == Element::LYRICS) {
            if (p->voice != -1 && p->voice != e->voice())
                  return;
            }
      if (p->system) {
            Element* ee = e;
            do {
                  if (ee->type() == Element::SYSTEM) {
                        if (p->system != ee)
                              return;
                        break;
                        }
                  ee = ee->parent();
                  } while (ee);
            }
      p->el.append(e);
      }

//---------------------------------------------------------
//   copypaste
//    copy measure 2, paste into measure 4
//---------------------------------------------------------

void TestCopyPasteSymbolList::copypaste(const char* name, const char* idx)
      {
      Score* score = readScore(DIR + QString("copypastesymbollist-%1.mscx").arg(name));
      score->doLayout();

      // select all elements of a certain type (copied and slightly adapted from
      // MuseScore::selectSimilar(Element* e, bool sameStaff) in file mscore/musescore.cpp
      ElementPattern pattern;
      pattern.subtypeValid = true;
//TODO      if (type == VOLTA_SEGMENT) {
            // Volta* volta = static_cast<VoltaSegment*>(e)->volta();
            // type    = volta->type();
            // subtype = volta->subtype();
            pattern.subtypeValid = false;
//            }
      if (!strcmp(idx, "articulation"))
            pattern.type = Element::ARTICULATION;
      else if (!strcmp(idx, "chordnames"))
            pattern.type = Element::HARMONY;
      else if (!strcmp(idx, "figuredbass"))
            pattern.type = Element::FIGURED_BASS;
      else if (!strcmp(idx, "lyrics"))
            pattern.type = Element::LYRICS;
      else
            return;
      pattern.subtype = 0; // TODO subtype;
      pattern.staff   = 0;                      // select from staff 0 only
      pattern.voice   = -1;
      pattern.system  = nullptr;
      score->scanElements(&pattern, collectMatch);
      foreach(Element* e, pattern.el) {
            score->select(e, SELECT_ADD, 0);
            }

      // copy selection to clipboard
      QVERIFY(score->selection().canCopy());
      QString mimeType = score->selection().mimeType();
      QVERIFY(!mimeType.isEmpty());
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, score->selection().mimeData());
      QApplication::clipboard()->setMimeData(mimeData);

      // select first chord in 4th measure
      Measure* m = score->firstMeasure();
      for (int i=0; i < 4; i++)
            m = m->nextMeasure();
      score->select(m->first()->element(0));

      // paste clipboard to selected chord (copied and simplified to this case
      // from ScoreView::normalPaste() in file mscore/scoreview.cpp)
      score->startCmd();
      const QMimeData* ms = QApplication::clipboard()->mimeData();
      if (ms == 0 || !ms->hasFormat(mimeSymbolListFormat)) {
            qDebug("no or wrong type mime data");
            return;
            }
      ChordRest* cr = 0;
      if (score->selection().state() == SEL_RANGE)
            cr = score->selection().firstChordRest();
      else if (score->selection().isSingle()) {
            Element* e = score->selection().element();
            if (e->type() != Element::NOTE && e->type() != Element::CHORD && e->type() != Element::REST) {
                  qDebug("cannot paste to %s", e->name());
                  return;
                  }
            if (e->type() == Element::NOTE)
                  e = static_cast<Note*>(e)->chord();
            cr  = static_cast<ChordRest*>(e);
            }
      if (cr == 0) {
            qDebug("no destination chord");
            return;
            }
      else if (cr->tuplet()) {
            qDebug("cannot paste mid-tuplet");
            return;
            }
      QByteArray data(ms->data(mimeSymbolListFormat));
//      qDebug("paste <%s>", data.data());
      XmlReader e(data);
      score->pasteSymbols(e, cr);
      score->endCmd();
      score->doLayout();

      QVERIFY(saveCompareScore(score, QString("copypastesymbollist-%1.mscx").arg(name),
         DIR + QString("copypastesymbollist-%1-ref.mscx").arg(name)));
      delete score;
      }


QTEST_MAIN(TestCopyPasteSymbolList)
#include "tst_copypastesymbollist.moc"

