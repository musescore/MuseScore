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

#include "score.h"
#include "volta.h"
#include "note.h"
#include "glissando.h"
#include "instrument.h"
#include "part.h"
#include "chord.h"
#include "trill.h"
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
                        Instrument* instr = c->part()->instrument(c->tick());
                        if (channel >= instr->channel().size()) {
                              qDebug() << "Channel " << channel << " too high. Max " << instr->channel().size();
                              channel = 0;
                              }
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
   int velo, int onTime, int offTime, int staffIdx)
      {
      if (!note->play())
            return;
      velo = note->customizeVelocity(velo);
      NPlayEvent ev(ME_NOTEON, channel, pitch, velo);
      ev.setOriginatingStaff(staffIdx);
      ev.setTuning(note->tuning());
      ev.notes.push_back(note);
      if (offTime < onTime)
            offTime = onTime;
      events->insert(std::pair<int, NPlayEvent>(onTime, ev));
      ev.setVelo(0);
      events->insert(std::pair<int, NPlayEvent>(offTime, ev));
      }

//---------------------------------------------------------
//   collectNote
//---------------------------------------------------------

static void collectNote(EventMap* events, int channel, const Note* note, int velo, int tickOffset, int staffIdx)
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
                              collectNote(events, channel, n, velo, tickOffset, staffIdx);
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
            const NoteEvent& e = nel[i]; // we make an explict const ref, not a const copy.  no need to copy as we won't change the original object.

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
            playNote(events, note, channel, p, velo, on, off, staffIdx);
            }

      // Bends
      for (Element* e: note->el()) {
            if (e == 0 || e->type() != Element::Type::BEND)
                  continue;
            Bend* bend = static_cast<Bend*>(e);
            if (!bend->playBend())
                  break;
            const QList<PitchValue>& points = bend->points();
            int pitchSize = points.size();

            double noteLen = note->playTicks();
            int lastPointTick = tick1;
            for(int pitchIndex = 0; pitchIndex < pitchSize-1; pitchIndex++) {
                  PitchValue pitchValue = points[pitchIndex];
                  PitchValue nextPitch  = points[pitchIndex+1];
                  int nextPointTick = tick1 + nextPitch.time / 60.0 * noteLen;
                  int pitch = pitchValue.pitch;

                  if (pitchIndex == 0 && (pitch == nextPitch.pitch)) {
                        int midiPitch = (pitch * 16384) / 1200 + 8192;
                        int msb = midiPitch / 128;
                        int lsb = midiPitch % 128;
                        NPlayEvent ev(ME_PITCHBEND, channel, lsb, msb);
                        ev.setOriginatingStaff(staffIdx);
                        events->insert(std::pair<int, NPlayEvent>(lastPointTick, ev));
                        lastPointTick = nextPointTick;
                        continue;
                        }
                  if (pitch == nextPitch.pitch && !(pitchIndex == 0 && pitch != 0)) {
                        lastPointTick = nextPointTick;
                        continue;
                        }

                  double pitchDelta = nextPitch.pitch - pitch;
                  double tickDelta  = nextPitch.time - pitchValue.time;
                  /*         B
                            /.                   pitch is 1/100 semitones
                    bend   / .  pitchDelta       time is in noteDuration/60
                          /  .                   midi pitch is 12/16384 semitones
                         A....
                       tickDelta   */
                  for (int i = lastPointTick; i <= nextPointTick; i += 16) {
                        double dx = ((i-lastPointTick) * 60) / noteLen;
                        int p = pitch + dx * pitchDelta / tickDelta;

                        // We don't support negative pitch, but Midi does. Let's center by adding 8192.
                        int midiPitch = (p * 16384) / 1200 + 8192;
                        // Representing pitch as two bytes
                        int msb = midiPitch / 128;
                        int lsb = midiPitch % 128;
                        NPlayEvent ev(ME_PITCHBEND, channel, lsb, msb);
                        ev.setOriginatingStaff(staffIdx);
                        events->insert(std::pair<int, NPlayEvent>(i, ev));
                        }
                  lastPointTick = nextPointTick;
                  }
            NPlayEvent ev(ME_PITCHBEND, channel, 0, 64); // 0:64 is 8192 - no pitch bend
            ev.setOriginatingStaff(staffIdx);
            events->insert(std::pair<int, NPlayEvent>(tick1+noteLen, ev));
            }
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
                  int staffIdx = staff->idx();
                  int velocity = staff->velocities().velo(seg->tick());
                  Instrument* instr = chord->part()->instrument(tick);
                  int channel = instr->channel(chord->upNote()->subchannel())->channel;
                  events->registerChannel(channel);

                  foreach (Articulation* a, chord->articulations()) {
                        instr->updateVelocity(&velocity,channel, a->subtypeName());
                        }

                  for (Chord* c : chord->graceNotesBefore()) {
                        for (const Note* note : c->notes())
                              collectNote(events, channel, note, velocity, tickOffset, staffIdx);
                        }

                  foreach (const Note* note, chord->notes())
                        collectNote(events, channel, note, velocity, tickOffset, staffIdx);

