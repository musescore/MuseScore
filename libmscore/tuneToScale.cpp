//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "measure.h"
#include "chordrest.h"
#include "chord.h"
#include "segment.h"
#include "staff.h"
#include "scale.h"

namespace Ms {

//---------------------------------------------------------
//   cmdJoinMeasure
//    join measures from m1 upto (including) m2
//---------------------------------------------------------

void Score::tuneToScale(const Scale& scale)
      {
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            int endStaff = staves().size();
            for (int staffIdx = 0; staffIdx < endStaff; ++staffIdx) {
                  for (Segment* s = m->first(Segment::Type::ChordRest); s; s = s->next(Segment::Type::ChordRest)) {
                        for (int v = 0; v < VOICES; ++v) {
                              ChordRest* cr = static_cast<ChordRest*>(s->element(staffIdx* VOICES + v));
                              if (cr == nullptr || cr->type() != Element::Type::CHORD) {
                                    continue;
                                    }
                              Chord* chord = static_cast<Chord*>(cr);
                              for (Note* n : chord->notes()) {
                                    scale.tuneNote(n);
                                    }
                              }
                        }
                  }
            }
      }
}
