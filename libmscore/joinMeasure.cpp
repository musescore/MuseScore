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
#include "undo.h"
#include "range.h"
#include "spanner.h"

namespace Ms {

//---------------------------------------------------------
//   cmdJoinMeasure
//    join measures from m1 upto (including) m2
//---------------------------------------------------------

void Score::cmdJoinMeasure(Measure* m1, Measure* m2)
      {
      if (!m1 || !m2)
            return;
      if (m1->isMMRest())
            m1 = m1->mmRestFirst();
      if (m2->isMMRest())
            m2 = m2->mmRestLast();
      startCmd();

      deselectAll();

      ScoreRange range;
      range.read(m1->first(), m2->last());

      Fraction tick1 = m1->tick();
      Fraction tick2 = m2->endTick();

      auto spanners = _spanner.findContained(tick1.ticks(), tick2.ticks());
      for (auto i : spanners)
            undo(new RemoveElement(i.value));

      for (auto i : spanner()) {
            Spanner* s = i.second;
            if (s->tick() >= tick1 && s->tick() < tick2)
                  s->setStartElement(0);
            if (s->tick2() >= tick1 && s->tick2() < tick2)
                  s->setEndElement(0);
            }

      deleteMeasures(m1, m2, true);

      MeasureBase* next = m2->next();
      const Fraction newTimesig = m1->timesig();
      Fraction newLen;
      for (Measure* mm = m1; mm; mm = mm->nextMeasure())  {
            newLen += mm->ticks();
            if (mm == m2)
                  break;
            }
      insertMeasure(ElementType::MEASURE, next, /* createEmptyMeasures*/ true);
      // The loop since measures are not currently linked in MuseScore
      for (Score* s : masterScore()->scoreList()) {
            Measure* ins = s->tick2measure(tick1);
            ins->undoChangeProperty(Pid::TIMESIG_NOMINAL, newTimesig);
//             TODO: there was a commented chunk of code regarding setting bar
//             line types. Should we handle them here too?
//             m->setEndBarLineType(m2->endBarLineType(), m2->endBarLineGenerated(),
//             m2->endBarLineVisible(), m2->endBarLineColor());
            }
      Measure* inserted = (next ? next->prevMeasure() : lastMeasure());
      inserted->adjustToLen(newLen, /* appendRests... */ false);

      range.write(this, m1->tick());

      // if there are some Time Signatures in joined measures, move last one to the next measure (if there is not one already)
      Segment* ts = inserted->last()->prev(SegmentType::TimeSig);
      if (ts && ts->rtick().isNotZero()) {
            for (Segment* insMSeg = inserted->last()->prev(SegmentType::TimeSig); insMSeg && insMSeg->rtick().isNotZero();
               insMSeg = insMSeg->prev(SegmentType::TimeSig)) {
                  for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                        if (TimeSig* insTimeSig = toTimeSig(insMSeg->element(staffIdx * VOICES))) {
                              TimeSig* lts = nullptr;
                              for (ScoreElement* l : insTimeSig->linkList()) {
                                    Score* score = l->score();
                                    TimeSig* timeSig = toTimeSig(l);
                                    Segment* tSeg = timeSig->segment();
                                    int track = timeSig->track();
                                    Measure* sNext = next ? score->tick2measure(next->tick()) : nullptr;
                                    Segment* nextTSeg = sNext ? sNext->undoGetSegmentR(SegmentType::TimeSig, Fraction(0, 1)) : nullptr;
                                    if (sNext && !nextTSeg->element(track)) {
                                        TimeSig* nsig = new TimeSig(*timeSig);
                                          nsig->setScore(score);
                                          nsig->setTrack(track);
                                          nsig->setParent(nextTSeg);

                                          score->undo(new AddElement(nsig));

                                          if (!lts)
                                                lts = nsig;
                                          else
                                                score->undo(new Link(lts, nsig));
                                          }
                                    score->undo(new RemoveElement(timeSig));
                                    if (tSeg->empty())
                                          score->undo(new RemoveElement(tSeg));
                                    }
                              }
                        }
                  }
            }
      endCmd();
      }

}

