//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "ledgerline.h"
#include "chord.h"
#include "measure.h"
#include "staff.h"
#include "system.h"
#include "score.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   LedgerLine
//---------------------------------------------------------

LedgerLine::LedgerLine(Score* s)
   : Element(s)
      {
      setSelectable(false);
      _width      = 0.;
      _len        = 0.;
      _next       = 0;
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF LedgerLine::pagePos() const
      {
      System* system = chord()->measure()->system();
      qreal yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(pageX(), yp);
      }

//---------------------------------------------------------
//   measureXPos
//---------------------------------------------------------

qreal LedgerLine::measureXPos() const
      {
      qreal xp = x();                   // chord relative
      xp += chord()->x();                // segment relative
      xp += chord()->segment()->x();     // measure relative
      return xp;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void LedgerLine::layout()
      {
      setLineWidth(score()->styleP(Sid::ledgerLineWidth) * chord()->mag());
      if (staff())
            setColor(staff()->staffType(tick())->color());
      qreal w2 = _width * .5;

      //Adjust Y position to staffType offset
      if (staffType())
            rypos() += staffType()->yoffset().val() * spatium();

      if (vertical)
            bbox().setRect(-w2, 0, w2, _len);
      else
            bbox().setRect(0, -w2, _len, w2);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void LedgerLine::draw(QPainter* painter) const
      {
      if (chord()->crossMeasure() == CrossMeasure::SECOND)
            return;
      painter->setPen(QPen(curColor(), _width, Qt::SolidLine, Qt::FlatCap));
      if (vertical)
            painter->drawLine(QLineF(0.0, 0.0, 0.0, _len));
      else
            painter->drawLine(QLineF(0.0, 0.0, _len, 0.0));
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void LedgerLine::spatiumChanged(qreal oldValue, qreal newValue)
      {
      _width = (_width / oldValue) * newValue;
      _len   = (_len / oldValue) * newValue;
      layout();
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void LedgerLine::writeProperties(XmlWriter& xml) const
      {
      xml.tag("lineWidth", _width / spatium());
      xml.tag("lineLen", _len / spatium());
      if (!vertical)
            xml.tag("vertical", vertical);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool LedgerLine::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "lineWidth")
            _width = e.readDouble() * spatium();
      else if (tag == "lineLen")
            _len = e.readDouble() * spatium();
      else if (tag == "vertical")
            vertical = e.readInt();
      else
            return false;
      return true;
      }

}
