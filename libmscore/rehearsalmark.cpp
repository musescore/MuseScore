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
#include "measure.h"
#include "system.h"

namespace Ms {

//---------------------------------------------------------
//   RehearsalMark
//---------------------------------------------------------

RehearsalMark::RehearsalMark(Score* s)
   : TextBase(s)
      {
      initSubStyle(SubStyleId::REHEARSAL_MARK);
      setSystemFlag(true);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void RehearsalMark::layout()
      {
      qreal y = placeAbove() ? styleP(Sid::rehearsalMarkPosAbove) : styleP(Sid::rehearsalMarkPosBelow) + staff()->height();
      setPos(QPointF(0.0, y));
      TextBase::layout1();
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
            autoplaceSegmentElement(styleP(Sid::rehearsalMarkMinDistance));
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant RehearsalMark::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::SUB_STYLE:
                  return int(SubStyleId::REHEARSAL_MARK);
            case Pid::PLACEMENT:
                  return score()->styleV(Sid::rehearsalMarkPlacement);
            default:
                  return TextBase::propertyDefault(id);
            }
      }

} // namespace Ms

