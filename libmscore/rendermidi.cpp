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

#include <set>
#include <deque>

#include "score.h"
#include "volta.h"
#include "note.h"
#include "instrument.h"
#include "part.h"
#include "chord.h"
#include "style.h"
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
//   updateSwing
//---------------------------------------------------------

void Score::updateSwing()
      {
      foreach (Staff* s, _staves) {
            s->swingList()->clear();
            }
      Measure* fm = firstMeasure();
      if (!fm)
            return;
      for (Segment* s = fm->first(Segment::Type::ChordRest); s; s = s->next1(Segment::Type::ChordRest)) {
            foreach (const Element* e, s->annotations()) {
                  if (e->type() != Element::Type::STAFF_TEXT)
                        continue;
                  const StaffText* st = static_cast<const StaffText*>(e);
                  if (st->xmlText().isEmpty())
                        continue;
                  Staff* staff = st->staff();
                  if (!st->swing())
                        continue;
                  SwingParameters sp;
                  sp.swingRatio = st->swingParameters()->swingRatio;
                  sp.swingUnit = st->swingParameters()->swingUnit;
                  if (st->systemFlag()) {
                        foreach (Staff* sta, _staves) {
                              sta->swingList()->insert(s->tick(),sp);
                              }
                        }
                  else
                        staff->swingList()->insert(s->tick(),sp);
                  }
            }
      }

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
                        int a = staff->part()->instrument(s->tick())->channelIdx(an);
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
                        if (n->tieFor() && n != n->tieFor()->endNote())
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
                  Instrument* instr = chord->part()->instrument(tick);
                  int channel = instr->channel(chord->upNote()->subchannel())->channel;

                  foreach (Articulation* a, chord->articulations()) {
                        instr->updateVelocity(&velocity,channel, a->subtypeName());
                  }

                  for (Chord* c : chord->graceNotesBefore()) {
                        for (const Note* note : c->notes())
                              collectNote(events, channel, note, velocity, tickOffset);
                        }

                  foreach (const Note* note, chord->notes())
                        collectNote(events, channel, note, velocity, tickOffset);

#if 0
                  // TODO: add support for grace notes after - see createPlayEvents()
                  QList<Chord*> gna;
                  chord->getGraceNotesAfter(&gna);
                  for (Chord* c : gna) {
                        for (const Note* note : c->notes())
                              collectNote(events, channel, note, velocity, tickOffset);
                        }
