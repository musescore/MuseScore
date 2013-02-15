//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "globals.h"
#include "scoreview.h"
#include "preferences.h"
#include "musescore.h"
#include "textpalette.h"
#include "texttools.h"
#include "edittools.h"
#include "inspector.h"

#include "libmscore/barline.h"
#include "libmscore/utils.h"
#include "libmscore/segment.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"
#include "libmscore/text.h"
#include "libmscore/spanner.h"
#include "libmscore/measure.h"
#include "libmscore/textframe.h"

extern TextPalette* textPalette;

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void ScoreView::startEdit(Element* e)
      {
      if (e->type() == Element::TBOX)
            e = static_cast<TBox*>(e)->getText();
      origEditObject = e;
      sm->postEvent(new CommandEvent("edit"));
      _score->end();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void ScoreView::startEdit(Element* element, int startGrip)
      {
      origEditObject = element;
      startEdit();
      if (startGrip == -1)
            curGrip = grips-1;
      else if (startGrip >= 0)
            curGrip = startGrip;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void ScoreView::startEdit()
      {
      _score->setLayoutAll(false);
      curElement  = 0;
      setFocus();
      if (!_score->undo()->active())
            _score->startCmd();
      _score->deselectAll();

      if (origEditObject->isSegment()) {        // if spanner segment
            SpannerSegment* ss = (SpannerSegment*)origEditObject;
            Spanner* spanner   = ss->spanner();
            Spanner* clone     = static_cast<Spanner*>(spanner->clone());
            clone->setLinks(spanner->links());
            int idx            = spanner->spannerSegments().indexOf(ss);
            editObject         = clone->spannerSegments()[idx];

            editObject->startEdit(this, startMove);
            _score->undoChangeElement(spanner, clone);
            }
      else {
            foreach(Element* e, origEditObject->linkList()) {
                  Element* ce = e->clone();
                  if (e == origEditObject) {
                        editObject = ce;
                        editObject->setSelected(true);
                        }
                  _score->undoChangeElement(e, ce);
                  }
            editObject->layout();
            editObject->startEdit(this, startMove);
            }
      curGrip = -1;
      updateGrips();
      _score->rebuildBspTree();     // we replaced elements
      _score->end();
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void ScoreView::endEdit()
      {
      setDropTarget(0);
      if (!editObject) {
            origEditObject = 0;
            return;
            }

      _score->addRefresh(editObject->canvasBoundingRect());
      for (int i = 0; i < grips; ++i)
            score()->addRefresh(grip[i]);

      editObject->endEdit();
      if (mscore->getInspector())
            mscore->getInspector()->setElement(0);

      if (editObject->isText()) {
            if (textPalette) {
                  textPalette->hide();
                  mscore->textTools()->kbAction()->setChecked(false);
                  }
            }
      else if (editObject->isSegment()) {
            Spanner* spanner  = static_cast<SpannerSegment*>(editObject)->spanner();
            Spanner* original = static_cast<SpannerSegment*>(origEditObject)->spanner();

            bool colorChanged = editObject->color() != origEditObject->color();
            if (!spanner->isEdited(original) && !colorChanged) {
                  UndoStack* undo = _score->undo();
                  undo->current()->unwind();
                  _score->select(editObject);
                  _score->addRefresh(editObject->canvasBoundingRect());
                  _score->addRefresh(origEditObject->canvasBoundingRect());
                  _score->deselect(editObject);
                  _score->select(origEditObject);
                  delete spanner;
                  origEditObject = 0;
                  editObject = 0;
                  grips = 0;
                  _score->endCmd();
                  mscore->endCmd();
                  return;
                  }
            // handle linked elements
            LinkedElements* le = original->links();
            Element* se = spanner->startElement();
            Element* ee = spanner->endElement();
            if (le && (se != original->startElement() || ee != original->endElement())) {
                  //
                  // apply anchor changes
                  // to linked elements
                  //
                  foreach(Element* e, *le) {
                        if (e == spanner)
                              continue;
                        Spanner* lspanner = static_cast<Spanner*>(e);
                        Element* lse = 0;
                        Element* lee = 0;
                        Score* sc = lspanner->score();

                        if (se->type() == Element::NOTE || se->type() == Element::CHORD) {
                              foreach(Element* e, *se->links()) {
                                    if (e->score() == sc && e->staffIdx() == se->staffIdx()) {
                                          lse = e;
                                          break;
                                          }
                                    }
                              foreach(Element* e, *ee->links()) {
                                    if (e->score() == sc && e->staffIdx() == ee->staffIdx()) {
                                          lee = e;
                                          break;
                                          }
                                    }
                              }
                        else if (se->type() == Element::SEGMENT) {
                              int tick   = static_cast<Segment*>(se)->tick();
                              Measure* m = sc->tick2measure(tick);
                              lse        = m->findSegment(Segment::SegChordRest, tick);

                              int tick2  = static_cast<Segment*>(ee)->tick();
                              m          = sc->tick2measure(tick2);
                              lee        = m->findSegment(Segment::SegChordRest, tick2);
                              }
                        else if (se->type() == Element::MEASURE) {
                              Measure* measure = static_cast<Measure*>(se);
                              int tick         = measure->tick();
                              lse              = sc->tick2measure(tick);

                              measure          = static_cast<Measure*>(ee);
                              tick             = measure->tick();
                              lee              = sc->tick2measure(tick);
                              }
                        Q_ASSERT(lse && lee);
                        score()->undo(new ChangeSpannerAnchor(lspanner, lse, lee));
                        }
                  }
            }
      _score->addRefresh(editObject->canvasBoundingRect());

      int tp = editObject->type();
      if (tp == Element::LYRICS)
            lyricsEndEdit();
      else if (tp == Element::HARMONY)
            harmonyEndEdit();
      else if (tp == Element::FIGURED_BASS)
            figuredBassEndEdit();
      _score->endCmd();
      mscore->endCmd();

      if (dragElement && (dragElement != editObject)) {
            curElement = dragElement;
            _score->select(curElement);
            _score->end();
            }
      editObject     = 0;
      origEditObject = 0;
      grips          = 0;
      }

//---------------------------------------------------------
//   doDragEdit
//---------------------------------------------------------

void ScoreView::doDragEdit(QMouseEvent* ev)
      {
      QPointF p     = toLogical(ev->pos());
      QPointF delta = p - startMove;

      if (qApp->keyboardModifiers() == Qt::ShiftModifier) {
            p.setX(0.0);
            delta.setX(0.0);
            }
      else if (qApp->keyboardModifiers() == Qt::ControlModifier) {
            if(editObject->type() == Element::BAR_LINE)
                  BarLine::setCtrlDrag(true);
            else {
                  p.setY(0.0);
                  delta.setY(0.0);
                  }
            }

      _score->setLayoutAll(false);
      score()->addRefresh(editObject->canvasBoundingRect());
      if (editObject->isText()) {
            Text* text = static_cast<Text*>(editObject);
            text->dragTo(p);
            }
      else {
            EditData ed;
            ed.view    = this;
            ed.curGrip = curGrip;
            ed.delta   = delta;
            ed.pos     = p;
            ed.hRaster = false;
            ed.vRaster = false;
            editObject->editDrag(ed);
            updateGrips();
            startMove = p;
            }
      _score->addRefresh(editObject->canvasBoundingRect());
      _score->update();
      }

//---------------------------------------------------------
//   endDragEdit
//---------------------------------------------------------

void ScoreView::endDragEdit()
      {
      _score->addRefresh(editObject->canvasBoundingRect());
      editObject->endEditDrag();
      updateGrips();
      // setDropTarget(0); // this also resets dropRectangle and dropAnchor
      _score->rebuildBspTree();
      _score->addRefresh(editObject->canvasBoundingRect());
      _score->end();
      }


