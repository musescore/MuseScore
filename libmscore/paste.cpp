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

#include "rest.h"
#include "staff.h"
#include "measure.h"
#include "harmony.h"
#include "breath.h"
#include "beam.h"
#include "figuredbass.h"
#include "ottava.h"
#include "part.h"
#include "lyrics.h"
#include "hairpin.h"
#include "tie.h"
#include "tuplet.h"
#include "utils.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   transposeChord
//---------------------------------------------------------

static void transposeChord(Chord* c, Interval srcTranspose)
      {
      // set note track
      // check if staffMove moves a note to a
      // nonexistant staff
      //
      int track  = c->track();
      int nn     = (track / VOICES) + c->staffMove();
      if (nn < 0 || nn >= c->score()->nstaves())
            c->setStaffMove(0);
      Part* part = c->staff()->part();
      Interval dstTranspose = part->instr()->transpose();

      if (srcTranspose != dstTranspose) {
            if (!dstTranspose.isZero()) {
                  dstTranspose.flip();
                  for (Note* n : c->notes()) {
                        int npitch;
                        int ntpc;
                        transposeInterval(n->pitch(), n->tpc1(), &npitch, &ntpc, dstTranspose, true);
                        n->setTpc2(ntpc);
                        }
                  }
            else {
                  for (Note* n : c->notes())
                        n->setTpc2(n->tpc1());
                  }
            }
      }

//---------------------------------------------------------
//   pasteStaff
//    return false if paste fails
//---------------------------------------------------------

