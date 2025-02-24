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

#include "arpeggio.h"
#include "articulation.h"
#include "bend.h"
#include "changeMap.h"
#include "chord.h"
#include "durationtype.h"
#include "dynamic.h"
#include "easeInOut.h"
#include "glissando.h"
#include "hairpin.h"
#include "instrument.h"
#include "measure.h"
#include "navigate.h"
#include "note.h"
#include "noteevent.h"
#include "part.h"
#include "rendermidi.h"
#include "repeat.h"
#include "repeatlist.h"
#include "score.h"
#include "segment.h"
#include "slur.h"
#include "staff.h"
#include "stafftextbase.h"
#include "style.h"
#include "sym.h"
#include "synthesizerstate.h"
#include "tempo.h"
#include "tie.h"
#include "tremolo.h"
#include "trill.h"
#include "undo.h"
#include "utils.h"
#include "vibrato.h"
#include "volta.h"

#include "global/log.h"

#include "audio/midi/event.h"

namespace Ms {

    //int printNoteEventLists(NoteEventList el, int prefix, int j){
    //    int k=0;
    //    for (NoteEvent event : el) {
    //        qDebug("%d: %d: %d pitch=%d ontime=%d duration=%d",prefix, j, k, event.pitch(), event.ontime(), event.len());
    //        k++;
    //    }
    //    return 0;
    //}
    //int printNoteEventLists(QList<NoteEventList> ell, int prefix){
    //    int j=0;
    //    for (NoteEventList el : ell) {
    //        printNoteEventLists(el,prefix,j);
    //        j++;
    //    }
    //    return 0;
    //}

struct SndConfig {
      bool useSND = false;
      int controller = -1;
      DynamicsRenderMethod method = DynamicsRenderMethod::SEG_START;

