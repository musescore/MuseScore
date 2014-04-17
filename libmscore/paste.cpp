//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "mscore.h"

#include "accidental.h"
#include "articulation.h"
#include "barline.h"
#include "beam.h"
#include "box.h"
#include "breath.h"
#include "chord.h"
#include "chordlist.h"
#include "clef.h"
#include "drumset.h"
#include "dynamic.h"
#include "figuredbass.h"
#include "hairpin.h"
#include "harmony.h"
#include "key.h"
#include "keysig.h"
#include "layoutbreak.h"
#include "lyrics.h"
#include "measure.h"
#include "navigate.h"
#include "note.h"
#include "noteevent.h"
#include "ottava.h"
#include "page.h"
#include "part.h"
#include "pedal.h"
#include "pitchspelling.h"
#include "repeat.h"
#include "rest.h"
#include "segment.h"
#include "sequencer.h"
#include "sig.h"
#include "slur.h"
#include "staff.h"
#include "stafftype.h"
#include "stafftext.h"
#include "style.h"
#include "system.h"
//#include "stringdata.h"
#include "tempo.h"
#include "tempotext.h"
#include "text.h"
#include "textline.h"
#include "tie.h"
#include "timesig.h"
#include "trill.h"
#include "tuplet.h"
#include "undo.h"
#include "utils.h"
#include "volta.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   pasteStaff
//---------------------------------------------------------