bool Score::pasteStaff(XmlReader& e, Segment* dst, int staffIdx)
      {
      Q_ASSERT(dst->segmentType() == Segment::Type::ChordRest);
      QList<Chord*> graceNotes;
      int dstStaffStart = staffIdx;
      int dstTick = dst->tick();
      bool done = false;
      bool pasted = false;
      int tickLen, staves;
      while (e.readNextStartElement()) {
            if (done)
                  break;
            if (e.name() != "StaffList") {
                  e.unknown();
                  break;
                  }
            QString version = e.attribute("version", "NONE");
            if (version != MSC_VERSION)
                  break;
            int tickStart     = e.intAttribute("tick", 0);
                tickLen       = e.intAttribute("len", 0);
            int srcStaffStart = e.intAttribute("staff", 0);
                staves        = e.intAttribute("staves", 0);
            int voiceOffset[VOICES];
            std::fill(voiceOffset,voiceOffset+VOICES,-1);

            e.setTickOffset(dstTick - tickStart);
            e.initTick(0);

            while (e.readNextStartElement()) {
                  if (done)
                        break;
                  if (e.name() != "Staff") {
                        e.unknown();
                        break;
                        }
                  e.setTransposeChromatic(0);
                  e.setTransposeDiatonic(0);

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

                        if (tag == "transposeChromatic")
                              e.setTransposeChromatic(e.readInt());
                        else if (tag == "transposeDiatonic")
                              e.setTransposeDiatonic(e.readInt());
                        else if (tag == "voice") {
                              int voiceId = e.attribute("id", "-1").toInt();
                              Q_ASSERT(voiceId >= 0 && voiceId < VOICES);
                              voiceOffset[voiceId] = e.readInt();
                              }
                        else if (tag == "tick") {
                              int tick = e.readInt();
                              e.initTick(tick);
                              int shift = tick - tickStart;
                              if (makeGap && !makeGap1(dstTick, dstStaffIdx, Fraction::fromTicks(tickLen),voiceOffset)) {
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
                              int tick = e.tick();
                              Measure* measure = tick2measure(tick);
                              tuplet->setParent(measure);
                              tuplet->setTick(tick);
                              int ticks = tuplet->duration().ticks();
                              int rticks = measure->endTick() - tick;
                              if (rticks < ticks) {
                                    qDebug("tuplet does not fit in measure");
                                    delete tuplet;
                                    return false;
                                    }
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
                              int tick = e.tick();
                              if (cr->isGrace())
                                    graceNotes.push_back(static_cast<Chord*>(cr));
                              else {
                                    e.incTick(cr->actualTicks());
                                    if (cr->type() == Element::Type::CHORD) {
                                          Chord* chord = static_cast<Chord*>(cr);
                                          for (int i = 0; i < graceNotes.size(); ++i) {
                                                Chord* gc = graceNotes[i];
                                                gc->setGraceIndex(i);
                                                transposeChord(gc, e.transpose());
                                                chord->add(gc);
                                                }
                                          graceNotes.clear();
                                          }
                                    //shorten last cr to fit in the space made by makeGap
                                    if ((tick - dstTick) + cr->actualTicks() > tickLen) {
                                          int newLength = tickLen - (tick - dstTick);
                                          cr->setDuration(newLength);
                                          cr->setDurationType(newLength);
                                          }
                                    pasteChordRest(cr, tick, e.transpose());
                                    }
                              }
                        else if (tag == "HairPin"
                           || tag == "Pedal"
                           || tag == "Ottava"
                           || tag == "Trill"
                           || tag == "TextLine"
                           || tag == "Volta") {
                              Spanner* sp = static_cast<Spanner*>(Element::name2Element(tag, this));
                              sp->setAnchor(Spanner::Anchor::SEGMENT);
                              sp->read(e);
                              sp->setTrack(dstStaffIdx * VOICES);
                              sp->setTrack2(dstStaffIdx * VOICES);
                              sp->setTick(e.tick());
                              addSpanner(sp);
                              }
                        else if (tag == "Slur") {
                              Spanner* sp = static_cast<Spanner*>(Element::name2Element(tag, this));
                              sp->read(e);
                              sp->setTrack(dstStaffIdx * VOICES);
                              sp->setTick(e.tick());
                              undoAddElement(sp);
                              }
                        else if (tag == "endSpanner") {
                              int id = e.intAttribute("id");
                              Spanner* spanner = e.findSpanner(id);
                              if (spanner) {
                                    // e.spanner().removeOne(spanner);
                                    spanner->setTick2(e.tick());
                                    removeSpanner(spanner);
                                    undoAddElement(spanner);
                                    if (spanner->type() == Element::Type::OTTAVA)
                                          spanner->staff()->updateOttava();
                                    else if (spanner->type() == Element::Type::HAIRPIN) {
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
                              int tick = e.tick();
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
                              if (!styleB(StyleIdx::concertPitch) && !interval.isZero()) {
                                    interval.flip();
                                    int rootTpc = transposeTpc(harmony->rootTpc(), interval, false);
                                    int baseTpc = transposeTpc(harmony->baseTpc(), interval, false);
                                    undoTransposeHarmony(harmony, rootTpc, baseTpc);
                                    }

                              int tick = e.tick();
                              Measure* m = tick2measure(tick);
                              Segment* seg = m->undoGetSegment(Segment::Type::ChordRest, tick);
                              if (seg->findAnnotationOrElement(Element::Type::HARMONY, dstStaffIdx * VOICES, dstStaffIdx * VOICES)) {
                                    QList<Element*> elements;
                                    foreach (Element* el, seg->annotations()) {
                                          if (el->type() == Element::Type::HARMONY
                                              && el->track() == dstStaffIdx * VOICES) {
                                                elements.append(el);
                                                }
                                          }
                                    foreach (Element* el, elements)
                                          undoRemoveElement(el);
                              }

                              harmony->setParent(seg);
                              undoAddElement(harmony);
                              }
                        else if (tag == "Dynamic"
                           || tag == "Symbol"
                           || tag == "FretDiagram"
                           || tag == "TremoloBar"
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

                              int tick = e.tick();
                              Measure* m = tick2measure(tick);
                              Segment* seg = m->undoGetSegment(Segment::Type::ChordRest, tick);
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
                              int tick = e.tick();
                              Measure* m = tick2measure(tick);
                              if (m->tick() && m->tick() == tick)
                                    m = m->prevMeasure();
                              Segment* segment = m->undoGetSegment(Segment::Type::Clef, tick);
                              clef->setParent(segment);
                              undoAddElement(clef);
                              }
                        else if (tag == "Breath") {
                              Breath* breath = new Breath(this);
                              breath->read(e);
                              breath->setTrack(dstStaffIdx * VOICES);
                              int tick = e.tick();
                              Measure* m = tick2measure(tick);
                              Segment* segment = m->undoGetSegment(Segment::Type::Breath, tick);
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
            }
      foreach (Score* s, scoreList())     // for all parts
            s->connectTies();

      // when pasting between different staff types (pitched->tablature)
      // fret/line has to be calculated:
      updateNotes();

      if (pasted) {                       //select only if we pasted something
            Segment* s1 = tick2segment(dstTick);
            Segment* s2 = tick2segment(dstTick + tickLen);
            int endStaff = dstStaffStart + staves;
            if (endStaff > nstaves())
                  endStaff = nstaves();
            _selection.setRange(s1, s2, dstStaffStart, endStaff);
            _selection.updateSelectedElements();

            //finding the first element that has a track
            //the canvas position will be set to this element
            Element* e = 0;
            Segment* s = s1;
            bool found = false;
            if (s2)
                  s2 = s2->next1MM();
            while (!found && s != s2) {
                  for (int i = dstStaffStart * VOICES; i < (endStaff + 1) * VOICES; i++) {
                        e = s->element(i);
                        if (e) {
                              found = true;
                              break;
                              }
                        }
                  s = s->next1MM();
                  }

            foreach(MuseScoreView* v, viewer)
                  v->adjustCanvasPosition(e, false);
            if (!selection().isRange())
                  _selection.setState(SelState::RANGE);
            }
      return true;
      }

//---------------------------------------------------------
//   pasteChordRest
//---------------------------------------------------------

void Score::pasteChordRest(ChordRest* cr, int tick, const Interval& srcTranspose)
      {
// qDebug("pasteChordRest %s at %d", cr->name(), tick);
      if (cr->type() == Element::Type::CHORD)
            transposeChord(static_cast<Chord*>(cr), srcTranspose);

      Measure* measure = tick2measure(tick);
      if (!measure)
            return;

      int measureEnd = measure->endTick();
      bool isGrace = (cr->type() == Element::Type::CHORD) && (((Chord*)cr)->noteType() != NoteType::NORMAL);
      if (!isGrace && (tick + cr->actualTicks() > measureEnd)) {
            if (cr->type() == Element::Type::CHORD) {
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
                        //if (measure->tick() != tick) {  // last measure
                        //      qDebug("==last measure %d != %d", measure->tick(), tick);
                        //      break;
                        //      }
                        Chord* c2 = static_cast<Chord*>(c->clone());
                        len = measure->ticks() > rest ? rest : measure->ticks();
                        TDuration d;
                        d.setVal(len);
                        // split the remaining part of the chord in the final target measure
                        // into a set of valid value chords
                        if (len == rest) {
                              QList<TDuration> dl = toDurationList(Fraction::fromTicks(len), true);
                              d = dl[0];
                              }
                        c2->setDurationType(d);
                        c2->setDuration(d.fraction());
                        rest -= c2->actualTicks();
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
                        TDuration d = TDuration(len);
                        // split the remaining part of the rest in the final target measure
                        // into a set of valid value rests
                        if (len == rest) {
                              QList<TDuration> dl = toDurationList(len, true);
                              d = dl[0];
                              }
                        r2->setDuration(d.fraction());
                        r2->setDurationType(d);
                        undoAddCR(r2, measure, tick);
                        rest -= d.fraction();
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
            if (version != MSC_VERSION)
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
                                    harmSegm          = meas->getSegment(Segment::Type::ChordRest, destTick);
                              }
                              Harmony* el = new Harmony(this);
                              el->setTrack(trackZeroVoice(destTrack));
                              el->read(e);
                              el->setTrack(trackZeroVoice(destTrack));
                              // transpose
                              Part* partDest = staff(track2staff(destTrack))->part();
                              Interval interval = partDest->instr()->transpose();
                              if (!styleB(StyleIdx::concertPitch) && !interval.isZero()) {
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
                                    if (!el->isFermata()
                                        && cr->type() == Element::Type::REST) {
                                          delete el;
                                          }
                                    else
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
                                                prevSegm = prevSegm->prev1(Segment::Type::ChordRest);
                                                // if there is a ChordRest in the dest. track
                                                // this segment is a (potential) f.b. location
                                                if (prevSegm->element(destTrack) != nullptr) {
                                                      done = true;
                                                      }
                                                // in any case, look for a f.b. in annotations:
                                                // if there is a f.b. element in the right track,
                                                // this is an (actual) f.b. location
                                                foreach (Element* a, prevSegm->annotations()) {
                                                      if (a->type() == Element::Type::FIGURED_BASS && a->track() == destTrack) {
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
                                                nextSegm = nextSegm->next1(Segment::Type::ChordRest);
                                          if (!nextSegm || nextSegm->tick() > destTick) {      // no ChordRest segm at this tick
                                                nextSegm = new Segment(prevSegm->measure(), Segment::Type::ChordRest, destTick);
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
                                          if (a->type() == Element::Type::FIGURED_BASS && a->track() == destTrack) {
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
                                    while (cr->type() != Element::Type::CHORD && currSegm) {
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
                                    if (cr->type() != Element::Type::CHORD) {
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

//---------------------------------------------------------
//   cmdPaste
//---------------------------------------------------------

PasteStatus Score::cmdPaste(const QMimeData* ms, MuseScoreView* view)
      {
      if (ms == 0) {
            qDebug("no application mime data");
            return PasteStatus::NO_MIME;
            }
      if ((_selection.isSingle()|| _selection.isList()) && ms->hasFormat(mimeSymbolFormat)) {
            QByteArray data(ms->data(mimeSymbolFormat));
            XmlReader e(data);
            QPointF dragOffset;
            Fraction duration(1, 4);
            Element::Type type = Element::readType(e, &dragOffset, &duration);

            QList<Element*> els;
            if (_selection.isSingle())
                  els.append(_selection.element());
            else
                  els.append(_selection.elements());

            if (type != Element::Type::INVALID) {
                  Element* el = Element::create(type, this);
                  if (el) {
                        el->read(e);
                        if (el) {
                              for (Element* target : els) {
                                    Element* nel = el->clone();
                                    addRefresh(target->abbox());   // layout() ?!
                                    DropData ddata;
                                    ddata.view       = view;
                                    ddata.element    = nel;
                                    ddata.duration   = duration;
                                    target->drop(ddata);
                                    if (_selection.element())
                                          addRefresh(_selection.element()->abbox());
                                    }
                              }
                              delete el;
                        }
                  }
            else
                  qDebug("cannot read type");
            }
      else if ((_selection.isRange() || _selection.isList())
         && ms->hasFormat(mimeStaffListFormat)) {
            ChordRest* cr = 0;
            if (_selection.isRange())
                  cr = _selection.firstChordRest();
            else if (_selection.isSingle()) {
                  Element* e = _selection.element();
                  if (e->type() != Element::Type::NOTE && !e->isChordRest()) {
                        qDebug("cannot paste to %s", e->name());
                        return PasteStatus::DEST_NO_CR;
                        }
                  if (e->type() == Element::Type::NOTE)
                        e = static_cast<Note*>(e)->chord();
                  cr  = static_cast<ChordRest*>(e);
                  }
            if (cr == 0)
                  return PasteStatus::NO_DEST;
            else if (cr->tuplet())
                  return PasteStatus::DEST_TUPLET;
            else {
                  QByteArray data(ms->data(mimeStaffListFormat));
                  if (MScore::debugMode)
                        qDebug("paste <%s>", data.data());
                  XmlReader e(data);
                  e.setPasteMode(true);
                  if (!pasteStaff(e, cr->segment(),cr->staffIdx())) {
                        return PasteStatus::TUPLET_CROSSES_BAR;
                        }
                  }
            }
      else if (ms->hasFormat(mimeSymbolListFormat)) {
            ChordRest* cr = 0;
            if (_selection.isRange())
                  cr = _selection.firstChordRest();
            else if (_selection.isSingle()) {
                  Element* e = _selection.element();
                  if (e->type() != Element::Type::NOTE && e->type() != Element::Type::REST && e->type() != Element::Type::CHORD) {
                        qDebug("cannot paste to %s", e->name());
                        return PasteStatus::DEST_NO_CR;
                        }
                  if (e->type() == Element::Type::NOTE)
                        e = static_cast<Note*>(e)->chord();
                  cr  = static_cast<ChordRest*>(e);
                  }
            if (cr == 0)
                  return PasteStatus::NO_DEST;
            else if (cr->tuplet())
                  return PasteStatus::DEST_TUPLET;
            else {
                  QByteArray data(ms->data(mimeSymbolListFormat));
                  if (MScore::debugMode)
                        qDebug("paste <%s>", data.data());
                  XmlReader e(data);
                  pasteSymbols(e, cr);
                  }
            }
      else {
            qDebug("cannot paste selState %d staffList %hhd",
               _selection.state(), ms->hasFormat(mimeStaffListFormat));
            foreach(const QString& s, ms->formats())
                  qDebug("  format %s", qPrintable(s));
            }
      return PasteStatus::PS_NO_ERROR;
      }
}
