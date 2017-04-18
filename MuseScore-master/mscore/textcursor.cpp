//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013-15 Werner Schweer & others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "textcursor.h"

#include "libmscore/input.h"
#include "libmscore/measure.h"
#include "libmscore/page.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/staff.h"
#include "libmscore/stafftype.h"
#include "libmscore/sym.h"
#include "libmscore/system.h"

#include "scoreview.h"

namespace Ms {

//---------------------------------------------------------
//   PositionCursor
//---------------------------------------------------------

void PositionCursor::setType(CursorType t)
      {
      _type = t;
      if (_type == CursorType::LOOP_IN) {
            // QColor cIn(Qt::green);
            QColor cIn("#2456aa");
            // cIn.setAlpha(90);
            setColor(cIn);
            }
      else if (_type == CursorType::LOOP_OUT) {
            // QColor cOut(Qt::red);
            QColor cOut("#2456aa");
            // cOut.setAlpha(90);
            setColor(cOut);
            }
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void PositionCursor::paint(QPainter* p)
      {
      if (!visible())
            return;
      QPointF points[3];
      qreal h = _sv->score()->spatium() * 2;

      qreal x = _rect.left();
      qreal y = _rect.top();

      switch(_type) {
            case CursorType::LOOP_IN:           // draw a right-pointing triangle
                  {
                  qreal tx = x - 1.0;
                  p->setPen(QPen(_color, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
                  p->drawLine(x, y, x, _rect.bottom());
                  points[0] = QPointF(tx, y);
                  points[1] = QPointF(tx, y + h);
                  points[2] = QPointF(tx + h, y + h * .5);
                  p->setBrush(_color);
                  p->drawConvexPolygon(points, 3);
                  }
                  break;
            case CursorType::LOOP_OUT:          // draw a left-pointing triangle
                  p->setPen(QPen(_color, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
                  p->drawLine(x, y, x, _rect.bottom());
                  points[0] = QPointF(x, y);
                  points[1] = QPointF(x, y + h);
                  points[2] = QPointF(x - h, y + h * .5);
                  p->setBrush(_color);
                  p->drawConvexPolygon(points, 3);
                  break;
            default:                            // fill the rectangle and add TAB string marks, if required
                  p->fillRect(_rect, color());
                  if (_sv->score()->noteEntryMode()) {
                        int         track       = _sv->score()->inputTrack();
                        if (track >= 0) {
                              Staff*      staff       = _sv->score()->staff(track2staff(track));
                              StaffType*  staffType   = staff->staffType();
                              if (staffType && staffType->group() == StaffGroup::TAB)
                                    staffType->drawInputStringMarks(p, _sv->score()->inputState().string(),
                                       track2voice(track), _rect);
                                    }
                        }
                  break;
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF PositionCursor::bbox() const
      {
      QRectF r;
      qreal h = _sv->score()->spatium() * 2;

      switch(_type) {
            case CursorType::LOOP_IN:
                  r.setRect(_rect.x(), _rect.y(), h, _rect.height());
                  break;
            case CursorType::LOOP_OUT:
                  r.setRect(_rect.x() - h, _rect.y(), h, _rect.height());
                  break;
            default:
                  r = _rect;
                  break;
            }
      return r.adjusted(-2, -2, 2, 2);
      }


//---------------------------------------------------------
//   move
//---------------------------------------------------------

void PositionCursor::move(int tick)
      {
      QRectF r(bbox());
      //
      // set mark height for whole system
      //
      if (_type == CursorType::LOOP_OUT)
        tick --;
      Score* score = _sv->score();
      Measure* measure = score->tick2measureMM(tick);
      if (measure == 0)
            return;
      qreal x;
      int offset = 0;

      Segment* s;
      for (s = measure->first(Segment::Type::ChordRest); s;) {
            int t1 = s->tick();
            int x1 = s->canvasPos().x();
            qreal x2;
            int t2;
            Segment* ns = s->next(Segment::Type::ChordRest);
            if (ns) {
                  t2 = ns->tick();
                  x2 = ns->canvasPos().x();
                  }
            else {
                  t2 = measure->endTick();
                  x2 = measure->canvasPos().x() + measure->width();
                  }
            t1 += offset;
            t2 += offset;
            if (tick >= t1 && tick < t2) {
                  int   dt = t2 - t1;
                  qreal dx = x2 - x1;
                  x = x1 + dx * (tick-t1) / dt;
                  break;
                  }
            s = ns;
            }
      if (s == 0)
            return;

      System* system = measure->system();
      if (system == 0)
            return;
      double y        = system->staffYpage(0) + system->page()->pos().y();
      double _spatium = score->spatium();

      qreal mag = _spatium / SPATIUM20;
      double w  = (_spatium * 2.0 + score->scoreFont()->width(SymId::noteheadBlack, mag))/3;
      double h  = 6 * _spatium;
      //
      // set cursor height for whole system
      //
      double y2 = 0.0;

      for (int i = 0; i < score->nstaves(); ++i) {
            SysStaff* ss = system->staff(i);
            if (!ss->show() || !score->staff(i)->show())
                  continue;
            y2 = ss->y() + ss->bbox().height();
            }
      h += y2;
      y -= 3 * _spatium;

      if (_type == CursorType::LOOP_IN) {
            x = x - _spatium + w/1.5;
            }
      else {
            x = x - _spatium * .5;
            }
      _tick = tick;
      _rect = QRectF(x, y, w, h);
      _sv->update(_sv->matrix().mapRect(r | bbox()).toRect().adjusted(-1,-1,1,1));
      }
}

