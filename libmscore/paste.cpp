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

namespace Ms {

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
            XmlReader e(data);
            QPointF dragOffset;
            Fraction duration(1, 4);
            Element::ElementType type = Element::readType(e, &dragOffset, &duration);
            if (type != Element::INVALID) {
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
                  if (e->type() != Element::NOTE && e->type() != Element::REST) {
                        qDebug("cannot paste to %s", e->name());
                        return;
                        }
                  if (e->type() == Element::NOTE)
                        e = static_cast<Note*>(e)->chord();
                  cr  = static_cast<ChordRest*>(e);
                  }
            if (cr == 0) {
                  qDebug("no destination for paste");
                  return;
                  }

            QByteArray data(ms->data(mimeStaffListFormat));
// qDebug("paste <%s>", data.data());
            XmlReader e(data);
            pasteStaff(e, cr);
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

void Score::pasteStaff(XmlReader& e, ChordRest* dst)
      {
      for (auto i :_spanner)
            i.second->setId(-1);

      QList<Chord*> graceNotes;
      int dstStaffStart = dst->staffIdx();
      int dstTick = dst->tick();

      while (e.readNextStartElement()) {
            if (e.name() != "StaffList") {
                  e.unknown();
                  break;
                  }

            int tickStart     = e.intAttribute("tick", 0);
            int tickLen       = e.intAttribute("len", 0);
            int srcStaffStart = e.intAttribute("staff", 0);
            int staves        = e.intAttribute("staves", 0);
            e.setTick(tickStart);

            for (int i = 0; i < staves; ++i) {
                  int staffIdx = i + dstStaffStart;
                  if (staffIdx >= nstaves())
                        break;
                  if (!makeGap1(dst->tick(), staffIdx, Fraction::fromTicks(tickLen))) {
                        qDebug("cannot make gap in staff %d at tick %d", staffIdx, dst->tick());
                        return;
                        }
                  }
            bool pasted = false;
            while (e.readNextStartElement()) {
                  if (e.name() != "Staff") {
                        e.unknown();
                        break;
                        }
                  int srcStaffIdx = e.attribute("id", "0").toInt();
                  int dstStaffIdx = srcStaffIdx - srcStaffStart + dstStaffStart;
                  if (dstStaffIdx >= nstaves())
                        break;
                  e.tuplets().clear();
                  while (e.readNextStartElement()) {
                        pasted = true;
                        const QStringRef& tag(e.name());

                        if (tag == "tick")
                              e.setTick(e.readInt());
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
                              sp->setTrack(dstStaffIdx * VOICES);
                              sp->read(e);
                              sp->setTick(e.tick() - tickStart + dstTick);
                              e.addSpanner(sp);
                              }
                        else if (tag == "endSpanner") {
                              int id = e.intAttribute("id");
                              Spanner* spanner = e.findSpanner(id);
                              if (spanner) {
                                    e.spanner().removeOne(spanner);
                                    spanner->setTick2(e.tick() - tickStart + dstTick);
                                    undoAddElement(spanner);
                                    if (spanner->type() == Element::OTTAVA) {
                                          Ottava* o = static_cast<Ottava*>(spanner);
                                          int shift = o->pitchShift();
                                          Staff* st = o->staff();
                                          st->pitchOffsets().setPitchOffset(o->tick(), shift);
                                          st->pitchOffsets().setPitchOffset(o->tick2(), 0);
                                          }
                                    else if (spanner->type() == Element::HAIRPIN) {
                                          Hairpin* hp = static_cast<Hairpin*>(spanner);
                                          updateHairpin(hp);
                                          }
                                    }
                              e.readNext();
                              }
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

      foreach(Spanner* sp, e.spanner()) {
            printf("  %s %p %p\n", sp->name(), sp->startElement(), sp->endElement());
            if (sp->tick() == -1 || sp->tickLen() == -1)
                  delete sp;
            }
      foreach (Score* s, scoreList())     // for all parts
            s->connectTies();
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

}

