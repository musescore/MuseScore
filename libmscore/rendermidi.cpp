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
#include "tie.h"
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
#include "synthesizer/event.h"
#include "segment.h"
#include "undo.h"
#include "utils.h"

namespace Ms {

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
      for (Segment* s = fm->first(Segment::Type::ChordRest); s; s = s->next1(Segment::Type::ChordRest)) {
            foreach(const Element* e, s->annotations()) {
                  if (e->type() == Element::Type::INSTRUMENT_CHANGE) {
                        Staff* staff = _staves[e->staffIdx()];
                        for (int voice = 0; voice < VOICES; ++voice)
                              staff->channelList(voice)->insert(s->tick(), 0);
                        continue;
                        }
                  if (e->type() != Element::Type::STAFF_TEXT)
                        continue;
                  const StaffText* st = static_cast<const StaffText*>(e);
                  for (int voice = 0; voice < VOICES; ++voice) {
                        QString an(st->channelName(voice));
                        if (an.isEmpty())
                              continue;
                        Staff* staff = _staves[st->staffIdx()];
                        int a = staff->part()->instr(s->tick())->channelIdx(an);
                        if (a != -1)
                              staff->channelList(voice)->insert(s->tick(), a);
                        }
                  }
            }

      for (Segment* s = fm->first(Segment::Type::ChordRest); s; s = s->next1(Segment::Type::ChordRest)) {
            foreach(Staff* st, _staves) {
                  int strack = st->idx() * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        if (!s->element(track))
                              continue;
                        Element* e = s->element(track);
                        if (e->type() != Element::Type::CHORD)
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
      if (!note->play())
            return;
      velo = note->customizeVelocity(velo);
      NPlayEvent ev(ME_NOTEON, channel, pitch, velo);
      ev.setTuning(note->tuning());
      ev.setNote(note);
      events->insert(std::pair<int, NPlayEvent>(onTime, ev));
      ev.setVelo(0);
      events->insert(std::pair<int, NPlayEvent>(offTime, ev));
      }

//---------------------------------------------------------
//   collectNote
//---------------------------------------------------------

static void collectNote(EventMap* events, int channel, const Note* note, int velo, int tickOffset)
      {
      if (!note->play() || note->hidden())      // do not play overlapping notes
            return;

      int pitch = note->ppitch();

      Chord* chord = note->chord();
      int ticks;
      int tieLen = 0;
      if (chord->isGrace()) {
            chord = static_cast<Chord*>(chord->parent());
            ticks = chord->actualTicks();
            tieLen = 0;
            }
      else {
            ticks = chord->actualTicks();
            // calculate additional length due to ties forward
            // taking NoteEvent length adjustments into account
            // but stopping at any note with multiple NoteEvents
            // and processing those notes recursively
            if (note->tieFor()) {
                  Note* n = note->tieFor()->endNote();
                  while (n) {
                        NoteEventList nel = n->playEvents();
                        if (nel.size() == 1) {
                              // add value of this note to main note
                              // if we wish to suppress first note of ornament,
                              // then do this regardless of number of NoteEvents
                              tieLen += (n->chord()->actualTicks() * (nel[0].len())) / 1000;
                              }
                        else {
                              // recurse
                              collectNote(events, channel, n, velo, tickOffset);
                              break;
                              }
                        if (n->tieFor())
                              n = n->tieFor()->endNote();
                        else
                              break;
                        }
                  }
            }

      int tick1 = chord->tick() + tickOffset;
      bool tieFor = note->tieFor();
      bool tieBack = note->tieBack();

      NoteEventList nel = note->playEvents();
      int nels = nel.size();
      for (int i = 0; i < nels; ++i) {
            const NoteEvent e = nel[i];
            // skip if note has a tie into it and only one NoteEvent
            // its length was already added to previous note
            // if we wish to suppress first note of ornament
            // then change "nels == 1" to "i == 0", and change "break" to "continue"
            if (tieBack && nels == 1)
                  break;
            int p = pitch + e.pitch();
            if (p < 0)
                  p = 0;
            else if (p > 127)
                  p = 127;
            int on  = tick1 + (ticks * e.ontime())/1000;
            int off = on + (ticks * e.len())/1000 - 1;
            if (tieFor && i == nels - 1)
                  off += tieLen;
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
      NPlayEvent event;
      event.setType(ME_CONTROLLER);
      event.setController(98);
      if (val)
            event.setValue(0x40 + 0x20  + i);
      else
            event.setValue(0x40 + 0x10  + i);

      event.setChannel(channel);
      events->insert(std::pair<int,NPlayEvent>(tick, event));

      event.setValue(k);
      events->insert(std::pair<int,NPlayEvent>(tick, event));
//      event.setValue(0x40 + i);
//      events->insert(std::pair<int,NPlayEvent>(tick, event));
      }

//---------------------------------------------------------
//   collectMeasureEvents
//---------------------------------------------------------

static void collectMeasureEvents(EventMap* events, Measure* m, Staff* staff, int tickOffset)
      {
      int firstStaffIdx = staff->idx();
      int nextStaffIdx  = firstStaffIdx + 1;

      Segment::Type st = Segment::Type::ChordRest;
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
                  if (cr == 0 || cr->type() != Element::Type::CHORD)
                        continue;

                  Chord* chord = static_cast<Chord*>(cr);
                  Staff* staff = chord->staff();
                  int velocity = staff->velocities().velo(seg->tick());
                  Instrument* instr = chord->staff()->part()->instr(tick);
                  int channel = instr->channel(chord->upNote()->subchannel()).channel;

                  foreach (Articulation* a, chord->articulations()) {
                        instr->updateVelocity(&velocity,channel, a->subtypeName());
                  }

                  for (Chord* c : chord->graceNotes()) {
                        for (const Note* note : c->notes())
                              collectNote(events, channel, note, velocity, tickOffset);
                        }
                  foreach (const Note* note, chord->notes())
                        collectNote(events, channel, note, velocity, tickOffset);
                  }
            }

      //
      // collect program changes and controller
      //
      for (Segment* s = m->first(Segment::Type::ChordRest); s; s = s->next(Segment::Type::ChordRest)) {
            // int tick = s->tick();
            foreach(Element* e, s->annotations()) {
                  if (e->type() != Element::Type::STAFF_TEXT
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
                              for (MidiCoreEvent event : nel->events) {
                                    event.setChannel(channel);
                                    NPlayEvent e(event);
                                    events->insert(std::pair<int, NPlayEvent>(tick, e));
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
      int tick  = h->tick();
      int velo  = st->velocities().velo(tick);
      int incr  = h->veloChange();
      int tick2 = h->tick2();

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

      switch (h->dynRange()) {
            case Dynamic::Range::STAFF:
                  st->velocities().setVelo(tick,  VeloEvent(VeloType::RAMP, velo));
                  st->velocities().setVelo(tick2, VeloEvent(VeloType::FIX, endVelo));
                  break;
            case Dynamic::Range::PART:
                  foreach(Staff* s, *st->part()->staves()) {
                        s->velocities().setVelo(tick,  VeloEvent(VeloType::RAMP, velo));
                        s->velocities().setVelo(tick2, VeloEvent(VeloType::FIX, endVelo));
                        }
                  break;
            case Dynamic::Range::SYSTEM:
                  foreach(Staff* s, _staves) {
                        s->velocities().setVelo(tick,  VeloEvent(VeloType::RAMP, velo));
                        s->velocities().setVelo(tick2, VeloEvent(VeloType::FIX, endVelo));
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
      int tick  = h->tick();
      int tick2 = h->tick2() - 1;

      switch(h->dynRange()) {
            case Dynamic::Range::STAFF:
                  st->velocities().remove(tick);
                  st->velocities().remove(tick2);
                  break;
            case Dynamic::Range::PART:
                  foreach(Staff* s, *st->part()->staves()) {
                        s->velocities().remove(tick);
                        s->velocities().remove(tick2);
                        }
                  break;
            case Dynamic::Range::SYSTEM:
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
      //    collect Dynamics
      //
      if (!firstMeasure())
            return;

      for (Staff* st : _staves) {
            VeloList& velo = st->velocities();
            velo.clear();
            velo.setVelo(0, 80);
            }
      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            Staff* st      = staff(staffIdx);
            VeloList& velo = st->velocities();
            Part* prt      = st->part();
            int partStaves = prt->nstaves();
            int partStaff  = Score::staffIdx(prt);

            for (Segment* s = firstMeasure()->first(); s; s = s->next1()) {
                  int tick = s->tick();
                  foreach (const Element* e, s->annotations()) {
                        if (e->staffIdx() != staffIdx)
                              continue;
                        if (e->type() != Element::Type::DYNAMIC)
                              continue;
                        const Dynamic* d = static_cast<const Dynamic*>(e);
                        int v            = d->velocity();
                        if (v < 1)     //  illegal value
                              continue;
                        int dStaffIdx = d->staffIdx();
                        switch(d->dynRange()) {
                              case Dynamic::Range::STAFF:
                                    if (dStaffIdx == staffIdx)
                                          velo.setVelo(tick, v);
                                    break;
                              case Dynamic::Range::PART:
                                    if (dStaffIdx >= partStaff && dStaffIdx < partStaff+partStaves) {
                                          for (int i = partStaff; i < partStaff+partStaves; ++i)
                                                staff(i)->velocities().setVelo(tick, v);
                                          }
                                    break;
                              case Dynamic::Range::SYSTEM:
                                    for (int i = 0; i < nstaves(); ++i)
                                          staff(i)->velocities().setVelo(tick, v);
                                    break;
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

//--------------------------------------------------------
//   swingAdjustParams
//--------------------------------------------------------

void Score::swingAdjustParams(Chord* chord, int& gateTime, int& ontime, int swingUnit, int swingRatio)
      {
      int tick = chord->rtick();
      // adjust for anacrusis
      Measure* cm = chord->measure();
      MeasureBase* pm = cm->prev();
      Element::Type pt = pm ? pm->type() : Element::Type::INVALID;
      if (!pm || pm->lineBreak() || pm->pageBreak() || pm->sectionBreak()
            || pt == Element::Type::VBOX || pt == Element::Type::HBOX
            || pt == Element::Type::FBOX || pt == Element::Type::TBOX) {
            int offset = (cm->timesig() - cm->len()).ticks();
            if (offset > 0)
                  tick += offset;
           }
      int swingBeat = swingUnit * 2;
      qreal ticksDuration = (qreal)chord->actualTicks();
      qreal swingTickAdjust = ((qreal)swingBeat) * (((qreal)(swingRatio-50))/100.0);
      qreal swingActualAdjust = (swingTickAdjust/ticksDuration) * 1000.0;
      ChordRest *ncr = nextChordRest(chord);

      //Check the position of the chord to apply changes accordingly
      if (tick % swingBeat == swingUnit) {
            if (!isSubdivided(chord,swingUnit)) {
                  ontime = ontime + swingActualAdjust;
                  }
            }
            int endTick = tick + ticksDuration;
            if ((endTick % swingBeat == swingUnit) && (!isSubdivided(ncr,swingUnit))) {
                  gateTime = gateTime + (swingActualAdjust/10);
                  }
      }

//---------------------------------------------------------
//   isSubdivided
//   Check for subdivided beat
//---------------------------------------------------------

bool Score::isSubdivided(ChordRest* chord, int swingUnit)
      {
      if (!chord)
            return false;
      ChordRest* prev = prevChordRest(chord);
      if (chord->actualTicks() < swingUnit || (prev && prev->actualTicks() < swingUnit))
            return true;
      else
            return false;
      }

//---------------------------------------------------------
//   renderChord
//    ontime in 1/1000 of duration
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
            //int n = 1 << tremolo->lines();
            //int l = 1000 / n;
            if (chord->tremoloChordType() == TremoloChordType::TremoloFirstNote) {
                  int t = MScore::division / (1 << (tremolo->lines() + chord->durationType().hooks()));
                  Segment::Type st = Segment::Type::ChordRest;
                  Segment* seg2 = seg->next(st);
                  int track = chord->track();
                  while (seg2 && !seg2->element(track))
                        seg2 = seg2->next(st);
                  Chord* c2 = seg2 ? static_cast<Chord*>(seg2->element(track)) : 0;
                  if (c2 && c2->type() == Element::Type::CHORD) {
                        int tnotes = qMin(notes, c2->notes().size());
                        int tticks = chord->actualTicks() * 2; // use twice the size
                        int n = tticks / t;
                        n /= 2;
                        int l = 2000 * t / tticks;
                        for (int k = 0; k < tnotes; ++k) {
                              NoteEventList* events = &ell[k];
                              events->clear();
                              int p1 = chord->notes()[k]->pitch();
                              int p2 = c2->notes()[k]->pitch();
                              int dpitch = p2 - p1;
                              for (int i = 0; i < n; ++i) {
                                    events->append(NoteEvent(0, l * i * 2, l));
                                    events->append(NoteEvent(dpitch, l * i * 2 + l, l));
                                    }
                              }
                        }
                  else
                        qDebug("Chord::renderTremolo: cannot find 2. chord");
                  }
            else if (chord->tremoloChordType() == TremoloChordType::TremoloSecondNote) {
                  for (int k = 0; k < notes; ++k) {
                        NoteEventList* events = &(ell)[k];
                        events->clear();
                        }
                  }
            else if (chord->tremoloChordType() == TremoloChordType::TremoloSingle) {
                  int t = MScore::division / (1 << (tremolo->lines() + chord->durationType().hooks()));
                  int n = chord->durationTicks() / t;
                  int l = 1000 / n;
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
            int l = 64;
            while (l * notes > chord->upNote()->playTicks())
                  l = 2*l / 3 ;
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
            int j = 0;
            for (int i = start; i != end; i += step) {
                  NoteEventList* events = &(ell)[i];
                  events->clear();
                  int ot = (l * j * 1000) / chord->upNote()->playTicks();
                  events->append(NoteEvent(0, ot, 1000 - ot));
                  j++;
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
                              case ArticulationType::Mordent: {
                                    //
                                    // create default playback for Mordent
                                    //
                                    events->clear();
                                    events->append(NoteEvent(0, 0, 125));
                                    Key key     = chord->staff()->key(chord->segment()->tick());
                                    int pitch   = chord->notes()[k]->pitch();
                                    int pitchDown = diatonicUpDown(key, pitch, -1);
                                    events->append(NoteEvent(pitchDown - pitch, 125, 125));
                                    events->append(NoteEvent(0, 250, 750));
                                    }
                                    break;
                              case ArticulationType::Prall:
                                    //
                                    // create default playback events for PrallSym
                                    //
                                    {
                                    events->clear();
                                    events->append(NoteEvent(0, 0, 125));
                                    Key key       = chord->staff()->key(chord->segment()->tick());
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
            if (nn == 0 && chord->tremoloChordType() != TremoloChordType::TremoloSecondNote) {
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

//---------------------------------------------------------
//   createPlayEvents
//    create default play events
//---------------------------------------------------------

void Score::createPlayEvents(Chord* chord)
      {
      int gateTime = 100;

      int tick = chord->tick();
      Slur* slur = 0;
      for (auto sp : _spanner.map()) {
            if (sp.second->type() != Element::Type::SLUR || sp.second->staffIdx() != chord->staffIdx())
                  continue;
            Slur* s = static_cast<Slur*>(sp.second);
            if (tick >= s->tick() && tick < s->tick2()) {
                  slur = s;
                  break;
                  }
            }
      // gateTime is 100% for slured notes
      if (!slur) {
            Instrument* instr = chord->staff()->part()->instr(tick);
            instr->updateGateTime(&gateTime, 0, "");
            }

      int n = chord->graceNotes().size();
      int ontime = 0;
      if (n) {
            //
            //  render grace notes:
            //  simplified implementation:
            //  - grace notes start on the beat of the main note
            //  - duration: acciacatura: 0.128 * duration of main note
            //              appoggiatura: 0.5  * duration of main note
            //  - the duration is divided by the number of grace notes
            //  - the grace note duration as notated does not matter
            //
            Chord* graceChord = chord->graceNotes()[0];
            ontime      = (graceChord->noteType() ==  NoteType::ACCIACCATURA) ? 128 : 500;
            int graceDuration = ontime / n;

            int on = 0;
            for (int i = 0; i < n; ++i) {
                  QList<NoteEventList> el;
                  Chord* gc = chord->graceNotes().at(i);
                  int nn = gc->notes().size();
                  for (int ii = 0; ii < nn; ++ii) {
                        NoteEventList nel;
                        nel.append(NoteEvent(0, on, graceDuration));
                        el.append(nel);
                        }

                  if (gc->playEventType() == PlayEventType::InvalidUser)
                        gc->score()->undo(new ChangeEventList(gc, el));
                  else if (gc->playEventType() == PlayEventType::Auto) {
                        for (int ii = 0; ii < nn; ++ii)
                              gc->notes()[ii]->setPlayEvents(el[ii]);
                        }
                  on += graceDuration;
                  }
            }

      // Check if swing needs to be applied
      int swingUnit = styleI(StyleIdx::swingUnit);
      if (swingUnit) {
            int swingRatio = styleI(StyleIdx::swingRatio);
            swingAdjustParams(chord, gateTime, ontime, swingUnit, swingRatio);
      }
      //
      //    render normal (and articulated) chords
      //
      QList<NoteEventList> el = renderChord(chord, gateTime, ontime);
      if (chord->playEventType() == PlayEventType::InvalidUser) {
            chord->score()->undo(new ChangeEventList(chord, el));
            }
      else if (chord->playEventType() == PlayEventType::Auto) {
            int n = chord->notes().size();
            for (int i = 0; i < n; ++i)
                  chord->notes()[i]->setPlayEvents(el[i]);
            }
      // dont change event list if type is PlayEventType::User
      }

void Score::createPlayEvents()
      {
      int etrack = nstaves() * VOICES;
      for (int track = 0; track < etrack; ++track) {
            for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
                  // skip linked staves, except primary
                  if (!m->score()->staff(track / VOICES)->primaryStaff())
                        continue;
                  const Segment::Type st = Segment::Type::ChordRest;
                  for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
                        Chord* chord = static_cast<Chord*>(seg->element(track));
                        if (chord == 0 || chord->type() != Element::Type::CHORD)
                              continue;
                        createPlayEvents(chord);
                        }
                  }
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

            //
            //    add metronome tick events
            //
            for (Measure* m = tick2measure(startTick); m; m = m->nextMeasure()) {
                  Fraction ts = sigmap()->timesig(m->tick()).timesig();

                  int tw = MScore::division * 4 / ts.denominator();
                  for (int i = 0; i < ts.numerator(); i++) {
                        int tick = m->tick() + i * tw + tickOffset;
                        NPlayEvent event;
                        event.setType(i == 0 ? ME_TICK1 : ME_TICK2);
                        events->insert(std::pair<int,NPlayEvent>(tick, event));
                        }
                  if (m->tick() + m->ticks() >= endTick)
                        break;
                  }
            //
            // create sustain pedal events
            //
            int utick1 = rs->utick;
            int utick2 = utick1 + rs->len;

            for (std::pair<int,Spanner*> sp : _spanner.map()) {
                  Spanner* s = sp.second;
                  if (s->type() != Element::Type::PEDAL)
                        continue;

                  int idx = s->staff()->channel(s->tick(), 0);
                  int channel = s->staff()->part()->instr(s->tick())->channel(idx).channel;
                  if (s->tick() >= utick1 && s->tick() < utick2) {
                        NPlayEvent event(ME_CONTROLLER, channel, CTRL_SUSTAIN, 127);
                        events->insert(std::pair<int,NPlayEvent>(s->tick() + tickOffset, event));
                        }

                  if (s->tick2() >= utick1 && s->tick2() < utick2) {
                        NPlayEvent event(ME_CONTROLLER, channel, CTRL_SUSTAIN, 0);
                        events->insert(std::pair<int,NPlayEvent>(s->tick2() + tickOffset, event));
                        }
                  }
            }
      }

}