#endif

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

                  Instrument* instr = e->part()->instrument(tick);
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
      setPlaylistDirty();
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
      if (h->hairpinType() == Hairpin::Type::CRESCENDO)
            {
            if (incr == 0 && velo < st->velocities().nextVelo(tick2-1))
                  endVelo = st->velocities().nextVelo(tick2-1);
            else
                  endVelo += incr;
            }
      else
            {
            if (incr == 0 && velo > st->velocities().nextVelo(tick2-1))
                  endVelo = st->velocities().nextVelo(tick2-1);
            else
                  endVelo -= incr;
            }

      if (endVelo > 127)
            endVelo = 127;
      else if (endVelo < 1)
            endVelo = 1;

      switch (h->dynRange()) {
            case Dynamic::Range::STAFF:
                  st->velocities().setVelo(tick,  VeloEvent(VeloType::RAMP, velo));
                  st->velocities().setVelo(tick2-1, VeloEvent(VeloType::FIX, endVelo));
                  break;
            case Dynamic::Range::PART:
                  foreach(Staff* s, *st->part()->staves()) {
                        s->velocities().setVelo(tick,  VeloEvent(VeloType::RAMP, velo));
                        s->velocities().setVelo(tick2-1, VeloEvent(VeloType::FIX, endVelo));
                        }
                  break;
            case Dynamic::Range::SYSTEM:
                  foreach(Staff* s, _staves) {
                        s->velocities().setVelo(tick,  VeloEvent(VeloType::RAMP, velo));
                        s->velocities().setVelo(tick2-1, VeloEvent(VeloType::FIX, endVelo));
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
            for (const auto& sp : _spanner.map()) {
                  Spanner* s = sp.second;
                  if (s->type() != Element::Type::HAIRPIN || sp.second->staffIdx() != staffIdx)
                        continue;
                  Hairpin* h = static_cast<Hairpin*>(s);
                  updateHairpin(h);
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
//   renderSpanners
//---------------------------------------------------------

void Score::renderSpanners(EventMap* events, int staffIdx)
      {
      foreach (const RepeatSegment* rs, *repeatList()) {
            int tickOffset = rs->utick - rs->tick;
            int utick1 = rs->utick;
            int tick1 = repeatList()->utick2tick(utick1);
            int tick2 = tick1 + rs->len;
            std::map<int, std::vector<std::pair<int, bool>>> channelPedalEvents = std::map<int, std::vector<std::pair<int, bool>>>();
            for (const auto& sp : _spanner.map()) {
                  Spanner* s = sp.second;
                  if (s->type() != Element::Type::PEDAL || (staffIdx != -1 && s->staffIdx() != staffIdx))
                        continue;

                  int idx = s->staff()->channel(s->tick(), 0);
                  int channel = s->part()->instrument(s->tick())->channel(idx)->channel;
                  channelPedalEvents.insert({channel, std::vector<std::pair<int, bool>>()});
                  std::vector<std::pair<int, bool>> pedalEventList = channelPedalEvents.at(channel);
                  std::pair<int, bool> lastEvent;

                  if (!pedalEventList.empty())
                        lastEvent = pedalEventList.back();
                  else
                        lastEvent = std::pair<int, bool>(0, true);

                  if (s->tick() >= tick1 && s->tick() < tick2) {
                        // Handle "overlapping" pedal segments (usual case for connected pedal line)
                        if (lastEvent.second == false && lastEvent.first >= (s->tick() + tickOffset + 2)) {
                              channelPedalEvents.at(channel).pop_back();
                              channelPedalEvents.at(channel).push_back(std::pair<int, bool>(s->tick() + tickOffset + 1, false));
                              }
                        channelPedalEvents.at(channel).push_back(std::pair<int, bool>(s->tick() + tickOffset + 2, true));
                        }
                  if (s->tick2() >= tick1 && s->tick2() <= tick2) {
                        int t = s->tick2() + tickOffset + 1;
                        if (t > repeatList()->last()->utick + repeatList()->last()->len)
                             t = repeatList()->last()->utick + repeatList()->last()->len;
                        channelPedalEvents.at(channel).push_back(std::pair<int, bool>(t, false));
                        }
                  }

            for (const auto& pedalEvents : channelPedalEvents) {
                  int channel = pedalEvents.first;
                  for (const auto& pe : pedalEvents.second) {
                        NPlayEvent event;
                        if (pe.second == true)
                              event = NPlayEvent(ME_CONTROLLER, channel, CTRL_SUSTAIN, 127);
                        else
                              event = NPlayEvent(ME_CONTROLLER, channel, CTRL_SUSTAIN, 0);
                        events->insert(std::pair<int,NPlayEvent>(pe.first, event));
                        }
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

void renderTremolo(Chord *chord, QList<NoteEventList> & ell){
    //
    //    process tremolo
    //
    Segment* seg = chord->segment();
    Tremolo* tremolo = chord->tremolo();
    int notes = chord->notes().size();
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
        if (t == 0) // avoid crash on very short tremolo
            t = 1;
        int n = chord->duration().ticks() / t;
        int l = 1000 / n;
        for (int k = 0; k < notes; ++k) {
            NoteEventList* events = &(ell)[k];
            events->clear();
            for (int i = 0; i < n; ++i)
                events->append(NoteEvent(0, l * i, l));
        }
    }
}

void renderArpeggio(Chord *chord, bool &gateEvents, QList<NoteEventList> & ell) {
    int notes = chord->notes().size();

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
    
int articulationExcursion(Key & key, int pitch, int delta)
    {
        if ( delta == 0) {
            return 0;
        }
        else {
            return diatonicUpDown(key,pitch,delta)-pitch;
        }
    }
    
void renderNoteArticulation(NoteEventList* events,
                        Chord *chord,
                        int pitch,
                        int tickspernote, // number of ticks, either _16h or _32nd, i.e., MScore::division/4 or MScore::division/8
                        const vector<int> & prefix,
                        const vector<int> & body,
                        bool repeatp, // repeatp=true means repeat the body as many times as possible to fill the time slice.
                        bool sustainp, // stretchp=true means the last note of the body is sustained to fill remaining time slice
                        const vector<int> & suffix
                        )
{
    events->clear();

    Key key       = chord->staff()->key(chord->segment()->tick());
    int space   = 1000;
    int maxticks = chord->actualTicks();
    int numrepeat = 1;
    int sustain   = 0;
    int ontime    = 0;

    int P = prefix.size();
    int B = body.size();
    int S = suffix.size();
    
    if (P + B + S <= 0 ) {
        return;
    }
    qreal ticksPerSecond = chord->score()->tempo(chord->tick()) * MScore::division;
    // I am hueristically declaring that the fastest possible trill is a 32nd note of a 120 bpm metronome.
    // This corresponds to 16 notes per second. 16 = 32 / (120 / 60).
    // Thus minduration = ticksPerSecond / 16.
    int mintickspernote = static_cast<int>(ticksPerSecond / 16); // minimum number of ticks for the shortest note in a trill or other articulation
    //qDebug("P=%d B=%d S=%d tickspernote(given)=%d minduration=%d maxticks=%d", P, B, S, tickspernote, mintickspernote, maxticks);

    // is the requested duration smaller than the minimum, if so, increase it to the minimum.
    tickspernote = max(tickspernote, mintickspernote);
    
    // calculate whether to shorten the duration value.
    if ( tickspernote*( P + B + S) <= maxticks ) {
        ; // plenty of space to play the notes without changing the requested trill note duration
    } else if ( tickspernote == mintickspernote ) {
        return; // the ornament is impossible to implement respecting the minimum duration and all the notes it contains
    } else {
        tickspernote = maxticks / (P + B + S) ; // integer division ignoring remainder
    }
    int millespernote = space * tickspernote / maxticks ; // rescale duration into per mille
    //qDebug("finally tickspernote = %d millespernote=%d", tickspernote, millespernote);

    // calculate the number of times to repeat the body, and sustain the last note of the body
    // 1000 = P + numrepeat*B+sustain + S
    if ( repeatp ) {
        numrepeat = (space - millespernote*(P + S)) / (millespernote*B);
    }
    if ( sustainp ) {
        sustain   = space - millespernote*(P + numrepeat*B + S);
    }

    // render the prefix
    //qDebug("prefix");
    for (int j=0; j<P; j++, ontime += millespernote) {
        //qDebug("   pitch=%d ontime=%d millespernote=%d", pitch, ontime, millespernote);
        events->append( NoteEvent( articulationExcursion(key, pitch, prefix[j]), ontime, millespernote));
    }
    //qDebug("body");
    if ( B > 0 ) {
       // render the body, but not the final repetion
       for (int r=0; r < numrepeat-1; r++){
           for (int j=0; j<B; j++, ontime += millespernote) {
               //qDebug("   (1) pitch=%d ontime=%d millspernote=%d", pitch, ontime, millespernote);
               events->append( NoteEvent( articulationExcursion(key, pitch, body[j]), ontime, millespernote));
           }
       }
       // render the final repetion of body, but not the final note of the repition
       for (int j=0; j<B-1; j++, ontime += millespernote) {
           //qDebug("   (2) pitch=%d ontime=%d millspernote=%d", pitch, ontime, millespernote);
           events->append( NoteEvent( articulationExcursion(key, pitch, body[j]), ontime, millespernote));
       }
       // render the final note of the final repeat of body
       //qDebug("   (3) pitch=%d ontime=%d millespernote=%d", pitch, ontime, millespernote);

       events->append( NoteEvent( articulationExcursion(key, pitch, body[B-1]), ontime, millespernote+sustain));
       ontime += (millespernote+sustain);
    }
    // render the suffix
    //qDebug("suffix");
    for (int j=0; j<S; j++, ontime += millespernote) {
        //qDebug("   pitch=%d ontime=%d millespernote=%d", pitch, ontime, millespernote);
        events->append( NoteEvent( articulationExcursion(key, pitch, suffix[j]), ontime, millespernote));
    }
}
    

void renderChordArticulation(Chord *chord, QList<NoteEventList> & ell, int & gateTime) {
    struct OrnamentExcursion {
        ArticulationType atype;
        set<MScore::OrnamentStyle> ostyles;
        int duration;
        vector<int> prefix;
        vector<int> body;
        bool repeatp;
        bool sustainp;
        vector<int> suffix;
    };

    Segment* seg = chord->segment();
    Instrument* instr = chord->part()->instrument(seg->tick());
    int channel  = 0;  // note->subchannel();
    int _16th = MScore::division / 4;
    int _32nd = _16th / 2;
    vector<int> emptypattern = {};
    set<MScore::OrnamentStyle> baroque  = {MScore::OrnamentStyle::BAROQUE};
    set<MScore::OrnamentStyle> defstyle = {MScore::OrnamentStyle::DEFAULT};
    set<MScore::OrnamentStyle> any      = {}; // empty set has the special meaning of any-style, rather than no-styles.
    deque<OrnamentExcursion>
       excursions = {
           //  articulation type           set of  duration       body         repeatp      suffix
           //                              styles          prefix                    sustainp
            {ArticulationType::Turn,        any,     _32nd, {},    {1,0,-1,0},   false, true, {}}
           ,{ArticulationType::Reverseturn, any,     _32nd, {},    {-1,0,1,0},   false, true, {}}
           ,{ArticulationType::Trill,       baroque, _32nd, {1,0}, {1,0},        true,  true, {}}
           ,{ArticulationType::Trill,       defstyle,_32nd, {0,1}, {0,1},        true,  true, {}}
           ,{ArticulationType::Plusstop,    baroque, _32nd, {0,-1},{0, -1},      true,  true, {}}
           ,{ArticulationType::Mordent,     any,     _32nd, {},    {0,-1,0},     false, true, {}}
           ,{ArticulationType::Prall,       any,     _32nd, {},    {0,1,0},      false, true, {}} // inverted mordent
           ,{ArticulationType::PrallPrall,  any,     _32nd, {1,0}, {1,0},        false, true, {}}
           ,{ArticulationType::PrallMordent,any,     _32nd, {},    {1,0,-1,0},   false, true, {}}
           ,{ArticulationType::UpPrall,     any,     _32nd, {-1,0},{1,0,1,0},    false, true, {}}
           ,{ArticulationType::UpMordent,   any,     _32nd, {-1,0},{1,0,-1,0},   false, true, {}}
           ,{ArticulationType::DownPrall,   any,     _32nd, {1,0,-1,0},
                                                                   {1,0,1,0},    false, true, {}}
           ,{ArticulationType::DownMordent, any,     _32nd, {1,0,-1,0},
                                                                   {1,0,-1,0},   false, true, {}}
           ,{ArticulationType::PrallUp,     any,     _32nd, {1,0}, {1,0},        false, true, {1}}
           ,{ArticulationType::PrallDown,   any,     _32nd, {1,0}, {1,0},        false, true, {-1}}
           ,{ArticulationType::Schleifer,   any,     _32nd, {},    {-2,-1,0},    false, true, {1,0,1,0}}
       };
    
    for (Articulation* a : chord->articulations()) {
        if ( false == a->playArticulation())
            continue;
        for (int k = 0; k < chord->notes().size(); ++k) {
            NoteEventList* events = &ell[k];
            int pitch   = chord->notes()[k]->epitch();
            auto oe = find_if(excursions.cbegin(), excursions.cend(),
                                 [a](OrnamentExcursion oe) {return  ( oe.atype == a->articulationType()
                                                                    && ( 0 == oe.ostyles.size()
                                                                        || oe.ostyles.end() != oe.ostyles.find(a->ornamentStyle())));
                                 });
            if ( oe != excursions.cend()) {
                renderNoteArticulation(events, chord, pitch, oe->duration,
                                       oe->prefix, oe->body, oe->repeatp, oe->sustainp, oe->suffix);
            } else {
                qDebug("MISSING %hhd %s", a->articulationType(), qPrintable(a->subtypeName()));
                instr->updateGateTime(&gateTime, channel, a->subtypeName());
            }
        }
    }
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

      int notes = chord->notes().size();
      for (int i = 0; i < notes; ++i)
            ell.append(NoteEventList());

      bool gateEvents = true;

      if (chord->tremolo()) {
          renderTremolo(chord, ell);

      }
      else if (chord->arpeggio()) {
          renderArpeggio(chord, gateEvents, ell);
      }

      if (!chord->articulations().isEmpty() && !chord->arpeggio()) {
          renderChordArticulation(chord, ell, gateTime);
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

    
void Score::createGraceNotesPlayEvents(QList<Chord*> gnb, int tick, Chord* chord, int &ontime){
    int n = gnb.size();
    if (n) {
        //
        //  render grace notes:
        //  simplified implementation:
        //  - grace notes start on the beat of the main note
        //  - duration: appoggiatura: 0.5  * duration of main note (2/3 for dotted notes, 4/7 for double-dotted)
        //              acciacatura: min of 0.5 * duration or 65ms fixed (independent of duration or tempo)
        //  - for appoggiaturas, the duration is divided by the number of grace notes
        //  - the grace note duration as notated does not matter
        //
        Chord* graceChord = gnb[0];
        if (graceChord->noteType() ==  NoteType::ACCIACCATURA) {
            qreal ticksPerSecond = tempo(tick) * MScore::division;
            int graceTimeMS = 65 * n;     // value determined empirically (TODO: make instrument-specific, like articulations)
            // 1000 occurs below for two different reasons:
            // number of milliseconds per second, also unit for ontime
            qreal chordTimeMS = (chord->actualTicks() / ticksPerSecond) * 1000;
            ontime = qMin(500, static_cast<int>((graceTimeMS / chordTimeMS) * 1000));
        }
        else if (chord->dots() == 1) {
            ontime = 667;
        }
        else if (chord->dots() == 2) {
            ontime = 571;
        }
        else {
            ontime = 500;
        }
        int graceDuration = ontime / n;
        
        int on = 0;
        for (int i = 0; i < n; ++i) {
            QList<NoteEventList> el;
            Chord* gc = gnb.at(i);
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
            Instrument* instr = chord->part()->instrument(tick);
            instr->updateGateTime(&gateTime, 0, "");
            }
          
      int ontime = 0;

      Score::createGraceNotesPlayEvents(chord->graceNotesBefore(), tick, chord, ontime);
     
      SwingParameters st = chord->staff()->swing(tick);
      int unit = st.swingUnit;
      int ratio = st.swingRatio;
      // Check if swing needs to be applied
      if (unit) {
            swingAdjustParams(chord, gateTime, ontime, unit, ratio);
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
      updateSwing();
      createPlayEvents();

      updateRepeatList(MScore::playRepeats);
      _foundPlayPosAfterRepeats = false;
      updateChannel();
      updateVelo();

      // create note & other events
      foreach (Staff* part, _staves)
            renderStaff(events, part);

      // create sustain pedal events
      renderSpanners(events, -1);

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
            }
      }
}

