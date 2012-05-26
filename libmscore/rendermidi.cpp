//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: rendermidi.cpp 5568 2012-04-22 10:08:43Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 \file
 render score into event list
*/

#include "score.h"
#include "volta.h"
#include "note.h"
#include "instrument.h"
#include "part.h"
#include "chord.h"
#include "style.h"
#include "ottava.h"
#include "slur.h"
#include "stafftext.h"
#include "repeat.h"
#include "articulation.h"
#include "arpeggio.h"
#include "durationtype.h"
#include "measure.h"
#include "tempo.h"
#include "sig.h"
#include "repeatlist.h"
#include "velo.h"
#include "dynamic.h"
#include "navigate.h"
#include "pedal.h"
#include "staff.h"
#include "hairpin.h"
#include "bend.h"
#include "tremolo.h"
#include "noteevent.h"
#include "segment.h"

//---------------------------------------------------------
//   updateChannel
//---------------------------------------------------------

void Score::updateChannel()
      {
      foreach(Staff* s, _staves) {
            for (int i = 0; i < VOICES; ++i)
                  s->channelList(i)->clear();
            }
      Measure* fm = firstMeasure();
      if (!fm)
            return;
      for (Segment* s = fm->first(SegChordRest); s; s = s->next1(SegChordRest)) {
            foreach(const Element* e, s->annotations()) {
                  if (e->type() != STAFF_TEXT)
                        continue;
                  const StaffText* st = static_cast<const StaffText*>(e);
                  for (int voice = 0; voice < VOICES; ++voice) {
                        QString an(st->channelName(voice));
                        if (an.isEmpty())
                              continue;
                        Staff* staff = _staves[st->staffIdx()];
                        int a = staff->part()->instr()->channelIdx(an);
                        if (a != -1)
                              staff->channelList(voice)->insert(s->tick(), a);
                        }
                  }
            }

      for (Segment* s = fm->first(SegChordRest | SegGrace); s; s = s->next1(SegChordRest | SegGrace)) {
            foreach(Staff* st, _staves) {
                  int strack = st->idx() * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        if (!s->element(track))
                              continue;
                        Element* e = s->element(track);
                        if (e->type() != CHORD)
                              continue;
                        Chord* c = static_cast<Chord*>(e);
                        int channel = st->channel(c->tick(), c->voice());
                        foreach (Note* note, c->notes()) {
                              if (note->hidden())
                                    continue;
                              if (note->tieBack())
                                    continue;
                              note->setSubchannel(channel);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   playNote
//---------------------------------------------------------

static void playNote(EventMap* events, const Note* note, int channel, int pitch,
   int velo, int onTime, int offTime)
      {
      velo = note->customizeVelocity(velo);
      Event ev(ME_NOTEON);
      ev.setChannel(channel);
      ev.setPitch(pitch);
      ev.setVelo(velo);
      ev.setTuning(note->tuning());
      ev.setNote(note);
      events->insertMulti(onTime, ev);
      ev.setVelo(0);
      events->insertMulti(offTime, ev);
      }

//---------------------------------------------------------
//   collectNote
//---------------------------------------------------------

static void collectNote(EventMap* events, int channel, const Note* note, int velo, int tickOffset, int gateTime)
      {
      if (note->hidden() || note->tieBack())       // do not play overlapping notes
            return;

      int pitch   = note->ppitch();
      int tick1   = note->chord()->tick() + tickOffset;
      int tick2   = tick1 + note->playTicks();
      int onTime  = tick1 + note->onTimeOffset() + note->onTimeUserOffset();
      int offTime = tick2 + note->offTimeOffset() + note->offTimeUserOffset() - 1;

      if (!note->playEvents().isEmpty()) {
            int ticks = note->playTicks();
            foreach(NoteEvent* e, note->playEvents()) {
                  int p = pitch + e->pitch();
                  if (p < 0)
                        p = 0;
                  else if (p > 127)
                        p = 127;
                  int on  = tick1 + (ticks * e->ontime())/1000;
                  int off = on + (ticks * e->len())/1000;
                  playNote(events, note, channel, p, velo, on, off);
                  }
            }
      else {
            offTime = tick1 + note->playTicks() * gateTime / 100 + note->offTimeOffset() + note->offTimeUserOffset() - 1;
            if (offTime - onTime <= 0)
                  offTime = onTime + 1;
            playNote(events, note, channel, pitch, velo, onTime, offTime);
            }
#if 0
      if (note->bend()) {
            Bend* bend = note->bend();
            int ticks = note->playTicks();
            const QList<PitchValue>& points = bend->points();

            // transform into midi values
            //    pitch is in 1/100 semitones
            //    midi pitch is 12/16384 semitones
            //
            //    time is in noteDuration/60

            int n = points.size();
            int tick1 = 0;
            for (int pt = 0; pt < n; ++pt) {
                  int pitch = points[pt].pitch;

                  if ((pt == 0) && (pitch == points[pt+1].pitch)) {
                        Event ev(ME_CONTROLLER);
                        ev.setChannel(channel);
                        ev.setController(CTRL_PITCH);
                        int midiPitch = (pitch * 16384) / 300;
                        ev.setValue(midiPitch);
                        events->insertMulti(tick, ev);
                        }
                  if (pitch != points[pt+1].pitch) {
                        int pitchDelta = points[pt+1].pitch - pitch;
                        int tick2      = (points[pt+1].time * ticks) / 60;
                        int dt = points[pt+1].time - points[pt].time;
                        for (int tick3 = tick1; tick3 < tick2; tick3 += 16) {
                              Event ev(ME_CONTROLLER);
                              ev.setChannel(channel);
                              ev.setController(CTRL_PITCH);

                              int dx = ((tick3-tick1) * 60) / ticks;
                              int p  = pitch + dx * pitchDelta / dt;

                              int midiPitch = (p * 16384) / 1200;
                              ev.setValue(midiPitch);
                              events->insertMulti(tick + tick3, ev);
                              }
                        tick1 = tick2;
                        }
                  if (pt == (n-2))
                        break;
                  }
            Event ev(ME_CONTROLLER);
            ev.setChannel(channel);
            ev.setController(CTRL_PITCH);
            ev.setValue(0);
            events->insertMulti(tick + ticks, ev);
            }
#endif
      }

//---------------------------------------------------------
//   OttavaShiftSegment
//---------------------------------------------------------

struct OttavaShiftSegment {
      int stick;
      int etick;
      int shift;
      };

//---------------------------------------------------------
//   playChord
//---------------------------------------------------------

static void playChord(EventMap* events, Chord* chord, Instrument* instr, int velocity, int tick, int ticks)
      {
      foreach (const Note* note, chord->notes()) {
            int channel = instr->channel(note->subchannel()).channel;
            playNote(events, note, channel, note->ppitch(), velocity, tick, tick + ticks);
            }
      }

//---------------------------------------------------------
//   collectMeasureEvents
//---------------------------------------------------------

static void collectMeasureEvents(EventMap* events, Measure* m, Part* part, int tickOffset)
      {
      int firstStaffIdx = m->score()->staffIdx(part);
      int nextStaffIdx  = firstStaffIdx + part->nstaves();

      SegmentTypes st = SegGrace | SegChordRest;
      int strack      = firstStaffIdx * VOICES;
      int etrack      = nextStaffIdx * VOICES;

      for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
            int tick = seg->tick();
            for (int track = strack; track < etrack; ++track) {
                  // skip linked staves, except primary
                  if (!m->score()->staff(track / VOICES)->primaryStaff()) {
                        track += VOICES-1;
                        continue;
                        }
                  Element* cr = seg->element(track);
                  if (cr && cr->type() == CHORD) {
                        Chord* chord = static_cast<Chord*>(cr);
                        Staff* staff = chord->staff();
                        int velocity = staff->velocities().velo(seg->tick());
// printf("velo %d(%d) %d\n", tick/480, tick/480/4, velocity);
                        Instrument* instr = chord->staff()->part()->instr(tick);
                        //
                        // adjust velocity for instrument, channel and
                        // depending on articulation marks
                        //
                        int channel = 0;  // note->subchannel();
                        instr->updateVelocity(&velocity, channel, "");
                        int gateTime = 100;
                        foreach (Articulation* a, *chord->getArticulations()) {
                              instr->updateVelocity(&velocity, channel, a->subtypeName());
                              instr->updateGateTime(&gateTime, channel, a->subtypeName());
                              }

                        Tremolo* tremolo = chord->tremolo();
                        if (tremolo) {
                              Fraction tl = tremolo->tremoloLen();
                              Fraction cl = chord->durationType().fraction();
                              Fraction r = cl / tl;
                              int repeats = r.numerator() / r.denominator();

                              if (chord->tremoloChordType() == TremoloFirstNote) {
                                    repeats /= 2;
                                    Segment* seg2 = seg->next(st);
                                    while (seg2 && !seg2->element(track))
                                          seg2 = seg2->next(st);
                                    ChordRest* cr = static_cast<ChordRest*>(seg2->element(track));
                                    if (cr && cr->type() == CHORD) {
                                          Chord* c2 = static_cast<Chord*>(cr);
                                          int tick = chord->tick() + tickOffset;
                                          for (int i = 0; i < repeats; ++i) {
                                                playChord(events, chord, instr, velocity, tick, tl.ticks());
                                                tick += tl.ticks();
                                                playChord(events, c2, instr, velocity, tick, tl.ticks());
                                                tick += tl.ticks();
                                                }
                                          }
                                    else
                                          qDebug("Tremolo: cannot find 2. chord\n");
                                    }
                              else if (chord->tremoloChordType() == TremoloSingle) {
                                    for (int i = 0; i < repeats; ++i) {
                                          int tick = chord->tick() + tickOffset + i * tl.ticks();
                                          playChord(events, chord, instr, velocity, tick, tl.ticks());
                                          }
                                    }
                              // else if (chord->tremoloChordType() == TremoloSecondNote)
                              //    ignore second note of two note tremolo
                              }
                        else {
                              //
                              // TODO: channel should be moved to chord instead of note
                              //
                              int channel = instr->channel(chord->upNote()->subchannel()).channel;
                              foreach(const Note* note, chord->notes()) {
                                    collectNote(events, channel, note, velocity, tickOffset, gateTime);
                                    }
                              }
                        }
                  }
            }

      //
      // collect program changes and controller
      //
      for (Segment* s = m->first(SegChordRest); s; s = s->next(SegChordRest)) {
            // int tick = s->tick();
            foreach(Element* e, s->annotations()) {
                  if (e->type() != STAFF_TEXT
                     || e->staffIdx() < firstStaffIdx
                     || e->staffIdx() >= nextStaffIdx)
                        continue;
                  const StaffText* st = static_cast<const StaffText*>(e);
                  int tick = s->tick() + tickOffset;

                  Instrument* instr = e->staff()->part()->instr(tick);
                  foreach (const ChannelActions& ca, *st->channelActions()) {
                        int channel = ca.channel;
                        foreach(const QString& ma, ca.midiActionNames) {
                              NamedEventList* nel = instr->midiAction(ma, channel);
                              if (!nel)
                                    continue;
                              int n = nel->events.size();
                              for (int i = n-1; i >= 0; --i) {
                                    Event event(nel->events[i]);
                                    event.setOntime(tick);
                                    event.setChannel(channel);
                                    events->insertMulti(tick, event);
                                    }
                              }
                        }
                  if (st->setAeolusStops()) {
                        Staff* staff = st->staff();
                        int voice   = 0;
                        int channel = staff->channel(tick, voice);

                        for (int i = 0; i < 4; ++i) {
                              for (int k = 0; k < 16; ++k) {
                                    if (st->getAeolusStop(i, k)) {
                                          Event event;
                                          event.setType(ME_CONTROLLER);
                                          event.setController(98);
                                          event.setValue(k);
                                          event.setOntime(tick);
                                          event.setChannel(channel);
                                          events->insertMulti(tick, event);
                                          }
                                    }
                              Event event(ME_CONTROLLER);
                              event.setController(98);
                              event.setValue(96 + i);
                              event.setOntime(tick);
                              event.setChannel(channel);
                              events->insertMulti(tick, event);

                              event.setValue(64 + i);
                              events->insertMulti(tick, event);
                              }
                        }
                  }
            foreach(Spanner* e, s->spannerFor()) {
                  if (e->staffIdx() < firstStaffIdx || e->staffIdx() >= nextStaffIdx)
                        continue;
                  if (e->type() == PEDAL) {
                        Segment* s1 = static_cast<Segment*>(e->startElement());
                        Segment* s2 = static_cast<Segment*>(e->endElement());
                        Staff* staff = e->staff();

                        int channel = staff->channel(s1->tick(), 0);

                        Event event(ME_CONTROLLER);
                        event.setChannel(channel);
                        event.setController(CTRL_SUSTAIN);

                        event.setValue(127);
                        events->insertMulti(s1->tick() + tickOffset, event);

                        event.setValue(0);
                        events->insertMulti(s2->tick() + tickOffset - 1, event);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   renderPart
//---------------------------------------------------------

void Score::renderPart(EventMap* events, Part* part)
      {
      Measure* lastMeasure = 0;
      foreach (const RepeatSegment* rs, *repeatList()) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            int tickOffset = rs->utick - rs->tick;
            for (Measure* m = tick2measure(startTick); m; m = m->nextMeasure()) {
                  if (lastMeasure && m->isRepeatMeasure()) {
                        int offset = m->tick() - lastMeasure->tick();
                        collectMeasureEvents(events, lastMeasure, part, tickOffset + offset);
                        }
                  else {
                        lastMeasure = m;
                        collectMeasureEvents(events, m, part, tickOffset);
                        }
                  if (m->tick() + m->ticks() >= endTick)
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   updateRepeatList
//---------------------------------------------------------

void Score::updateRepeatList(bool expandRepeats)
      {
      if (!expandRepeats) {
            foreach(RepeatSegment* s, *repeatList())
                  delete s;
            repeatList()->clear();
            Measure* m = lastMeasure();
            if (m == 0)
                  return;
            RepeatSegment* s = new RepeatSegment;
            s->tick  = 0;
            s->len   = m->tick() + m->ticks();
            s->utick = 0;
            s->utime = 0.0;
            s->timeOffset = 0.0;
            repeatList()->append(s);
            }
      else
            repeatList()->unwind();
      if (MScore::debugMode)
            repeatList()->dump();
      setPlaylistDirty(true);
      }

//---------------------------------------------------------
//   toEList
//    export score to event list
//---------------------------------------------------------

void Score::toEList(EventMap* events)
      {
      updateRepeatList(_playRepeats);
      _foundPlayPosAfterRepeats = false;
      updateChannel();
      updateVelo();
      foreach (Part* part, _parts)
            renderPart(events, part);

      // add metronome ticks
      foreach (const RepeatSegment* rs, *repeatList()) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            int tickOffset = rs->utick - rs->tick;
            for (Measure* m = tick2measure(startTick); m; m = m->nextMeasure()) {
                  Fraction ts = sigmap()->timesig(m->tick()).timesig();

                  int tw = 0;
                  switch(ts.denominator()) {
                        case  1: tw = MScore::division * 4; break;
                        case  2: tw = MScore::division * 2; break;
                        case  4: tw = MScore::division; break;
                        case  8: tw = MScore::division / 2; break;
                        case 16: tw = MScore::division / 4; break;
                        case 32: tw = MScore::division / 8; break;
                        }

                  if (tw == 0)
                        continue;
                  for (int i = 0; i < ts.numerator(); i++) {
                        int tick = m->tick() + i * tw + tickOffset;
                        Event event;
                        event.setType(i == 0 ? ME_TICK1 : ME_TICK2);
                        events->insertMulti(tick, event);
                        }
                  if (m->tick() + m->ticks() >= endTick)
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   updateHairpin
//---------------------------------------------------------

void Score::updateHairpin(Hairpin* h)
      {
      Staff* st = h->staff();
      int tick  = h->segment()->tick();
      int velo  = st->velocities().velo(tick);
      int incr  = h->veloChange();

      Segment* es = static_cast<Segment*>(h->endElement());
      if (!es || es->parent() == 0)
            return;
      int tick2 = es->tick() - 1;

      //
      // If velocity increase/decrease is zero, then assume
      // the end velocity is taken from the next velocity
      // event (the next dynamics symbol after the hairpin).
      //

      int endVelo = velo;
      if (incr == 0)
            endVelo = st->velocities().nextVelo(tick2+1);
      else
            endVelo += incr;

      if (endVelo > 127)
            endVelo = 127;
      else if (endVelo < 1)
            endVelo = 1;

      switch(h->dynType()) {
            case DYNAMIC_STAFF:
                  st->velocities().setVelo(tick,  VeloEvent(VELO_RAMP, velo));
                  st->velocities().setVelo(tick2, VeloEvent(VELO_FIX, endVelo));
                  break;
            case DYNAMIC_PART:
                  foreach(Staff* s, *st->part()->staves()) {
                        s->velocities().setVelo(tick,  VeloEvent(VELO_RAMP, velo));
                        s->velocities().setVelo(tick2, VeloEvent(VELO_FIX, endVelo));
                        }
                  break;
            case DYNAMIC_SYSTEM:
                  foreach(Staff* s, _staves) {
                        s->velocities().setVelo(tick,  VeloEvent(VELO_RAMP, velo));
                        s->velocities().setVelo(tick2, VeloEvent(VELO_FIX, endVelo));
                        }
                  break;
            }
      }

//---------------------------------------------------------
//   removeHairpin
//---------------------------------------------------------

void Score::removeHairpin(Hairpin* h)
      {
      Staff* st = h->staff();
      int tick  = h->segment()->tick();
      Segment* es = static_cast<Segment*>(h->endElement());
      if (!es)
            return;
      int tick2 = es->tick() - 1;

      switch(h->dynType()) {
            case DYNAMIC_STAFF:
                  st->velocities().remove(tick);
                  st->velocities().remove(tick2);
                  break;
            case DYNAMIC_PART:
                  foreach(Staff* s, *st->part()->staves()) {
                        s->velocities().remove(tick);
                        s->velocities().remove(tick2);
                        }
                  break;
            case DYNAMIC_SYSTEM:
                  foreach(Staff* s, _staves) {
                        s->velocities().remove(tick);
                        s->velocities().remove(tick2);
                        }
                  break;
            }
      }

//---------------------------------------------------------
//   updateVelo
//    calculate velocity for all notes
//---------------------------------------------------------

void Score::updateVelo()
      {
      //
      //    collect Dynamics & Ottava & Hairpins
      //
      if (!firstMeasure())
            return;

      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            Staff* st      = staff(staffIdx);
            VeloList& velo = st->velocities();
            velo.clear();
            velo.setVelo(0, 80);
            Part* prt      = st->part();
            int partStaves = prt->nstaves();
            int partStaff  = Score::staffIdx(prt);

            for (Segment* s = firstMeasure()->first(); s; s = s->next1()) {
                  int tick = s->tick();
                  foreach(const Element* e, s->annotations()) {
                        if (e->staffIdx() != staffIdx)
                              continue;
                        if (e->type() != DYNAMIC)
                              continue;
                        const Dynamic* d = static_cast<const Dynamic*>(e);
                        int v            = d->velocity();
                        if (v < 1)     //  illegal value
                              continue;
                        int dStaffIdx = d->staffIdx();
                        switch(d->dynType()) {
                              case DYNAMIC_STAFF:
                                    if (dStaffIdx == staffIdx)
                                          velo.setVelo(tick, v);
                                    break;
                              case DYNAMIC_PART:
                                    if (dStaffIdx >= partStaff && dStaffIdx < partStaff+partStaves) {
                                          for (int i = partStaff; i < partStaff+partStaves; ++i) {
                                                staff(i)->velocities().setVelo(tick, v);
//                                                printf("   setVelo %d %d\n", tick, v);
                                                }
                                          }
                                    break;
                              case DYNAMIC_SYSTEM:
                                    for (int i = 0; i < nstaves(); ++i)
                                          staff(i)->velocities().setVelo(tick, v);
                                    break;
                              }
                        }
                  foreach(Element* e, s->spannerFor()) {
                        if (e->staffIdx() != staffIdx)
                              continue;
                        if (e->type() == HAIRPIN) {
                              Hairpin* h = static_cast<Hairpin*>(e);
                              updateHairpin(h);
                              }
                        }
                  }
            }
      }
