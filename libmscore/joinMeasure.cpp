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
      if (!m2)
            return;
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

      deleteMeasures(m1, m2);

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

      endCmd();
      }

}

