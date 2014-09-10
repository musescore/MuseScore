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
#include "inspector/inspector.h"

#include "libmscore/barline.h"
#include "libmscore/utils.h"
#include "libmscore/segment.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"
#include "libmscore/text.h"
#include "libmscore/spanner.h"
#include "libmscore/measure.h"
#include "libmscore/textframe.h"

namespace Ms {

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void ScoreView::startEdit(Element* e)
      {
      if (!e || !e->isEditable()) {
            qDebug("The element cannot be edited");
            return;
            }
      if (e->type() == Element::Type::TBOX)
            e = static_cast<TBox*>(e)->getText();
      editObject = e;
      sm->postEvent(new CommandEvent("edit"));
      _score->end();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void ScoreView::startEdit(Element* element, int startGrip)
      {
      if (!element || !element->isEditable()) {
            qDebug("The element cannot be edited");
            return;
            }
      editObject = element;
      startEdit();
      if (startGrip == -1)
            curGrip = defaultGrip;
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
      editObject->startEdit(this, data.startMove);
      curGrip = -1;
      updateGrips();
      _score->end();
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void ScoreView::endEdit()
      {
      setDropTarget(0);
      if (!editObject)
            return;
      editObject->endEditDrag();
      _score->addRefresh(editObject->canvasBoundingRect());
      for (int i = 0; i < grips; ++i)
            score()->addRefresh(grip[i]);

      editObject->endEdit();
      if (mscore->inspector())
            mscore->inspector()->setElement(0);

      _score->addRefresh(editObject->canvasBoundingRect());

      Element::Type tp = editObject->type();
      if (tp == Element::Type::LYRICS)
            lyricsEndEdit();
      else if (tp == Element::Type::HARMONY)
            harmonyEndEdit();
      else if (tp == Element::Type::FIGURED_BASS)
            figuredBassEndEdit();
      else if (editObject->isText()) {
            Text* text = static_cast<Text*>(editObject);
            if (text->isEmpty())
                  _score->undoRemoveElement(text);
            editObject = nullptr;
            }

      _score->endCmd();
      mscore->endCmd();

      if (dragElement && (dragElement != editObject)) {
            curElement = dragElement;
            _score->select(curElement);
            _score->end();
            }
      editObject     = nullptr;
      grips          = 0;
      }

//---------------------------------------------------------
//   editElementDragTransition
//    (start dragEdit)
//---------------------------------------------------------

bool ScoreView::editElementDragTransition(QMouseEvent* ev)
      {
      data.startMove = toLogical(ev->pos());
      data.lastPos   = data.startMove;
      data.pos       = data.startMove;
      data.view      = this;

      Element* e = elementNear(data.startMove);
      if (e && (e == editObject) && (editObject->isText())) {
            if (editObject->mousePress(data.startMove, ev)) {
                  _score->addRefresh(editObject->canvasBoundingRect());
                  _score->end();
                  }
            return true;
            }
      int i;
      qreal a = grip[0].width() * 1.0;
      for (i = 0; i < grips; ++i) {
            if (grip[i].adjusted(-a, -a, a, a).contains(data.startMove)) {
                  curGrip = i;
                  data.curGrip = i;
                  updateGrips();
                  score()->end();
                  break;
                  }
            }
      return i != grips;
      }

//---------------------------------------------------------
//   doDragEdit
//---------------------------------------------------------

void ScoreView::doDragEdit(QMouseEvent* ev)
      {
      data.lastPos = data.pos;
      data.pos     = toLogical(ev->pos());

      // on bar lines, Ctrl (single bar line) and Shift (precision drag) modifiers can be active independently
      if (editObject->type() == Element::Type::BAR_LINE) {
            if (qApp->keyboardModifiers() & Qt::ShiftModifier)
                  BarLine::setShiftDrag(true);
            if (qApp->keyboardModifiers() & Qt::ControlModifier)
                  BarLine::setCtrlDrag(true);
            }
      // on other elements, BOTH Ctrl (vert. constrain) and Shift (horiz. constrain) modifiers = NO constrain
      else {
            if (qApp->keyboardModifiers() == Qt::ShiftModifier)
                  data.pos.setX(data.lastPos.x());
            if (qApp->keyboardModifiers() == Qt::ControlModifier)
                  data.pos.setY(data.lastPos.y());
            }
      data.delta = data.pos - data.lastPos;

      _score->setLayoutAll(false);
      score()->addRefresh(editObject->canvasBoundingRect());
      if (editObject->isText()) {
            Text* text = static_cast<Text*>(editObject);
            text->dragTo(data.pos);
            }
      else {
            data.hRaster = false;
            data.vRaster = false;
            editObject->editDrag(data);
            updateGrips();
            }
      QRectF r(editObject->canvasBoundingRect());
      _score->addRefresh(r);
      _score->update();
      }

//---------------------------------------------------------
//   endDragEdit
//---------------------------------------------------------

void ScoreView::endDragEdit()
      {
      _score->addRefresh(editObject->canvasBoundingRect());
      editObject->endEditDrag();
      setDropTarget(0);
      updateGrips();
      _score->rebuildBspTree();
      _score->addRefresh(editObject->canvasBoundingRect());
      _score->end();
      }

}

