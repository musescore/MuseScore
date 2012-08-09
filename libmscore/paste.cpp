//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: cmd.cpp 5644 2012-05-17 10:53:29Z lasconic $
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <assert.h>
#include "score.h"
#include "utils.h"
#include "key.h"
#include "clef.h"
#include "navigate.h"
#include "slur.h"
#include "note.h"
#include "rest.h"
#include "chord.h"
#include "text.h"
#include "sig.h"
#include "staff.h"
#include "part.h"
#include "style.h"
#include "page.h"
#include "barline.h"
#include "tuplet.h"
#include "xml.h"
#include "ottava.h"
#include "trill.h"
#include "pedal.h"
#include "hairpin.h"
#include "textline.h"
#include "keysig.h"
#include "volta.h"
#include "dynamic.h"
#include "box.h"
#include "harmony.h"
#include "system.h"
#include "stafftext.h"
#include "articulation.h"
#include "layoutbreak.h"
#include "drumset.h"
#include "beam.h"
#include "lyrics.h"
#include "pitchspelling.h"
#include "measure.h"
#include "tempo.h"
#include "sig.h"
#include "undo.h"
#include "timesig.h"
#include "repeat.h"
#include "tempotext.h"
#include "clef.h"
#include "noteevent.h"
#include "breath.h"
#include "tablature.h"
#include "stafftype.h"
#include "segment.h"
#include "chordlist.h"
#include "mscore.h"
#include "accidental.h"
#include "sequencer.h"

//---------------------------------------------------------
//   cmdPaste
//---------------------------------------------------------

void Score::cmdPaste(MuseScoreView* view)
      {
      const QMimeData* ms = QApplication::clipboard()->mimeData();
      if (ms == 0) {
            qDebug("no application mime data");
            return;
            }
      if (selection().isSingle() && ms->hasFormat(mimeSymbolFormat)) {
            QByteArray data(ms->data(mimeSymbolFormat));
            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(data, &err, &line, &column)) {
                  qDebug("error reading paste data at line %d column %d: %s",
                     line, column, qPrintable(err));
                  qDebug("%s", data.data());
                  return;
                  }
            docName = "--";
            QDomElement e = doc.documentElement();
            QPointF dragOffset;
            Fraction duration(1, 4);
            ElementType type = Element::readType(e, &dragOffset, &duration);
            if (type != INVALID) {
                  Element* el = Element::create(type, this);
                  if (el) {
                        el->read(e);
                        addRefresh(selection().element()->abbox());   // layout() ?!
                        DropData ddata;
                        ddata.view       = view;
                        ddata.element    = el;
                        ddata.duration   = duration;
                        selection().element()->drop(ddata);
                        if (selection().element())
                              addRefresh(selection().element()->abbox());
                        }
                  }
            else
                  qDebug("cannot read type");
            }
      else if ((selection().state() == SEL_RANGE || selection().state() == SEL_LIST)
         && ms->hasFormat(mimeStaffListFormat)) {
            ChordRest* cr = 0;
            if (selection().state() == SEL_RANGE) {
                  cr = selection().firstChordRest();
                  }
            else if (selection().isSingle()) {
                  Element* e = selection().element();
                  if (e->type() != NOTE && e->type() != REST) {
                        qDebug("cannot paste to %s", e->name());
                        return;
                        }
                  if (e->type() == NOTE)
                        e = static_cast<Note*>(e)->chord();
                  cr  = static_cast<ChordRest*>(e);
                  }
            if (cr == 0) {
                  qDebug("no destination for paste");
                  return;
                  }

            QByteArray data(ms->data(mimeStaffListFormat));
// qDebug("paste <%s>", data.data());
            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(data, &err, &line, &column)) {
                  qDebug("error reading paste data at line %d column %d: %s",
                     line, column, qPrintable(err));
                  qDebug("%s", data.data());
                  return;
                  }
            docName = "--";
            pasteStaff(doc.documentElement(), cr);
            }
      else if (ms->hasFormat(mimeSymbolListFormat) && selection().isSingle()) {
            qDebug("cannot paste symbol list to element");
            }
      else {
            qDebug("cannot paste selState %d staffList %d",
               selection().state(), ms->hasFormat(mimeStaffListFormat));
            foreach(const QString& s, ms->formats())
                  qDebug("  format %s", qPrintable(s));
            }
      }

//---------------------------------------------------------
//   pasteStaff
//---------------------------------------------------------

