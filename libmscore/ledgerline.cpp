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

namespace Ms {

//---------------------------------------------------------
//   LedgerLine
//---------------------------------------------------------

LedgerLine::LedgerLine(Score* s)
   : Line(s, false)
      {
      setSelectable(false);
      _next = 0;
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
      setLineWidth(score()->styleP(StyleIdx::ledgerLineWidth) * chord()->mag());
      if (staff())
            setColor(staff()->color());
      Line::layout();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void LedgerLine::draw(QPainter* painter) const
      {
      if (chord()->crossMeasure() == CrossMeasure::SECOND)
            return;
      Line::draw(painter);
      }

}
