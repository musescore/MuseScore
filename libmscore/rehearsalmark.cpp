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
            // rehearsal mark on first chordrest of measure should align over barline
            rxpos() -= s->x();
#if 0
            // if there is a clef, align right after that instead
            Segment* p = segment()->prev(Segment::Type::Clef);
            if (p)
                  rxpos() += p->next()->x();
#endif
            }
      adjustReadPos();
      }

}

