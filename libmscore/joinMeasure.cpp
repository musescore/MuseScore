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

namespace Ms {

//---------------------------------------------------------
//   cmdJoinMeasure
//    join measures from m1 upto (including) m2
//---------------------------------------------------------

void Score::cmdJoinMeasure(Measure* m1, Measure* m2)
      {
      startCmd();

      deselectAll();

      ScoreRange range;
      range.read(m1->first(), m2->last());

      undoRemoveMeasures(m1, m2);
      Measure* m = new Measure(this);
      m->setEndBarLineType(m2->endBarLineType(), m2->endBarLineGenerated(),
         m2->endBarLineVisible(), m2->endBarLineColor());

      m->setTick(m1->tick());
      m->setTimesig(m1->timesig());
      Fraction f;
      for (Measure* mm = m1; mm; mm = mm->nextMeasure())  {
            f += mm->len();
            if (mm == m2)
                  break;
            }
      m->setLen(f);
      undo(new InsertMeasure(m, m2->next()));

      range.write(this, m1->tick());

      endCmd();
      }

}