void Score::pasteStaff(XmlReader& e, ChordRest* dst)
      {
      clearSpannerIds();
      QList<Chord*> graceNotes;
      int dstStaffStart = dst->staffIdx();
      int dstTick = dst->tick();
      bool done = false;
      while (e.readNextStartElement()) {
            if (done)
                  break;
            if (e.name() != "StaffList") {
                  e.unknown();
                  break;
                  }
            QString version = e.attribute("version", "NONE");
            if(version != MSC_VERSION)
                  break;
            int tickStart     = e.intAttribute("tick", 0);
            int tickLen       = e.intAttribute("len", 0);
            int srcStaffStart = e.intAttribute("staff", 0);
            int staves        = e.intAttribute("staves", 0);
            e.setTick(tickStart);

            bool pasted = false;
            while (e.readNextStartElement()) {
                  if (done)
                        break;
                  if (e.name() != "Staff") {
                        e.unknown();
                        break;
                        }
                  int srcStaffIdx = e.attribute("id", "0").toInt();
                  int dstStaffIdx = srcStaffIdx - srcStaffStart + dstStaffStart;
                  if (dstStaffIdx >= nstaves()) {
                        done = true; // break main loop, nothing more to paste
                        break;
                        }

                  e.tuplets().clear();
                  bool makeGap  = true;
                  while (e.readNextStartElement()) {
                        pasted = true;
                        const QStringRef& tag(e.name());

                        if (tag == "tick") {
                              int tick = e.readInt();
                              e.setTick(tick);
                              int shift = tick - tickStart;
                              if (makeGap && !makeGap1(dstTick + shift, dstStaffIdx, Fraction::fromTicks(tickLen - shift))) {
                                    qDebug("cannot make gap in staff %d at tick %d", dstStaffIdx, dstTick + shift);
                                    done = true; // break main loop, cannot make gap
                                    break;
                                    }
                              makeGap = false; // create gap only once per staff
                              }
                        else if (tag == "Tuplet") {
                              Tuplet* tuplet = new Tuplet(this);
                              tuplet->setTrack(e.track());
                              tuplet->read(e);
                              int tick = e.tick() - tickStart + dstTick;
                              Measure* measure = tick2measure(tick);
                              tuplet->setParent(measure);
                              tuplet->setTick(tick);
                              e.addTuplet(tuplet);
                              }
                        else if (tag == "Chord" || tag == "Rest" || tag == "RepeatMeasure") {
                              ChordRest* cr = static_cast<ChordRest*>(Element::name2Element(tag, this));
                              cr->setTrack(e.track());
                              cr->read(e);
                              cr->setSelected(false);
                              int voice = cr->voice();
                              int track = dstStaffIdx * VOICES + voice;
                              cr->setTrack(track);
                              int tick = e.tick() - tickStart + dstTick;
                              if (cr->isGrace())
                                    graceNotes.push_back(static_cast<Chord*>(cr));
                              else {
                                    e.setTick(e.tick() + cr->actualTicks());
                                    if (cr->type() == Element::CHORD) {
                                          Chord* chord = static_cast<Chord*>(cr);
                                          for (int i = 0; i < graceNotes.size(); ++i) {
                                                Chord* gc = graceNotes[i];
                                                gc->setGraceIndex(i);
                                                chord->add(gc);
                                                }
                                          graceNotes.clear();
                                          }
                                    pasteChordRest(cr, tick);
                                    }
                              }
                        else if (tag == "HairPin"
                           || tag == "Pedal"
                           || tag == "Ottava"
                           || tag == "Trill"
                           || tag == "TextLine"
                           || tag == "Slur"
                           || tag == "Volta") {
                              Spanner* sp = static_cast<Spanner*>(Element::name2Element(tag, this));
                              sp->setAnchor(Spanner::ANCHOR_SEGMENT);
                              sp->read(e);
                              sp->setTrack(dstStaffIdx * VOICES);
                              sp->setTrack2(dstStaffIdx * VOICES);
                              sp->setTick(e.tick() - tickStart + dstTick);
                              addSpanner(sp);
                              }
                        else if (tag == "endSpanner") {
                              int id = e.intAttribute("id");
                              Spanner* spanner = findSpanner(id);
                              if (spanner) {
                                    // e.spanner().removeOne(spanner);
                                    spanner->setTick2(e.tick() - tickStart + dstTick);
                                    removeSpanner(spanner);
                                    undoAddElement(spanner);
                                    if (spanner->type() == Element::OTTAVA) {
                                          Ottava* o = static_cast<Ottava*>(spanner);
                                          o->staff()->updateOttava(o);
                                          }
                                    else if (spanner->type() == Element::HAIRPIN) {
                                          Hairpin* hp = static_cast<Hairpin*>(spanner);
                                          updateHairpin(hp);
                                          }
                                    }
                              e.readNext();
                              }
/* unused: Lyrics are now part of <Chord>
 * and parenting lyrics to segment is wrong anyway!
                        else if (tag == "Lyrics") {
                              Lyrics* lyrics = new Lyrics(this);
                              lyrics->setTrack(e.track());
                              lyrics->read(e);
                              lyrics->setTrack(dstStaffIdx * VOICES);
                              int tick = e.tick() - tickStart + dstTick;
                              Segment* segment = tick2segment(tick);
                              if (segment) {
                                    lyrics->setParent(segment);
                                    undoAddElement(lyrics);
                                    }
                              else {
                                    delete lyrics;
                                    qDebug("no segment found for lyrics");
                                    }
                              }
*/
                        else if (tag == "Harmony") {
                              Harmony* harmony = new Harmony(this);
                              harmony->setTrack(e.track());
                              harmony->read(e);
                              harmony->setTrack(dstStaffIdx * VOICES);
                              // transpose
                              Part* partDest = staff(dstStaffIdx)->part();
                              Interval interval = partDest->instr()->transpose();
                              if (!styleB(ST_concertPitch) && !interval.isZero()) {
                                    interval.flip();
                                    int rootTpc = transposeTpc(harmony->rootTpc(), interval, false);
                                    int baseTpc = transposeTpc(harmony->baseTpc(), interval, false);
                                    undoTransposeHarmony(harmony, rootTpc, baseTpc);
                                    }

                              int tick = e.tick() - tickStart + dstTick;
                              Measure* m = tick2measure(tick);
                              Segment* seg = m->undoGetSegment(Segment::SegChordRest, tick);
                              harmony->setParent(seg);
                              undoAddElement(harmony);
                              }
                        else if (tag == "Dynamic"
                           || tag == "Symbol"
                           || tag == "FretDiagram"
                           || tag == "Marker"
                           || tag == "Jump"
                           || tag == "Image"
                           || tag == "Text"
                           || tag == "StaffText"
                           || tag == "TempoText"
                           || tag == "FiguredBass"
                           ) {
                              Element* el = Element::name2Element(tag, this);
                              el->setTrack(dstStaffIdx * VOICES);             // a valid track might be necessary for el->read() to work

                              int tick = e.tick() - tickStart + dstTick;
                              Measure* m = tick2measure(tick);
                              Segment* seg = m->undoGetSegment(Segment::SegChordRest, tick);
                              el->setParent(seg);
                              el->read(e);

                              // be sure to paste the element in the destination track;
                              // setting track needs to be repeated, as it might have been overwritten by el->read()
                              el->setTrack(dstStaffIdx * VOICES);
                              undoAddElement(el);
                              }
                        else if (tag == "Clef") {
                              Clef* clef = new Clef(this);
                              clef->read(e);
                              clef->setTrack(dstStaffIdx * VOICES);
                              int tick = e.tick() - tickStart + dstTick;
                              Measure* m = tick2measure(tick);
                              if (m->tick() && m->tick() == tick)
                                    m = m->prevMeasure();
                              Segment* segment = m->undoGetSegment(Segment::SegClef, tick);
                              clef->setParent(segment);
                              undoAddElement(clef);
                              }
                        else if (tag == "Breath") {
                              Breath* breath = new Breath(this);
                              breath->read(e);
                              breath->setTrack(dstStaffIdx * VOICES);
                              int tick = e.tick() - tickStart + dstTick;
                              Measure* m = tick2measure(tick);
                              Segment* segment = m->undoGetSegment(Segment::SegBreath, tick);
                              breath->setParent(segment);
                              undoAddElement(breath);
                              }
                        else if (tag == "Beam") {
                              Beam* beam = new Beam(this);
                              beam->setTrack(e.track());
                              beam->read(e);
                              beam->setParent(0);
                              e.addBeam(beam);
                              }
                        else if (tag == "BarLine") {
                              e.skipCurrentElement();    // ignore bar line
                              }
                        else {
                              qDebug("PasteStaff: element %s not handled", tag.toUtf8().data());
                              e.skipCurrentElement();    // ignore
                              }
                        }

                  foreach (Tuplet* tuplet, e.tuplets()) {
                        if (tuplet->elements().isEmpty()) {
                              // this should not happen and is a sign of input file corruption
                              qDebug("Measure:pasteStaff(): empty tuplet");
                              delete tuplet;
                              }
                        else {
                              Measure* measure = tick2measure(tuplet->tick());
                              tuplet->setParent(measure);
                              tuplet->sortElements();
                              }
                        }
                  }

            if (pasted) { //select only if we pasted something
                  Segment* s1 = tick2segment(dstTick);
                  Segment* s2 = tick2segment(dstTick + tickLen);
                  int endStaff = dstStaffStart + staves;
                  if (endStaff > nstaves())
                        endStaff = nstaves();
                  _selection.setRange(s1, s2, dstStaffStart, endStaff);
                  _selection.updateSelectedElements();
                  foreach(MuseScoreView* v, viewer)
                        v->adjustCanvasPosition(s1, false);
                  if (selection().state() != SEL_RANGE)
                        _selection.setState(SEL_RANGE);
                  }
            }
      foreach (Score* s, scoreList())     // for all parts
            s->connectTies();

      // when pasting between different staff types (pitched->tablature)
      // fret/line has to be calculated:
      updateNotes();
      }

