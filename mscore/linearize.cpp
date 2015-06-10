//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 File handling: loading and saving.
 */

#include "config.h"
#include "globals.h"
#include "musescore.h"
#include "scoreview.h"
#include "exportmidi.h"
#include "libmscore/xml.h"
#include "libmscore/element.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/sig.h"
#include "libmscore/clef.h"
#include "libmscore/key.h"
#include "instrdialog.h"
#include "libmscore/score.h"
#include "libmscore/page.h"
#include "libmscore/dynamic.h"
#include "file.h"
#include "libmscore/style.h"
#include "libmscore/tempo.h"
#include "libmscore/select.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/utils.h"
#include "libmscore/barline.h"
#include "libmscore/slur.h"
#include "libmscore/hairpin.h"
#include "libmscore/ottava.h"
#include "libmscore/textline.h"
#include "libmscore/pedal.h"
#include "libmscore/trill.h"
#include "libmscore/volta.h"
#include "libmscore/timesig.h"
#include "libmscore/box.h"
#include "libmscore/excerpt.h"
#include "libmscore/system.h"
#include "libmscore/tuplet.h"
#include "libmscore/keysig.h"
#include "libmscore/measure.h"
#include "libmscore/undo.h"
#include "libmscore/repeatlist.h"
#include "libmscore/beam.h"
#include "libmscore/stafftype.h"
#include "libmscore/revisions.h"
#include "libmscore/lyrics.h"
#include "libmscore/segment.h"
#include "libmscore/tempotext.h"
#include "libmscore/sym.h"
#include "libmscore/image.h"
#include "synthesizer/msynthesizer.h"
#include "svggenerator.h"
#include "libmscore/tiemap.h"
#include "libmscore/tie.h"
#include "libmscore/measurebase.h"

#include "importmidi/importmidi_instrument.h"

#include "libmscore/chordlist.h"
#include "libmscore/mscore.h"
#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"


namespace Ms {

  void appendCopiesOfMeasures(Score * score,Measure * fm,Measure * lm) {

      Score * fscore = fm->score();

      fscore->select(fm,SelectType::SINGLE,0);
      fscore->select(lm,SelectType::RANGE,score->nstaves()-1);
      QString mimeType = fscore->selection().mimeType();
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, fscore->selection().mimeData());
      fscore->deselectAll();


      Measure * last = 0;
      last = static_cast<Measure*>(score->insertMeasure(Element::Type::MEASURE,0,false));

      score->select(last);
      score->startCmd();
      score->cmdPaste(mimeData,0);
      score->endCmd();
      score->deselectAll();
   }

   Score * MuseScore::linearize(Score* old_score)
      {

      Score* score = old_score->clone();
      
      //old_score->deselectAll(); 
      // Figure out repeat structure and traverse it
      old_score->repeatList()->unwind();
      old_score->setPlaylistDirty();

      bool copy=false;
      foreach (const RepeatSegment* rs, *(old_score->repeatList()) ) {
         int startTick  = rs->tick;
         int endTick    = startTick + rs->len;

         qDebug("Segment %i-%i",startTick,endTick);

         Measure * mf = old_score->tick2measure(startTick);
         Measure * ml = ml;


         if (!copy && startTick==0) ml = score->tick2measure(startTick);

         for (ml=mf; ml; ml = ml->nextMeasure()) {
            if (ml->tick() + ml->ticks() >= endTick) break;
         }

         // First segment can be done in-place
         if (!copy) {
            if (startTick==0) // keep first segment in place
               ml = ml?ml->nextMeasure():ml;
            else { // remove everything and copy things over
               copy=true;
               ml = score->firstMeasure();
            }

            // Remove all measures past the first jump
            if (ml) {  
               score->select(ml,SelectType::SINGLE);
               score->select(score->lastMeasure(),SelectType::RANGE);
               score->startCmd();
               score->cmdDeleteSelectedMeasures();
               score->endCmd(); 
            }
         }
         
         if (copy) appendCopiesOfMeasures(score,mf,ml);

         copy = true;;
      }

      
      // Remove volta markers
      for (const std::pair<int,Spanner*>& p : score->spannerMap().map()) {
         Spanner* s = p.second;
         if (s->type() != Element::Type::VOLTA) continue;
         //qDebug("VOLTA!");
         score->removeSpanner(s);
      }

      for(Measure * m = score->firstMeasure(); m; m=m->nextMeasure()) {
         // Remove repeats
         if (m->repeatFlags()!=Repeat::NONE) {


            m->setRepeatFlags(Repeat::NONE);
            m->setRepeatCount(0);
         }
         // Remove coda/fine labels and jumps
         for (auto e : m->el())
            if (e->type() == Element::Type::MARKER || 
               e->type() == Element::Type::JUMP) {
               //qDebug("JUMP? %s",qPrintable(e->userName()));
               score->deleteItem(e);
            }
      }

      score->lastMeasure()->setEndBarLineType(BarLineType::END, false);
      
      // score->deselectAll();
      //old_score->deselectAll();

      // Postprocessing stuff
      score->setLayoutAll(true);
      score->fixTicks();
      score->doLayout();

      return score;
   }

   bool MuseScore::newLinearized(Score* old_score)
   {
      Score * score = linearize(old_score);
      setCurrentScoreView(appendScore(score));

      return true;
   }
}