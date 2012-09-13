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

//---------------------------------------------------------
//   cmdJoinMeasure
//    join measures from m1 upto (including) m2
//---------------------------------------------------------

void Score::cmdJoinMeasure(Measure* m1, Measure* m2)
      {
      startCmd();

      ScoreRange range;
      range.read(m1->first(), m2->last(), 0, nstaves() * VOICES);

      undo(new RemoveMeasures(m1, m2));
      Measure* m = static_cast<Measure*>(insertMeasure(Element::MEASURE, m2->next(), true));
      fixTicks();

      m->setTick(m1->tick());
      m->setTimesig(m1->timesig());
      Fraction f;
      for (Measure* mm = m1; mm; mm = mm->nextMeasure())  {
            f += mm->len();
            if (mm == m2)
                  break;
            }
      m->setLen(f);
      range.write(0, m);

      endCmd();
      }