//---------------------------------------------------------
//   pasteChordRest
//---------------------------------------------------------

void Score::pasteChordRest(ChordRest* cr, int tick)
      {
// qDebug("pasteChordRest %s at %d", cr->name(), tick);
      if (cr->type() == Element::CHORD) {
            // set note track
            // check if staffMove moves a note to a
            // nonexistant staff
            //
            int track  = cr->track();
            Chord* c   = static_cast<Chord*>(cr);
            Part* part = cr->staff()->part();
            int nn     = (track / VOICES) + c->staffMove();
            if (nn < 0 || nn >= nstaves())
                  c->setStaffMove(0);

            if (!styleB(ST_concertPitch) && part->instr()->transpose().chromatic) {
                  Interval interval = part->instr()->transpose();
                  if (!interval.isZero()) {
                        interval.flip();
                        for (Note* n : c->notes()) {
                              int npitch;
                              int ntpc;
                              transposeInterval(n->pitch(), n->tpc1(), &npitch, &ntpc, interval, true);
                              n->setTpc2(ntpc);
                              }
                        }
                  }
            }

      Measure* measure = tick2measure(tick);
      if (!measure)
            return;

      int measureEnd = measure->endTick();
      bool isGrace = (cr->type() == Element::CHORD) && (((Chord*)cr)->noteType() != NOTE_NORMAL);
      if (!isGrace && (tick + cr->actualTicks() > measureEnd)) {
            if (cr->type() == Element::CHORD) {
                  // split Chord
                  Chord* c = static_cast<Chord*>(cr);
                  int rest = c->actualTicks();
                  int len  = measureEnd - tick;
                  rest    -= len;
                  TDuration d;
                  d.setVal(len);
                  c->setDurationType(d);
                  c->setDuration(d.fraction());
                  undoAddCR(c, measure, tick);
                  while (rest) {
                        tick += c->actualTicks();
                        measure = tick2measure(tick);
                        if (measure->tick() != tick) {  // last measure
                              qDebug("==last measure %d != %d", measure->tick(), tick);
                              break;
                              }
                        Chord* c2 = static_cast<Chord*>(c->clone());
                        len = measure->ticks() > rest ? rest : measure->ticks();
                        TDuration d;
                        d.setVal(len);
                        c2->setDurationType(d);
                        rest -= len;
                        undoAddCR(c2, measure, tick);

                        QList<Note*> nl1 = c->notes();
                        QList<Note*> nl2 = c2->notes();

                        for (int i = 0; i < nl1.size(); ++i) {
                              Tie* tie = new Tie(this);
                              tie->setStartNote(nl1[i]);
                              tie->setEndNote(nl2[i]);
                              tie->setTrack(c->track());
                              Tie* tie2 = nl1[i]->tieFor();
                              if (tie2) {
                                    nl2[i]->setTieFor(nl1[i]->tieFor());
                                    tie2->setStartNote(nl2[i]);
                                    }
                              nl1[i]->setTieFor(tie);
                              nl2[i]->setTieBack(tie);
                              }
                        c = c2;
                        }
                  }
            else {
                  // split Rest
                  Rest* r       = static_cast<Rest*>(cr);
                  Fraction rest = r->duration();

                  while (!rest.isZero()) {
                        Rest* r2      = static_cast<Rest*>(r->clone());
                        measure       = tick2measure(tick);
                        Fraction mlen = Fraction::fromTicks(measure->tick() + measure->ticks() - tick);
                        Fraction len  = rest > mlen ? mlen : rest;
                        r2->setDuration(len);
                        r2->setDurationType(TDuration(len));
                        undoAddCR(r2, measure, tick);
                        rest -= len;
                        tick += r2->actualTicks();
                        }
                  delete r;
                  }
            }
      else {
            undoAddCR(cr, measure, tick);
            }
      }