void Score::pasteStaff(const QDomElement& de, ChordRest* dst)
      {
      beams.clear();
      spanner.clear();
//      QList<Tuplet*> invalidTuplets;

      for (Segment* s = firstMeasure()->first(Segment::SegChordRest); s; s = s->next1(Segment::SegChordRest)) {
            foreach(Spanner* e, s->spannerFor())
                  e->setId(-1);
            }
      int dstStaffStart = dst->staffIdx();
      int dstTick = dst->tick();
      for (QDomElement e = de; !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() != "StaffList") {
                  domError(e);
                  continue;
                  }
            int tickStart     = e.attribute("tick","0").toInt();
            int tickLen       = e.attribute("len", "0").toInt();
            int srcStaffStart = e.attribute("staff", "0").toInt();
            int staves        = e.attribute("staves", "0").toInt();
            curTick           = tickStart;

            QSet<int> blackList;
            for (int i = 0; i < staves; ++i) {
                  int staffIdx = i + dstStaffStart;
                  if (staffIdx >= nstaves())
                        break;
                  if (!makeGap1(dst->tick(), staffIdx, Fraction::fromTicks(tickLen))) {
qDebug("cannot make gap in staff %d at tick %d", staffIdx, dst->tick());
                        blackList.insert(staffIdx);
                        }
                  }
            bool pasted = false;
            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  if (ee.tagName() != "Staff") {
                        domError(ee);
                        continue;
                        }
                  int srcStaffIdx = ee.attribute("id", "0").toInt();
                  if(blackList.contains(srcStaffIdx))
                        continue;
                  int dstStaffIdx = srcStaffIdx - srcStaffStart + dstStaffStart;
                  if (dstStaffIdx >= nstaves())
                        break;
                  QList<Tuplet*> tuplets;
                  QList<Spanner*> spanner;
                  for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                        pasted = true;
                        const QString& tag(eee.tagName());
                        if (tag == "tick")
                              curTick = eee.text().toInt();
                        else if (tag == "Tuplet") {
                              Tuplet* tuplet = new Tuplet(this);
                              tuplet->setTrack(curTrack);
                              tuplet->read(eee, &tuplets, &spanner);
                              int tick = curTick - tickStart + dstTick;
                              Measure* measure = tick2measure(tick);
                              tuplet->setParent(measure);
                              tuplet->setTick(tick);
                              tuplets.append(tuplet);
                              }
                        else if (tag == "Slur") {
                              Slur* slur = new Slur(this);
                              slur->read(eee);
                              slur->setTrack(dstStaffIdx * VOICES);
                              spanner.append(slur);
                              }
                        else if (tag == "Chord" || tag == "Rest" || tag == "RepeatMeasure") {
                              ChordRest* cr = static_cast<ChordRest*>(Element::name2Element(tag, this));
                              cr->setTrack(curTrack);
                              cr->read(eee, &tuplets, &spanner);
                              cr->setSelected(false);
                              int voice = cr->voice();
                              int track = dstStaffIdx * VOICES + voice;
                              cr->setTrack(track);
                              int tick = curTick - tickStart + dstTick;
#if 0
                              //
                              // check for tuplet
                              //
                              if (cr->tuplet()) {
                                    Tuplet* tuplet = cr->tuplet();
                                    if (tuplet->elements().isEmpty()) {
                                          Measure* measure = tick2measure(tick);
                                          int measureEnd = measure->tick() + measure->ticks();
                                          if (tick + tuplet->actualTicks() > measureEnd) {
                                                invalidTuplets.append(tuplet);
                                                cr->setDuration(tuplet->duration());
                                                cr->setDurationType(cr->duration());
                                                cr->setTuplet(0);
                                                tuplet->add(cr);
                                                qDebug("cannot paste tuplet across bar line");
                                                }
                                          }
                                    else {
                                          foreach(Tuplet* t, invalidTuplets) {
                                                if (tuplet == t) {
                                                      delete cr;
                                                      cr = 0;
                                                      break;
                                                      }
                                                }
                                          }
                                    }
                              if (cr == 0)
                                    continue;
#endif
                              curTick += cr->actualTicks();
                              pasteChordRest(cr, tick);
                              }
                        else if (tag == "HairPin"
                           || tag == "Pedal"
                           || tag == "Ottava"
                           || tag == "Trill"
                           || tag == "TextLine"
                           || tag == "Volta") {
                              Spanner* sp = static_cast<Spanner*>(Element::name2Element(tag, this));
                              sp->setTrack(dstStaffIdx * VOICES);
                              sp->read(eee);
                              int tick = curTick - tickStart + dstTick;
                              Measure* m = tick2measure(tick);
                              Segment* segment = m->undoGetSegment(Segment::SegChordRest, tick);
                              sp->setStartElement(segment);
                              sp->setParent(segment);
                              undoAddElement(sp);
                              }
                        else if (tag == "endSpanner") {
                              int id = eee.attribute("id").toInt();
                              Spanner* e = findSpanner(id);
                              if (e) {
                                    int tick = curTick - tickStart + dstTick;
                                    Measure* m = tick2measure(tick);
                                    Segment* seg = m->undoGetSegment(Segment::SegChordRest, tick);
                                    e->setEndElement(seg);
                                    seg->addSpannerBack(e);
                                    if (e->type() == OTTAVA) {
                                          Ottava* o = static_cast<Ottava*>(e);
                                          int shift = o->pitchShift();
                                          Staff* st = o->staff();
                                          int tick1 = static_cast<Segment*>(o->startElement())->tick();
                                          st->pitchOffsets().setPitchOffset(tick1, shift);
                                          st->pitchOffsets().setPitchOffset(tick, 0);
                                          }
                                    else if (e->type() == HAIRPIN) {
                                          Hairpin* hp = static_cast<Hairpin*>(e);
                                          updateHairpin(hp);
                                          }
                                    }
                              }

                        else if (tag == "Lyrics") {
                              Lyrics* lyrics = new Lyrics(this);
                              lyrics->setTrack(curTrack);
                              lyrics->read(eee);
                              lyrics->setTrack(dstStaffIdx * VOICES);
                              int tick = curTick - tickStart + dstTick;
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
                        else if (tag == "Harmony") {
                              Harmony* harmony = new Harmony(this);
                              harmony->setTrack(curTrack);
                              harmony->read(eee);
                              harmony->setTrack(dstStaffIdx * VOICES);
                              //transpose
                              Part* partDest = staff(dstStaffIdx)->part();
                              Part* partSrc = staff(srcStaffIdx)->part();
                              Interval intervalDest = partDest->instr()->transpose();
                              Interval intervalSrc = partSrc->instr()->transpose();
                              Interval interval = Interval(intervalSrc.diatonic - intervalDest.diatonic, intervalSrc.chromatic - intervalDest.chromatic);
                              if (!styleB(ST_concertPitch)) {
                                    int rootTpc = transposeTpc(harmony->rootTpc(), interval, false);
                                    int baseTpc = transposeTpc(harmony->baseTpc(), interval, false);
                                    undoTransposeHarmony(harmony, rootTpc, baseTpc);
                                    }

                              int tick = curTick - tickStart + dstTick;
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
                           ) {
                              Element* e = Element::name2Element(tag, this);
                              e->setTrack(dstStaffIdx * VOICES);

                              int tick = curTick - tickStart + dstTick;
                              Measure* m = tick2measure(tick);
                              Segment* seg = m->undoGetSegment(Segment::SegChordRest, tick);
                              e->setParent(seg);
                              e->read(eee);

                              undoAddElement(e);
                              }
                        else if (tag == "Clef") {
                              Clef* clef = new Clef(this);
                              clef->read(eee);
                              clef->setTrack(dstStaffIdx * VOICES);
                              int tick = curTick - tickStart + dstTick;
                              Measure* m = tick2measure(tick);
                              if (m->tick() && m->tick() == tick)
                                    m = m->prevMeasure();
                              Segment* segment = m->undoGetSegment(Segment::SegClef, tick);
                              clef->setParent(segment);
                              undoAddElement(clef);
                              }
                        else if (tag == "Breath") {
                              Breath* breath = new Breath(this);
                              breath->read(eee);
                              breath->setTrack(dstStaffIdx * VOICES);
                              int tick = curTick - tickStart + dstTick;
                              Measure* m = tick2measure(tick);
                              Segment* segment = m->undoGetSegment(Segment::SegBreath, tick);
                              breath->setParent(segment);
                              undoAddElement(breath);
                              }
                        else if (tag == "BarLine") {
                              // ignore bar line
                              }
                        else {
                              domError(eee);
                              continue;
                              }
                        }
                  foreach(Spanner* s, spanner) {
                        if (s->type() == SLUR)
                              undoAddElement(s);
                        }
                  foreach (Tuplet* tuplet, tuplets) {
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

/*      foreach(Tuplet* t, invalidTuplets) {
            t->measure()->remove(t);
            delete t;
            }
*/
      connectTies();
//      updateNotes();    // TODO: undoable version needed

//      layoutFlags |= LAYOUT_FIX_PITCH_VELO;
      }

//---------------------------------------------------------
//   pasteChordRest
//---------------------------------------------------------

void Score::pasteChordRest(ChordRest* cr, int tick)
      {
// qDebug("pasteChordRest %s at %d", cr->name(), tick);
      if (cr->type() == CHORD) {
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
                        foreach(Note* n, c->notes()) {
                              int npitch;
                              int ntpc;
                              transposeInterval(n->pitch(), n->tpc(), &npitch, &ntpc, interval, true);
                              n->setPitch(npitch, ntpc);
                              }
                        }
                  }
            }

      Measure* measure = tick2measure(tick);
      bool isGrace = (cr->type() == CHORD) && (((Chord*)cr)->noteType() != NOTE_NORMAL);
      int measureEnd = measure->tick() + measure->ticks();
      if (tick >= measureEnd)       // end of score
            return;

      if (!isGrace && (tick + cr->actualTicks() > measureEnd)) {
            if (cr->type() == CHORD) {
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
