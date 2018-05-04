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
#include "fret.h"
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
#include "image.h"
#include "repeat.h"
#include "chord.h"
#include "tremolo.h"
#include "slur.h"
#include "articulation.h"

namespace Ms {

//---------------------------------------------------------
//   transposeChord
//---------------------------------------------------------

static void transposeChord(Chord* c, Interval srcTranspose, int tick)
      {
      // set note track
      // check if staffMove moves a note to a
      // nonexistent staff
      //
      int track  = c->track();
      int nn     = (track / VOICES) + c->staffMove();
      if (nn < 0 || nn >= c->score()->nstaves())
            c->setStaffMove(0);
      Part* part = c->part();
      Interval dstTranspose = part->instrument(tick)->transpose();

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

bool Score::pasteStaff(XmlReader& e, Segment* dst, int dstStaff)
      {
      Q_ASSERT(dst->segmentType() == SegmentType::ChordRest);
      QList<Chord*> graceNotes;
      int dstTick = dst->tick();
      bool pasted = false;
      int tickLen = 0;
      int staves  = 0;
      bool done   = false;
      while (e.readNextStartElement()) {
            if (done)
                  break;
            if (e.name() != "StaffList") {
                  e.unknown();
                  break;
                  }
            QString version = e.attribute("version", "NONE");
            if (!MScore::testMode) {
                  if (version != MSC_VERSION) {
                        qDebug("pasteStaff: bad version");
                        break;
                        }
                  }
            int tickStart     = e.intAttribute("tick", 0);
                tickLen       = e.intAttribute("len", 0);
            int staffStart    = e.intAttribute("staff", 0);
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
                  e.setTrack(srcStaffIdx * VOICES);
                  e.setTrackOffset((dstStaff - staffStart) * VOICES);
                  int dstStaffIdx = e.track() / VOICES;
                  if (dstStaffIdx >= dst->score()->nstaves()) {
                        qDebug("paste beyond staves");
                        done = true;
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
                        else if (tag == "move" || tag == "tick") {
                              int tick = tag == "move" ? e.readFraction().ticks() : e.readInt();
                              e.initTick(tick);
                              int shift = tick - tickStart;
                              if (makeGap && !makeGap1(dstTick, dstStaffIdx, Fraction::fromTicks(tickLen), voiceOffset)) {
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
                              // no paste into local time signature
                              if (staff(dstStaffIdx)->isLocalTimeSignature(tick)) {
                                    MScore::setError(DEST_LOCAL_TIME_SIGNATURE);
                                    return false;
                                    }
                              Measure* measure = tick2measure(tick);
                              tuplet->setParent(measure);
                              tuplet->setTick(tick);
                              int ticks = tuplet->actualTicks();
                              int rticks = measure->endTick() - tick;
                              if (rticks < ticks) {
                                    delete tuplet;
                                    MScore::setError(TUPLET_CROSSES_BAR);
                                    return false;
                                    }
                              e.addTuplet(tuplet);
                              }
                        else if (tag == "Chord" || tag == "Rest" || tag == "RepeatMeasure") {
                              ChordRest* cr = toChordRest(Element::name2Element(tag, this));
                              cr->setTrack(e.track());
                              cr->read(e);
                              cr->setSelected(false);
                              int tick = e.tick();
                              // no paste into local time signature
                              if (staff(dstStaffIdx)->isLocalTimeSignature(tick)) {
                                    MScore::setError(DEST_LOCAL_TIME_SIGNATURE);
                                    return false;
                                    }
                              if (cr->isGrace())
                                    graceNotes.push_back(toChord(cr));
                              else {
                                    e.incTick(cr->actualTicks());
                                    if (cr->isChord()) {
                                          Chord* chord = toChord(cr);
                                          // disallow tie across barline within two-note tremolo
                                          // tremolos can potentially still straddle the barline if no tie is required
                                          // but these will be removed later
                                          if (chord->tremolo() && chord->tremolo()->twoNotes()) {
                                                Measure* m = tick2measure(tick);
                                                int ticks = cr->actualTicks();
                                                int rticks = m->endTick() - tick;
                                                if (rticks < ticks || (rticks != ticks && rticks < ticks * 2)) {
                                                      MScore::setError(DEST_TREMOLO);
                                                      return false;
                                                      }
                                                }
                                          for (int i = 0; i < graceNotes.size(); ++i) {
                                                Chord* gc = graceNotes[i];
                                                gc->setGraceIndex(i);
                                                transposeChord(gc, e.transpose(), tick);
                                                chord->add(gc);
                                                }
                                          graceNotes.clear();
                                          }
                                    // delete pending ties, they are not selected when copy
                                    if ((tick - dstTick) + cr->actualTicks() >= tickLen) {
                                          if (cr->isChord()) {
                                                Chord* c = toChord(cr);
                                                for (Note* note: c->notes()) {
                                                      Tie* tie = note->tieFor();
                                                      if (tie) {
                                                            note->setTieFor(0);
                                                            delete tie;
                                                            }
                                                      }
                                                }
                                          }
                                    // shorten last cr to fit in the space made by makeGap
                                    if ((tick - dstTick) + cr->actualTicks() > tickLen) {
                                          int newLength = tickLen - (tick - dstTick);
                                          // check previous CR on same track, if it has tremolo, delete the tremolo
                                          // we don't want a tremolo and two different chord durations
                                          if (cr->isChord()) {
                                                Segment* s = tick2leftSegment(tick - 1);
                                                if (s) {
                                                      ChordRest* crt = toChordRest(s->element(cr->track()));
                                                      if (!crt)
                                                            crt = s->nextChordRest(cr->track(), true);
                                                      if (crt && crt->isChord()) {
                                                            Chord* chrt = toChord(crt);
                                                            Tremolo* tr = chrt->tremolo();
                                                            if (tr) {
                                                                  tr->setChords(chrt, toChord(cr));
                                                                  chrt->remove(tr);
                                                                  delete tr;
                                                                  }
                                                            }
                                                      }
                                                }
                                          if (!cr->tuplet()/*|| cr->actualTicks() - newLength > (cr->tuplet()->ratio().numerator() + 1 ) / 2*/) {
                                                // shorten duration
                                                // exempt notes in tuplets, since we don't allow copy of partial tuplet anyhow
                                                // TODO: figure out a reasonable fudge factor to make sure shorten tuplets appropriately if we do ever copy a partial tuplet
                                                cr->setDuration(Fraction::fromTicks(newLength));
                                                cr->setDurationType(newLength);
                                                }
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
                              Spanner* sp = toSpanner(Element::name2Element(tag, this));
                              sp->setAnchor(Spanner::Anchor::SEGMENT);
                              sp->read(e);
                              sp->setTrack(e.track());
                              sp->setTrack2(e.track());
                              sp->setTick(e.tick());
                              addSpanner(sp);
                              }
                        else if (tag == "Slur") {
                              Slur* sp = new Slur(this);
                              sp->read(e);
                              sp->setTrack(e.track());
                              sp->setTick(e.tick());
                              // check if we saw endSpanner / stop element already
                              int id = e.spannerId(sp);
                              const SpannerValues* sv = e.spannerValues(id);
                              if (sv) {
                                    sp->setTick2(sv->tick2);
                                    sp->setTrack2(sv->track2);
                                    }
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
                                    if (spanner->isOttava())
                                          spanner->staff()->updateOttava();
                                    }
                              e.readNext();
                              }
                        else if (tag == "Harmony") {
                              Harmony* harmony = new Harmony(this);
                              harmony->setTrack(e.track());
                              harmony->read(e);
                              harmony->setTrack(e.track());
                              // transpose
                              Part* partDest = staff(e.track() / VOICES)->part();
                              Interval interval = partDest->instrument(e.tick())->transpose();
                              if (!styleB(StyleIdx::concertPitch) && !interval.isZero()) {
                                    interval.flip();
                                    int rootTpc = transposeTpc(harmony->rootTpc(), interval, true);
                                    int baseTpc = transposeTpc(harmony->baseTpc(), interval, true);
                                    undoTransposeHarmony(harmony, rootTpc, baseTpc);
                                    }

                              int tick = e.tick();
                              Measure* m = tick2measure(tick);
                              Segment* seg = m->undoGetSegment(SegmentType::ChordRest, tick);
                              if (seg->findAnnotationOrElement(ElementType::HARMONY, e.track(), e.track())) {
                                    QList<Element*> elements;
                                    foreach (Element* el, seg->annotations()) {
                                          if (el->isHarmony() && el->track() == e.track()) {
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
                              el->setTrack(e.track());      // a valid track might be necessary for el->read() to work
                              el->read(e);

                              Measure* m = tick2measure(e.tick());
                              Segment* seg = m->undoGetSegment(SegmentType::ChordRest, e.tick());
                              el->setParent(seg);

                              // be sure to paste the element in the destination track;
                              // setting track needs to be repeated, as it might have been overwritten by el->read()
                              // preserve *voice* from source, though
                              el->setTrack((e.track() / VOICES) * VOICES + el->voice());

                              undoAddElement(el);
                              }
                        else if (tag == "Clef") {
                              Clef* clef = new Clef(this);
                              clef->read(e);
                              clef->setTrack(e.track());
                              int tick = e.tick();
                              Measure* m = tick2measure(tick);
                              if (m->tick() && m->tick() == tick)
                                    m = m->prevMeasure();
                              Segment* segment = m->undoGetSegment(SegmentType::Clef, tick);
                              clef->setParent(segment);
                              undoChangeElement(segment->element(e.track()), clef);
                              }
                        else if (tag == "Breath") {
                              Breath* breath = new Breath(this);
                              breath->read(e);
                              breath->setTrack(e.track());
                              int tick = e.tick();
                              Measure* m = tick2measure(tick);
                              Segment* segment = m->undoGetSegment(SegmentType::Breath, tick);
                              breath->setParent(segment);
                              undoChangeElement(segment->element(e.track()), breath);
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

                  for (Tuplet* tuplet : e.tuplets()) {
                        Q_ASSERT(!tuplet->elements().empty());
                        Measure* measure = tick2measure(tuplet->tick());
                        tuplet->setParent(measure);
                        tuplet->sortElements();
                        }
                  }
            }
      for (Score* s : scoreList())     // for all parts
            s->connectTies();

      if (pasted) {                       //select only if we pasted something
//TODO?            if (styleB(StyleIdx::createMultiMeasureRests))
//                  createMMRests();
            Segment* s1 = tick2segmentMM(dstTick);
            Segment* s2 = tick2segmentMM(dstTick + tickLen);
            int endStaff = dstStaff + staves;
            if (endStaff > nstaves())
                  endStaff = nstaves();
            //check and add truly invisible rests insted of gaps
            //TODO: look if this could be done different
            Measure* dstM = tick2measureMM(dstTick);
            Measure* endM = tick2measureMM(dstTick + tickLen);
            for (int i = dstStaff; i < endStaff; i++) {
                  for (Measure* m = dstM; m && m != endM->nextMeasureMM(); m = m->nextMeasureMM())
                        m->checkMeasure(i);
                  }
            _selection.setRange(s1, s2, dstStaff, endStaff);
            _selection.updateSelectedElements();

            //finding the first element that has a track
            //the canvas position will be set to this element
            Element* e = 0;
            Segment* s = s1;
            bool found = false;
            if (s2)
                  s2 = s2->next1MM();
            while (!found && s != s2) {
                  for (int i = dstStaff * VOICES; i < (endStaff + 1) * VOICES; i++) {
                        e = s->element(i);
                        if (e) {
                              found = true;
                              break;
                              }
                        }
                  s = s->next1MM();
                  }

            for (MuseScoreView* v : viewer)
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
// qDebug("pasteChordRest %s at %d, len %d/%d", cr->name(), tick, cr->duration().numerator(), cr->duration().denominator() );
      if (cr->isChord())
            transposeChord(toChord(cr), srcTranspose, tick);

      Measure* measure = tick2measure(tick);
      if (!measure)
            return;

      // we can paste a measure rest as such only at start of measure
      // and only if the lengths of the rest and measure match
      // otherwise, we need to convert to duration rest(s)
      // and potentially split the rest up (eg, 5/4 => whole + quarter)
      bool convertMeasureRest = cr->isRest() && cr->durationType().type() == TDuration::DurationType::V_MEASURE
         && (tick != measure->tick() || cr->duration() != measure->len());

      int measureEnd = measure->endTick();
      bool isGrace = cr->isChord() && toChord(cr)->noteType() != NoteType::NORMAL;

      // if note is too long to fit in measure, split it up with a tie across the barline
      // exclude tuplets from consideration
      // we have already disallowed a tuplet from crossing the barline, so there is no problem here
      // but due to rounding, it might appear from actualTicks() that the last note is too long by a couple of ticks

      if (!isGrace && !cr->tuplet() && (tick + cr->actualTicks() > measureEnd || convertMeasureRest)) {
            if (cr->isChord()) {
                  // split Chord
                  Chord* c = toChord(cr);
                  int rest = c->actualTicks();
                  bool firstpart = true;
                  while (rest) {
                        measure = tick2measure(tick);
                        Chord* c2 = firstpart ? c : toChord(c->clone());
                        if (!firstpart)
                              c2->removeMarkings(true);
                        int mlen = measure->tick() + measure->ticks() - tick;
                        int len = mlen > rest ? rest : mlen;
                        std::vector<TDuration> dl = toDurationList(Fraction::fromTicks(len), true);
                        TDuration d = dl[0];
                        c2->setDurationType(d);
                        c2->setDuration(d.fraction());
                        rest -= c2->actualTicks();
                        undoAddCR(c2, measure, tick);

                        std::vector<Note*> nl1 = c->notes();
                        std::vector<Note*> nl2 = c2->notes();

                        if (!firstpart)
                             for (unsigned i = 0; i < nl1.size(); ++i) {
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
                        firstpart = false;
                        tick += c->actualTicks();
                        }
                  }
            else if (cr->isRest()) {
                  // split Rest
                  Rest* r       = toRest(cr);
                  Fraction rest = r->duration();

                  bool firstpart = true;
                  while (!rest.isZero()) {
                        Rest* r2      = firstpart ? r : toRest(r->clone());
                        measure       = tick2measure(tick);
                        Fraction mlen = Fraction::fromTicks(measure->tick() + measure->ticks() - tick);
                        Fraction len  = rest > mlen ? mlen : rest;
                        std::vector<TDuration> dl = toDurationList(len, false);
                        TDuration d = dl[0];
                        r2->setDuration(d.fraction());
                        r2->setDurationType(d);
                        undoAddCR(r2, measure, tick);
                        rest -= d.fraction();
                        tick += r2->actualTicks();
                        firstpart = false;
                        }
                  }
            else if (cr->isRepeatMeasure()) {
                  RepeatMeasure* rm = toRepeatMeasure(cr);
                  std::vector<TDuration> list = toDurationList(rm->actualDuration(), true);
                  for (auto dur : list) {
                        Rest* r = new Rest(this, dur);
                        r->setTrack(cr->track());
                        Fraction rest = r->duration();
                        while (!rest.isZero()) {
                              Rest* r2      = toRest(r->clone());
                              measure       = tick2measure(tick);
                              Fraction mlen = Fraction::fromTicks(measure->tick() + measure->ticks() - tick);
                              Fraction len  = rest > mlen ? mlen : rest;
                              std::vector<TDuration> dl = toDurationList(len, false);
                              TDuration d = dl[0];
                              r2->setDuration(d.fraction());
                              r2->setDurationType(d);
                              undoAddCR(r2, measure, tick);
                              rest -= d.fraction();
                              tick += r2->actualTicks();
                              }
                        delete r;
                        }
                  delete cr;
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
      int   lastTick    = lastSegment()->tick();

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

                        if (tag == "Harmony" || tag == "FretDiagram") {
                              //
                              // Harmony elements (= chord symbols) are positioned respecting
                              // the original tickOffset: advance to destTick (or near)
                              // same for FretDiagram elements
                              //
                              Segment* harmSegm;
                              for (harmSegm = startSegm; harmSegm && (harmSegm->tick() < destTick);
                                          harmSegm = harmSegm->nextCR())
                                    ;
                              // if destTick overshot, no dest. segment: create one
                              if (destTick >= lastTick) {
                                    harmSegm = nullptr;
                                    }
                              else if (!harmSegm || harmSegm->tick() > destTick) {
                                    Measure* meas     = tick2measure(destTick);
                                    harmSegm          = meas ? meas->undoGetSegment(SegmentType::ChordRest, destTick) : nullptr;
                              }
                              if (destTrack >= maxTrack || harmSegm == nullptr) {
                                    qDebug("PasteSymbols: no track or segment for %s", tag.toUtf8().data());
                                    e.skipCurrentElement();       // ignore
                                    continue;
                                    }
                              if (tag == "Harmony") {
                                    Harmony* el = new Harmony(this);
                                    el->setTrack(trackZeroVoice(destTrack));
                                    el->read(e);
                                    el->setTrack(trackZeroVoice(destTrack));
                                    // transpose
                                    Part* partDest = staff(track2staff(destTrack))->part();
                                    Interval interval = partDest->instrument(destTick)->transpose();
                                    if (!styleB(StyleIdx::concertPitch) && !interval.isZero()) {
                                          interval.flip();
                                          int rootTpc = transposeTpc(el->rootTpc(), interval, true);
                                          int baseTpc = transposeTpc(el->baseTpc(), interval, true);
                                          undoTransposeHarmony(el, rootTpc, baseTpc);
                                          }
                                    el->setParent(harmSegm);
                                    undoAddElement(el);
                                    }
                              else {
                                    FretDiagram* el = new FretDiagram(this);
                                    el->setTrack(trackZeroVoice(destTrack));
                                    el->read(e);
                                    el->setTrack(trackZeroVoice(destTrack));
                                    el->setParent(harmSegm);
                                    undoAddElement(el);
                                    }
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
                              ChordRest* cr = toChordRest(currSegm->element(destTrack));

                              if (tag == "Articulation") {
                                    Articulation* el = new Articulation(this);
                                    el->read(e);
                                    el->setTrack(destTrack);
                                    el->setParent(cr);
                                    if (!el->isFermata() && cr->isRest())
                                          delete el;
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
                                                prevSegm = prevSegm->prev1(SegmentType::ChordRest);
                                                // if there is a ChordRest in the dest. track
                                                // this segment is a (potential) f.b. location
                                                if (prevSegm->element(destTrack) != nullptr) {
                                                      done = true;
                                                      }
                                                // in any case, look for a f.b. in annotations:
                                                // if there is a f.b. element in the right track,
                                                // this is an (actual) f.b. location
                                                foreach (Element* a, prevSegm->annotations()) {
                                                      if (a->isFiguredBass() && a->track() == destTrack) {
                                                            onNoteFB = toFiguredBass(a);
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
                                                nextSegm = nextSegm->next1(SegmentType::ChordRest);
                                          if (!nextSegm || nextSegm->tick() > destTick) {      // no ChordRest segm at this tick
                                                nextSegm = new Segment(prevSegm->measure(), SegmentType::ChordRest, destTick);
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
                                          ticks = toChordRest(currSegm->element(destTrack))->duration().ticks();
                                    // in both cases, look for an existing f.b. element in segment and remove it, if found
                                    FiguredBass* oldFB = nullptr;
                                    foreach (Element* a, currSegm->annotations()) {
                                          if (a->isFiguredBass() && a->track() == destTrack) {
                                                oldFB = toFiguredBass(a);
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
                                    while (!cr->isChord() && currSegm) {
                                          currSegm = currSegm->nextCR(destTrack);
                                          if (currSegm)
                                                cr = toChordRest(currSegm->element(destTrack));
                                          else
                                                break;
                                          }
                                    if (currSegm == nullptr) {
                                          qDebug("PasteSymbols: no segment for Lyrics");
                                          e.skipCurrentElement();
                                          continue;
                                          }
                                    if (!cr->isChord()) {
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

void Score::cmdPaste(const QMimeData* ms, MuseScoreView* view)
      {
      if (ms == 0) {
            qDebug("no application mime data");
            MScore::setError(NO_MIME);
            return;
            }
      if ((_selection.isSingle() || _selection.isList()) && ms->hasFormat(mimeSymbolFormat)) {
            QByteArray data(ms->data(mimeSymbolFormat));
            XmlReader e(data);
            QPointF dragOffset;
            Fraction duration(1, 4);
            ElementType type = Element::readType(e, &dragOffset, &duration);

            QList<Element*> els;
            if (_selection.isSingle())
                  els.append(_selection.element());
            else
                  els.append(_selection.elements());

            if (type != ElementType::INVALID) {
                  Element* el = Element::create(type, this);
                  if (el) {
                        el->read(e);
                        if (el) {
                              for (Element* target : els) {
                                    Element* nel = el->clone();
                                    addRefresh(target->abbox());   // layout() ?!
                                    EditData ddata(view);
                                    ddata.view       = view;
                                    ddata.element    = nel;
                                    ddata.duration   = duration;
                                    if (target->acceptDrop(ddata)) {
                                          target->drop(ddata);
                                          if (_selection.element())
                                                addRefresh(_selection.element()->abbox());
                                          }
                                    }
                              }
                              delete el;
                        }
                  }
            else
                  qDebug("cannot read type");
            }
      else if ((_selection.isRange() || _selection.isList()) && ms->hasFormat(mimeStaffListFormat)) {
            ChordRest* cr = 0;
            if (_selection.isRange())
                  cr = _selection.firstChordRest();
            else if (_selection.isSingle()) {
                  Element* e = _selection.element();
                  if (!e->isNote() && !e->isChordRest()) {
                        qDebug("cannot paste to %s", e->name());
                        MScore::setError(DEST_NO_CR);
                        return;
                        }
                  if (e->isNote())
                        e = toNote(e)->chord();
                  cr  = toChordRest(e);
                  }
            if (cr == 0) {
                  MScore::setError(NO_DEST);
                  return;
                  }
            else if (cr->tuplet()) {
                  MScore::setError(DEST_TUPLET);
                  return;
                  }
            else {
                  QByteArray data(ms->data(mimeStaffListFormat));
                  if (MScore::debugMode)
                        qDebug("paste <%s>", data.data());
                  XmlReader e(data);
                  e.setPasteMode(true);
                  if (!pasteStaff(e, cr->segment(), cr->staffIdx()))
                        return;
                  }
            }
      else if (ms->hasFormat(mimeSymbolListFormat)) {
            ChordRest* cr = 0;
            if (_selection.isRange())
                  cr = _selection.firstChordRest();
            else if (_selection.isSingle()) {
                  Element* e = _selection.element();
                  if (!e->isNote() && !e->isRest() && !e->isChord()) {
                        qDebug("cannot paste to %s", e->name());
                        MScore::setError(DEST_NO_CR);
                        return;
                        }
                  if (e->isNote())
                        e = toNote(e)->chord();
                  cr  = toChordRest(e);
                  }
            if (cr == 0) {
                  MScore::setError(NO_DEST);
                  return;
                  }
            else if (cr->tuplet()) {
                  MScore::setError(DEST_TUPLET);
                  return;
                  }
            else {
                  QByteArray data(ms->data(mimeSymbolListFormat));
                  if (MScore::debugMode)
                        qDebug("paste <%s>", data.data());
                  XmlReader e(data);
                  pasteSymbols(e, cr);
                  }
            }
      else if (ms->hasImage()) {
            QImage im = qvariant_cast<QImage>(ms->imageData());
            QByteArray ba;
            QBuffer buffer(&ba);
            buffer.open(QIODevice::WriteOnly);
            im.save(&buffer, "PNG");

            Image* image = new Image(this);
            image->setImageType(ImageType::RASTER);
            image->loadFromData("dragdrop", ba);

            QList<Element*> els;
            if (_selection.isSingle())
                  els.append(_selection.element());
            else
                  els.append(_selection.elements());

            for (Element* target : els) {
                  Element* nel = image->clone();
                  addRefresh(target->abbox());   // layout() ?!
                  EditData ddata(view);
                  ddata.view       = view;
                  ddata.element    = nel;
                  // ddata.duration   = duration;
                  target->drop(ddata);
                  if (_selection.element())
                        addRefresh(_selection.element()->abbox());
                  }
            delete image;
            }
      else {
            qDebug("cannot paste selState %d staffList %s",
               int(_selection.state()), (ms->hasFormat(mimeStaffListFormat))? "true" : "false");
            for (const QString& s : ms->formats())
                  qDebug("  format %s", qPrintable(s));
            }
      }
}
