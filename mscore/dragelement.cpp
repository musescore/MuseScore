//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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

namespace Ms {

//---------------------------------------------------------
//   testElementDragTransition
//---------------------------------------------------------

bool ScoreView::testElementDragTransition(QMouseEvent* ev)
      {
      if (curElement == 0 || !curElement->isMovable() || QApplication::mouseButtons() != Qt::LeftButton)
            return false;
      if (curElement->type() == Element::Type::MEASURE) {
            System* dragSystem = (System*)(curElement->parent());
            int staffIdx  = getStaff(dragSystem, data.startMove);
            dragStaff = score()->staff(staffIdx);
            if (staffIdx == 0)
                  return false;
            }
      QPoint delta = ev->pos() - startMoveI;
      return delta.manhattanLength() > 2;
      }

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void ScoreView::startDrag()
      {
      dragElement = curElement;
      data.startMove  -= dragElement->userOff();
      _score->startCmd();

      if (dragElement->type() == Element::Type::MEASURE) {
            staffUserDist = dragStaff->userDist();
            }
      else {
            foreach(Element* e, _score->selection().elements())
                  e->setStartDragPosition(e->userOff());
            }
      _score->end();
      }

//---------------------------------------------------------
//   doDragElement
//---------------------------------------------------------

void ScoreView::doDragElement(QMouseEvent* ev)
      {
      QPointF delta = toLogical(ev->pos()) - data.startMove;

      QPointF pt(delta);
      if (qApp->keyboardModifiers() == Qt::ShiftModifier)
            pt.setX(dragElement->userOff().x());
      else if (qApp->keyboardModifiers() == Qt::ControlModifier)
            pt.setY(dragElement->userOff().y());

      data.hRaster = mscore->hRaster();
      data.vRaster = mscore->vRaster();
      data.delta   = pt;
      data.pos     = toLogical(ev->pos());

      if (dragElement->type() == Element::Type::MEASURE) {
            qreal dist      = dragStaff->userDist() + delta.y();
            int partStaves  = dragStaff->part()->nstaves();
            StyleIdx i      = (partStaves > 1) ? StyleIdx::akkoladeDistance : StyleIdx::staffDistance;
            qreal _spatium  = _score->spatium();
            qreal styleDist = _score->styleS(i).val() * _spatium;

            if ((dist + styleDist) < _spatium)  // limit minimum distance to spatium
                  dist = -styleDist + _spatium;

            dragStaff->setUserDist(dist);
            data.startMove += delta;
            _score->doLayoutSystems();
            _score->layoutSpanner();
            update();
            loopUpdate(getAction("loop")->isChecked());
            return;
            }

      for (Element* e : _score->selection().elements()) {
            QRectF r = e->drag(&data);
            _score->addRefresh(r);
            }

      if (_score->playNote()) {
            Element* e = _score->selection().element();
            if (e)
                  mscore->play(e);
            _score->setPlayNote(false);
            }

      Element* e = _score->getSelectedElement();
      if (e) {
            QLineF anchor = e->dragAnchor();
            if (!anchor.isNull())
                  setDropAnchor(anchor);
            else
                  setDropTarget(0); // this also resets dropAnchor
            }
      _score->update();
      }

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void ScoreView::endDrag()
      {
      if (dragElement->type() == Element::Type::MEASURE) {
            qreal userDist = dragStaff->userDist();
            dragStaff->setUserDist(staffUserDist);
            score()->undo(new ChangeStaffUserDist(dragStaff, userDist));
            }
      else {
            foreach(Element* e, _score->selection().elements()) {
                  e->endDrag();
                  e->score()->undoPropertyChanged(e, P_ID::USER_OFF, e->startDragPosition());
                  }
            }
      _score->setLayoutAll(true);
      dragElement = 0;
      setDropTarget(0); // this also resets dropAnchor
      _score->endCmd();
      mscore->endCmd();
      }
}

