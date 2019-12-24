//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "scoreview.h"
#include "libmscore/score.h"
#include "musescore.h"
#include "libmscore/staff.h"
#include "libmscore/utils.h"
#include "libmscore/undo.h"
#include "libmscore/part.h"
#include "tourhandler.h"

namespace Ms {

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void ScoreView::startDrag()
      {
      editData.grips = 0;
      editData.clearData();
      editData.startMove  -= editData.element->offset();

      _score->startCmd();

      for (Element* e : _score->selection().elements())
            e->startDrag(editData);
      }

//---------------------------------------------------------
//   doDragElement
//---------------------------------------------------------

void ScoreView::doDragElement(QMouseEvent* ev)
      {
      QPointF delta = toLogical(ev->pos()) - editData.startMove;

      TourHandler::startTour("autoplace-tour");

      QPointF pt(delta);
      if (qApp->keyboardModifiers() == Qt::ShiftModifier)
            pt.setX(editData.element->offset().x());
      else if (qApp->keyboardModifiers() == Qt::ControlModifier)
            pt.setY(editData.element->offset().y());

      editData.lastPos = editData.pos;
      editData.hRaster = mscore->hRaster();
      editData.vRaster = mscore->vRaster();
      editData.delta   = pt;
      editData.pos     = toLogical(ev->pos());

      const Selection& sel = _score->selection();
      const bool filterType = sel.isRange();
      const ElementType type = editData.element->type();

      for (Element* e : sel.elements()) {
            if (filterType && type != e->type())
                  continue;
            _score->addRefresh(e->drag(editData));
            }

      Element* e = _score->getSelectedElement();
      if (e) {
            if (_score->playNote()) {
                  mscore->play(e);
                  _score->setPlayNote(false);
                  }
            QLineF anchor = e->dragAnchor();

            if (!anchor.isNull())
                  setDropAnchor(anchor);
            else
                  setDropTarget(0); // this also resets dropAnchor
            }
      updateGrips();
      _score->update();
      }

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void ScoreView::endDrag()
      {
      for (Element* e : _score->selection().elements()) {
            e->endDrag(editData);
            e->triggerLayout();
            }
      setDropTarget(0); // this also resets dropAnchor
      _score->endCmd();
      if (editData.element->normalModeEditBehavior() == Element::EditBehavior::Edit && _score->selection().element() == editData.element)
            startEdit(/* editMode */ false);
      }
}

