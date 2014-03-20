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
      if (e->type() == Element::TBOX)
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
      editObject = element;
      startEdit();
      if (startGrip == -1)
            aGrip.curGrip = aGrip.grips-1;
      else if (startGrip >= 0)
            aGrip.curGrip = startGrip;
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
      aGrip.curGrip = -1;
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
/*
      Element* page = editObject;
      while (page->parent())
            page = page->parent();
      QPointF pageOffset(page->pos());
*/

      _score->addRefresh(editObject->canvasBoundingRect());
      for (int i = 0; i < aGrip.grips; ++i)
            score()->addRefresh(aGrip.grip[i]);

      updateRefreshAlines();
/*
      for (int i = 0; i < aGrip.aLines; ++i) {
            if( aGrip.vert[i] ){
                  QRectF vline (
                                aGrip.aLine[i].x() + pageOffset.x() - aGrip.grip[0].width()/2,
                                0,
                                aGrip.grip[0].width(),
                                page->height());
                  score()->addRefresh(vline);
                  }
            else{
                  QRectF hline (
                                pageOffset.x(),
                                aGrip.aLine[i].y() - aGrip.grip[0].height()/2,
                                page->width(),
                                aGrip.grip[0].height()
                                );
                  score()->addRefresh(hline);
                  }
            }

*/
      editObject->endEdit();
      if (mscore->getInspector())
            mscore->getInspector()->setElement(0);

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
      aGrip.grips          = 0;
      }

//---------------------------------------------------------
//   doDragEdit
//---------------------------------------------------------

void ScoreView::doDragEdit(QMouseEvent* ev)
      {
      QPointF p     = toLogical(ev->pos());
      QPointF delta = p - data.startMove;

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
            ed.curGrip = aGrip.curGrip;
            ed.delta   = delta;
            ed.pos     = p;
            ed.hRaster = false;
            ed.vRaster = false;
            editObject->editDrag(ed);
            updateGrips();
            data.startMove = p;
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