#if 0
                  // TODO: add support for grace notes after - see createPlayEvents()
                  QList<Chord*> gna;
                  chord->getGraceNotesAfter(&gna);
                  for (Chord* c : gna) {
                        for (const Note* note : c->notes())
                              collectNote(events, channel, note, velocity, tickOffset, staffIdx);
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
                  for (const ChannelActions& ca : *st->channelActions()) {
                        int channel = instr->channel().at(ca.channel)->channel;
                        for(const QString& ma : ca.midiActionNames) {
                              NamedEventList* nel = instr->midiAction(ma, ca.channel);
                              if (!nel)
                                    continue;
                              for (MidiCoreEvent event : nel->events) {
                                    event.setChannel(channel);
                                    NPlayEvent e(event);
                                    e.setOriginatingStaff(firstStaffIdx);
                                    if (e.dataA() == CTRL_PROGRAM)
                                          events->insert(std::pair<int, NPlayEvent>(tick-1, e));
                                    else
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
            Measure* m = firstMeasure();
            if (m == 0)
                  return;
            RepeatSegment* s = new RepeatSegment;
            s->tick  = 0;
            s->utick = 0;
            s->utime = 0.0;
            s->timeOffset = 0.0;
            do {
                  s->addMeasure(m);
                  m = m->nextMeasure();
                  }
            while (m);
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
      if (h->hairpinType() == Hairpin::Type::CRESCENDO) {
            if (incr == 0 && velo < st->velocities().nextVelo(tick2-1))
                  endVelo = st->velocities().nextVelo(tick2-1);
            else
                  endVelo += incr;
            }
      else {
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
            int endTick    = startTick + rs->len();
            int tickOffset = rs->utick - rs->tick;
            for (Measure* m = tick2measure(startTick); m; m = m->nextMeasure()) {
                  if (lastMeasure && m->isRepeatMeasure(staff)) {
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

void Score::renderSpanners(EventMap* events)
      {
      foreach (const RepeatSegment* rs, *repeatList()) {
            int tickOffset = rs->utick - rs->tick;
            int utick1 = rs->utick;
            int tick1 = repeatList()->utick2tick(utick1);
            int tick2 = tick1 + rs->len();
            std::map<int, std::vector<std::pair<int, std::pair<bool, int>>>> channelPedalEvents;
            for (const auto& sp : _spanner.map()) {
                  Spanner* s = sp.second;
                  if (s->type() != Element::Type::PEDAL)
                        continue;

                  int staff = s->staffIdx();
                  int idx = s->staff()->channel(s->tick(), 0);
                  int channel = s->part()->instrument(s->tick())->channel(idx)->channel;
                  channelPedalEvents.insert({channel, std::vector<std::pair<int, std::pair<bool, int>>>()});
                  std::vector<std::pair<int, std::pair<bool, int>>> pedalEventList = channelPedalEvents.at(channel);
                  std::pair<int, std::pair<bool, int>> lastEvent;

                  if (!pedalEventList.empty())
                        lastEvent = pedalEventList.back();
                  else
                        lastEvent = std::pair<int, std::pair<bool, int>>(0, std::pair<bool, int>(true, staff));

                  if (s->tick() >= tick1 && s->tick() < tick2) {
                        // Handle "overlapping" pedal segments (usual case for connected pedal line)
                        if (lastEvent.second.first == false && lastEvent.first >= (s->tick() + tickOffset + 2)) {
                              channelPedalEvents.at(channel).pop_back();
                              channelPedalEvents.at(channel).push_back(std::pair<int, std::pair<bool, int>>(s->tick() + tickOffset + 1, std::pair<bool, int>(false, staff)));
                              }
                        channelPedalEvents.at(channel).push_back(std::pair<int, std::pair<bool, int>>(s->tick() + tickOffset + 2, std::pair<bool, int>(true, staff)));
                        }
                  if (s->tick2() >= tick1 && s->tick2() <= tick2) {
                        int t = s->tick2() + tickOffset + 1;
                        if (t > repeatList()->last()->utick + repeatList()->last()->len())
                              t = repeatList()->last()->utick + repeatList()->last()->len();
                        channelPedalEvents.at(channel).push_back(std::pair<int, std::pair<bool, int>>(t, std::pair<bool, int>(false, staff)));
                        }
                  }

            for (const auto& pedalEvents : channelPedalEvents) {
                  int channel = pedalEvents.first;
                  for (const auto& pe : pedalEvents.second) {
                        NPlayEvent event;
                        if (pe.second.first == true)
                              event = NPlayEvent(ME_CONTROLLER, channel, CTRL_SUSTAIN, 127);
                        else
                              event = NPlayEvent(ME_CONTROLLER, channel, CTRL_SUSTAIN, 0);
                        event.setOriginatingStaff(pe.second.second);
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

const Drumset* getDrumset(const Chord* chord)
      {
      if (chord->staff() && chord->staff()->isDrumStaff()) {
            const Drumset* ds = chord->staff()->part()->instrument(chord->tick())->drumset();
            return ds;
            }
      return nullptr;
      }

//---------------------------------------------------------
//   renderTremolo
//---------------------------------------------------------

void renderTremolo(Chord* chord, QList<NoteEventList>& ell)
      {
      Segment* seg = chord->segment();
      Tremolo* tremolo = chord->tremolo();
      int notes = chord->notes().size();

      // check if tremolo was rendered before for drum staff
      const Drumset* ds = getDrumset(chord);
      if (ds) {
            for (Note* n : chord->notes()) {
                  DrumInstrumentVariant div = ds->findVariant(n->pitch(), chord->articulations(), chord->tremolo());
                  if (div.pitch != INVALID_PITCH && div.tremolo == tremolo->tremoloType())
                        return; // already rendered
                  }
            }

      // we cannot render buzz roll with MIDI events only
      if (tremolo->tremoloType() == TremoloType::BUZZ_ROLL)
            return;

      // render tremolo with multiple events
      if (chord->tremoloChordType() == TremoloChordType::TremoloFirstNote) {
            int t = MScore::division / (1 << (tremolo->lines() + chord->durationType().hooks()));
            Segment::Type st = Segment::Type::ChordRest;
            Segment* seg2 = seg->next(st);
            int track = chord->track();
            while (seg2 && !seg2->element(track))
                  seg2 = seg2->next(st);
            Chord* c2 = seg2 ? static_cast<Chord*>(seg2->element(track)) : 0;
            if (c2 && c2->type() == Element::Type::CHORD) {
                  int notes2 = c2->notes().size();
                  int tnotes = qMax(notes, notes2);
                  int tticks = chord->actualTicks() * 2; // use twice the size
                  int n = tticks / t;
                  n /= 2;
                  int l = 2000 * t / tticks;
                  for (int k = 0; k < tnotes; ++k) {
                        NoteEventList* events;
                        if (k < notes) {
                              // first chord has note
                              events = &ell[k];
                              events->clear();
                              }
                        else {
                              // otherwise reuse note 0
                              events = &ell[0];
                              }
                        if (k < notes && k < notes2) {
                              // both chords have note
                              int p1 = chord->notes()[k]->pitch();
                              int p2 = c2->notes()[k]->pitch();
                              int dpitch = p2 - p1;
                              for (int i = 0; i < n; ++i) {
                                    events->append(NoteEvent(0, l * i * 2, l));
                                    events->append(NoteEvent(dpitch, l * i * 2 + l, l));
                                    }
                              }
                        else if (k < notes) {
                              // only first chord has note
                              for (int i = 0; i < n; ++i)
                                    events->append(NoteEvent(0, l * i * 2, l));
                              }
                        else {
                              // only second chord has note
                              // reuse note 0 of first chord
                              int p1 = chord->notes()[0]->pitch();
                              int p2 = c2->notes()[k]->pitch();
                              int dpitch = p2-p1;
                              for (int i = 0; i < n; ++i)
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

//---------------------------------------------------------
//   renderArpeggio
//---------------------------------------------------------

void renderArpeggio(Chord *chord, QList<NoteEventList> & ell)
      {
      int notes = chord->notes().size();
      int l = 64;
      while (l && (l * notes > chord->upNote()->playTicks()))
            l = 2*l / 3;
      int start, end, step;
      bool up = chord->arpeggio()->arpeggioType() != ArpeggioType::DOWN && chord->arpeggio()->arpeggioType() != ArpeggioType::DOWN_STRAIGHT;
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

//---------------------------------------------------------
//   convertLine
// find the line in clefF corresponding to lineL2 in clefR
//---------------------------------------------------------

int convertLine (int lineL2, ClefType clefL, ClefType clefR) {
      int lineR2 = lineL2;
      int goalpitch = line2pitch(lineL2, clefL, Key::C);
      int p;
      while ( (p = line2pitch(lineR2, clefR, Key::C)) > goalpitch && p < 127)
            lineR2++;
      while ( (p = line2pitch(lineR2, clefR, Key::C)) < goalpitch &&  p > 0)
            lineR2--;
      return lineR2;
      }

//---------------------------------------------------------
//   convertLine
// find the line in clef for NoteL corresponding to lineL2 in clef for noteR
// for example middle C is line 10 in Treble clef, but is line -2 in Bass clef.
//---------------------------------------------------------

int convertLine(int lineL2, Note *noteL, Note *noteR)
      {
      return convertLine(lineL2,
         noteL->chord()->staff()->clef(noteL->chord()->tick()),
         noteR->chord()->staff()->clef(noteR->chord()->tick()));
      }

//---------------------------------------------------------
//   articulationExcursion
// noteL is the note to measure the deltastep from, i.e., ornaments are w.r.t. this note
// noteR is the note to search backward from to find accidentals.
//    for ornament calculation noteL and noteR are the same, but for glissando they are
//     the start end end note of glissando.
// deltastep is the number of diatonic steps between the base note and this articulation step.
//---------------------------------------------------------

int articulationExcursion(Note *noteL, Note *noteR, int deltastep)
      {
      if (0 == deltastep)
            return 0;
      Chord *chordL = noteL->chord();
      Chord *chordR = noteR->chord();
      int pitchL = noteL->pitch();
      int tickL = chordL->tick();
      // we canot use staffL = chord->staff() because that won't correspond to the noteL->line()
      //   in the case the user has pressed Shift-Cmd->Up or Shift-Cmd-Down.
      //   Therefore we have to take staffMove() into account using vStaffIdx().
      Staff * staffL = noteL->score()->staff(chordL->vStaffIdx());
      ClefType clefL = staffL->clef(tickL);
      // line represents the ledger line of the staff.  0 is the top line, 1, is the space between the top 2 lines,
      //  ... 8 is the bottom line.
      int lineL     = noteL->line();
      // we use line - deltastep, because lines are oriented from top to bottom, while step is oriented from bottom to top.
      int lineL2    = lineL - deltastep;
      Measure* measureR = chordR->segment()->measure();

      Segment* segment = noteL->chord()->segment();
      int lineR2 = convertLine(lineL2, noteL, noteR);
      // is there another note in this segment on the same line?
      // if so, use its pitch exactly.
      int halfsteps = 0;
      int staffIdx = noteL->chord()->staff()->idx(); // cannot use staffL->idx() because of staffMove()
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;
      bool done = false;
      for (int track = startTrack; track < endTrack; ++track) {
            Element* e = segment->element(track);
            if (!e || e->type() != Element::Type::CHORD)
                  continue;
            Chord* chord = static_cast<Chord*>(e);
            for (Note* note : chord->notes()) {
                  if (note->tieBack())
                        continue;
                  int pc = (note->line() + 700) % 7;
                  int pc2 = (lineL2 + 700) % 7;
                  if (pc2 == pc) {
                        // e.g., if there is an F# note at this staff/tick, then force every F to be F#.
                        int octaves = (note->line() - lineL2) / 7;
                        halfsteps = note->pitch() + 12 * octaves - pitchL;
                        done = true;
                        break;
                        }
                  }
            if (!done) {
                  if (staffL->isPitchedStaff()) {
                        bool error = false;
                        AccidentalVal acciv2 = measureR->findAccidental(chordR->segment(), chordR->staff()->idx(), lineR2, error);
                        int acci2 = int(acciv2);
                        // we have to add ( noteL->ppitch() - noteL->epitch() ) which is the delta for transposing instruments.
                        halfsteps = line2pitch(lineL-deltastep, clefL, Key::C) + noteL->ppitch() - noteL->epitch() + acci2 - pitchL;
                        }
                  else {
                        // cannot rely on accidentals or key signatures
                        halfsteps = deltastep;
                        }
                  }
            }
      return halfsteps;
      }

//---------------------------------------------------------
// totalTiedNoteTicks
//      return the total of the actualTicks of the given note plus
//      the chain of zero or more notes tied to it to the right.
//---------------------------------------------------------
int totalTiedNoteTicks(Note* note)
      {
      int total = note->chord()->actualTicks();
      while (note->tieFor() && note->tieFor()->endNote() && (note->chord()->tick() < note->tieFor()->endNote()->chord()->tick())) {
            note = note->tieFor()->endNote();
            total += note->chord()->actualTicks();
            }
      return total;
      };


//---------------------------------------------------------
//   renderNoteArticulation
// prefix, vector of int, normally something like {0,-1,0,1} modeling the prefix of tremblement relative to the base note
// body, vector of int, normally something like {0,-1,0,1} modeling the possibly repeated tremblement relative to the base note
// tickspernote, number of ticks, either _16h or _32nd, i.e., MScore::division/4 or MScore::division/8
// repeatp, true means repeat the body as many times as possible to fill the time slice.
// sustainp, true means the last note of the body is sustained to fill remaining time slice
//---------------------------------------------------------

bool renderNoteArticulation(NoteEventList* events, Note * note, bool chromatic, int requestedTicksPerNote,
   const vector<int> & prefix, const vector<int> & body,
   bool repeatp, bool sustainp, const vector<int> & suffix,
   int fastestFreq=64, int slowestFreq=8 // 16 Hz and 8 Hz
   )
      {

      events->clear();
      Chord *chord = note->chord();
      int maxticks = totalTiedNoteTicks(note);
      int space = 1000 * maxticks;
      int numrepeat = 1;
      int sustain   = 0;
      int ontime    = 0;

      int p = prefix.size();
      int b = body.size();
      int s = suffix.size();
      int ticksPerNote = 0;

      if (p + b + s <= 0 )
            return false;

      int tick = chord->tick();
      qreal tempo = chord->score()->tempo(tick);
      int ticksPerSecond = tempo * MScore::division;

      int minTicksPerNote = int(ticksPerSecond / fastestFreq);
      int maxTicksPerNote = (0 == slowestFreq) ? 0 : int(ticksPerSecond / slowestFreq);

      // for fast tempos, we have to slow down the tremblement frequency, i.e., increase the ticks per note
      if (requestedTicksPerNote >= minTicksPerNote)
            ;
      else { // try to divide the requested frequency by a power of 2 if possible, if not, use the maximum frequency, ie., minTicksPerNote
            ticksPerNote = requestedTicksPerNote;
            while (ticksPerNote < minTicksPerNote) {
                  ticksPerNote *= 2; // decrease the tremblement frequency
                  }
            if (ticksPerNote > maxTicksPerNote)
                  ticksPerNote = minTicksPerNote;
            }

      ticksPerNote = max(requestedTicksPerNote, minTicksPerNote);

      if (slowestFreq <= 0) // no slowest freq given such as something silly like glissando with 4 notes over 8 counts.
            ;
      else if (ticksPerNote <= maxTicksPerNote)
            ;
      else {
            // for slow tempos, such as adagio, we may need to speed up the tremblement freqency, i.e., decrease the ticks per note, to make it sound reasonable.
            ticksPerNote = requestedTicksPerNote ;
            while (ticksPerNote > maxTicksPerNote) {
                  ticksPerNote /= 2;
                  }
            if (ticksPerNote < minTicksPerNote)
                  ticksPerNote = minTicksPerNote;
            }
      // calculate whether to shorten the duration value.
      if ( ticksPerNote*(p + b + s) <= maxticks )
            ; // plenty of space to play the notes without changing the requested trill note duration
      else if ( ticksPerNote == minTicksPerNote )
            return false; // the ornament is impossible to implement respecting the minimum duration and all the notes it contains
      else {
            ticksPerNote = maxticks / (p + b + s);  // integer division ignoring remainder
            if ( slowestFreq <= 0 )
                  ;
            else if ( ticksPerNote < minTicksPerNote )
                  return false;
            }

      int millespernote = space * ticksPerNote  / maxticks;  // rescale duration into per mille

      // local function:
      // look ahead in the given vector to see if the current note is the same pitch as the next note or next several notes.
      // If so, increment the duration by the appropriate note duration, and increment the index, j, to the next note index
      // of a different pitch.
      // The total duration of the tied note is returned, and the index is modified.
      auto tieForward = [millespernote] (int & j, const vector<int> & vec) {
               int size = vec.size();
               int duration = millespernote;
               while ( j < size-1 && vec[j] == vec[j+1] ) {
                     duration += millespernote;
                     j++;
                     }
               return duration;
               };

      // local function:
      auto makeEvent = [note,chord,chromatic,events] (int pitch, int ontime, int duration) {
               events->append( NoteEvent(chromatic ? pitch : articulationExcursion(note,note,pitch),
                  ontime/chord->actualTicks(),
                  duration/chord->actualTicks()));
               return ontime + duration;
               };

      // calculate the number of times to repeat the body, and sustain the last note of the body
      // 1000 = P + numrepeat*B+sustain + S
      if (repeatp)
            numrepeat = (space - millespernote*(p + s)) / (millespernote * b);
      if (sustainp)
            sustain   = space - millespernote*(p + numrepeat * b + s);
      // render the prefix
      for (int j=0; j < p; j++)
            ontime = makeEvent(prefix[j], ontime, tieForward(j,prefix));

      if (b > 0) {
            // render the body, but not the final repetion
            for (int r = 0; r < numrepeat-1; r++) {
                  for (int j=0; j < b; j++)
                        ontime = makeEvent(body[j], ontime, millespernote);
                  }
            // render the final repetion of body, but not the final note of the repition
            for (int j = 0; j < b - 1; j++)
                  ontime = makeEvent(body[j], ontime, millespernote);
            // render the final note of the final repeat of body
            ontime = makeEvent(body[b-1], ontime, millespernote+sustain);
            }
      // render the suffix
      for (int j = 0; j < s; j++)
            ontime = makeEvent(suffix[j], ontime, tieForward(j,suffix));

      return true;
      }

//---------------------------------------------------------
//   renderNoteArticulation
//---------------------------------------------------------

bool renderNoteArticulation(NoteEventList* events, Note * note, bool chromatic, ArticulationType articulationType, MScore::OrnamentStyle ornamentStyle)
      {
      if (!note->staff()->isPitchedStaff()) // not enough info in tab staff
            return false;
      // This struct specifies how to render an articulation.
      //   atype - the articulation type to implement, such as ArticulationType::Turn
      //   ostyles - the actual ornament has a property called ornamentStyle whose value is
      //             a value of type MScore::OrnamentStyle.  This ostyles field indicates the
      //             the set of ornamentStyles which apply to this rendition.
      //   duration - the default duration for each note in the rendition, the final duration
      //            rendered might be less than this if an articulation is attached to a note of
      //            short duration.
      //   prefix - vector of integers. indicating which notes to play at the beginning of rendering the
      //            articulation.  0 represents the principle note, 1==> the note diatonically 1 above
      //            -1 ==> the note diatonically 1 below.  E.g., in the key of G, if a turn articulation
      //            occures above the note F#, then 0==>F#, 1==>G, -1==>E.
      //            These integers indicate which notes actual notes to play when rendering the ornamented
      //            note.   However, if the same integer appears several times adjacently such as {0,0,0,1}
      //            That means play the notes tied.  e.g., F# followed by G, but the duration of F# is 3x the
      //            duration of the G.
      //    body   - notes to play comprising the body of the rendered ornament.
      //            The body differs from the prefix and suffix in several ways.
      //            * body does not support tied notes: {0,0,0,1} means play 4 distinct notes (not tied).
      //            * if there is sufficient duration in the principle note, AND repeatep is true, then body
      //               will be rendered multiple times, as the duration allows.
      //            * to avoid a time gap (or rest) in rendering the articulation, if sustainp is true,
      //               then the final note of the body will be sustained to fill the left-over time.
      //    suffix - similar to prefix but played once at the end of the rendered ornament.
      //    repeatp  - whether the body is repeatable in its entirety.
      //    sustainp - whether the final note of the body should be sustained to fill the remaining duration.
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
      int _16th = MScore::division / 4;
      int _32nd = _16th / 2;
      vector<int> emptypattern = {};
      set<MScore::OrnamentStyle> baroque  = {MScore::OrnamentStyle::BAROQUE};
      set<MScore::OrnamentStyle> defstyle = {MScore::OrnamentStyle::DEFAULT};
      set<MScore::OrnamentStyle> any; // empty set has the special meaning of any-style, rather than no-styles.

      vector<OrnamentExcursion> excursions = {
            //  articulation type           set of  duration       body         repeatp      suffix
            //                              styles          prefix                    sustainp
             {ArticulationType::Turn,        any,     _32nd, {},    {1,0,-1,0},   false, true, {}}
            ,{ArticulationType::Reverseturn, any,     _32nd, {},    {-1,0,1,0},   false, true, {}}
            ,{ArticulationType::Trill,       baroque, _32nd, {1,0}, {1,0},        true,  true, {}}
            ,{ArticulationType::Trill,       defstyle,_32nd, {0,1}, {0,1},        true,  true, {}}
            ,{ArticulationType::Plusstop,    baroque, _32nd, {0,-1},{0, -1},      true,  true, {}}
            ,{ArticulationType::Mordent,     any,     _32nd, {},    {0,-1,0},     false, true, {}}
            ,{ArticulationType::Prall,       defstyle,_32nd, {},    {0,1,0},      false, true, {}} // inverted mordent
            ,{ArticulationType::Prall,       baroque, _32nd, {1,0,1},{0},         false, true, {}} // short trill
            ,{ArticulationType::PrallPrall,  any,     _32nd, {1,0}, {1,0},        false, true, {}}
            ,{ArticulationType::PrallMordent,any,     _32nd, {},    {1,0,-1,0},   false, true, {}}
            ,{ArticulationType::LinePrall,   any,     _32nd, {2,2,2},{1,0},       true,  true, {}}
            ,{ArticulationType::UpPrall,     any,     _16th, {-1,0},{1,0},        true,  true, {1,0}} // p 144 Ex 152 [1]
            ,{ArticulationType::UpMordent,   any,     _16th, {-1,0},{1,0},        true,  true, {-1,0}} // p 144 Ex 152 [1]
            ,{ArticulationType::DownPrall,   any,     _16th, {1,1,1,0}, {1,0},    true,  true, {}} // p136 Cadence Appuyee [1] [2]
            ,{ArticulationType::DownMordent, any,     _16th, {1,1,1,0}, {1,0},    true,  true, {-1, 0}} // p136 Cadence Appuyee + mordent [1] [2]
            ,{ArticulationType::PrallUp,     any,     _16th, {1,0}, {1,0},        true,  true, {-1,0}} // p136 Double Cadence [1]
            ,{ArticulationType::PrallDown,   any,     _16th, {1,0}, {1,0},        true,  true, {-1,0,0,0}} // p144 ex 153 [1]
            ,{ArticulationType::Schleifer,   any,     _32nd, {},    {0},          false, true, {}}
      };

      // [1] Some of the articulations/ornaments in the excursions table above come from
      // Baroque Music, Style and Performance A Handbook, by Robert Donington,(c) 1982
      // ISBN 0-393-30052-8, W. W. Norton & Company, Inc.

      // [2] In some cases, the example from [1] does not preserve the timing.
      // For example, illustrates 2+1/4 counts per half note.

      for (auto & oe: excursions) {
            if (oe.atype == articulationType
                && ( 0 == oe.ostyles.size()
                    || oe.ostyles.end() != oe.ostyles.find(ornamentStyle))) {
                  return renderNoteArticulation(events, note, chromatic, oe.duration,
                                                oe.prefix, oe.body, oe.repeatp, oe.sustainp, oe.suffix);
                }
      }
      return false;
      }

//---------------------------------------------------------
//   renderNoteArticulation
//---------------------------------------------------------
bool renderNoteArticulation(NoteEventList* events, Note * note, bool chromatic, Trill::Type trillType, MScore::OrnamentStyle ornamentStyle)
      {
      map<Trill::Type,ArticulationType> articulationMap = {
            {Trill::Type::TRILL_LINE,      ArticulationType::Trill}
           ,{Trill::Type::UPPRALL_LINE,    ArticulationType::UpPrall}
           ,{Trill::Type::DOWNPRALL_LINE,  ArticulationType::DownPrall}
           ,{Trill::Type::PRALLPRALL_LINE, ArticulationType::Trill}
           };
      auto it = articulationMap.find(trillType);
      if (it == articulationMap.cend() )
            return false;
      else
            return renderNoteArticulation(events, note, chromatic, it->second, ornamentStyle);
      }

//---------------------------------------------------------
//   noteHasGlissando
// true if note is the end of a glissando
//---------------------------------------------------------

bool noteHasGlissando(Note *note)
      {
      for (Spanner* spanner : note->spannerFor()) {
            if ((spanner->type() == Element::Type::GLISSANDO)
               && spanner->endElement()
               && (Element::Type::NOTE == spanner->endElement()->type()))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   renderGlissando
//---------------------------------------------------------

void renderGlissando(NoteEventList* events, Note *notestart)
      {
      vector<int> empty = {};
      int Cnote = 60; // pitch of middle C
      int pitchstart = notestart->ppitch();
      int linestart = notestart->line();

      set<int> blacknotes = {  1,  3,    6, 8, 10};
      set<int> whitenotes = {0,  2, 4, 5, 7,  9, 11};

      for (Spanner* spanner : notestart->spannerFor()) {
            if (spanner->type() == Element::Type::GLISSANDO) {
                  Glissando *glissando = static_cast<Glissando *>(spanner);
                  MScore::GlissandoStyle glissandoStyle = glissando->glissandoStyle();
                  Element* ee = spanner->endElement();
                  // only consider glissando connnected to NOTE.
                  if (glissando->playGlissando() && Element::Type::NOTE == ee->type()) {
                        vector<int> body;
                        Note *noteend = static_cast<Note *>(ee);
                        int pitchend   = noteend->ppitch();
                        bool direction= pitchend >  pitchstart;
                        if (pitchend == pitchstart)
                              continue; // next spanner
                        if (glissandoStyle == MScore::GlissandoStyle::DIATONIC) { // scale obeying accidentals
                              int line;
                              int p = pitchstart;
                              // iterate as long as we haven't past the pitchend.
                              for (line = linestart; (direction) ? (p<pitchend) : (p>pitchend);
                                 (direction) ? line-- : line++) {
                                    int halfsteps = articulationExcursion(notestart, noteend, linestart - line);
                                    p = pitchstart + halfsteps;
                                    if (direction ? p < pitchend : p > pitchend)
                                          body.push_back(halfsteps);
                                    }
                              }
                        else {
                              for (int p = pitchstart; direction ? p < pitchend : p > pitchend; p += (direction ? 1 : -1)) {
                                    bool choose = false;
                                    int mod = ((p - Cnote) + 1200) % 12;
                                    switch (glissandoStyle) {
                                          case MScore::GlissandoStyle::CHROMATIC:
                                                choose = true;
                                                break;
                                          case MScore::GlissandoStyle::WHITE_KEYS: // white note
                                                choose = (whitenotes.find(mod) != whitenotes.end());
                                                break;
                                          case MScore::GlissandoStyle::BLACK_KEYS: // black note
                                                choose =  (blacknotes.find(mod) != blacknotes.end());
                                                break;
                                          default:
                                                choose = false;
                                          }
                                    if (choose)
                                          body.push_back(p - pitchstart);
                                    }
                              }
                        renderNoteArticulation(events, notestart, true, MScore::division, empty, body, false, true, empty, 16, 0);
                        }
                  }
            }
      }

//---------------------------------------------------------
// findFirstTrill
//  search the spanners in the score, finding the first one
//  which overlaps this chord and is of type Element::Type::TRILL
//---------------------------------------------------------

Trill* findFirstTrill(Chord *chord) {
      auto spanners = chord->score()->spannerMap().findOverlapping(1+chord->tick(), chord->tick() + chord->actualTicks() - 1);
      for (auto i : spanners) {
            if (i.value->type() != Element::Type::TRILL)
                  continue;
            if (i.value->track() != chord->track())
                  continue;
            Trill *trill = static_cast<Trill *>(i.value);
            if (trill->playArticulation() == false)
                  continue;
            return trill;
            }
      return nullptr;
      }


//---------------------------------------------------------
//   renderChordArticulation
//---------------------------------------------------------

void renderChordArticulation(Chord *chord, QList<NoteEventList> & ell, int & gateTime)
      {
      Segment* seg = chord->segment();
      Instrument* instr = chord->part()->instrument(seg->tick());
      int channel  = 0;  // note->subchannel();

      for (int k = 0; k < chord->notes().size(); ++k) {
            NoteEventList* events = &ell[k];
            Note *note = chord->notes()[k];
            Trill *trill;

            if (noteHasGlissando(note))
                  renderGlissando(events, note);
            else if (chord->staff()->isPitchedStaff()  && (trill = findFirstTrill(chord)) != nullptr) {
                  renderNoteArticulation(events, note, false, trill->trillType(), trill->ornamentStyle());
                  }
            else {
                  for (Articulation* a : chord->articulations()) {
                        if ( false == a->playArticulation())
                              continue;
                        if (! renderNoteArticulation(events, note, false, a->articulationType(), a->ornamentStyle()))
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

      if (chord->tremolo()) {
            renderTremolo(chord, ell);
            }
      else if (chord->arpeggio() && chord->arpeggio()->playArpeggio()) {
            renderArpeggio(chord, ell);
            return ell;  // dont apply gateTime to arpeggio events
            }
      else
            renderChordArticulation(chord, ell, gateTime);
      //
      //    apply gateTime
      //
      for (int i = 0; i < notes; ++i) {
            NoteEventList* el = &ell[i];
            if (el->size() == 0 && chord->tremoloChordType() != TremoloChordType::TremoloSecondNote) {
                  el->append(NoteEvent(0, ontime, 1000-ontime));
                  }
            for ( NoteEvent &e : ell[i])
                  e.setLen(e.len() * gateTime / 100);
            }
      return ell;
      }

void Score::createGraceNotesPlayEvents(QList<Chord*> gnb, int tick, Chord* chord, int &ontime)
      {
      int n = gnb.size();
      int graceDuration = 0;
      bool drumset = (getDrumset(chord) != nullptr);
      const qreal ticksPerSecond = tempo(tick) * MScore::division;
      const qreal chordTimeMS = (chord->actualTicks() / ticksPerSecond) * 1000;
      if (drumset) {
            int flamDuration = 15; //ms
            graceDuration = flamDuration / chordTimeMS * 1000; //ratio 1/1000 from the main note length
            ontime = graceDuration * n;
            }
      else if (n) {
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
                  int graceTimeMS = 65 * n;     // value determined empirically (TODO: make instrument-specific, like articulations)
                  // 1000 occurs below as a unit for ontime
                  ontime = qMin(500, static_cast<int>((graceTimeMS / chordTimeMS) * 1000));
                  }
            else if (chord->dots() == 1)
                  ontime = 667;
            else if (chord->dots() == 2)
                  ontime = 571;
            else
                  ontime = 500;

            graceDuration = ontime / n;
            }

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
      if (unit && !chord->tuplet()) {
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
                        Element* el = seg->element(track);
                        if (el == 0 || el->type() != Element::Type::CHORD)
                              continue;
                        Chord* chord = static_cast<Chord*>(el);
                        createPlayEvents(chord);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   renderMetronome
//---------------------------------------------------------

void Score::renderMetronome(EventMap* events, Measure* m, int tickOffset)
      {
      int msrTick = m->tick();
      qreal tempo = tempomap()->tempo(msrTick);
      TimeSigFrac timeSig = sigmap()->timesig(msrTick).nominal();


      int clickTicks = timeSig.isBeatedCompound(tempo) ? timeSig.beatTicks() : timeSig.dUnitTicks();
      int endTick = m->endTick();

      int rtick;

      if (m->isAnacrusis()) {
            int rem = m->ticks() % clickTicks;
            msrTick += rem;
            rtick = rem + timeSig.ticksPerMeasure() - m->ticks();
            }
      else
            rtick = 0;

      for (int tick = msrTick; tick < endTick; tick += clickTicks, rtick+=clickTicks)
            events->insert(std::pair<int,NPlayEvent>(tick + tickOffset, NPlayEvent(timeSig.rtick2beatType(rtick))));
      }

//---------------------------------------------------------
//   renderMidi
//    export score to event list
//---------------------------------------------------------

void Score::renderMidi(EventMap* events)
      {
      renderMidi(events, true, MScore::playRepeats);
      }

void Score::renderMidi(EventMap* events, bool metronome, bool expandRepeats)
      {
      updateSwing();
      createPlayEvents();

      updateRepeatList(expandRepeats);
      updateChannel();
      updateVelo();

      // create note & other events
      foreach (Staff* part, _staves)
            renderStaff(events, part);
      events->fixupMIDI();

      // create sustain pedal events
      renderSpanners(events);

      if (!metronome)
            return;
      // add metronome ticks
      foreach (const RepeatSegment* rs, *repeatList()) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len();
            int tickOffset = rs->utick - rs->tick;

            //
            //    add metronome tick events
            //
            for (Measure* m = tick2measure(startTick); m; m = m->nextMeasure()) {
                  renderMetronome(events, m, tickOffset);
                  if (m->tick() + m->ticks() >= endTick)
                        break;
                  }
            }
      }
}

