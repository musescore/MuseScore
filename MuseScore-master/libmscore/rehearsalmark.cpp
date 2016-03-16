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
   : Text(s)
      {
      setTextStyleType(TextStyleType::REHEARSAL_MARK);
      }

void RehearsalMark::layout()
      {
      setPos(textStyle().offset(spatium()));
      Text::layout1();
      Segment* s = segment();
      if (s && !s->rtick()) {
            // first CR of measure, decide whether to align to barline
            if (!s->prev()) {
                  // measure with no clef / keysig / timesig
                  rxpos() -= s->x();
                  }
            else if (textStyle().align() & AlignmentFlags::RIGHT) {
                  // measure with clef / keysig / timesig, rehearsal mark right aligned
                  // align left edge of rehearsal to barline if that is further to left
                  qreal leftX = bbox().x();
                  qreal barlineX = -s->x();
                  rxpos() += qMin(leftX, barlineX) + width();
                  }
            }
      adjustReadPos();
      }

}

