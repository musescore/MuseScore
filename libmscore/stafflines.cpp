//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "stafflines.h"
#include "system.h"
#include "measure.h"
#include "score.h"
#include "stafftype.h"
#include "staff.h"

// Anatomy of StaffLines:
//
//    step         - The possible vertical positions of a note are counted as steps.
//                   The top staff line is step position zero.
//    lines        - number of visible staff lines
//    lineDistance - The distance between lines, measured in step units. A standard five line
//                   staff has a line distance of two steps.
//    stepDistance - The distance between steps measured in scaled spatium/2 units. A standard five
//                   line staff has a step distance of 0.5 which results in a line distance of
//                   one spatium. The spatium unit is scaled by staff size.
//    yoffset      - vertical offset to align with other staves of different height
//    stepOffset   - This value changes the staff line step numbering.
//

namespace Ms {

//---------------------------------------------------------
//   StaffLines
//---------------------------------------------------------

StaffLines::StaffLines(Score* s)
   : Element(s)
      {
      setSelectable(false);
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF StaffLines::pagePos() const
      {
      System* system = measure()->system();
      return QPointF(measure()->x() + system->x(), system->staff(staffIdx())->y() + system->y());
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF StaffLines::canvasPos() const
      {
      QPointF p(pagePos());
      Element* e = parent();
      while (e) {
            if (e->type() == ElementType::PAGE) {
                  p += e->pos();
                  break;
                  }
            e = e->parent();
            }
      return p;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffLines::layout()
      {
      layoutForWidth(measure()->width());
      }

//---------------------------------------------------------
//   layoutForWidth
//---------------------------------------------------------

void StaffLines::layoutForWidth(qreal w)
      {
      const Staff* s = staff();
      qreal _spatium = spatium();
      qreal dist     = _spatium;
      setPos(QPointF(0.0, 0.0));
      int _lines;
      if (s) {
            setMag(s->mag(measure()->tick()));
            setColor(s->color());
            const StaffType* st = s->staffType(measure()->tick());
            dist         *= st->lineDistance().val();
            _lines        = st->lines();
            rypos()       = st->yoffset().val() * _spatium;
//            if (_lines == 1)
//                  rypos() = 2 * _spatium;
            }
      else {
            _lines = 5;
            setColor(MScore::defaultColor);
            }
      lw       = score()->styleS(Sid::staffLineWidth).val() * _spatium;
      qreal x1 = pos().x();
      qreal x2 = x1 + w;
      qreal y  = pos().y();
      bbox().setRect(x1, -lw * .5 + y, w, (_lines-1) * dist + lw);

      lines.clear();
      for (int i = 0; i < _lines; ++i) {
            lines.push_back(QLineF(x1, y, x2, y));
            y += dist;
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffLines::draw(QPainter* painter) const
      {
      painter->setPen(QPen(curColor(), lw, Qt::SolidLine, Qt::FlatCap));
      painter->drawLines(lines);
      }

//---------------------------------------------------------
//   y1
//---------------------------------------------------------

qreal StaffLines::y1() const
      {
      System* system = measure()->system();
/*      if (system == 0 || staffIdx() >= system->staves()->size())
            return 0.0;
      */
      return system->staff(staffIdx())->y() + ipos().y();
      }

}