//---------------------------------------------------------
//   pasteSymbols
//
//    pastes a list of symbols into cr and following ChordRest's
//
//    (Note: info about delta ticks is currently ignored)
//---------------------------------------------------------

void Score::pasteSymbols(XmlReader& e, ChordRest* dst)
      {
      Segment* currSegm = dst->segment();
      int   destTick    = 0;              // the tick and track to place the pasted element at
      int   destTrack   = 0;
      bool  done        = false;
      int   segDelta    = 0;
      Segment* startSegm= currSegm;
      int   startTick   = dst->tick();    // the initial tick and track where to start pasting
      int   startTrack  = dst->track();
      int   maxTrack    = ntracks();

      while (e.readNextStartElement()) {
            if (done)
                  break;
            if (e.name() != "SymbolList") {
                  e.unknown();
                  break;
                  }
            QString version = e.attribute("version", "NONE");
            if(version != MSC_VERSION)
                  break;

            while (e.readNextStartElement()) {
                  if (done)
                        break;
                  const QStringRef& tag(e.name());

                  if (tag == "trackOffset") {
                        destTrack = startTrack + e.readInt();
                        currSegm  = startSegm;
                        }
                  else if (tag == "tickOffset")
                        destTick = startTick + e.readInt();
                  else if (tag == "segDelta")
                        segDelta = e.readInt();
                  else {

                        if (tag == "Harmony") {
                              //
                              // Harmony elements (= chord symbols) are positioned respecting
                              // the original tickOffset: advance to destTick (or near)
                              //
                              Segment* harmSegm;
                              for (harmSegm = startSegm; harmSegm && (harmSegm->tick() < destTick);
                                          harmSegm = harmSegm->nextCR())
                                    ;
                              // if destTick overshot, no dest. segment: create one
                              if (!harmSegm || harmSegm->tick() > destTick) {
                                    Measure* meas     = tick2measure(destTick);
                                    harmSegm          = meas->getSegment(Segment::SegChordRest, destTick);
                              }
                              Harmony* el = new Harmony(this);
                              el->setTrack(trackZeroVoice(destTrack));
                              el->read(e);
                              el->setTrack(trackZeroVoice(destTrack));
                              // transpose
                              Part* partDest = staff(track2staff(destTrack))->part();
                              Interval interval = partDest->instr()->transpose();
                              if (!styleB(ST_concertPitch) && !interval.isZero()) {
                                    interval.flip();
                                    int rootTpc = transposeTpc(el->rootTpc(), interval, false);
                                    int baseTpc = transposeTpc(el->baseTpc(), interval, false);
                                    undoTransposeHarmony(el, rootTpc, baseTpc);
                                    }
                              el->setParent(harmSegm);
                              undoAddElement(el);
                              }
                        else {
                              //
                              // All other elements are positioned respecting the distance in chords
                              //
                              for ( ; currSegm && segDelta > 0; segDelta--)
                                    currSegm = currSegm->nextCR(destTrack);
                              // check the intended dest. track and segment exist
                              if (destTrack >= maxTrack || currSegm == nullptr) {
                                    qDebug("PasteSymbols: no track or segment for %s", tag.toUtf8().data());
                                    e.skipCurrentElement();       // ignore
                                    continue;
                                    }
                              // check there is a segment element in the required track
                              if (currSegm->element(destTrack) == nullptr) {
                                    qDebug("PasteSymbols: no track element for %s", tag.toUtf8().data());
                                    e.skipCurrentElement();
                                    continue;
                                    }
                              ChordRest* cr = static_cast<ChordRest*>(currSegm->element(destTrack));

                              if (tag == "Articulation") {
                                    Articulation* el = new Articulation(this);
                                    el->read(e);
                                    el->setTrack(destTrack);
                                    el->setParent(cr);
                                    undoAddElement(el);
                                    }
                              else if (tag == "FiguredBass") {
                                    // FiguredBass always belongs to first staff voice
                                    destTrack = trackZeroVoice(destTrack);
                                    int ticks;
                                    FiguredBass* el = new FiguredBass(this);
                                    el->setTrack(destTrack);
                                    el->read(e);
                                    el->setTrack(destTrack);
                                    // if f.b. is off-note, we have to locate a place before currSegm
                                    // where an on-note f.b. element could (potentially) be
                                    // (while having an off-note f.b. without an on-note one before it
                                    // is un-idiomatic, possible mismatch in rhythmic patterns between
                                    // copy source and paste destination does not allow to be too picky)
                                    if (!el->onNote()) {
                                          FiguredBass* onNoteFB = nullptr;
                                          Segment*     prevSegm = currSegm;
                                          bool         done = false;
                                          while (prevSegm) {
                                                if (done)
                                                      break;
                                                prevSegm = prevSegm->prev1(Segment::SegChordRest);
                                                // if there is a ChordRest in the dest. track
                                                // this segment is a (potential) f.b. location
                                                if (prevSegm->element(destTrack) != nullptr) {
                                                      done = true;
                                                      }
                                                // in any case, look for a f.b. in annotations:
                                                // if there is a f.b. element in the right track,
                                                // this is an (actual) f.b. location
                                                foreach (Element* a, prevSegm->annotations()) {
                                                      if (a->type() == Element::FIGURED_BASS && a->track() == destTrack) {
                                                            onNoteFB = static_cast<FiguredBass*>(a);
                                                            done = true;
                                                            }
                                                      }
                                                }
                                          if (!prevSegm) {
                                                qDebug("PasteSymbols: can't place off-note FiguredBass");
                                                delete el;
                                                continue;
                                                }
                                          // by default, split on-note duration in half: half on-note and half off-note
                                          int totTicks = currSegm->tick() - prevSegm->tick();
                                          int destTick = prevSegm->tick() + totTicks / 2;
                                          ticks        = totTicks / 2;
                                          if (onNoteFB)
                                                onNoteFB->setTicks(totTicks / 2);
                                          // look for a segment at this tick; if none, create one
                                          Segment * nextSegm = prevSegm;
                                          while (nextSegm && nextSegm->tick() < destTick)
                                                nextSegm = nextSegm->next1(Segment::SegChordRest);
                                          if (!nextSegm || nextSegm->tick() > destTick) {      // no ChordRest segm at this tick
                                                nextSegm = new Segment(prevSegm->measure(), Segment::SegChordRest, destTick);
                                                if (!nextSegm) {
                                                      qDebug("PasteSymbols: can't find or create destination segment for FiguredBass");
                                                      delete el;
                                                      continue;
                                                      }
                                                undoAddElement(nextSegm);
                                                }
                                          currSegm = nextSegm;
                                          }
                                    else
                                          // by default, assign to FiguredBass element the duration of the chord it refers to
                                          ticks = static_cast<ChordRest*>(currSegm->element(destTrack))->durationTicks();
                                    // in both cases, look for an existing f.b. element in segment and remove it, if found
                                    FiguredBass* oldFB = nullptr;
                                    foreach (Element* a, currSegm->annotations()) {
                                          if (a->type() == Element::FIGURED_BASS && a->track() == destTrack) {
                                                oldFB = static_cast<FiguredBass*>(a);
                                                break;
                                                }
                                          }
                                    if (oldFB)
                                          undoRemoveElement(oldFB);
                                    el->setParent(currSegm);
                                    el->setTicks(ticks);
                                    undoAddElement(el);
                                    }
                              else if (tag == "Lyrics") {
                                    // with lyrics, skip rests
                                    while (cr->type() != Element::CHORD && currSegm) {
                                          currSegm = currSegm->nextCR(destTrack);
                                          if (currSegm)
                                                cr = static_cast<ChordRest*>(currSegm->element(destTrack));
                                          else
                                                break;
                                          }
                                    if (currSegm == nullptr) {
                                          qDebug("PasteSymbols: no segment for Lyrics");
                                          e.skipCurrentElement();
                                          continue;
                                          }
                                    if (cr->type() != Element::CHORD) {
                                          qDebug("PasteSymbols: can't paste Lyrics to rest");
                                          e.skipCurrentElement();
                                          continue;
                                          }
                                    Lyrics* el = new Lyrics(this);
                                    el->setTrack(destTrack);
                                    el->read(e);
                                    el->setTrack(destTrack);
                                    el->setParent(cr);
                                    undoAddElement(el);
                                    }
                              else {
                                    qDebug("PasteSymbols: element %s not handled", tag.toUtf8().data());
                                    e.skipCurrentElement();    // ignore
                                    }
                              }           // if !Harmony
                        }                 // if element
                  }                       // outer while readNextstartElement()
            }                             // inner while readNextstartElement()
      }                                   // pasteSymbolList()

}
