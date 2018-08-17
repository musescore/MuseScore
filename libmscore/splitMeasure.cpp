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
#include "tuplet.h"
#include "spanner.h"
#include "undo.h"

namespace Ms {

//---------------------------------------------------------
//   cmdSplitMeasure
//---------------------------------------------------------

void Score::cmdSplitMeasure(ChordRest* cr)
      {
      startCmd();
      splitMeasure(cr->segment());
      endCmd();
      }

//---------------------------------------------------------
//   splitMeasure
//    return true on success
//---------------------------------------------------------

void Score::splitMeasure(Segment* segment)
      {
      if (segment->rtick() == 0) {
            MScore::setError(CANNOT_SPLIT_MEASURE_FIRST_BEAT);
            return;
            }
      if (segment->splitsTuplet()) {
            MScore::setError(CANNOT_SPLIT_MEASURE_TUPLET);
            return;
            }
      Measure* measure = segment->measure();

      ScoreRange range;
      range.read(measure->first(), measure->last());

      int stick = measure->tick();
      int etick = measure->endTick();

      std::list<std::tuple<Spanner*, int, int>> sl;
      for (auto i : spanner()) {
            Spanner* s = i.second;
            Element* start = s->startElement();
            Element* end = s->endElement();
            if (s->tick() >= stick && s->tick() < etick)
                  start = nullptr;
            if (s->tick2() >= stick && s->tick2() < etick)
                  end = nullptr;
            if (start != s->startElement() || end != s->endElement())
                  undo(new ChangeStartEndSpanner(s, start, end));
            if (s->tick() < stick && s->tick2() > stick)
                  sl.push_back(make_tuple(s, s->tick(), s->ticks()));
            }

      MeasureBase* nm = measure->next();
      undoRemoveMeasures(measure, measure);
      undoInsertTime(measure->tick(), -measure->ticks());

      // create empty measures:
      insertMeasure(ElementType::MEASURE, nm, true);
      Measure* m2 = toMeasure(nm ? nm->prev() : lastMeasure());
      insertMeasure(ElementType::MEASURE, m2, true);
      Measure* m1 = toMeasure(m2->prev());

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

      for (auto i : sl) {
            Spanner* s = std::get<0>(i);
            int t      = std::get<1>(i);
            int ticks  = std::get<2>(i);
            if (s->tick() != t)
                  s->undoChangeProperty(Pid::SPANNER_TICK, t);
            if (s->ticks() != ticks)
                  s->undoChangeProperty(Pid::SPANNER_TICKS, ticks);
            }
      }
}

