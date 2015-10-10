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

  // From svgc.cpp
  
  QString getInstrumentName(Instrument * in);
  QSet<Note *> * mark_tie_ends(QList<const Element*> const &elems);

/*
void note_row(QTextStream * qts, int tick, float pos, QSet<Note *> * notes, QSet<Note *> * ongoing, TempoMap * tempomap) {
   (*qts) << (notes->isEmpty()?"R ":"N ") << tempomap->tick2time(tick) << ',' << pos;

   // Notes still sounding from before
   QSetIterator<Note *> i(*ongoing);
   while (i.hasNext()) {
      Note * cur = i.next();
      int end = cur->chord()->tick() + cur->chord()->actualTicks();
      if (end<=tick) ongoing->remove(cur);
      else (*qts) << ' ' << cur->pitch();
   }

   // New notes 
   if (!notes->isEmpty()) {
      (*qts) << ';';
      QSetIterator<Note *> j(*notes);
      while (j.hasNext()) {
         Note * cur = j.next();
        (*qts) << ' ' << cur->pitch();
        (*ongoing) << cur;
      }
      notes->clear();
   }

   (*qts) << endl;
}*/


QJsonObject collectMLData(Score* score, const QMap<int,qreal>& t2t);

bool MuseScore::saveMLData(Score * cs, const QString& saveName, const QString& partsName) {

  QJsonObject partsinfo;
  qreal scale_tempo = 1.0;

  if (!partsName.isEmpty()) {
    QFile partsfile(partsName);
    partsfile.open(QIODevice::ReadOnly | QIODevice::Text);
    partsinfo = QJsonDocument::fromJson(partsfile.readAll()).object();
    
    if (partsinfo.contains("scale_tempo"))
      scale_tempo = partsinfo["scale_tempo"].toDouble();
  }

  //qreal rel_tempo = cs->tempomap()->relTempo();
  cs->tempomap()->setRelTempo(scale_tempo);

  QMap<int,qreal> tick2time; // is bypassed, if empty!

  QJsonArray result;

  Score* thisScore = cs->rootScore();
  if (partsinfo.isEmpty()) {
  	result.push_back(collectMLData(cs, tick2time));
  }
  else {
    if (partsinfo.contains("onsets")) {
      QJsonObject onsets = partsinfo["onsets"].toObject(); 
      QJsonArray ticks = onsets["ticks"].toArray();
      QJsonArray times = onsets["times"].toArray();

      for(int i=0;i<ticks.size();i++)
        tick2time[ticks[i].toInt()] = times[i].toDouble();
    }

    // Number parts just the same as exporting metadata
    int pi = 1;
    foreach( Part * part, cs->parts()) {
      part->setId(QString::number(pi++));
    }

    if (partsinfo.contains("excerpts")) {
      result.push_back(collectMLData(cs, tick2time));

	    foreach (Excerpt* e, thisScore->excerpts())  {
	    	Score * tScore = e->partScore();
	    	result.push_back(collectMLData(tScore, tick2time));
	    }
    }
	}

  QFile file(saveName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);    
  file.write(QJsonDocument(result).toJson());
  file.close();

	return true;
}

QJsonObject collectMLData(Score* score, const QMap<int,qreal>& t2t) {

      score->repeatList()->unwind();
      if (score->repeatList()->size()>1) {
        score = mscore->linearize(score,true);
      }

      QJsonObject qts = QJsonObject();

      // Instruments
      QJsonArray iar;
      foreach( Part * part, score->parts()) {
         QString iname = getInstrumentName(part->instrument());
         if (iname.length()>0)
            iar.push_back(iname);
      }
      qts["instruments"] = iar;

      // Total ticks
      Measure* lastm = score->lastMeasure();
      int total_ticks = lastm->tick()+lastm->ticks();
   
      QSet<Note *> * tie_ends = NULL; 

      QMap<int,int> just_tied; // just the end of tied note
      QMap<int,int> is_rest;
      QMap<int,QSet<Note*>> ongoing;

      foreach( Page* page, score->pages() ) {
        foreach( System* sys, *(page->systems()) ) {

          // Collect together all elements belonging to this system!
          QList<const Element*> elems;
          foreach(MeasureBase *m, sys->measures())
             m->scanElements(&elems, collectElements, false);
          sys->scanElements(&elems, collectElements, false);

          tie_ends = mark_tie_ends(elems);

          foreach(const Element * e, elems) {

             if (e->type() == Element::Type::NOTE || 
                 e->type() == Element::Type::REST) {

                ChordRest * cr = (e->type()==Element::Type::NOTE?
                               (ChordRest*)( ((Note*)e)->chord()):(ChordRest*)e);

                int tick = cr->segment()->tick();

                // NB! ar[17] = !ar.contains(17); would not work as expected...
                just_tied.insert(tick,just_tied.value(tick,true) && 
                                (e->type() == Element::Type::NOTE && 
                                  tie_ends->contains((Note*)e)));
                is_rest.insert(tick,is_rest.value(tick,true) && 
                                (e->type() == Element::Type::REST));

             }
          }
        
          delete tie_ends;

          foreach(const Element * e, elems) {
            if (e->type() == Element::Type::NOTE) {
              Note * cur = (Note*)e;
              int beg = cur->chord()->tick();
              int end = beg + cur->chord()->actualTicks();

              QMap<int, int>::iterator i = is_rest.find(beg);
              while (i != is_rest.end() && i.key() < end) {
                  if (!ongoing.contains(i.key())) ongoing[i.key()] = QSet<Note*>();
                  ongoing[i.key()] << cur;
                  ++i;
              }
            }
          }
        }
      }

      QJsonArray samples, times, sound;

      bool use_t2t = !t2t.isEmpty();
      TempoMap * tempomap = score->tempomap();

      bool was_rest = false;
      foreach(int tick, is_rest.keys()){
        if (just_tied[tick] || (was_rest && is_rest[tick])) continue;
        was_rest = (bool)(is_rest[tick]);

        //ticks.push_back(tick);
        times.push_back(use_t2t?t2t[tick]:tempomap->tick2time(tick));
        samples.push_back((int)
          ((22050.0*8/4096)*
          (use_t2t?t2t[tick]:tempomap->tick2time(tick)))
          );

        QJsonArray csound;
        foreach(const Note * n, ongoing[tick])
          csound.push_back(n->ppitch());

        sound.push_back(csound);
      }

      samples.push_back((int)
        ((22050.0*8/4096)*
        (t2t.isEmpty()?score->tempomap()->tick2time(total_ticks):
         t2t.value(total_ticks, // Provide a linear approximation as default (important for old exercises)
              t2t.first() + (t2t.last()-t2t.first())*total_ticks/(t2t.lastKey()-t2t.firstKey()))))
        );


      qts["pitches"] = sound;
      //qts["times"] = times;
      qts["samples"] = samples;

      return qts;
  }
}
