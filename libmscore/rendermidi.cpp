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
#include "undo.h"
#include "utils.h"
#include "rendermidi.h"

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
      for (Segment* s = fm->first(Segment::SegChordRest); s; s = s->next1(Segment::SegChordRest)) {
            foreach(const Element* e, s->annotations()) {
                  if (e->type() != Element::STAFF_TEXT)
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

      for (Segment* s = fm->first(Segment::SegChordRestGrace); s; s = s->next1(Segment::SegChordRestGrace)) {
            foreach(Staff* st, _staves) {
                  int strack = st->idx() * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        if (!s->element(track))
                              continue;
                        Element* e = s->element(track);
                        if (e->type() != Element::CHORD)
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
      events->insert(std::pair<int, Event>(onTime, ev));
      ev.setVelo(0);
      events->insert(std::pair<int, Event>(offTime, ev));
      }

//---------------------------------------------------------
//   collectNote
//---------------------------------------------------------

static void collectNote(EventMap* events, int channel, const Note* note, int velo, int tickOffset)
      {
      if (note->hidden() || note->tieBack())       // do not play overlapping notes
            return;

      int pitch = note->ppitch();
      int tick1 = note->chord()->tick() + tickOffset;

      int ticks = note->playTicks();
      foreach(const NoteEvent& e, note->playEvents()) {
            int p = pitch + e.pitch();
            if (p < 0)
                  p = 0;
            else if (p > 127)
                  p = 127;
            int on  = tick1 + (ticks * e.ontime())/1000;
            int off = on + (ticks * e.len())/1000 - 1;
            playNote(events, note, channel, p, velo, on, off);
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
//   aeolusSetStop
//---------------------------------------------------------

static void aeolusSetStop(int tick, int channel, int i, int k, bool val, EventMap* events)
      {
      Event event(ME_CONTROLLER);
      event.setController(98);
      if (val)
            event.setValue(0x40 + 0x20  + i);
      else
            event.setValue(0x40 + 0x10  + i);

      event.setOntime(tick);
      event.setChannel(channel);
      events->insert(std::pair<int,Event>(tick, event));

      event.setValue(k);
      events->insert(std::pair<int,Event>(tick, event));
//      event.setValue(0x40 + i);
//      events->insert(std::pair<int,Event>(tick, event));
      }

//---------------------------------------------------------
//   collectMeasureEvents
//---------------------------------------------------------

static void collectMeasureEvents(EventMap* events, Measure* m, Staff* staff, int tickOffset)
      {
      int firstStaffIdx = staff->idx();
      int nextStaffIdx  = firstStaffIdx + 1;

      Segment::SegmentTypes st = Segment::SegChordRestGrace;
      int strack = firstStaffIdx * VOICES;
      int etrack = nextStaffIdx * VOICES;

      for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
            int tick = seg->tick();
            for (int track = strack; track < etrack; ++track) {
                  // skip linked staves, except primary
                  if (!m->score()->staff(track / VOICES)->primaryStaff()) {
                        track += VOICES-1;
                        continue;
                        }
                  Element* cr = seg->element(track);
                  if (cr == 0 || cr->type() != Element::CHORD)
                        continue;

                  Chord* chord = static_cast<Chord*>(cr);
                  Staff* staff = chord->staff();
                  int velocity = staff->velocities().velo(seg->tick());
                  Instrument* instr = chord->staff()->part()->instr(tick);

                  int channel = instr->channel(chord->upNote()->subchannel()).channel;
                  foreach(const Note* note, chord->notes())
                        collectNote(events, channel, note, velocity, tickOffset);
                  }
            }

      //
      // collect program changes and controller
      //
      for (Segment* s = m->first(Segment::SegChordRest); s; s = s->next(Segment::SegChordRest)) {
            // int tick = s->tick();
            foreach(Element* e, s->annotations()) {
                  if (e->type() != Element::STAFF_TEXT
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
                                    events->insert(std::pair<int, Event>(tick, event));
                                    }
                              }
                        }
                  if (st->setAeolusStops()) {
                        Staff* staff = st->staff();
                        int voice   = 0;
                        int channel = staff->channel(tick, voice);

                        for (int i = 0; i < 4; ++i) {
                              static int num[4] = { 12, 13, 16, 16 };
                              for (int k = 0; k < num[i]; ++k)
                                    aeolusSetStop(tick, channel, i, k, st->getAeolusStop(i, k), events);
                              }
                        }
                  }
            for(Spanner* e = s->spannerFor(); e; e = e->next()) {
                  if (e->staffIdx() < firstStaffIdx || e->staffIdx() >= nextStaffIdx)
                        continue;
                  if (e->type() == Element::PEDAL) {
                        Segment* s1 = static_cast<Segment*>(e->startElement());
                        Segment* s2 = static_cast<Segment*>(e->endElement());
                        if (s2 == 0) {
                              qDebug("CollectMeasureEvents: spanner %s: no end element at measure %d, staff %d",
                                 e->name(), s1->measure()->no(), e->staffIdx());
                              continue;
                              }
                        Staff* staff = e->staff();

                        int channel = staff->channel(s1->tick(), 0);

                        Event event(ME_CONTROLLER);
                        event.setChannel(channel);
                        event.setController(CTRL_SUSTAIN);

                        event.setValue(127);
                        events->insert(std::pair<int,Event>(s1->tick() + tickOffset, event));

                        event.setValue(0);
                        events->insert(std::pair<int,Event>(s2->tick() + tickOffset - 1, event));
                        }
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

      switch(h->dynRange()) {
            case Element::DYNAMIC_STAFF:
                  st->velocities().setVelo(tick,  VeloEvent(VELO_RAMP, velo));
                  st->velocities().setVelo(tick2, VeloEvent(VELO_FIX, endVelo));
                  break;
            case Element::DYNAMIC_PART:
                  foreach(Staff* s, *st->part()->staves()) {
                        s->velocities().setVelo(tick,  VeloEvent(VELO_RAMP, velo));
                        s->velocities().setVelo(tick2, VeloEvent(VELO_FIX, endVelo));
                        }
                  break;
            case Element::DYNAMIC_SYSTEM:
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

      switch(h->dynRange()) {
            case Element::DYNAMIC_STAFF:
                  st->velocities().remove(tick);
                  st->velocities().remove(tick2);
                  break;
            case Element::DYNAMIC_PART:
                  foreach(Staff* s, *st->part()->staves()) {
                        s->velocities().remove(tick);
                        s->velocities().remove(tick2);
                        }
                  break;
            case Element::DYNAMIC_SYSTEM:
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
                        if (e->type() != Element::DYNAMIC)
                              continue;
                        const Dynamic* d = static_cast<const Dynamic*>(e);
                        int v            = d->velocity();
                        if (v < 1)     //  illegal value
                              continue;
                        int dStaffIdx = d->staffIdx();
                        switch(d->dynRange()) {
                              case Element::DYNAMIC_STAFF:
                                    if (dStaffIdx == staffIdx)
                                          velo.setVelo(tick, v);
                                    break;
                              case Element::DYNAMIC_PART:
                                    if (dStaffIdx >= partStaff && dStaffIdx < partStaff+partStaves) {
                                          for (int i = partStaff; i < partStaff+partStaves; ++i)
                                                staff(i)->velocities().setVelo(tick, v);
                                          }
                                    break;
                              case Element::DYNAMIC_SYSTEM:
                                    for (int i = 0; i < nstaves(); ++i)
                                          staff(i)->velocities().setVelo(tick, v);
                                    break;
                              }
                        }
                  for (Spanner* e = s->spannerFor(); e; e = e->next()) {
                        if (e->staffIdx() != staffIdx)
                              continue;
                        if (e->type() == Element::HAIRPIN) {
                              Hairpin* h = static_cast<Hairpin*>(e);
                              updateHairpin(h);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   renderStaff
//---------------------------------------------------------

void Score::renderStaff(EventMap* events, Staff* staff)
      {
      Measure* lastMeasure = 0;
      foreach (const RepeatSegment* rs, *repeatList()) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            int tickOffset = rs->utick - rs->tick;
            for (Measure* m = tick2measure(startTick); m; m = m->nextMeasure()) {
                  if (lastMeasure && m->isRepeatMeasure(staff->part())) {
                        int offset = m->tick() - lastMeasure->tick();
                        collectMeasureEvents(events, lastMeasure, staff, tickOffset + offset);
                        }
                  else {
                        lastMeasure = m;
                        collectMeasureEvents(events, lastMeasure, staff, tickOffset);
                        }
                  if (m->tick() + m->ticks() >= endTick)
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   gateTime
//---------------------------------------------------------

static int gateTime(Chord* chord)
      {
      QList<Slur*> slurs;
      int track = chord->track();
      int gateTime = 100;
      Score* score = chord->score();
      for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
            const Segment::SegmentTypes st = Segment::SegChordRestGrace;
            for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
                  ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
                  if (cr == 0)
                        continue;
                  for (Spanner* spanner = cr->spannerFor(); spanner; spanner = spanner->next()) {
                        if (spanner->type() == Element::SLUR)
                              slurs.append(static_cast<Slur*>(spanner));
                        }
                  for (Spanner* spanner = cr->spannerBack(); spanner; spanner = spanner->next()) {
                        if (spanner->type() == Element::SLUR)
                              slurs.removeOne(static_cast<Slur*>(spanner));
                        }
                  if (cr == chord) {
                        if (slurs.isEmpty()) {
                              Instrument* instr = cr->staff()->part()->instr(seg->tick());
                              int channel = 0;
                              instr->updateGateTime(&gateTime, channel, "");
                              }
                        break;
                        }
                  }
            }
      return gateTime;
      }

//---------------------------------------------------------
//   renderChord
//---------------------------------------------------------

static QList<NoteEventList> renderChord(Chord* chord, int gateTime, int ontime)
      {
      QList<NoteEventList> ell;
      if (chord->notes().isEmpty())
            return ell;
      Segment* seg = chord->segment();

      int notes = chord->notes().size();
      for (int i = 0; i < notes; ++i)
            ell.append(NoteEventList());

      bool gateEvents = true;
      //
      //    process tremolo
      //
      Tremolo* tremolo = chord->tremolo();
      if (tremolo) {
            int n = 1 << tremolo->lines();
            int l = 1000 / n;
            if (chord->tremoloChordType() == TremoloFirstNote) {
                  Segment::SegmentTypes st = Segment::SegChordRestGrace;
                  Segment* seg2 = seg->next(st);
                  int track = chord->track();
                  while (seg2 && !seg2->element(track))
                        seg2 = seg2->next(st);
                  Chord* c2 = static_cast<Chord*>(seg2->element(track));
                  if (c2 && c2->type() == Element::CHORD) {
                        int tnotes = qMin(notes, c2->notes().size());
                        n /= 2;
                        for (int k = 0; k < tnotes; ++k) {
                              NoteEventList* events = &ell[k];
                              events->clear();
                              int p1 = chord->notes()[k]->pitch();
                              int p2 = c2->notes()[k]->pitch();
                              for (int i = 0; i < n; ++i) {
                                    events->append(NoteEvent(p1, l * i * 2, l));
                                    events->append(NoteEvent(p2, l * i * 2 + l, l));
                                    }
                              }
                        }
                  else
                        qDebug("Chord::renderTremolo: cannot find 2. chord\n");
                  }
            else if (chord->tremoloChordType() == TremoloSecondNote) {
                  }
            else if (chord->tremoloChordType() == TremoloSingle) {
                  for (int k = 0; k < notes; ++k) {
                        NoteEventList* events = &(ell)[k];
                        events->clear();
                        for (int i = 0; i < n; ++i)
                              events->append(NoteEvent(0, l * i, l));
                        }
                  }
            }
      else if (chord->arpeggio()) {
            gateEvents = false;     // dont apply gateTime to arpeggio events
            int l = 1000 / notes;

            int start, end, step;
            bool up = chord->arpeggio()->arpeggioType() != ArpeggioType::DOWN;
            if (up) {
                  start = 0;
                  end   = notes;
                  step  = 1;
                  }
            else {
                  start = notes - 1;
                  end   = -1;
                  step  = -1;
                  }
            for (int i = start; i != end; i += step) {
                  NoteEventList* events = &(ell)[i];
                  events->clear();
                  events->append(NoteEvent(0, l * i, 1000 - l * i));
                  }
            }

      if (!chord->articulations().isEmpty() && !chord->arpeggio()) {
            Instrument* instr = chord->staff()->part()->instr(seg->tick());
            int channel  = 0;  // note->subchannel();

//qDebug("Chord");
            foreach (Articulation* a, chord->articulations()) {
                  ArticulationType type = a->articulationType();
                  for (int k = 0; k < notes; ++k) {
                        NoteEventList* events = &ell[k];

                        switch (type) {
                              case Articulation_Mordent: {
                                    //
                                    // create default playback for Mordent
                                    //
                                    events->clear();
                                    events->append(NoteEvent(0, 0, 125));
                                    int key     = chord->staff()->key(chord->segment()->tick()).accidentalType();
                                    int pitch   = chord->notes()[k]->pitch();
                                    int pitchDown = diatonicUpDown(key, pitch, -1);
                                    events->append(NoteEvent(pitchDown - pitch, 125, 125));
                                    events->append(NoteEvent(0, 250, 750));
                                    }
                                    break;
                              case Articulation_Prall:
                                    //
                                    // create default playback events for PrallSym
                                    //
                                    {
                                    events->clear();
                                    events->append(NoteEvent(0, 0, 125));
                                    int key       = chord->staff()->key(chord->segment()->tick()).accidentalType();
                                    int pitch     = chord->notes()[k]->pitch();
                                    int pitchUp = diatonicUpDown(key, pitch, 1);
                                    events->append(NoteEvent(pitchUp - pitch, 125, 125));
                                    events->append(NoteEvent(0, 250, 750));
                                    }
                                    break;
                              default:
//qDebug("   %s", qPrintable(a->subtypeName()));
                                    instr->updateGateTime(&gateTime, channel, a->subtypeName());
                                    break;
                              }
                        }
                  }
            }

      //
      //    apply gateTime
      //
      if (!gateEvents)
            return ell;
      for (int i = 0; i < notes; ++i) {
            NoteEventList* el = &ell[i];
            int nn = el->size();
            if (nn == 0) {
                  el->append(NoteEvent(0, ontime, 1000-ontime));
                  ++nn;
                  }
            for (int i = 0; i < nn; ++i) {
                  NoteEvent* e = &(*el)[i];
                  e->setLen(e->len() * gateTime / 100);
                  }
            }
      return ell;
      }

QList<NoteEventList> renderChord(Chord* chord)
      {
      return renderChord(chord, gateTime(chord), 0);
      }

//---------------------------------------------------------
//   createPlayEvents
//    create default play events
//---------------------------------------------------------

static void createPlayEvents(Chord* chord, int gateTime, const QList<Chord*>& graceNotes)
      {
      int n = graceNotes.size();
      int ontime = 0;
      if (n) {
            //
            //    render chords with grace notes
            //
            ontime = (graceNotes[0]->noteType() ==  NOTE_ACCIACCATURA) ? 128 : 500;
            qreal scale = (qreal)graceNotes[0]->duration().denominator() / (qreal)chord->duration().denominator();
            int graceDuration = (ontime * scale)/ n;

            int on = 0;
            for (int i = 0; i < n; ++i) {
                  QList<NoteEventList> el;
                  Chord* gc = graceNotes[i];
                  int nn = gc->notes().size();
                  for (int ii = 0; ii < nn; ++ii) {
                        NoteEventList nel;
                        nel.append(NoteEvent(0, on, graceDuration));
                        el.append(nel);
                        }

                  if (gc->userPlayEvents())
                        chord->score()->undo(new ChangeEventList(chord, el, false));
                  else {
                        for (int ii = 0; ii < nn; ++ii)
                              gc->notes()[ii]->setPlayEvents(el[ii]);
                        }
                  on += graceDuration;
                  }
            }
      //
      //    render normal (and articulated) chords
      //
      QList<NoteEventList> el = renderChord(chord, gateTime, ontime);
      if (chord->userPlayEvents())
            chord->score()->undo(new ChangeEventList(chord, el, false));
      else {
            int n = chord->notes().size();
            for (int i = 0; i < n; ++i)
                  chord->notes()[i]->setPlayEvents(el[i]);
            }
      }

void createPlayEvents(Chord* chord)
      {
      QList<Chord*> gl;       // list of grace chords
      Segment* s = chord->segment();
      while (s->prev()) {
            s = s->prev();
            if (s->segmentType() != Segment::SegGrace)
                  break;
            Element* cr = s->element(chord->track());
            if (cr && cr->type() == Element::CHORD)
                  gl.prepend(static_cast<Chord*>(cr));
            }
      createPlayEvents(chord, gateTime(chord), gl);
      }

static void createPlayEvents(Measure* m, int track, QList<Slur*>* slurs)
      {
      // skip linked staves, except primary
      if (!m->score()->staff(track / VOICES)->primaryStaff())
            return;
      QList<Chord*> graceNotes;
      const Segment::SegmentTypes st = Segment::SegChordRestGrace;
      for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
            ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
            if (cr == 0)
                  continue;
            for (Spanner* spanner = cr->spannerFor(); spanner; spanner = spanner->next()) {
                  if (spanner->type() == Element::SLUR)
                        slurs->append(static_cast<Slur*>(spanner));
                  }
            for (Spanner* spanner = cr->spannerBack(); spanner; spanner = spanner->next()) {
                  if (spanner->type() == Element::SLUR)
                        slurs->removeOne(static_cast<Slur*>(spanner));
                  }
            if (cr->type() != Element::CHORD)
                  continue;
            Chord* chord = static_cast<Chord*>(cr);
            if (chord->noteType() != NOTE_NORMAL) {
                  graceNotes.append(chord);
                  continue;
                  }
            int gateTime = 100;
            if (slurs->isEmpty()) {
                  Instrument* instr = cr->staff()->part()->instr(seg->tick());
                  int channel = 0;
                  instr->updateGateTime(&gateTime, channel, "");
                  }
            createPlayEvents(chord, gateTime, graceNotes);
            graceNotes.clear();
            }
      }

void Score::createPlayEvents()
      {
      QList<Slur*> slurs;
      int etrack = nstaves() * VOICES;
      for (int track = 0; track < etrack; ++track) {
            for (Measure* m = firstMeasure(); m; m = m->nextMeasure())
                  ::createPlayEvents(m, track, &slurs);
            }
      }

//---------------------------------------------------------
//   renderMidi
//    export score to event list
//---------------------------------------------------------

void Score::renderMidi(EventMap* events)
      {
      createPlayEvents();

      updateRepeatList(MScore::playRepeats);
      _foundPlayPosAfterRepeats = false;
      updateChannel();
      updateVelo();

      foreach (Staff* part, _staves)
            renderStaff(events, part);

      // add metronome ticks
      foreach (const RepeatSegment* rs, *repeatList()) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            int tickOffset = rs->utick - rs->tick;
            for (Measure* m = tick2measure(startTick); m; m = m->nextMeasure()) {
                  Fraction ts = sigmap()->timesig(m->tick()).timesig();

                  int tw = MScore::division * 4 / ts.denominator();
                  for (int i = 0; i < ts.numerator(); i++) {
                        int tick = m->tick() + i * tw + tickOffset;
                        Event event;
                        event.setType(i == 0 ? ME_TICK1 : ME_TICK2);
                        events->insert(std::pair<int,Event>(tick, event));
                        }
                  if (m->tick() + m->ticks() >= endTick)
                        break;
                  }
            }
      }

