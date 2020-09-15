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
#include "tie.h"
#include "undo.h"
#include "utils.h"

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
      if (segment->rtick().isZero()) {
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

      Fraction stick = measure->tick();
      Fraction etick = measure->endTick();

      std::list<std::tuple<Spanner*, Fraction, Fraction>> sl;
      for (auto i : spanner()) {
            Spanner* s = i.second;
            Element* start = s->startElement();
            Element* end = s->endElement();
            if (s->tick() >= stick && s->tick() < etick)
                  start = nullptr;
            if (s->tick2() >= stick && s->tick2() < etick)
                  end = nullptr;
            if (start != s->startElement() || end != s->endElement()) {
                  sl.push_back(std::make_tuple(s, s->tick(), s->ticks()));
                  undo(new ChangeStartEndSpanner(s, start, end));
                  }
            if (s->tick() < stick && s->tick2() > stick)
                  sl.push_back(std::make_tuple(s, s->tick(), s->ticks()));
            }

      // Make sure ties are the beginning the split measure are restored.
      std::vector<Tie*> ties;
      for (int track = 0; track < ntracks(); track++) {
            Chord* chord = measure->findChord(stick, track);
            if (chord)
                  for (Note* note : chord->notes()) {
                        Tie* tie = note->tieBack();
                        if (tie)
                              ties.push_back(tie->clone());
                        }
                  }

      MeasureBase* nm = measure->next();

      undoRemoveMeasures(measure, measure);
      undoInsertTime(measure->tick(), -measure->ticks());

      // create empty measures:
      insertMeasure(ElementType::MEASURE, nm, true, false);
      Measure* m2 = toMeasure(nm ? nm->prev() : lastMeasure());
      insertMeasure(ElementType::MEASURE, m2, true, false);
      Measure* m1 = toMeasure(m2->prev());

      Fraction tick = segment->tick();
      m1->setTick(measure->tick());
      m2->setTick(tick);
      Fraction ticks1 = segment->tick() - measure->tick();
      Fraction ticks2 = measure->ticks() - ticks1;
      m1->setTimesig(measure->timesig());
      m2->setTimesig(measure->timesig());
      ticks1.reduce();
      ticks2.reduce();
      // Now make sure this reduction doesn't go 'beyond' the original measure's
      // actual denominator for both resultant measures.
      if (ticks1.denominator() < measure->ticks().denominator()) {
            if (measure->ticks().denominator() % m1->timesig().denominator() == 0) {
                  int mult = measure->ticks().denominator() / ticks1.denominator();
                  // *= operator audomatically reduces via GCD, so rather do literal multiplication:
                  ticks1.setDenominator(ticks1.denominator() * mult);
                  ticks1.setNumerator(ticks1.numerator() * mult);
                  }
            }
      if (ticks2.denominator() < measure->ticks().denominator()) {
            if (measure->ticks().denominator() % m2->timesig().denominator() == 0) {
                  int mult = measure->ticks().denominator() / ticks2.denominator();
                  ticks2.setDenominator(ticks2.denominator() * mult);
                  ticks2.setNumerator(ticks2.numerator() * mult);
                  }
            }
      m1->adjustToLen(ticks1, false);
      m2->adjustToLen(ticks2, false);
      range.write(this, m1->tick());

      // Restore ties the the beginning of the split measure.
      for (auto tie : ties) {
            tie->setEndNote(searchTieNote(tie->startNote()));
            undoAddElement(tie);
            }

      for (auto i : sl) {
            Spanner* s      = std::get<0>(i);
            Fraction t      = std::get<1>(i);
            Fraction ticks  = std::get<2>(i);
            if (s->tick() != t)
                  s->undoChangeProperty(Pid::SPANNER_TICK, t);
            if (s->ticks() != ticks)
                  s->undoChangeProperty(Pid::SPANNER_TICKS, ticks);
            }
      }
}