      SndConfig() {}
      SndConfig(bool use, int c, DynamicsRenderMethod me) : useSND(use), controller(c), method(me) {}
      };

bool graceNotesMerged(Chord *chord);

//---------------------------------------------------------
//   updateSwing
//---------------------------------------------------------

void Score::updateSwing()
      {
      for (Staff* s : qAsConst(_staves)) {
            s->clearSwingList();
            }
      Measure* fm = firstMeasure();
      if (!fm)
            return;
      for (Segment* s = fm->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
            for (const Element* e : s->annotations()) {
                  if (!e->isStaffTextBase())
                        continue;
                  const StaffTextBase* st = toStaffTextBase(e);
                  if (st->xmlText().isEmpty())
                        continue;
                  Staff* staff = st->staff();
                  if (!st->swing())
                        continue;
                  SwingParameters sp;
                  sp.swingRatio = st->swingParameters()->swingRatio;
                  sp.swingUnit = st->swingParameters()->swingUnit;
                  if (st->systemFlag()) {
                        for (Staff* sta : qAsConst(_staves)) {
                              sta->insertIntoSwingList(s->tick(),sp);
                              }
                        }
                  else
                        staff->insertIntoSwingList(s->tick(),sp);
                  }
            }
      }

//---------------------------------------------------------
//   updateCapo
//---------------------------------------------------------

void Score::updateCapo()
      {
      for (Staff* s : qAsConst(_staves)) {
            s->clearCapoList();
            }
      Measure* fm = firstMeasure();
      if (!fm)
            return;
      for (Segment* s = fm->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
            for (Element* e : s->annotations()) {
                  if (e->isHarmony()) {
                        toHarmony(e)->realizedHarmony().setDirty(true);
                        }
                  if (!e->isStaffTextBase())
                        continue;
                  const StaffTextBase* st = toStaffTextBase(e);
                  if (st->xmlText().isEmpty())
                        continue;
                  Staff* staff = st->staff();
                  if (st->capo() == 0)
                        continue;
                  staff->insertIntoCapoList(s->tick(),st->capo());
                  }
            }
      }

//---------------------------------------------------------
//   updateChannel
//---------------------------------------------------------

void Score::updateChannel()
      {
      for (Staff*& s : staves()) {
            for (int i = 0; i < VOICES; ++i)
                  s->clearChannelList(i);
            }
      Measure* fm = firstMeasure();
      if (!fm)
            return;
      for (Segment* s = fm->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
            for (const Element* e : s->annotations()) {
                  if (e->isInstrumentChange()) {
                        for (Staff*& staff : *e->part()->staves()) {
                              for (int voice = 0; voice < VOICES; ++voice)
                                    staff->insertIntoChannelList(voice, s->tick(), 0);
                              }
                        continue;
                        }
                  if (!e->isStaffTextBase())
                        continue;
                  const StaffTextBase* st = toStaffTextBase(e);
                  for (int voice = 0; voice < VOICES; ++voice) {
                        QString an(st->channelName(voice));
                        if (an.isEmpty())
                              continue;
                        Staff* staff = Score::staff(st->staffIdx());
                        int a = staff->part()->instrument(s->tick())->channelIdx(an);
                        if (a != -1)
                              staff->insertIntoChannelList(voice, s->tick(), a);
                        }
                  }
            }

      for (auto it = spanner().cbegin(); it != spanner().cend(); ++it) {
            Spanner* spanner = (*it).second;
            if (!spanner->isVolta())
                  continue;
            Volta* volta = toVolta(spanner);
            volta->setChannel();
            }

      for (Segment* s = fm->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
            for (Staff*& st : staves()) {
                  int strack = st->idx() * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        if (!s->element(track))
                              continue;
                        Element* e = s->element(track);
                        if (e->type() != ElementType::CHORD)
                              continue;
                        Chord* c = toChord(e);
                        int channel = st->channel(c->tick(), c->voice());
                        Instrument* instr = c->part()->instrument(c->tick());
                        if (channel >= instr->channel().size()) {
                              qDebug() << "Channel " << channel << " too high. Max " << instr->channel().size();
                              channel = 0;
                              }
                        for (Note* note : c->notes()) {
                              if (note->hidden())
                                    continue;
                              note->setSubchannel(channel);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   Converts midi time (noteoff - noteon) to milliseconds
//---------------------------------------------------------
int toMilliseconds(float tempo, float midiTime) {
      float ticksPerSecond = (float)MScore::division * tempo;
      int time = (int)((midiTime / ticksPerSecond) * 1000.0f);
      if (time > 0x7fff) //maximum possible value
            time = 0x7fff;
      return time;
      }

//---------------------------------------------------------
//   Detects if a note is a start of a glissando
//---------------------------------------------------------
bool isGlissandoFor(const Note* note) {
      for (Spanner* spanner : note->spannerFor())
            if (spanner->type() == ElementType::GLISSANDO)
                  return true;
      return false;
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
      ev.setNote(note);
      if (offTime < onTime)
            offTime = onTime;
      events->insert(std::pair<int, NPlayEvent>(onTime, ev));
      // adds portamento for continuous glissando
      for (Spanner* spanner : note->spannerFor()) {
            if (spanner->type() == ElementType::GLISSANDO) {
                  Glissando *glissando = toGlissando(spanner);
                  if (glissando->glissandoStyle() == GlissandoStyle::PORTAMENTO) {
                        Note* nextNote = toNote(spanner->endElement());
                        double pitchDelta = (static_cast<double>(nextNote->ppitch()) - pitch) * 50.0;
                        double timeDelta = static_cast<double>(offTime - onTime);
                        if (!qFuzzyIsNull(pitchDelta) && !qFuzzyIsNull(timeDelta)) {
                              double timeStep = std::abs(timeDelta / pitchDelta * 20.0);
                              double t = 0.0;
                              QList<int> onTimes;
                              EaseInOut easeInOut(static_cast<qreal>(glissando->easeIn()) / 100.0,
                                    static_cast<qreal>(glissando->easeOut()) / 100.0);
                              easeInOut.timeList(static_cast<int>((timeDelta + timeStep * 0.5) / timeStep), int(timeDelta), &onTimes);
                              double nTimes = static_cast<double>(onTimes.size() - 1);
                              for (int& time : onTimes) {
                                    int p = static_cast<int>((t / nTimes) * pitchDelta);
                                    int timeStamp = std::min(onTime + time, offTime - 1);
                                    int midiPitch = (p * 16384) / 1200 + 8192;
                                    NPlayEvent evb(ME_PITCHBEND, channel, midiPitch % 128, midiPitch / 128);
                                    evb.setOriginatingStaff(staffIdx);
                                    events->insert(std::pair<int, NPlayEvent>(timeStamp, evb));
                                    t += 1.0;
                                    }
                              ev.setVelo(0);
                              events->insert(std::pair<int, NPlayEvent>(offTime, ev));
                              NPlayEvent evb(ME_PITCHBEND, channel, 0, 64); // 0:64 is 8192 - no pitch bend
                              evb.setOriginatingStaff(staffIdx);
                              events->insert(std::pair<int, NPlayEvent>(offTime, evb));
                              return;
                              }
                        }
                  }
            }

      ev.setVelo(0);
      events->insert(std::pair<int, NPlayEvent>(offTime, ev));
      }

//---------------------------------------------------------
//   collectNote
//---------------------------------------------------------

static void collectNote(EventMap* events, int channel, const Note* note, qreal velocityMultiplier, int tickOffset, Staff* staff, SndConfig config)
      {
      if (!note->play() || note->hidden())      // do not play overlapping notes
            return;
      Chord* chord = note->chord();

      int staffIdx = staff->idx();
      int ticks;
      int tieLen = 0;
      if (chord->isGrace()) {
            Q_ASSERT( !graceNotesMerged(chord)); // this function should not be called on a grace note if grace notes are merged
            chord = toChord(chord->parent());
            }

      ticks = chord->actualTicks().ticks(); // ticks of the actual note
      // calculate additional length due to ties forward
      // taking NoteEvent length adjustments into account
      // but stopping at any note with multiple NoteEvents
      // and processing those notes recursively
      if (note->tieFor()) {
            Note* n = note->tieFor()->endNote();
            while (n) {
                  NoteEventList nel = n->playEvents();
                  if (nel.size() == 1 && !isGlissandoFor(n)) {
                        // add value of this note to main note
                        // if we wish to suppress first note of ornament,
                        // then do this regardless of number of NoteEvents
                        tieLen += (n->chord()->actualTicks().ticks() * (nel[0].len())) / 1000;
                        }
                  else {
                        // recurse
                        collectNote(events, channel, n, velocityMultiplier, tickOffset, staff, config);
                        break;
                        }
                  if (n->tieFor() && n != n->tieFor()->endNote())
                        n = n->tieFor()->endNote();
                  else
                        break;
                  }
            }

      int tick1    = chord->tick().ticks() + tickOffset;
      bool tieFor  = note->tieFor();
      bool tieBack = note->tieBack();

      NoteEventList nel = note->playEvents();
      int nels = nel.size();
      for (int i = 0, pitch = note->ppitch(); i < nels; ++i) {
            const NoteEvent& e = nel[i]; // we make an explicit const ref, not a const copy.  no need to copy as we won't change the original object.

            // skip if note has a tie into it and only one NoteEvent
            // its length was already added to previous note
            // if we wish to suppress first note of ornament
            // then change "nels == 1" to "i == 0", and change "break" to "continue"
            if (tieBack && nels == 1 && !isGlissandoFor(note))
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

            // Get the velocity used for this note from the staff
            // This allows correct playback of tremolos even without SND enabled.
            int velo;
            Fraction nonUnwoundTick = Fraction::fromTicks(on - tickOffset);
            if (config.useSND) {
                  switch (config.method) {
                        case DynamicsRenderMethod::FIXED_MAX:
                              velo = 127;
                              break;
                        case DynamicsRenderMethod::SEG_START:
                        default:
                              velo = staff->velocities().val(nonUnwoundTick);
                              break;
                        }
                  }
            else {
                  velo = staff->velocities().val(nonUnwoundTick);
                  }

            velo *= velocityMultiplier;
            playNote(events, note, channel, p, qBound(1, velo, 127), on, off, staffIdx);
            }

      // Single-note dynamics
      // Find any changes, and apply events
      if (config.useSND) {
            ChangeMap& veloEvents = staff->velocities();
            ChangeMap& multEvents = staff->velocityMultiplications();
            Fraction stick = chord->tick();
            Fraction etick = stick + chord->ticks();
            auto changes = veloEvents.changesInRange(stick, etick);
            auto multChanges = multEvents.changesInRange(stick, etick);

            std::map<int, int> velocityMap;
            for (auto& change : changes) {
                  int lastVal = -1;
                  int endPoint = change.second.ticks();
                  for (int t = change.first.ticks(); t <= endPoint; t++) {
                        int velo = veloEvents.val(Fraction::fromTicks(t));
                        if (velo == lastVal)
                              continue;
                        lastVal = velo;

                        velocityMap[t] = velo;
                        }
                  }


            qreal CONVERSION_FACTOR = MidiRenderer::ARTICULATION_CONV_FACTOR;
            for (auto& change : multChanges) {
                  // Ignore fix events: they are available as cached ramp starts
                  // and considering them ends up with multiplying twice effectively
                  if (change.first == change.second)
                        continue;

                  int lastVal = MidiRenderer::ARTICULATION_CONV_FACTOR;
                  int endPoint = change.second.ticks();
                  int lastVelocity = 0;
                  auto lastValocityIt = velocityMap.upper_bound(change.first.ticks());
                  if (lastValocityIt != velocityMap.end())
                        lastVelocity = lastValocityIt->second;
                  else if (!velocityMap.empty())
                        lastVelocity = velocityMap.cbegin()->second;

                  for (int t = change.first.ticks(); t <= endPoint; t++) {
                        int mult = multEvents.val(Fraction::fromTicks(t));
                        if (mult == lastVal || mult == CONVERSION_FACTOR)
                              continue;
                        lastVal = mult;

                        qreal realMult = mult / CONVERSION_FACTOR;
                        if (velocityMap.find(t) != velocityMap.end()) {
                              lastVelocity = velocityMap[t];
                              velocityMap[t] *= realMult;
                              }
                        else {
                              velocityMap[t] = lastVelocity * realMult;
                              }
                        }
                  }


            for (auto point = velocityMap.cbegin(); point != velocityMap.cend(); ++point) {
                  // NOTE:JT if we ever want to use poly aftertouch instead of CC, this is where we want to
                  // be using it. Instead of ME_CONTROLLER, use ME_POLYAFTER (but duplicate for each note in chord)
                  NPlayEvent event = NPlayEvent(ME_CONTROLLER, channel, config.controller, qBound(0, point->second, 127));
                  event.setOriginatingStaff(staffIdx);
                  events->insert(std::make_pair(point->first + tickOffset, event));
                  }
            }

      // Bends
      for (Element* e : note->el()) {
            if (e == 0 || e->type() != ElementType::BEND)
                  continue;
            Bend* bend = toBend(e);
            if (!bend->playBend())
                  break;
            const QList<PitchValue>& points = bend->points();
            int pitchSize = points.size();

            double noteLen = note->playTicks();
            int lastPointTick = tick1;
            for (int pitchIndex = 0; pitchIndex < pitchSize-1; pitchIndex++) {
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
            events->insert(std::pair<int, NPlayEvent>(tick1+int(noteLen), ev));
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
//   collectProgramChanges
//---------------------------------------------------------

static void collectProgramChanges(EventMap* events, Measure const * m, Staff* staff, int tickOffset)
      {
      int firstStaffIdx = staff->idx();
      int nextStaffIdx  = firstStaffIdx + 1;

      //
      // collect program changes and controller
      //
      for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (Element* e : s->annotations()) {
                  if (!e->isStaffTextBase() || e->staffIdx() < firstStaffIdx || e->staffIdx() >= nextStaffIdx)
                        continue;
                  const StaffTextBase* st1 = toStaffTextBase(e);
                  Fraction tick = s->tick() + Fraction::fromTicks(tickOffset);

                  Instrument* instr = e->part()->instrument(tick);
                  for (const ChannelActions& ca : *st1->channelActions()) {
                        int channel = instr->channel().at(ca.channel)->channel();
                        for (const QString& ma : ca.midiActionNames) {
                              NamedEventList* nel = instr->midiAction(ma, ca.channel);
                              if (!nel)
                                    continue;
                              for (MidiCoreEvent event : nel->events) {
                                    event.setChannel(channel);
                                    NPlayEvent e1(event);
                                    e1.setOriginatingStaff(firstStaffIdx);
                                    if (e1.dataA() == CTRL_PROGRAM)
                                          events->insert(std::pair<int, NPlayEvent>(tick.ticks()-1, e1));
                                    else
                                          events->insert(std::pair<int, NPlayEvent>(tick.ticks(), e1));
                                    }
                              }
                        }
                  if (st1->setAeolusStops()) {
                        Staff* s1 = st1->staff();
                        int voice   = 0;
                        int channel = s1->channel(tick, voice);

                        for (int i = 0; i < 4; ++i) {
                              static int num[4] = { 12, 13, 16, 16 };
                              for (int k = 0; k < num[i]; ++k)
                                    aeolusSetStop(tick.ticks(), channel, i, k, st1->getAeolusStop(i, k), events);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   getControllerFromCC
//---------------------------------------------------------

static int getControllerFromCC(int cc)
      {
      int controller = -1;

      switch (cc) {
            case 1:
                  controller = CTRL_MODULATION;
                  break;
            case 2:
                  controller = CTRL_BREATH;
                  break;
            case 4:
                  controller = CTRL_FOOT;
                  break;
            case 11:
                  controller = CTRL_EXPRESSION;
                  break;
            default:
                  break;
            }

      return controller;
      }

//---------------------------------------------------------
//    renderHarmony
///    renders chord symbols
//---------------------------------------------------------
static void renderHarmony(EventMap* events, Measure const * m, Harmony* h, int tickOffset)
      {
      if (!h->isRealizable())
            return;

      Staff* staff = m->score()->staff(h->track() / VOICES);
      IF_ASSERT_FAILED(staff) {
          return;
      }

      const Channel* channel = staff->part()->harmonyChannel();
      IF_ASSERT_FAILED(channel)
            return;

      events->registerChannel(channel->channel());
      if (!staff->primaryStaff())
            return;

      int staffIdx = staff->idx();
      int velocity = staff->velocities().val(h->tick());

      RealizedHarmony r = h->getRealizedHarmony();
      QList<int> pitches = r.pitches();

      NPlayEvent ev(ME_NOTEON, channel->channel(), 0, velocity);
      ev.setHarmony(h);
      Fraction duration = r.getActualDuration(h->tick().ticks() + tickOffset);

      int onTime = h->tick().ticks() + tickOffset;
      int offTime = onTime + duration.ticks();

      ev.setOriginatingStaff(staffIdx);
      ev.setTuning(0.0);

      //add play events
      for (int p : qAsConst(pitches)) {
            ev.setPitch(p);
            ev.setVelo(velocity);
            events->insert(std::pair<int, NPlayEvent>(onTime, ev));
            ev.setVelo(0);
            events->insert(std::pair<int, NPlayEvent>(offTime, ev));
            }
      }

//---------------------------------------------------------
//   collectMeasureEventsSimple
//    the original, velocity-only method of collecting events.
//---------------------------------------------------------

void MidiRenderer::collectMeasureEventsSimple(EventMap* events, Measure const * m, const StaffContext& sctx, int tickOffset)
      {
      int firstStaffIdx = sctx.staff->idx();
      int nextStaffIdx  = firstStaffIdx + 1;

      SegmentType st = SegmentType::ChordRest;
      int strack = firstStaffIdx * VOICES;
      int etrack = nextStaffIdx * VOICES;

      for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
            int tick = seg->tick().ticks();

            //render harmony
            if (sctx.renderHarmony) {
                  for (Element* e : seg->annotations()) {
                        if (!e || (e->track() < strack) || (e->track() >= etrack))
                              continue;
                        Harmony* h = nullptr;
                        if (e->isHarmony())
                              h = toHarmony(e);
                        else if (e->isFretDiagram())
                              h = toFretDiagram(e)->harmony();
                        if (!h || !h->play())
                              continue;
                        renderHarmony(events, m, h, tickOffset);
                        }
                  }

            for (int track = strack; track < etrack; ++track) {
                  // skip linked staves, except primary
                  if (!m->score()->staff(track / VOICES)->primaryStaff()) {
                        track += VOICES-1;
                        continue;
                        }
                  Element* cr = seg->element(track);
                  if (cr == 0 || cr->type() != ElementType::CHORD)
                        continue;

                  Chord* chord = toChord(cr);
                  Staff* st1   = chord->staff();
                  Instrument* instr = chord->part()->instrument(Fraction::fromTicks(tick));
                  int channel = instr->channel(chord->upNote()->subchannel())->channel();
                  events->registerChannel(channel);

                  qreal veloMultiplier = 1;
                  for (Articulation*& a : chord->articulations()) {
                        if (a->playArticulation()) {
                              veloMultiplier *= instr->getVelocityMultiplier(a->articulationName());
                              }
                        }

                  SndConfig config;       // dummy

                  if (!graceNotesMerged(chord))
                        for (Chord*& c : chord->graceNotesBefore())
                              for (const Note* note : c->notes())
                                    collectNote(events, channel, note, veloMultiplier, tickOffset, st1, config);

                  for (const Note* note : chord->notes())
                        collectNote(events, channel, note, veloMultiplier, tickOffset, st1, config);

                  if (!graceNotesMerged(chord))
                        for (Chord*& c : chord->graceNotesAfter())
                              for (const Note* note : c->notes())
                                    collectNote(events, channel, note, veloMultiplier, tickOffset, st1, config);
                  }
            }
      }

//---------------------------------------------------------
//   collectMeasureEventsDefault
//    this uses only CC events to control note velocity, and sets the
//    note-on velocity to always be 127 (max). This is the method that allows
//    single note dynamics, but only works if the soundfont supports it.
//    Method is one of:
//          FIXED_MAX - default: velocity is fixed at 127
//          SEG_START - note-on velocity is the same as the start velocity of the seg
//---------------------------------------------------------

void MidiRenderer::collectMeasureEventsDefault(EventMap* events, Measure const * m, const StaffContext& sctx, int tickOffset)
      {
      int controller = getControllerFromCC(sctx.cc);

      if (controller == -1) {
            qDebug("controller for CC %d not valid", sctx.cc);
            return;
            }

      int firstStaffIdx = sctx.staff->idx();
      int nextStaffIdx  = firstStaffIdx + 1;

      SegmentType st = SegmentType::ChordRest;
      int strack = firstStaffIdx * VOICES;
      int etrack = nextStaffIdx * VOICES;
      for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
            Fraction tick = seg->tick();

            //render harmony
            if (sctx.renderHarmony) {
                  for (Element* e : seg->annotations()) {
                        if (!e || (e->track() < strack) || (e->track() >= etrack))
                              continue;
                        Harmony* h = nullptr;
                        if (e->isHarmony())
                              h = toHarmony(e);
                        else if (e->isFretDiagram())
                              h = toFretDiagram(e)->harmony();
                        if (!h || !h->play())
                              continue;
                        renderHarmony(events, m, h, tickOffset);
                        }
                  }

            for (int track = strack; track < etrack; ++track) {
                  // Skip linked staves, except primary
                  Staff* st1 = m->score()->staff(track / VOICES);
                  if (!st1->primaryStaff()) {
                        track += VOICES - 1;
                        continue;
                        }

                  Element* cr = seg->element(track);
                  if (!cr)
                        continue;

                  if (!cr->isChord())
                        continue;

                  Chord* chord = toChord(cr);

                  Instrument* instr = st1->part()->instrument(tick);
                  int subchannel = chord->upNote()->subchannel();
                  int channel = instr->channel(subchannel)->channel();

                  events->registerChannel(channel);

                  // Get a velocity multiplier
                  qreal veloMultiplier = 1;
                  for (Articulation*& a : chord->articulations()) {
                        if (a->playArticulation()) {
                              veloMultiplier *= instr->getVelocityMultiplier(a->articulationName());
                              }
                        }

                  bool useSND = instr->singleNoteDynamics();
                  SndConfig config = SndConfig(useSND, controller, sctx.method);

                  //
                  // Add normal note events
                  //

                  if (!graceNotesMerged(chord))
                        for (Chord*& c : chord->graceNotesBefore())
                              for (const Note* note : c->notes())
                                    collectNote(events, channel, note, veloMultiplier, tickOffset, st1, config);

                  for (const Note* note : chord->notes())
                        collectNote(events, channel, note, veloMultiplier, tickOffset, st1, config);

                  if (!graceNotesMerged(chord))
                        for (Chord*& c : chord->graceNotesAfter())
                              for (const Note* note : c->notes())
                                    collectNote(events, channel, note, veloMultiplier, tickOffset, st1, config);
                  }
            }
      }

//---------------------------------------------------------
//   collectMeasureEvents
//    redirects to the correct function based on the passed method
//---------------------------------------------------------

void MidiRenderer::collectMeasureEvents(EventMap* events, Measure const * m, const StaffContext& sctx, int tickOffset)
      {
      switch (sctx.method) {
            case DynamicsRenderMethod::SIMPLE:
                  collectMeasureEventsSimple(events, m, sctx, tickOffset);
                  break;
            case DynamicsRenderMethod::SEG_START:
            case DynamicsRenderMethod::FIXED_MAX:
                  collectMeasureEventsDefault(events, m, sctx, tickOffset);
                  break;
            default:
                  qDebug("Unrecognized dynamics method: %d", int(sctx.method));
                  break;
            }

      collectProgramChanges(events, m, sctx.staff, tickOffset);
      }

//---------------------------------------------------------
//   updateHairpin
//---------------------------------------------------------

void Score::updateHairpin(Hairpin* h)
      {
      Staff* st = h->staff();
      Fraction tick  = h->tick();
      Fraction tick2 = h->tick2();
      int veloChange  = h->veloChange();
      ChangeMethod method = h->veloChangeMethod();

      // Make the change negative when the hairpin is a diminuendo
      HairpinType htype = h->hairpinType();
      ChangeDirection direction = ChangeDirection::INCREASING;
      if (htype == HairpinType::DECRESC_HAIRPIN || htype == HairpinType::DECRESC_LINE) {
            veloChange *= -1;
            direction = ChangeDirection::DECREASING;
            }

      switch (h->dynRange()) {
            case Dynamic::Range::STAFF:
                  st->velocities().addRamp(tick, tick2, veloChange, method, direction);
                  break;
            case Dynamic::Range::PART:
                  for (Staff*& s : *st->part()->staves()) {
                        s->velocities().addRamp(tick, tick2, veloChange, method, direction);
                        }
                  break;
            case Dynamic::Range::SYSTEM:
                  for (Staff*& s : _staves) {
                        s->velocities().addRamp(tick, tick2, veloChange, method, direction);
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

      for (Staff* st : qAsConst(_staves)) {
            st->velocities().clear();
            st->velocityMultiplications().clear();
            }
      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            Staff* st      = staff(staffIdx);
            ChangeMap& velo = st->velocities();
            ChangeMap& mult = st->velocityMultiplications();
            Part* prt      = st->part();
            int partStaves = prt->nstaves();
            int partStaff  = Score::staffIdx(prt);

            for (Segment* s = firstMeasure()->first(); s; s = s->next1()) {
                  Fraction tick = s->tick();
                  for (const Element* e : s->annotations()) {
                        if (e->staffIdx() != staffIdx)
                              continue;
                        if (e->type() != ElementType::DYNAMIC)
                              continue;
                        const Dynamic* d = toDynamic(e);
                        int v            = d->velocity();

                        // treat an invalid dynamic as no change, i.e. a dynamic set to 0
                        if (v < 1)
                              continue;

                        v = qBound(1, v, 127);     //  illegal values

                        // If a dynamic has 'velocity change' update its ending
                        int change = d->changeInVelocity();
                        ChangeDirection direction = ChangeDirection::INCREASING;
                        if (change < 0) {
                              direction = ChangeDirection::DECREASING;
                              }

                        int dStaffIdx = d->staffIdx();
                        switch(d->dynRange()) {
                              case Dynamic::Range::STAFF:
                                    if (dStaffIdx == staffIdx) {
                                          velo.addFixed(tick, v);
                                          if (change != 0) {
                                                Fraction etick = tick + d->velocityChangeLength();
                                                ChangeMethod method = ChangeMethod::NORMAL;
                                                velo.addRamp(tick, etick, change, method, direction);
                                                }
                                          }
                                    break;
                              case Dynamic::Range::PART:
                                    if (dStaffIdx >= partStaff && dStaffIdx < partStaff+partStaves) {
                                          for (int i = partStaff; i < partStaff+partStaves; ++i) {
                                                ChangeMap& stVelo = staff(i)->velocities();
                                                stVelo.addFixed(tick, v);
                                                if (change != 0) {
                                                      Fraction etick = tick + d->velocityChangeLength();
                                                      ChangeMethod method = ChangeMethod::NORMAL;
                                                      stVelo.addRamp(tick, etick, change, method, direction);
                                                      }
                                                }
                                          }
                                    break;
                              case Dynamic::Range::SYSTEM:
                                    for (int i = 0; i < nstaves(); ++i) {
                                          ChangeMap& stVelo = staff(i)->velocities();
                                          stVelo.addFixed(tick, v);
                                          if (change != 0) {
                                                Fraction etick = tick + d->velocityChangeLength();
                                                ChangeMethod method = ChangeMethod::NORMAL;
                                                stVelo.addRamp(tick, etick, change, method, direction);
                                                }
                                          }
                                    break;
                              }
                        }
                  if (s->isChordRestType()) {
                        for (int i = staffIdx * VOICES; i < (staffIdx + 1) * VOICES; ++i) {
                              Element* el = s->element(i);
                              if (!el || !el->isChord())
                                    continue;

                              Chord* chord = toChord(el);
                              Instrument* instr = chord->part()->instrument();

                              qreal veloMultiplier = 1;
                              for (Articulation*& a : chord->articulations()) {
                                    if (a->playArticulation()) {
                                          veloMultiplier *= instr->getVelocityMultiplier(a->articulationName());
                                          }
                                    }

                              if (qFuzzyCompare(veloMultiplier, 1.0))
                                    continue;

                              // TODO this should be a (configurable?) constant somewhere
                              static Fraction ARTICULATION_CHANGE_TIME_MAX = Fraction(1, 16);
                              Fraction ARTICULATION_CHANGE_TIME = qMin(s->ticks(), ARTICULATION_CHANGE_TIME_MAX);
                              int start = veloMultiplier * MidiRenderer::ARTICULATION_CONV_FACTOR;
                              int change = (veloMultiplier - 1) * MidiRenderer::ARTICULATION_CONV_FACTOR;
                              mult.addFixed(chord->tick(), start);
                              mult.addRamp(chord->tick(), chord->tick() + ARTICULATION_CHANGE_TIME, change, ChangeMethod::NORMAL, ChangeDirection::DECREASING);
                              }
                        }
                  }
            for (const auto& sp : _spanner.map()) {
                  Spanner* s = sp.second;
                  if (s->type() != ElementType::HAIRPIN || sp.second->staffIdx() != staffIdx)
                        continue;
                  Hairpin* h = toHairpin(s);
                  updateHairpin(h);
                  }
            }

      for (Staff* st : qAsConst(_staves)) {
            st->velocities().cleanup();
            st->velocityMultiplications().cleanup();
            }

      for (auto it = spanner().cbegin(); it != spanner().cend(); ++it) {
            Spanner* spanner = (*it).second;
            if (!spanner->isVolta())
                  continue;
            Volta* volta = toVolta(spanner);
            volta->setVelocity();
            }
      }

//---------------------------------------------------------
//   renderStaffSegment
//---------------------------------------------------------

void MidiRenderer::renderStaffChunk(const Chunk& chunk, EventMap* events, const StaffContext& sctx)
      {
      Measure const * const start = chunk.startMeasure();
      Measure const * const end = chunk.endMeasure();
      const int tickOffset = chunk.tickOffset();

      Measure const * lastMeasure = start->prevMeasure();

      for (Measure const * m = start; m != end; m = m->nextMeasure()) {
            if (lastMeasure && m->isRepeatMeasure(sctx.staff)) {
                  int offset = (m->tick() - lastMeasure->tick()).ticks();
                  collectMeasureEvents(events, lastMeasure, sctx, tickOffset + offset);
                  }
            else {
                  lastMeasure = m;
                  collectMeasureEvents(events, lastMeasure, sctx, tickOffset);
                  }
            }
      }

//---------------------------------------------------------
//   renderSpanners
//---------------------------------------------------------

void MidiRenderer::renderSpanners(const Chunk& chunk, EventMap* events)
      {
      const int tickOffset = chunk.tickOffset();
      const int tick1 = chunk.tick1();
      const int tick2 = chunk.tick2();

      std::map<int, std::vector<std::pair<int, std::pair<bool, int> > > > channelPedalEvents;
      for (const auto& sp : score->spannerMap().map()) {
            Spanner* s = sp.second;

            int staff = s->staffIdx();
            int idx = s->staff()->channel(s->tick(), 0);
            int channel = s->part()->instrument(s->tick())->channel(idx)->channel();

            if (s->isPedal() || s->isLetRing()) {
                  channelPedalEvents.insert({channel, std::vector<std::pair<int, std::pair<bool, int> > >()});
                  std::vector<std::pair<int, std::pair<bool, int> > > pedalEventList = channelPedalEvents.at(channel);
                  std::pair<int, std::pair<bool, int> > lastEvent;

                  if (!pedalEventList.empty())
                        lastEvent = pedalEventList.back();
                  else
                        lastEvent = std::pair<int, std::pair<bool, int> >(0, std::pair<bool, int>(true, staff));

                  int st = s->tick().ticks();
                  if (st >= tick1 && st < tick2) {
                        // Handle "overlapping" pedal segments (usual case for connected pedal line)
                        if (lastEvent.second.first == false && lastEvent.first >= (st + tickOffset + 2)) {
                              channelPedalEvents.at(channel).pop_back();
                              channelPedalEvents.at(channel).push_back(std::pair<int, std::pair<bool, int> >(st + tickOffset + (2 - MScore::pedalEventsMinTicks), std::pair<bool, int>(false, staff)));
                              }
                        int a = st + tickOffset + (3 - MScore::pedalEventsMinTicks);
                        channelPedalEvents.at(channel).push_back(std::pair<int, std::pair<bool, int> >(a, std::pair<bool, int>(true, staff)));
                        }
                  if (s->tick2().ticks() >= tick1 && s->tick2().ticks() <= tick2) {
                        int t = s->tick2().ticks() + tickOffset + (2 - MScore::pedalEventsMinTicks);
                        const RepeatSegment& lastRepeat = *score->repeatList().back();
                        if (t > lastRepeat.utick + lastRepeat.len())
                              t = lastRepeat.utick + lastRepeat.len();
                        channelPedalEvents.at(channel).push_back(std::pair<int, std::pair<bool, int> >(t, std::pair<bool, int>(false, staff)));
                        }
                  }
            else if (s->isVibrato()) {
                  int stick = s->tick().ticks();
                  int etick = s->tick2().ticks();
                  if (stick >= tick2 || etick < tick1)
                        continue;

                  if (stick < tick1)
                        stick = tick1;
                  if (etick > tick2)
                        etick = tick2;

                  // from start to end of trill, send bend events at regular interval
                  Vibrato* t = toVibrato(s);
                  // guitar vibrato, up only
                  int spitch = 0; // 1/8 (100 is a semitone)
                  int epitch = 12;
                  if (t->vibratoType() == Vibrato::Type::GUITAR_VIBRATO_WIDE) {
                        spitch = 0; // 1/4
                        epitch = 25;
                        }
                  // vibrato with whammy bar up and down
                  else if (t->vibratoType() == Vibrato::Type::VIBRATO_SAWTOOTH_WIDE) {
                        spitch = 25; // 1/16
                        epitch = -25;
                        }
                  else if (t->vibratoType() == Vibrato::Type::VIBRATO_SAWTOOTH) {
                        spitch = 12;
                        epitch = -12;
                        }

                  int j = 0;
                  int delta = MScore::division / 8; // 1/8 note
                  int lastPointTick = stick;
                  while (lastPointTick < etick) {
                        int pitch = (j % 4 < 2) ? spitch : epitch;
                        int nextPitch = ((j+1) % 4 < 2) ? spitch : epitch;
                        int nextPointTick = lastPointTick + delta;
                        for (int i = lastPointTick; i <= nextPointTick; i += 16) {
                              double dx = ((i - lastPointTick) * 60) / delta;
                              int p = pitch + dx * (nextPitch - pitch) / delta;
                              int midiPitch = (p * 16384) / 1200 + 8192;
                              int msb = midiPitch / 128;
                              int lsb = midiPitch % 128;
                              NPlayEvent ev(ME_PITCHBEND, channel, lsb, msb);
                              ev.setOriginatingStaff(staff);
                              events->insert(std::pair<int, NPlayEvent>(i + tickOffset, ev));
                              }
                        lastPointTick = nextPointTick;
                        j++;
                        }
                  NPlayEvent ev(ME_PITCHBEND, channel, 0, 64); // no pitch bend
                  ev.setOriginatingStaff(staff);
                  events->insert(std::pair<int, NPlayEvent>(etick + tickOffset, ev));
                  }
            else
                  continue;
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

//--------------------------------------------------------
//   swingAdjustParams
//--------------------------------------------------------

void Score::swingAdjustParams(Chord* chord, int& gateTime, int& ontime, int swingUnit, int swingRatio)
      {
      Fraction tick = chord->rtick() + chord->measure()->anacrusisOffset();

      int swingBeat           = swingUnit * 2;
      qreal ticksDuration     = (qreal)chord->actualTicks().ticks();
      qreal swingTickAdjust   = ((qreal)swingBeat) * (((qreal)(swingRatio-50))/100.0);
      qreal swingActualAdjust = (swingTickAdjust/ticksDuration) * 1000.0;
      ChordRest *ncr          = nextChordRest(chord);

      //Check the position of the chord to apply changes accordingly
      if (tick.ticks() % swingBeat == swingUnit) {
            if (!isSubdivided(chord,swingUnit)) {
                  ontime = ontime + swingActualAdjust;
                  }
            }
      int endTick = tick.ticks() + ticksDuration;
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
      if (chord->actualTicks().ticks() < swingUnit || (prev && prev->actualTicks().ticks() < swingUnit))
            return true;
      else
            return false;
      }

const Drumset* getDrumset(const Chord* chord)
      {
      if (chord->staff() && chord->staff()->isDrumStaff(chord->tick())) {
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
      int notes = int(chord->notes().size());

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
            if (t == 0) // avoid crash on very short tremolo
                  t = 1;
            SegmentType st = SegmentType::ChordRest;
            Segment* seg2 = seg->next(st);
            int track = chord->track();
            while (seg2 && !seg2->element(track))
                  seg2 = seg2->next(st);

            if (!seg2)
                  return;

            Element* s2El = seg2->element(track);
            if (s2El) {
                  if (!s2El->isChord())
                        return;
                  }
            else
                  return;

            Chord* c2 = toChord(s2El);
            if (c2->type() == ElementType::CHORD) {
                  int notes2 = int(c2->notes().size());
                  int tnotes = qMax(notes, notes2);
                  int tticks = chord->ticks().ticks() * 2; // use twice the size
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
            int n = chord->ticks().ticks() / t;
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
      int notes = int(chord->notes().size());
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

            auto tempoRatio = chord->score()->tempomap()->tempo(chord->tick().ticks()) / Score::defaultTempo();
            int ot = (l * j * 1000) / chord->upNote()->playTicks() *
               tempoRatio * chord->arpeggio()->Stretch();

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
//   articulationExcursion -- an articulation such as a trill, or modant consists of several notes
// played in succession.  The pitch offsets of each such note in the sequence can be represented either
// as a number of steps in the diatonic scale, or in half steps as on a piano keyboard.
// this function, articulationExcursion, takes deltastep indicating the number of steps in the
// diatonic scale, and calculates (and returns) the number of half steps, taking several things into account.
// E.g., the key signature, a trill from e to f, is to be understood as a trill between E and F# if we are
// in the key of G.
// E.g., if previously (looking backward in time) in the same measure there is another note on the same
// staff line/space, and that note has an accidental (sharp,flat,natural,etc), then we want to match that
// tone exactly.
// E.g., If there are multiple notes on the same line/space, then we only consider the most
// recent one, but avoid looking forward in time after the current note.
// E.g., Also if there is an accidental     // on a note one (or more) octaves above or below we
// observe its accidental as well.
// E.g., Still another case is that if two staves are involved (such as a glissando between two
// notes on different staves) then we have to search both staves for the most recent accidental.
//
// noteL is the note to measure the deltastep from, i.e., ornaments are w.r.t. this note
// noteR is the note to search backward from to find accidentals.
//    for ornament calculation noteL and noteR are the same, but for glissando they are
//     the start end end note of glissando.
// deltastep is the desired number of diatonic steps between the base note and this articulation step.
//---------------------------------------------------------

int articulationExcursion(Note *noteL, Note *noteR, int deltastep)
      {
      if (0 == deltastep)
            return 0;
      Chord *chordL = noteL->chord();
      Chord *chordR = noteR->chord();
      int epitchL = noteL->epitch();
      Fraction tickL = chordL->tick();
      // we cannot use staffL = chord->staff() because that won't correspond to the noteL->line()
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
            Element *e = segment->element(track);
            if (!e || e->type() != ElementType::CHORD)
                  continue;
            Chord* chord = toChord(e);
            if (chord->vStaffIdx() != chordL->vStaffIdx())
                  continue;
            for (Note* note : chord->notes()) {
                  if (note->tieBack())
                        continue;
                  int pc = (note->line() + 700) % 7;
                  int pc2 = (lineL2 + 700) % 7;
                  if (pc2 == pc) {
                        // e.g., if there is an F# note at this staff/tick, then force every F to be F#.
                        int octaves = (note->line() - lineL2) / 7;
                        halfsteps = note->epitch() + 12 * octaves - epitchL;
                        done = true;
                        break;
                        }
                  }
            if (!done) {
                  if (staffL->isPitchedStaff(segment->tick())) {
                        bool error = false;
                        AccidentalVal acciv2 = measureR->findAccidental(chordR->segment(), chordR->vStaffIdx(), lineR2, error);
                        int acci2 = int(acciv2);
                        // epitch (effective pitch) is a visible pitch so line2pitch returns exactly that.
                        halfsteps = line2pitch(lineL-deltastep, clefL, Key::C) + acci2 - epitchL;
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
      Fraction total = note->chord()->actualTicks();
      while (note->tieFor() && note->tieFor()->endNote() && (note->chord()->tick() < note->tieFor()->endNote()->chord()->tick())) {
            note = note->tieFor()->endNote();
            total += note->chord()->actualTicks();
            }
      return total.ticks();
      }

//---------------------------------------------------------
//   renderNoteArticulation
// prefix, vector of int, normally something like {0,-1,0,1} modeling the prefix of tremblement relative to the base note
// body, vector of int, normally something like {0,-1,0,1} modeling the possibly repeated tremblement relative to the base note
// tickspernote, number of ticks, either _16h or _32nd, i.e., MScore::division/4 or MScore::division/8
// repeatp, true means repeat the body as many times as possible to fill the time slice.
// sustainp, true means the last note of the body is sustained to fill remaining time slice
//---------------------------------------------------------

bool renderNoteArticulation(NoteEventList* events, Note* note, bool chromatic, int requestedTicksPerNote,
   const std::vector<int>& prefix, const std::vector<int>& body,
   bool repeatp, bool sustainp, const std::vector<int>& suffix,
   int fastestFreq=64, int slowestFreq=8 // 64 Hz and 8 Hz
   )
      {
      events->clear();
      Chord *chord = note->chord();
      int maxticks = totalTiedNoteTicks(note);
      int space = 1000 * maxticks;
      int numrepeat = 1;
      int sustain   = 0;
      int ontime    = 0;

      int gnb = note->chord()->graceNotesBefore().size();
      int p = int(prefix.size());
      int b = int(body.size());
      int s = int(suffix.size());
      int gna = note->chord()->graceNotesAfter().size();

      int ticksPerNote = 0;

      if (gnb + p + b + s + gna <= 0 )
            return false;

      Fraction tick = chord->tick();
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

      ticksPerNote = std::max(requestedTicksPerNote, minTicksPerNote);

      if (slowestFreq <= 0) // no slowest freq given such as something silly like glissando with 4 notes over 8 counts.
            ;
      else if (ticksPerNote <= maxTicksPerNote) // in a good range, so we don't need to adjust ticksPerNote
            ;
      else {
            // for slow tempos, such as adagio, we may need to speed up the tremblement frequency, i.e., decrease the ticks per note, to make it sound reasonable.
            ticksPerNote = requestedTicksPerNote;
            while (ticksPerNote > maxTicksPerNote) {
                  ticksPerNote /= 2;
                  }
            if (ticksPerNote < minTicksPerNote)
                  ticksPerNote = minTicksPerNote;
            }
      // calculate whether to shorten the duration value.
      if ( ticksPerNote*(gnb + p + b + s + gna) <= maxticks )
            ; // plenty of space to play the notes without changing the requested trill note duration
      else if ( ticksPerNote == minTicksPerNote )
            return false; // the ornament is impossible to implement respecting the minimum duration and all the notes it contains
      else {
            ticksPerNote = maxticks / (gnb + p + b + s + gna);  // integer division ignoring remainder
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
      auto tieForward = [millespernote] (int & j, const std::vector<int> & vec) {
            int size = int(vec.size());
            int duration = millespernote;
            while ( j < size-1 && vec[j] == vec[j+1] ) {
                  duration += millespernote;
                  j++;
                  }
            return duration;
            };

      // local function:
      //   append a NoteEvent either by calculating an articulationExcursion or by
      //   the given chromatic relative pitch.
      //   RETURNS the new ontime value.  The caller is expected to assign this value.
      auto makeEvent = [note,chord,chromatic,events] (int pitch, int ontime, int duration) {
            events->append( NoteEvent(chromatic ? pitch : articulationExcursion(note,note,pitch),
               ontime/chord->actualTicks().ticks(),
               duration/chord->actualTicks().ticks()));
            return ontime + duration;
            };

      // local function:
      //    Given a chord from a grace note, (normally the chord contains a single note) and create
      //    a NoteEvent as if the grace note were part of the articulation (such as trill).  This
      //    local function works for the graceNotesBefore() and also graceNotesAfter().
      //    If the grace note has play=false, then it will sound as a rest, but the other grace
      //    notes will still play.  This means graceExtend simply omits the call to append( NoteEvent(...))
      //    but still updates ontime +=millespernote.
      //    RETURNS the new value of ontime, so caller must make an assignment to the return value.
      auto graceExtend = [millespernote,chord,events] (int notePitch, QVector<Chord*> graceNotes, int ontime) {
            for (Chord* c : graceNotes) {
                  for (Note* n : c->notes()) {
                        // NoteEvent takes relative pitch as first argument.
                        // The pitch is relative to the pitch of the note, the event is rendering
                        if (n->play())
                              events->append( NoteEvent(n->pitch() - notePitch,
                                 ontime/chord->actualTicks().ticks(),
                                 millespernote/chord->actualTicks().ticks()));
                        }
                  ontime += millespernote;
                  }
            return ontime;
            };

      // calculate the number of times to repeat the body, and sustain the last note of the body
      // 1000 = P + numrepeat*B+sustain + S
      if (repeatp)
            numrepeat = (space - millespernote*(gnb + p + s + gna)) / (millespernote * b);
      if (sustainp)
            sustain   = space - millespernote*(gnb + p + numrepeat * b + s + gna);
      // render the graceNotesBefore
      ontime = graceExtend(note->pitch(),note->chord()->graceNotesBefore(), ontime);

      // render the prefix
      for (int j=0; j < p; j++)
            ontime = makeEvent(prefix[j], ontime, tieForward(j,prefix));

      if (b > 0) {
            // Check that we are doing a glissando
            bool isGlissando = false;
            QList<int> onTimes;
            for (Spanner* spanner : note->spannerFor()) {
                  if (spanner->type() == ElementType::GLISSANDO) {
                        Glissando* glissando = toGlissando(spanner);
                        EaseInOut easeInOut(static_cast<qreal>(glissando->easeIn())/100.0,
                              static_cast<qreal>(glissando->easeOut())/100.0);
                        easeInOut.timeList(b, millespernote * b, &onTimes);
                        isGlissando = true;
                        break;
                        }
                  }
            if (isGlissando) {
                  // render the body, i.e. the glissando
                  for (int j = 0; j < b - 1; j++)
                        makeEvent(body[j], onTimes[j], onTimes[j + 1] - onTimes[j]);
                  makeEvent(body[b - 1], onTimes[b-1], (millespernote * b - onTimes[b-1]) + sustain);
                  }
            else {
                  // render the body, but not the final repetition
                  for (int r = 0; r < numrepeat - 1; r++) {
                        for (int j = 0; j < b; j++)
                              ontime = makeEvent(body[j], ontime, millespernote);
                        }
                  // render the final repetition of body, but not the final note of the repetition
                  for (int j = 0; j < b - 1; j++)
                        ontime = makeEvent(body[j], ontime, millespernote);
                  // render the final note of the final repeat of body
                  ontime = makeEvent(body[b - 1], ontime, millespernote + sustain);
                  }
            }
      // render the suffix
      for (int j = 0; j < s; j++)
            ontime = makeEvent(suffix[j], ontime, tieForward(j,suffix));
      // render graceNotesAfter
      graceExtend(note->pitch(), note->chord()->graceNotesAfter(), ontime);
      return true;
      }

// This struct specifies how to render an articulation.
//   atype - the articulation type to implement, such as SymId::ornamentTurn
//   ostyles - the actual ornament has a property called ornamentStyle whose value is
//             a value of type MScore::OrnamentStyle.  This ostyles field indicates the
//             the set of ornamentStyles which apply to this rendition.
//   duration - the default duration for each note in the rendition, the final duration
//            rendered might be less than this if an articulation is attached to a note of
//            short duration.
//   prefix - vector of integers. indicating which notes to play at the beginning of rendering the
//            articulation.  0 represents the principle note, 1==> the note diatonically 1 above
//            -1 ==> the note diatonically 1 below.  E.g., in the key of G, if a turn articulation
//            occurs above the note F#, then 0==>F#, 1==>G, -1==>E.
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
      SymId atype;
      std::set<MScore::OrnamentStyle> ostyles;
      int duration;
      std::vector<int> prefix;
      std::vector<int> body;
      bool repeatp;
      bool sustainp;
      std::vector<int> suffix;
      };

std::set<MScore::OrnamentStyle> baroque  = {MScore::OrnamentStyle::BAROQUE};
std::set<MScore::OrnamentStyle> defstyle = {MScore::OrnamentStyle::DEFAULT};
std::set<MScore::OrnamentStyle> any; // empty set has the special meaning of any-style, rather than no-styles.
int _16th = MScore::division / 4;
int _32nd = _16th / 2;

std::vector<OrnamentExcursion> excursions = {
      //  articulation type            set of  duration       body         repeatp      suffix
      //                               styles          prefix                    sustainp
      { SymId::ornamentTurn,                any, _32nd, {},    {1,0,-1,0},   false, true, {}}
      ,{SymId::ornamentTurnInverted,        any, _32nd, {},    {-1,0,1,0},   false, true, {}}
      ,{SymId::ornamentTurnSlash,           any, _32nd, {},    {-1,0,1,0},   false, true, {}}
      ,{SymId::ornamentTrill,           baroque, _32nd, {1,0}, {1,0},        true,  true, {}}
      ,{SymId::ornamentTrill,          defstyle, _32nd, {0,1}, {0,1},        true,  true, {}}
      ,{SymId::brassMuteClosed,         baroque, _32nd, {0,-1},{0, -1},      true,  true, {}}
      ,{SymId::brassMuteClosed,        defstyle, _32nd, {},    {0},          false, true, {}} // regular hand-stopped brass
      ,{SymId::ornamentMordent,             any, _32nd, {},    {0,-1,0},     false, true, {}}
      ,{SymId::ornamentShortTrill,     defstyle, _32nd, {},    {0,1,0},      false, true, {}} // inverted mordent
      ,{SymId::ornamentShortTrill,      baroque, _32nd, {1,0,1},{0},         false, true, {}} // short trill
      ,{SymId::ornamentTremblement,         any, _32nd, {1,0}, {1,0},        false, true, {}}
      ,{SymId::ornamentPrallMordent,        any, _32nd, {},    {1,0,-1,0},   false, true, {}}
      ,{SymId::ornamentLinePrall,           any, _32nd, {2,2,2},{1,0},       true,  true, {}}
      ,{SymId::ornamentUpPrall,             any, _16th, {-1,0},{1,0},        true,  true, {1,0}} // p 144 Ex 152 [1]
      ,{SymId::ornamentUpMordent,           any, _16th, {-1,0},{1,0},        true,  true, {-1,0}} // p 144 Ex 152 [1]

      ,{SymId::ornamentPrecompMordentUpperPrefix, any, _16th, {1,1,1,0}, {1,0},    true,  true, {}} // p136 Cadence Appuyee [1] [2]
      ,{SymId::ornamentDownMordent,         any, _16th, {1,1,1,0}, {1,0},    true,  true, {-1, 0}} // p136 Cadence Appuyee + mordent [1] [2]
      ,{SymId::ornamentPrallUp,             any, _16th, {1,0}, {1,0},        true,  true, {-1,0}} // p136 Double Cadence [1]
      ,{SymId::ornamentPrallDown,           any, _16th, {1,0}, {1,0},        true,  true, {-1,0,0,0}} // p144 ex 153 [1]
      ,{SymId::ornamentPrecompSlide,        any, _32nd, {},    {0},          false, true, {}}

      ,{SymId::ornamentShake3,              any, _32nd, {1,0}, {1,0},        true,  true, {}}
      ,{SymId::ornamentShakeMuffat1,        any, _32nd, {1,0}, {1,0},        true,  true, {}}

      ,{ SymId::ornamentTremblementCouperin,any, _32nd, { 1, 1 }, { 0, 1 },  true, true, { 0, 0 } }
      ,{ SymId::ornamentPinceCouperin,      any, _32nd, { 0 },    { 0, -1 }, true, true, { 0, 0 } }

      // [1] Some of the articulations/ornaments in the excursions table above come from
      // Baroque Music, Style and Performance A Handbook, by Robert Donington,(c) 1982
      // ISBN 0-393-30052-8, W. W. Norton & Company, Inc.

      // [2] In some cases, the example from [1] does not preserve the timing.
      // For example, illustrates 2+1/4 counts per half note.
      };

//---------------------------------------------------------
//   renderNoteArticulation
//---------------------------------------------------------

bool renderNoteArticulation(NoteEventList* events, Note * note, bool chromatic, SymId articulationType, MScore::OrnamentStyle ornamentStyle)
      {
      if (!note->staff()->isPitchedStaff(note->tick())) // not enough info in tab staff
            return false;

      std::vector<int> emptypattern = {};
      for (auto& oe : excursions) {
            if (oe.atype == articulationType && (0 == oe.ostyles.size()
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
      std::map<Trill::Type,SymId> articulationMap = {
             {Trill::Type::TRILL_LINE,      SymId::ornamentTrill      }
            ,{Trill::Type::UPPRALL_LINE,    SymId::ornamentUpPrall    }
            ,{Trill::Type::DOWNPRALL_LINE,  SymId::ornamentPrecompMordentUpperPrefix  }
            ,{Trill::Type::PRALLPRALL_LINE, SymId::ornamentTrill      }
            };
      auto it = articulationMap.find(trillType);
      if (it == articulationMap.cend())
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
            if ((spanner->type() == ElementType::GLISSANDO)
               && spanner->endElement()
               && (ElementType::NOTE == spanner->endElement()->type()))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   glissandoPitchOffsets
//---------------------------------------------------------

bool glissandoPitchOffsets(const Spanner* spanner, std::vector<int>& pitchOffsets)
{
      if (!spanner->endElement()->isNote())
            return false;
      const Glissando* glissando = toGlissando(spanner);
      if (!glissando->playGlissando())
            return false;
      GlissandoStyle glissandoStyle = glissando->glissandoStyle();
      if (glissandoStyle == GlissandoStyle::PORTAMENTO)
            return false;
      // only consider glissando connected to NOTE.
      Note* noteStart = toNote(spanner->startElement());
      Note* noteEnd = toNote(spanner->endElement());
      int pitchStart = noteStart->ppitch();
      int pitchEnd = noteEnd->ppitch();
      if (pitchEnd == pitchStart)
            return false;
      int direction = pitchEnd > pitchStart ? 1 : -1;
      pitchOffsets.clear();
      if (glissandoStyle == GlissandoStyle::DIATONIC) {
            int lineStart = noteStart->line();
            // scale obeying accidentals
            for (int line = lineStart, pitch = pitchStart; (direction == 1) ? (pitch < pitchEnd) : (pitch > pitchEnd); line -= direction) {
                  int halfSteps = articulationExcursion(noteStart, noteEnd, lineStart - line);
                  pitch = pitchStart + halfSteps;
                  if ((direction == 1) ? (pitch < pitchEnd) : (pitch > pitchEnd))
                        pitchOffsets.push_back(halfSteps);
                  }
            return pitchOffsets.size() > 0;
            }
      if (glissandoStyle == GlissandoStyle::CHROMATIC) {
            for (int pitch = pitchStart; pitch != pitchEnd; pitch += direction)
                  pitchOffsets.push_back(pitch - pitchStart);
            return true;
            }
      static std::vector<bool> whiteNotes = { true, false, true, false, true, true, false, true, false, true, false, true };
      int Cnote = 60; // pitch of middle C
      bool notePick = glissandoStyle == GlissandoStyle::WHITE_KEYS;
      for (int pitch = pitchStart; pitch != pitchEnd; pitch += direction) {
            int idx = ((pitch - Cnote) + 1200) % 12;
            if (whiteNotes[idx] == notePick)
                  pitchOffsets.push_back(pitch - pitchStart);
            }
      return true;
}

//---------------------------------------------------------
//   renderGlissando
//---------------------------------------------------------

void renderGlissando(NoteEventList* events, Note *notestart)
      {
      std::vector<int> empty = {};
      std::vector<int> body;
      for (Spanner* spanner : notestart->spannerFor()) {
            if (spanner->type() == ElementType::GLISSANDO
            && toGlissando(spanner)->playGlissando()
            && glissandoPitchOffsets(spanner, body))
                  renderNoteArticulation(events, notestart, true, MScore::division, empty, body, false, true, empty, 16, 0);
            }
      }



//---------------------------------------------------------
// findFirstTrill
//  search the spanners in the score, finding the first one
//  which overlaps this chord and is of type ElementType::TRILL
//---------------------------------------------------------

Trill* findFirstTrill(Chord *chord)
      {
      auto spanners = chord->score()->spannerMap().findOverlapping(1+chord->tick().ticks(),
         chord->tick().ticks() + chord->actualTicks().ticks() - 1);
      for (auto i : spanners) {
            if (i.value->type() != ElementType::TRILL)
                  continue;
            if (i.value->track() != chord->track())
                  continue;
            Trill *trill = toTrill (i.value);
            if (trill->playArticulation() == false)
                  continue;
            return trill;
            }
      return nullptr;
      }

// In the case that graceNotesBefore or graceNotesAfter are attached to a note
// with an articulation such as a trill, then the grace notes are/will-be/have-been
// already merged into the articulation.
// So this predicate, graceNotesMerged, checks for this condition to avoid calling
// functions which would re-emit the grace notes by a different algorithm.

bool graceNotesMerged(Chord* chord)
      {
      if (findFirstTrill(chord))
            return true;
      for (Articulation*& a : chord->articulations())
            for (auto& oe : excursions)
                  if ( oe.atype == a->symId() )
                        return true;
      return false;
      }

//---------------------------------------------------------
//   renderChordArticulation
//---------------------------------------------------------

void renderChordArticulation(Chord* chord, QList<NoteEventList> & ell, int & gateTime)
      {
      Segment* seg = chord->segment();
      Instrument* instr = chord->part()->instrument(seg->tick());
      int channel  = 0;  // note->subchannel();

      for (unsigned k = 0; k < chord->notes().size(); ++k) {
            NoteEventList* events = &ell[k];
            Note *note = chord->notes()[k];
            Trill *trill;

            if (noteHasGlissando(note))
                  renderGlissando(events, note);
            else if (chord->staff()->isPitchedStaff(chord->tick())  && (trill = findFirstTrill(chord)) != nullptr) {
                  renderNoteArticulation(events, note, false, trill->trillType(), trill->ornamentStyle());
                  }
            else {
                  for (Articulation*& a : chord->articulations()) {
                        if (!a->playArticulation())
                              continue;
                        if (!renderNoteArticulation(events, note, false, a->symId(), a->ornamentStyle()))
                              instr->updateGateTime(&gateTime, channel, a->articulationName());
                        }
                  }
            }
      }

//---------------------------------------------------------
//   shouldRenderNote
//---------------------------------------------------------

static bool shouldRenderNote(Note* n)
      {
      while (n->tieBack() && n != n->tieBack()->startNote()) {
            n = n->tieBack()->startNote();
            if (findFirstTrill(n->chord()))
                  // The previous tied note probably has events for this note too.
                  // That is, we don't need to render this note separately.
                  return false;
            for (Articulation*& a : n->chord()->articulations()) {
                  if (a->isOrnament()) {
                        return false;
                        }
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   renderChord
//    ontime and trailtime in 1/1000 of duration
//    ontime signifies how much gap to leave, i.e., how late the note should start because of graceNotesBefore which have already been rendered
//    trailtime signifies how much gap to leave after the note to allow for graceNotesAfter to be rendered
//---------------------------------------------------------

static QList<NoteEventList> renderChord(Chord* chord, int gateTime, int ontime, int trailtime)
      {
      QList<NoteEventList> ell;
      if (chord->notes().empty())
            return ell;

      size_t notes = chord->notes().size();
      for (size_t i = 0; i < notes; ++i)
            ell.append(NoteEventList());

      bool arpeggio = false;
      if (chord->tremolo()) {
            renderTremolo(chord, ell);
            }
      else if (chord->arpeggio() && chord->arpeggio()->playArpeggio()) {
            renderArpeggio(chord, ell);
            arpeggio = true;
            }
      else
            renderChordArticulation(chord, ell, gateTime);

      // Check each note and apply gateTime
      for (unsigned i = 0; i < notes; ++i) {
            NoteEventList* el = &ell[i];
            if (!shouldRenderNote(chord->notes()[i])) {
                  el->clear();
                  continue;
                  }
            if (arpeggio)
                  continue; // don't add extra events and apply gateTime to arpeggio

            // If we are here then we still need to render the note.
            // Render its body if necessary and apply gateTime.
            if (el->size() == 0 && chord->tremoloChordType() != TremoloChordType::TremoloSecondNote) {
                  el->append(NoteEvent(0, ontime, 1000 - ontime - trailtime));
                  }
            if (trailtime == 0) // if trailtime is non-zero that means we have graceNotesAfter, so we don't need additional gate time.
                  for (NoteEvent& e : ell[i])
                        e.setLen(e.len() * gateTime / 100);
            }
      return ell;
      }

//---------------------------------------------------------
//   createGraceNotesPlayEvent
// as a side effect of createGraceNotesPlayEvents, ontime and trailtime (passed by ref)
// are modified.  ontime reflects the time needed to play the grace-notes-before, and
// trailtime reflects the time for the grace-notes-after.  These are used by the caller
// to effect the on/off time of the main note
//---------------------------------------------------------

void Score::createGraceNotesPlayEvents(const Fraction& tick, Chord* chord, int& ontime, int& trailtime)
      {
      QVector<Chord*> gnb = chord->graceNotesBefore();
      QVector<Chord*> gna = chord->graceNotesAfter();
      int nb = gnb.size();
      int na = gna.size();
      if (0 == nb + na) {
            return; // return immediately if no grace notes to deal with
            }
      // return immediately if the chord has a trill or articulation which effectively plays the graces notes.
      if (graceNotesMerged(chord)) {
            return;
            }
      // if there are graceNotesBefore and also graceNotesAfter, and the before grace notes are
      // not ACCIACCATURA, then the total time of all of them will be 50% of the time of the main note.
      // if the before grace notes are ACCIACCATURA then the grace notes after (if there are any).
      // get 50% of the time of the main note.
      // this is achieved by the two floating point weights: weighta and weightb whose total is 1.0
      // assuring that all the grace notes get the same duration, and their total is 50%.
      // exception is if the note is dotted or double-dotted; see below.
      float weighta = float(na) / (nb+na);
      float weightb = float(nb) / (nb+na);

      int graceDuration = 0;
      bool drumset = (getDrumset(chord) != nullptr);
      const qreal ticksPerSecond = tempo(tick) * MScore::division;
      const qreal chordTimeMS = (chord->actualTicks().ticks() / ticksPerSecond) * 1000;
      if (drumset) {
            int flamDuration = 15; //ms
            graceDuration = flamDuration / chordTimeMS * 1000; //ratio 1/1000 from the main note length
            ontime = graceDuration * nb;
            }
      else if (nb) {
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
            if (graceChord->noteType() ==  NoteType::ACCIACCATURA || nb > 1) { // treat multiple subsequent grace notes as acciaccaturas
                  int graceTimeMS = 65 * nb;     // value determined empirically (TODO: make instrument-specific, like articulations)
                  // 1000 occurs below as a unit for ontime
                  ontime = qMin(500, static_cast<int>((graceTimeMS / chordTimeMS) * 1000));
                  weightb = 0.0;
                  weighta = 1.0;
                  }
            else if (chord->dots() == 1)
                  ontime = floor(667 * weightb);
            else if (chord->dots() == 2)
                  ontime = floor(571 * weightb);
            else
                  ontime = floor(500 * weightb);

            graceDuration = ontime / nb;
            }

      for (int i = 0, on = 0; i < nb; ++i) {
            QList<NoteEventList> el;
            Chord* gc = gnb.at(i);
            size_t nn = gc->notes().size();
            for (unsigned ii = 0; ii < nn; ++ii) {
                  NoteEventList nel;
                  nel.append(NoteEvent(0, on, graceDuration));
                  el.append(nel);
                  }

            if (gc->playEventType() == PlayEventType::Auto)
                  gc->setNoteEventLists(el);
            on += graceDuration;
            }
      if (na) {
            if (chord->dots() == 1)
                  trailtime = floor(667 * weighta);
            else if (chord->dots() == 2)
                  trailtime = floor(571 * weighta);
            else
                  trailtime = floor(500 * weighta);
            int graceDuration1 = trailtime / na;
            int on = 1000 - trailtime;
            for (int i = 0; i < na; ++i) {
                  QList<NoteEventList> el;
                  Chord* gc = gna.at(i);
                  size_t nn = gc->notes().size();
                  for (size_t ii = 0; ii < nn; ++ii) {
                        NoteEventList nel;
                        nel.append(NoteEvent(0, on, graceDuration1)); // NoteEvent(pitch,ontime,len)
                        el.append(nel);
                        }

                  if (gc->playEventType() == PlayEventType::Auto)
                        gc->setNoteEventLists(el);
                  on += graceDuration1;
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

      Fraction tick = chord->tick();
      Slur* slur = 0;
      for (auto sp : _spanner.map()) {
            if (!sp.second->isSlur() || sp.second->staffIdx() != chord->staffIdx())
                  continue;
            Slur* s = toSlur(sp.second);
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

      int ontime    = 0;
      int trailtime = 0;
      createGraceNotesPlayEvents(tick, chord, ontime, trailtime); // ontime and trailtime are modified by this call depending on grace notes before and after

      SwingParameters st = chord->staff()->swing(tick);
      int unit           = st.swingUnit;
      int ratio          = st.swingRatio;
      // Check if swing needs to be applied
      if (unit && !chord->tuplet()) {
            swingAdjustParams(chord, gateTime, ontime, unit, ratio);
            }
      //
      //    render normal (and articulated) chords
      //
      QList<NoteEventList> el = renderChord(chord, gateTime, ontime, trailtime);
      if (chord->playEventType() == PlayEventType::Auto)
            chord->setNoteEventLists(el);
      // don't change event list if type is PlayEventType::User
      }

void Score::createPlayEvents(Measure const * start, Measure const * const end)
      {
      if (!start)
            start = firstMeasure();

      int etrack = nstaves() * VOICES;
      for (int track = 0; track < etrack; ++track) {
            bool rangeEnded = false;
            for (Measure const * m = start; m; m = m->nextMeasure()) {
                  constexpr SegmentType st = SegmentType::ChordRest;

                  if (m == end)
                        rangeEnded = true;
                  if (rangeEnded) {
                        // The range has ended, but we should collect events
                        // for tied notes. So we'll check if this is the case.
                        const Segment* seg = m->first(st);
                        const Element* e = seg->element(track);
                        bool tie = false;
                        if (e && e->isChord()) {
                              for (const Note* n : toChord(e)->notes()) {
                                    if (n->tieBack()) {
                                          tie = true;
                                          break;
                                          }
                                    }
                              }
                        if (!tie)
                              break;
                        }

                  // skip linked staves, except primary
                  if (!m->score()->staff(track / VOICES)->primaryStaff())
                        continue;
                  for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
                        Element* e = seg->element(track);
                        if (e == 0 || !e->isChord())
                              continue;
                        createPlayEvents(toChord(e));
                        }
                  }
            }
      }

//---------------------------------------------------------
//   renderMetronome
///   add metronome tick events
//---------------------------------------------------------

void MidiRenderer::renderMetronome(const Chunk& chunk, EventMap* events)
      {
      const int tickOffset = chunk.tickOffset();
      Measure const * const start = chunk.startMeasure();
      Measure const * const end = chunk.endMeasure();

      for (Measure const * m = start; m != end; m = m->nextMeasure())
            renderMetronome(events, m, Fraction::fromTicks(tickOffset));
      }

//---------------------------------------------------------
//   renderMetronome
///   add metronome tick events
//---------------------------------------------------------

void MidiRenderer::renderMetronome(EventMap* events, Measure const * m, const Fraction& tickOffset)
      {
      int msrTick         = m->tick().ticks();
      qreal tempo         = score->tempomap()->tempo(msrTick);
      TimeSigFrac timeSig = score->sigmap()->timesig(msrTick).nominal();

      int clickTicks      = timeSig.isBeatedCompound(tempo) ? timeSig.beatTicks() : timeSig.dUnitTicks();
      int endTick         = m->endTick().ticks();

      int rtick;

      if (m->isAnacrusis()) {
            int rem = m->ticks().ticks() % clickTicks;
            msrTick += rem;
            rtick = rem + timeSig.ticksPerMeasure() - m->ticks().ticks();
            }
      else
            rtick = 0;

      for (int tick = msrTick; tick < endTick; tick += clickTicks, rtick += clickTicks)
            events->insert(std::pair<int,NPlayEvent>(tick + tickOffset.ticks(), NPlayEvent(timeSig.rtick2beatType(rtick))));
      }

//---------------------------------------------------------
//   renderMidi
//    export score to event list
//---------------------------------------------------------

void Score::renderMidi(EventMap* events, const SynthesizerState& synthState)
      {
      renderMidi(events, true, MScore::playRepeats, synthState);
      }

void Score::renderMidi(EventMap* events, bool metronome, bool expandRepeats, const SynthesizerState& synthState)
      {
      bool expandRepeatsBackup = masterScore()->expandRepeats();
      masterScore()->setExpandRepeats(expandRepeats);
      MidiRenderer::Context ctx(synthState);
      ctx.metronome = metronome;
      ctx.renderHarmony = true;
      MidiRenderer(this).renderScore(events, ctx);
      masterScore()->setExpandRepeats(expandRepeatsBackup);
      }

void MidiRenderer::renderScore(EventMap* events, const Context& ctx)
      {
      updateState();
      for (const Chunk& chunk : chunks) {
            renderChunk(chunk, events, ctx);
            }
      }

void MidiRenderer::renderChunk(const Chunk& chunk, EventMap* events, const Context& ctx)
      {
      // TODO: avoid doing it multiple times for the same measures
      score->createPlayEvents(chunk.startMeasure(), chunk.endMeasure());

      score->updateChannel();
      score->updateVelo();

      SynthesizerState s = score->synthesizerState();
      int method = s.method();
      int cc = s.ccToUse();

      // check if the score synth settings are actually set
      // if not, use the global synth state
      if (method == -1) {
            method = ctx.synthState.method();
            cc = ctx.synthState.ccToUse();

            if (method == -1) {
                  // fall back to defaults - this may be needed to pass tests,
                  // since sometimes the synth state is not init
                  method = 1;
                  cc = 2;
                  qDebug("Had to fall back to defaults to render measure");
                  }
            }

      DynamicsRenderMethod renderMethod = DynamicsRenderMethod::SIMPLE;
      switch (method) {
            case 0:
                  renderMethod = DynamicsRenderMethod::SIMPLE;
                  break;
            case 1:
                  renderMethod = DynamicsRenderMethod::SEG_START;
                  break;
            case 2:
                  renderMethod = DynamicsRenderMethod::FIXED_MAX;
                  break;
            default:
                  qDebug("Unrecognized dynamics method: %d", method);
                  break;
            }

      // create note & other events
      for (Staff*& st : score->staves()) {
            StaffContext sctx;
            sctx.staff = st;
            sctx.method = renderMethod;
            sctx.cc = cc;
            sctx.renderHarmony = ctx.renderHarmony;
            renderStaffChunk(chunk, events, sctx);
            }
      events->fixupMIDI();

      // create sustain pedal events
      renderSpanners(chunk, events);

      if (ctx.metronome)
            renderMetronome(chunk, events);

      // NOTE:JT this is a temporary fix for duplicate events until polyphonic aftertouch support
      // can be implemented. This removes duplicate SND events.
      int lastChannel = -1;
      int lastController = -1;
      int lastValue = -1;
      for (auto i = events->begin(); i != events->end();) {
            if (i->second.type() == ME_CONTROLLER) {
                  auto& event = i->second;
                  if (event.channel() == lastChannel &&
                     event.controller() == lastController &&
                     event.value() == lastValue) {
                        i = events->erase(i);
                        }
                  else {
                        lastChannel = event.channel();
                        lastController = event.controller();
                        lastValue = event.value();
                        i++;
                        }
                  }
            else {
                  i++;
                  }
            }
      }

//---------------------------------------------------------
//   MidiRenderer::updateState
//---------------------------------------------------------

void MidiRenderer::updateState()
      {
      if (needUpdate) {
            // Update the related structures inside score
            // to avoid doing it multiple times on chunks rendering
            score->updateSwing();
            score->updateCapo();

            updateChunksPartition();

            needUpdate = false;
            }
      }

//---------------------------------------------------------
//   MidiRenderer::canBreakChunk
///   Helper function for updateChunksPartition
///   Determines whether it is allowed to break MIDI
///   rendering chunk at given measure.
//---------------------------------------------------------

bool MidiRenderer::canBreakChunk(const Measure* last)
      {
      Score* score = last->score();

      // Check for hairpins that overlap measure end:
      // hairpins should be inside one chunk, if possible
      const int endTick = last->endTick().ticks();
      const auto& spanners = score->spannerMap().findOverlapping(endTick - 1, endTick);
      for (const auto& interval : spanners) {
            const Spanner* sp = interval.value;
            if (sp->isHairpin() && sp->tick2().ticks() > endTick)
                  return false;
            }

      // Repeat measures rely on the previous measure
      // being properly rendered, disallow breaking
      // chunk at repeat measure.
      if (const Measure* next = last->nextMeasure())
            for (Staff*& staff : score->staves()) {
                  if (next->isRepeatMeasure(staff))
                        return false;
                  }

      return true;
      }

//---------------------------------------------------------
//   MidiRenderer::updateChunksPartition
//---------------------------------------------------------

void MidiRenderer::updateChunksPartition()
      {
      chunks.clear();

      const RepeatList& repeatList = score->repeatList();

      for (const RepeatSegment* rs : repeatList) {
            const int tickOffset = rs->utick - rs->tick;

            if (!minChunkSize) {
                  // just make chunks corresponding to repeat segments
                  chunks.emplace_back(tickOffset, rs->firstMeasure(), rs->lastMeasure());
                  continue;
                  }

            Measure const * const end = rs->lastMeasure()->nextMeasure();
            int count = 0;
            bool needBreak = false;
            Measure const * chunkStart = nullptr;
            for (Measure const * m = rs->firstMeasure(); m != end; m = m->nextMeasure()) {
                  if (!chunkStart)
                        chunkStart = m;
                  if ((++count) >= minChunkSize)
                        needBreak = true;
                  if (needBreak && canBreakChunk(m)) {
                        chunks.emplace_back(tickOffset, chunkStart, m);
                        chunkStart = nullptr;
                        needBreak = false;
                        count = 0;
                        }
                  }
            if (chunkStart) // last measures did not get added to chunk list
                  chunks.emplace_back(tickOffset, chunkStart, rs->lastMeasure());
            }

      if (score != repeatList.score()) {
            // Repeat list may belong to another linked score (e.g. MasterScore).
            // Update chunks to make them contain measures from the currently
            // rendered score.
            for (Chunk& ch : chunks) {
                  Measure* first = score->tick2measure(ch.startMeasure()->tick());
                  Measure* last = score->tick2measure(ch.lastMeasure()->tick());
                  ch = Chunk(ch.tickOffset(), first, last);
                  }
            }
      }

//---------------------------------------------------------
//   MidiRenderer::getChunkAt
//---------------------------------------------------------

MidiRenderer::Chunk MidiRenderer::getChunkAt(int utick)
      {
      updateState();

      auto it = std::upper_bound(chunks.begin(), chunks.end(), utick, [](int utick, const Chunk& ch) {
                  return utick < ch.utick1();
                  });
      if (it == chunks.begin())
            return Chunk();
      --it;
      const Chunk& ch = *it;
      if (ch.utick2() <= utick)
            return Chunk();
      return ch;
      }

//---------------------------------------------------------
//   RangeMap::setOccupied
//---------------------------------------------------------

void RangeMap::setOccupied(int tick1, int tick2)
      {
      auto it1 = status.upper_bound(tick1);
      const bool beforeBegin = (it1 == status.begin());
      if (beforeBegin || (--it1)->second != Range::BEGIN) {
            if (!beforeBegin && it1->first == tick1)
                  status.erase(it1);
            else
                  status.insert(std::make_pair(tick1, Range::BEGIN));
            }

      const auto it2 = status.lower_bound(tick2);
      const bool afterEnd = (it2 == status.end());
      if (afterEnd || it2->second != Range::END) {
            if (!afterEnd && it2->first == tick2)
                  status.erase(it2);
            else
                  status.insert(std::make_pair(tick2, Range::END));
            }
      }

//---------------------------------------------------------
//   RangeMap::occupiedRangeEnd
//---------------------------------------------------------

int RangeMap::occupiedRangeEnd(int tick) const
      {
      const auto it = status.upper_bound(tick);
      if (it == status.begin())
            return tick;
      const int rangeEnd = (it == status.end()) ? tick : it->first;
      if (it->second == Range::END)
            return rangeEnd;
      return tick;
      }
}
