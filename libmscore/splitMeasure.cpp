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
#include "segment.h"
#include "chordrest.h"
#include "range.h"

namespace Ms {

//---------------------------------------------------------
//   cmdSplitMeasure
//---------------------------------------------------------

void Score::cmdSplitMeasure(ChordRest* cr)
      {
      Segment* segment = cr->segment();
      Measure* measure = segment->measure();

      ScoreRange range;
      range.read(measure->first(), measure->last());

      startCmd();
      deleteItem(measure);

      // create empty measures:
      Measure* m2 = static_cast<Measure*>(insertMeasure(Element::Type::MEASURE, measure->next(), true, false));
      Measure* m1 = static_cast<Measure*>(insertMeasure(Element::Type::MEASURE, m2, true, false));

      int tick = segment->tick();
      m1->setTick(measure->tick());
      m2->setTick(tick);
      int ticks1 = segment->tick() - measure->tick();
      int ticks2 = measure->ticks() - ticks1;
      m1->setTimesig(measure->timesig());
      m2->setTimesig(measure->timesig());
      m1->adjustToLen(Fraction::fromTicks(ticks1), false);
      m2->adjustToLen(Fraction::fromTicks(ticks2), false);
      range.write(this, m1->tick());

      endCmd();
      }

}

