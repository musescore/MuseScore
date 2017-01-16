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

#include "score.h"
#include "rehearsalmark.h"

namespace Ms {

//---------------------------------------------------------
//   RehearsalMark
//---------------------------------------------------------

RehearsalMark::RehearsalMark(Score* s)
   : Text(SubStyle::REHEARSAL_MARK, s)
      {
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void RehearsalMark::layout()
      {
      if (autoplace())
            setUserOff(QPointF());
//      setPos(textStyle().offset(spatium()));
      setPos(QPointF());
      Text::layout1();
      Segment* s = segment();
      if (s) {
            if (!s->rtick()) {
                  // first CR of measure, decide whether to align to barline
                  if (!s->prev() && align() & Align::CENTER) {
                        // measure with no clef / keysig / timesig
                        rxpos() -= s->x();
                        }
                  else if (align() & Align::RIGHT) {
                        // measure with clef / keysig / timesig, rehearsal mark right aligned
                        // align left edge of rehearsal to barline if that is further to left
                        qreal leftX = bbox().x();
                        qreal barlineX = -s->x();
                        rxpos() += qMin(leftX, barlineX) + width();
                        }
                  }
            if (autoplace()) {
                  Shape s1 = s->staffShape(staffIdx()).translated(s->pos());
                  Shape s2 = shape().translated(s->pos());
                  qreal d  = s2.minVerticalDistance(s1);
                  if (d > 0)
                        setUserOff(QPointF(0.0, -d));
                  }
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant RehearsalMark::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::SUB_STYLE:
                  return int(SubStyle::REHEARSAL_MARK);
            default:
                  return Text::propertyDefault(id);
            }
      }

} // namespace Ms

