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
//   updateGrips
//    if (curGrip == -1) then initialize to element
//    default grip
//---------------------------------------------------------

void ScoreView::updateGrips()
      {
      if (!editData.element)
            return;

      double dx = 1.5 / _matrix.m11();
      double dy = 1.5 / _matrix.m22();

      for (const QRectF& r : editData.grip)
            score()->addRefresh(r.adjusted(-dx, -dy, dx, dy));

      qreal w   = 8.0 / _matrix.m11();
      qreal h   = 8.0 / _matrix.m22();
      QRectF r(-w*.5, -h*.5, w, h);

      editData.grip.resize(editData.grips);

      if (editData.grips) {
            for (QRectF& gr : editData.grip)
                  gr = r;
            editData.element->updateGrips(editData);

            // updateGrips returns grips in page coordinates,
            // transform to view coordinates:

            Element* page = editData.element;
            while (page->parent())
                  page = page->parent();
            QPointF pageOffset(page->pos());

            for (QRectF& grip : editData.grip) {
                  grip.translate(pageOffset);
                  score()->addRefresh(grip.adjusted(-dx, -dy, dx, dy));
                  }

            QPointF anchor = editData.element->gripAnchor(editData.curGrip);
            if (!anchor.isNull())
                  setDropAnchor(QLineF(anchor + pageOffset, editData.grip[int(editData.curGrip)].center()));
            else
                  setDropTarget(0); // this also resets dropAnchor
            }
      score()->addRefresh(editData.element->canvasBoundingRect());
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void ScoreView::startEditMode(Element* e)
      {
      if (!e || !e->isEditable()) {
            qDebug("The element cannot be edited");
            return;
            }
      if (score()->undoStack()->active())
            score()->endCmd();
      editData.element = e;
      changeState(ViewState::EDIT);
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void ScoreView::startEdit(Element* element, Grip startGrip)
      {
      if (!element || !element->isEditable()) {
            qDebug("The element cannot be edited");
            return;
            }

      const bool forceStartEdit = (state == ViewState::EDIT && element != editData.element);
      editData.element = element;
      if (forceStartEdit) // call startEdit() forcibly to reinitialize edit mode.
            startEdit();
      else
            changeState(ViewState::EDIT);

      if (startGrip != Grip::NO_GRIP)
            editData.curGrip = startGrip;
      }

//---------------------------------------------------------
//   startEdit
//    enter state EDIT
//---------------------------------------------------------

void ScoreView::startEdit()
      {
      if (editData.element->isTBox())
            editData.element = toTBox(editData.element)->text();
      setFocus();
      editData.grips   = 0;
      editData.curGrip = Grip(0);
      editData.clearData();

      editData.element->startEdit(editData);
      updateGrips();

      QGuiApplication::inputMethod()->reset();
      QGuiApplication::inputMethod()->update(Qt::ImCursorRectangle);
      setAttribute(Qt::WA_InputMethodEnabled, editData.element->isTextBase());
      _score->update();
      setCursor(QCursor(Qt::ArrowCursor));
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void ScoreView::endEdit()
      {
      setAttribute(Qt::WA_InputMethodEnabled, false);
      setDropTarget(0);
      if (!editData.element)
            return;
      _score->addRefresh(editData.element->canvasBoundingRect());
      for (int i = 0; i < editData.grips; ++i)
            score()->addRefresh(editData.grip[i]);
      editData.element->endEdit(editData);

      if (editData.element) {
            _score->addRefresh(editData.element->canvasBoundingRect());
            ElementType tp = editData.element->type();
            if (tp == ElementType::LYRICS)
                  lyricsEndEdit();
            else if (tp == ElementType::HARMONY)
                  harmonyEndEdit();
            else if (tp == ElementType::FIGURED_BASS)
                  figuredBassEndEdit();
            }
      editData.clearData();
      mscore->updateInspector();
      }

//---------------------------------------------------------
//   doDragEdit
//---------------------------------------------------------

void ScoreView::doDragEdit(QMouseEvent* ev)
      {
      editData.lastPos   = editData.pos;
      editData.pos       = toLogical(ev->pos());
      editData.modifiers = qApp->keyboardModifiers();

      if (!editData.element->isBarLine()) {
            // on other elements, BOTH Ctrl (vert. constrain) and Shift (horiz. constrain) modifiers = NO constrain
            if (qApp->keyboardModifiers() == Qt::ShiftModifier)
                  editData.pos.setX(editData.lastPos.x());
            if (qApp->keyboardModifiers() == Qt::ControlModifier)
                  editData.pos.setY(editData.lastPos.y());
            }
      editData.delta = editData.pos - editData.lastPos;
      score()->addRefresh(editData.element->canvasBoundingRect());

      if (editData.element->isTextBase()) {
            toTextBase(editData.element)->dragTo(editData);
#if 0
            if (editData.element->shape().translated(editData.element->pagePos()).contains(editData.pos)) {
                  qDebug("in");
                  toTextBase(editData.element)->dragTo(editData);
                  }
            else {
                  qDebug("out");
                  editData.hRaster = false;
                  editData.vRaster = false;
                  editData.element->editDrag(editData);
                  updateGrips();
                  }
#endif
            }
      else {
            editData.hRaster = false;
            editData.vRaster = false;
            editData.element->editDrag(editData);
            updateGrips();
            }
      QRectF r(editData.element->canvasBoundingRect());
      _score->addRefresh(r);
      _score->update();
      }

//---------------------------------------------------------
//   endDragEdit
//---------------------------------------------------------

void ScoreView::endDragEdit()
      {
      _score->addRefresh(editData.element->canvasBoundingRect());

      editData.element->endEditDrag(editData);
      updateGrips();
      _score->addRefresh(editData.element->canvasBoundingRect());
      setDropTarget(0);
      score()->endCmd();            // calls update()
      _score->rebuildBspTree();
      }
}

