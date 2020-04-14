//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2019 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 File handling: loading and saving.
 */

#include "xml.h"
#include "element.h"
#include "note.h"
#include "chord.h"
#include "rest.h"
#include "sig.h"
#include "clef.h"
#include "key.h"
#include "score.h"
#include "page.h"
#include "dynamic.h"
#include "style.h"
#include "tempo.h"
#include "select.h"
#include "staff.h"
#include "part.h"
#include "utils.h"
#include "barline.h"
#include "slur.h"
#include "hairpin.h"
#include "ottava.h"
#include "textline.h"
#include "pedal.h"
#include "trill.h"
#include "volta.h"
#include "timesig.h"
#include "box.h"
#include "excerpt.h"
#include "system.h"
#include "tuplet.h"
#include "keysig.h"
#include "measure.h"
#include "undo.h"
#include "repeatlist.h"
#include "beam.h"
#include "stafftype.h"
#include "revisions.h"
#include "lyrics.h"
#include "segment.h"
#include "tempotext.h"
#include "sym.h"
#include "image.h"
#include "tiemap.h"
#include "tie.h"
#include "measurebase.h"
#include "chordlist.h"
#include "mscore.h"


namespace Ms {

static void removeRepeatMarkings(Score* score)
      {

      // remove bar-level repeats
      for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
            m->setRepeatStart(false);
            m->setRepeatEnd(false);
            }

      // remove volta markers
      auto smap = score->spannerMap().map();
      for (auto it = smap.cbegin(); it != smap.cend(); ++it) {
            Spanner* s = (*it).second;
            if (!s || !s->isVolta()) continue;
            score->removeSpanner(s);
            }

      // remove coda/fine labels and jumps
      QList<Element*> elems;
      score->scanElements(&elems, collectElements, false);
      for (auto e : elems) {
            if (e->isMarker() || e->isJump())
                  score->deleteItem(e);
            else if (e->isBarLine())
                  toBarLine(e)->setBarLineType(BarLineType::NORMAL);
            }

      // set the last bar line to end symbol
      score->lastMeasure()->setEndBarLineType(BarLineType::END, false);
      Segment* last = score->lastMeasure()->segments().last();
      if (last->segmentType() == SegmentType::EndBarLine) {
            auto els = last->elist();
            for (uint i = 0; i < els.size(); i++) {
                  if (!els[i]) continue;
                  toBarLine(els[i])->setBarLineType(BarLineType::END);
                  }
            }
      }


//---------------------------------------------------------
//   createExcerpts
//    re-create all the excerpts once the master score
//    has been unrolled
//---------------------------------------------------------

static void createExcerpts(MasterScore* cs, QList<Excerpt *> excerpts)
      {
      // borrowed from musescore.cpp endsWith(".pdf")
      for (Excerpt* e: excerpts) {
            Score* nscore = new Score(e->oscore());
            e->setPartScore(nscore);
            nscore->style().set(Sid::createMultiMeasureRests, true);
            cs->startCmd();
            cs->undo(new AddExcerpt(e));
            Excerpt::createExcerpt(e);

            // borrowed from excerptsdialog.cpp
            // a new excerpt is created in AddExcerpt, make sure the parts are filed
            for (Excerpt* ee : e->oscore()->excerpts())
                  if (ee->partScore() == nscore && ee != e) {
                        ee->parts().clear();
                        ee->parts().append(e->parts());
                        }

            cs->endCmd();
            }
      }

//---------------------------------------------------------
//   unrollRepeats
//    unroll all the repeats
//---------------------------------------------------------

MasterScore* MasterScore::unrollRepeats()
      {

      MasterScore* original = this;
      
      // create a copy of the original score to play with
      MasterScore* score = original->clone();

      // Give it an appropriate name
      score->setName(original->title()+"_unrolled");

      // figure out repeat structure
      original->setExpandRepeats(true);

      // if no repeats, just return the score as-is
      if (original->repeatList().size() == 1)
            return score;

      // remove excerpts for now (they are re-created after unrolling master score)
      QList<Excerpt *> excerpts;
      for (Excerpt* e : score->excerpts()) {
            excerpts.append(new Excerpt(*e,false));
            score->masterScore()->deleteExcerpt(e);
            }

      // follow along with the repeatList
      bool first = true;
      for (const RepeatSegment* rs: original->repeatList()) {
            Fraction startTick = Fraction::fromTicks(rs->tick);
            Fraction endTick   = Fraction::fromTicks(rs->tick + rs->len());

            // first segment left from clone, everything past that removed
            if (first) {
                  if (endTick <= score->lastMeasure()->tick()) // check if we actually need to remove any measures
                        score->deleteMeasures(score->tick2measure(endTick), score->lastMeasure());
                  first = false;
                  }
            else {// just append this section from the original to the new score
                  score->appendMeasuresFromScore(original, startTick, endTick);
                  }    
            }

      removeRepeatMarkings(score);

      score->fixTicks();

      score->setLayoutAll();
      score->doLayout();

      // re-create excerpt parts
      if (!excerpts.isEmpty())
            createExcerpts(score, excerpts);

      return score;
      }
}
