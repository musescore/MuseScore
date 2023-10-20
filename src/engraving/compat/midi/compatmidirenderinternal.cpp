/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 \file
 render score into event list
*/

#include "compatmidirender.h"
#include "compatmidirenderinternal.h"

#include <set>
#include <cmath>

#include "compat/midi/event.h"
#include "style/style.h"
#include "types/constants.h"

#include "dom/arpeggio.h"
#include "dom/articulation.h"
#include "dom/bend.h"
#include "dom/changeMap.h"
#include "dom/chord.h"
#include "dom/durationtype.h"
#include "dom/dynamic.h"
#include "dom/easeInOut.h"
#include "dom/glissando.h"
#include "dom/hairpin.h"
#include "dom/instrument.h"
#include "dom/letring.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/measurerepeat.h"
#include "dom/note.h"
#include "dom/noteevent.h"
#include "dom/palmmute.h"
#include "dom/part.h"
#include "dom/repeatlist.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/sig.h"
#include "dom/slur.h"
#include "dom/staff.h"
#include "dom/stafftextbase.h"
#include "dom/stretchedbend.h"
#include "dom/swing.h"
#include "dom/synthesizerstate.h"
#include "dom/tempo.h"
#include "dom/tie.h"
#include "dom/tremolo.h"
#include "dom/trill.h"
#include "dom/undo.h"
#include "dom/utils.h"
#include "dom/vibrato.h"
#include "dom/volta.h"

#include "log.h"

namespace mu::engraving {
static PitchWheelSpecs wheelSpec;
static int LET_RING_MAX_TICKS = Constants::DIVISION * 16;

struct CollectNoteParams {
    double velocityMultiplier = 1.;
    int tickOffset = 0;
    int graceOffsetOn = 0;
    int graceOffsetOff = 0;
    int endLetRingTick = 0;
    bool letRingNote = false;
    MidiInstrumentEffect effect = MidiInstrumentEffect::NONE;
    bool callAllSoundOff = false;//NoteOn silence channel
};

struct PlayNoteParams {
    int channel = 0;
    int pitch = 0;
    int velo = 0;
    int onTime = 0;
    int offTime = 0;
    int offset = 0;
    int staffIdx = 0;
    MidiInstrumentEffect effect = MidiInstrumentEffect::NONE;
    bool callAllSoundOff = false;//NoteOn silence channel
};

struct VibratoParams {
    int lowPitch = 0;
    int highPitch = 0;
    int period = 0;
};

static uint32_t getChannel(const Instrument* instr, const Note* note, MidiInstrumentEffect effect,
                           const CompatMidiRendererInternal::Context& context);

//---------------------------------------------------------
//   Converts midi time (noteoff - noteon) to milliseconds
//---------------------------------------------------------
int toMilliseconds(float tempo, float midiTime)
{
    float ticksPerSecond = (float)Constants::DIVISION * tempo;
    int time = (int)((midiTime / ticksPerSecond) * 1000.0f);
    if (time > 0x7fff) { //maximum possible value
        time = 0x7fff;
    }
    return time;
}

//---------------------------------------------------------
//   Detects if a note is a start of a glissando
//---------------------------------------------------------
bool isGlissandoFor(const Note* note)
{
    for (Spanner* spanner : note->spannerFor()) {
        if (spanner->type() == ElementType::GLISSANDO) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   Detects if a note is an end of a glissando
//---------------------------------------------------------
bool isGlissandoBack(const Note* note)
{
    for (Spanner* spanner : note->spannerBack()) {
        if (spanner->type() == ElementType::GLISSANDO) {
            return true;
        }
    }
    return false;
}

static void collectGlissando(int channel, MidiInstrumentEffect effect,
                             int onTime, int offTime,
                             int pitchDelta,
                             PitchWheelRenderer& pitchWheelRenderer, staff_idx_t staffIdx)
{
    const float scale = (float)wheelSpec.mLimit / wheelSpec.mAmplitude;

    PitchWheelRenderer::PitchWheelFunction func;
    func.mStartTick = onTime;
    func.mEndTick = offTime;

    auto linearFunc = [startTick = onTime, endTick = offTime, pitchDelta, scale] (uint32_t tick) {
        float x = (float)(tick - startTick) / (endTick - startTick);
        return pitchDelta * x * scale;
    };
    func.func = linearFunc;

    pitchWheelRenderer.addPitchWheelFunction(func, channel, staffIdx, effect);
}

static Fraction getPlayTicksForBend(const Note* note)
{
    Tie* tie = note->tieFor();
    if (!tie) {
        return note->chord()->actualTicks();
    }

    Fraction stick = note->chord()->tick();
    Note* nextNote = tie->endNote();
    while (tie) {
        nextNote = tie->endNote();
        for (EngravingItem* e : nextNote->el()) {
            if (e && (e->type() == ElementType::BEND)) {
                return nextNote->chord()->tick() - stick;
            }
        }

        if (nextNote->stretchedBend()) {
            return nextNote->chord()->tick() - stick;
        }

        tie = nextNote->tieFor();
    }

    return nextNote->chord()->tick() + nextNote->chord()->actualTicks() - stick;
}

//---------------------------------------------------------
//   playNote
//---------------------------------------------------------
static void playNote(EventsHolder& events, const Note* note, PlayNoteParams params, PitchWheelRenderer& pitchWheelRenderer)
{
    if (!note->play()) {
        return;
    }

    if (note->userVelocity() != 0) {
        params.velo = note->customizeVelocity(params.velo);
    }

    if (params.callAllSoundOff && params.onTime != 0) {
        NPlayEvent ev1(ME_CONTROLLER, params.channel, CTRL_ALL_NOTES_OFF, 0);
        ev1.setEffect(params.effect);
        events[params.channel].insert(std::pair<int, NPlayEvent>(params.onTime - 1, ev1));
    }

    NPlayEvent ev(ME_NOTEON, params.channel, params.pitch, params.velo);
    ev.setOriginatingStaff(params.staffIdx);
    ev.setTuning(note->tuning());
    ev.setNote(note);
    ev.setEffect(params.effect);
    if (params.offTime > 0 && params.offTime < params.onTime) {
        return;
    }

    events[params.channel].insert(std::pair<int, NPlayEvent>(std::max(0, params.onTime - params.offset), ev));
    // adds portamento for continuous glissando
    for (Spanner* spanner : note->spannerFor()) {
        if (spanner->type() == ElementType::GLISSANDO) {
            Glissando* glissando = toGlissando(spanner);
            if (glissando->glissandoStyle() == GlissandoStyle::PORTAMENTO) {
                Note* nextNote = toNote(spanner->endElement());
                double pitchDelta = nextNote->ppitch() - params.pitch;
                int timeDelta = params.offTime - params.onTime;
                if (pitchDelta != 0 && timeDelta != 0) {
                    collectGlissando(params.channel, params.effect, params.onTime, params.offTime, pitchDelta, pitchWheelRenderer,
                                     glissando->staffIdx());
                }
            }
        }
    }

    ev.setVelo(0);
    if (!note->part()->instrument(note->tick())->useDrumset()
        && params.offTime != -1) {
        events[params.channel].insert(std::pair<int, NPlayEvent>(std::max(0, params.offTime - params.offset), ev));
    }
}

static void collectVibrato(int channel,
                           int onTime, int offTime,
                           const VibratoParams& vibratoParams,
                           PitchWheelRenderer& pitchWheelRenderer, MidiInstrumentEffect effect, staff_idx_t staffIdx)
{
    const uint16_t vibratoPeriod = vibratoParams.period;
    const uint32_t duration = offTime - onTime;
    const float scale = 2 * (float)wheelSpec.mLimit / wheelSpec.mAmplitude / 100;

    if (duration < vibratoPeriod) {
        return;
    }

    const int pillarAmplitude = (vibratoParams.highPitch - vibratoParams.lowPitch);

    PitchWheelRenderer::PitchWheelFunction func;
    func.mStartTick = onTime;
    func.mEndTick = offTime - duration % vibratoPeriod;//removed last points to make more smooth of the end

    int lowPitch = vibratoParams.lowPitch;
    auto vibratoFunc = [startTick = onTime, pillarAmplitude, vibratoPeriod, lowPitch, scale] (uint32_t tick) {
        float x = (float)(tick - startTick) / vibratoPeriod;
        return (pillarAmplitude * 2 / M_PI * asin(sin(2 * M_PI * x)) + lowPitch) * scale;
    };
    func.func = vibratoFunc;

    pitchWheelRenderer.addPitchWheelFunction(func, channel, staffIdx, effect);
}

static void collectBend(const PitchValues& playData, staff_idx_t staffIdx,
                        int channel,
                        int onTime, int offTime,
                        PitchWheelRenderer& pitchWheelRenderer, MidiInstrumentEffect effect)
{
    size_t pitchSize = playData.size();

    const float scale = 2 * (float)wheelSpec.mLimit / wheelSpec.mAmplitude / PitchValue::PITCH_FOR_SEMITONE;
    uint32_t duration = offTime - onTime;

    for (size_t i = 0; i < pitchSize - 1; i++) {
        PitchValue curValue = playData.at(i);
        PitchValue nextValue = playData.at(i + 1);

        //! y = a x^2 + b - curve
        float curTick = (float)curValue.time * duration / PitchValue::MAX_TIME;
        float nextTick = (float)nextValue.time * duration / PitchValue::MAX_TIME;

        float a = (float)(nextValue.pitch - curValue.pitch) / ((curTick - nextTick) * (curTick - nextTick));
        float b = curValue.pitch;

        uint32_t x0 = curValue.time * duration / PitchValue::MAX_TIME;

        PitchWheelRenderer::PitchWheelFunction func;
        func.mStartTick = onTime + x0;
        uint32_t startTimeNextPoint = nextValue.time * duration / PitchValue::MAX_TIME;
        func.mEndTick = onTime + startTimeNextPoint;

        auto bendFunc = [ startTick = func.mStartTick, scale,
                          a, b] (uint32_t tick) {
            float x = (float)(tick - startTick);

            float y = a * x * x + b;

            return y * scale;
        };
        func.func = bendFunc;
        pitchWheelRenderer.addPitchWheelFunction(func, channel, staffIdx, effect);
    }
    PitchWheelRenderer::PitchWheelFunction func;
    func.mStartTick = onTime + playData.at(pitchSize - 1).time * duration / PitchValue::MAX_TIME;
    func.mEndTick = offTime;

    if (func.mEndTick == func.mStartTick) {
        return;
    }

    //! y = releaseValue linear curve
    uint32_t releaseValue = playData.at(pitchSize - 1).pitch * scale;
    auto bendFunc = [releaseValue] (uint32_t tick) {
        UNUSED(tick)
        return releaseValue;
    };
    func.func = bendFunc;
    pitchWheelRenderer.addPitchWheelFunction(func, channel, staffIdx, effect);
}

static bool letRingShouldApply(const NoteEvent& event, const Note* note)
{
    if (note->hasSlideFromNote()) {
        return false;
    }

    if (isGlissandoBack(note)) {
        return true;
    }

    if (event.slide() || isGlissandoFor(note)) {
        return false;
    }

    return true;
}

static void renderSnd(EventsHolder& events, const Chord* chord, int noteChannel, int tickOffset, int sndController)
{
    Staff* staff = chord->staff();
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
            if (velo == lastVal) {
                continue;
            }
            lastVal = velo;

            velocityMap[t] = velo;
        }
    }

    double CONVERSION_FACTOR = CompatMidiRendererInternal::ARTICULATION_CONV_FACTOR;
    for (auto& change : multChanges) {
        // Ignore fix events: they are available as cached ramp starts
        // and considering them ends up with multiplying twice effectively
        if (change.first == change.second) {
            continue;
        }

        int lastVal = CompatMidiRendererInternal::ARTICULATION_CONV_FACTOR;
        int endPoint = change.second.ticks();
        int lastVelocity = velocityMap.upper_bound(change.first.ticks())->second;
        for (int t = change.first.ticks(); t <= endPoint; t++) {
            int mult = multEvents.val(Fraction::fromTicks(t));
            if (mult == lastVal || mult == CONVERSION_FACTOR) {
                continue;
            }
            lastVal = mult;

            double realMult = mult / CONVERSION_FACTOR;
            if (velocityMap.find(t) != velocityMap.end()) {
                lastVelocity = velocityMap[t];
                velocityMap[t] *= realMult;
            } else {
                velocityMap[t] = lastVelocity * realMult;
            }
        }
    }

    for (auto point = velocityMap.cbegin(); point != velocityMap.cend(); ++point) {
        // NOTE:JT if we ever want to use poly aftertouch instead of CC, this is where we want to
        // be using it. Instead of ME_CONTROLLER, use ME_POLYAFTER (but duplicate for each note in chord)
        NPlayEvent event = NPlayEvent(ME_CONTROLLER, noteChannel, sndController, std::clamp(point->second, 0, 127));
        event.setOriginatingStaff(chord->staffIdx());
        events[noteChannel].insert(std::make_pair(point->first + tickOffset, event));
    }
}

//---------------------------------------------------------
//   collectNote
//---------------------------------------------------------

static void collectNote(EventsHolder& events, const Note* note, const CollectNoteParams& noteParams, Staff* staff,
                        PitchWheelRenderer& pitchWheelRenderer, const CompatMidiRendererInternal::Context& context)
{
    if (!note->play() || note->hidden()) {      // do not play overlapping notes
        return;
    }

    Chord* chord = note->chord();
    Instrument* instr = chord->part()->instrument(chord->tick());
    MidiInstrumentEffect noteEffect = noteParams.effect;

    int noteChannel = getChannel(instr, note, noteEffect, context);

    int tieLen = 0;
    if (chord->isGrace()) {
        assert(!CompatMidiRendererInternal::graceNotesMerged(chord));      // this function should not be called on a grace note if grace notes are merged
        chord = toChord(chord->explicitParent());
    }

    int ticks = chord->actualTicks().ticks();   // ticks of the actual note
    // calculate additional length due to ties forward
    // taking NoteEvent length adjustments into account
    if (note->tieFor()) {
        const Note* n = note->tieFor()->endNote();
        while (n) {
            NoteEventList nel = n->playEvents();
            if (!nel.empty()) {
                tieLen += (n->chord()->actualTicks().ticks() * (nel[0].len())) / 1000;
            }

            if (n->tieFor() && n != n->tieFor()->endNote()) {
                n = n->tieFor()->endNote();
            } else {
                break;
            }
        }
    }

    int tick1    = chord->tick().ticks() + noteParams.tickOffset;
    bool tieFor  = note->tieFor();
    bool tieBack = note->tieBack();

    NoteEventList nel = note->playEvents();
    size_t nels = nel.size();
    for (size_t i = 0; i < nels; ++i) {
        const NoteEvent& e = nel[i];     // we make an explicit const ref, not a const copy.  no need to copy as we won't change the original object.

        // skip if note has a tie into it and only one NoteEvent
        // its length was already added to previous note
        // if we wish to suppress first note of ornament
        // then change "nels == 1" to "i == 0", and change "break" to "continue"
        if (tieBack && nels == 1 && !isGlissandoFor(note)) {
            break;
        }

        int p = std::clamp(note->ppitch() + e.pitch(), 0, 127);
        int on = tick1 + (ticks * e.ontime()) / 1000;
        int off = on + (ticks * e.len()) / 1000 - 1;

        if (note->deadNote()) {
            const double ticksPerSecond = chord->score()->tempo(chord->tick()).val * Constants::DIVISION;
            constexpr double deadNoteDurationInSec = 0.05;
            const double deadNoteDurationInTicks = ticksPerSecond * deadNoteDurationInSec;
            if (off - on > deadNoteDurationInTicks) {
                off = on + deadNoteDurationInTicks;
            }
        } else {
            if (tieFor && i == nels - 1) {
                off += tieLen;
            }

            if (noteParams.letRingNote && letRingShouldApply(e, note)) {
                off = std::max(off, noteParams.endLetRingTick);
                if (off - on > LET_RING_MAX_TICKS) {
                    off = on + LET_RING_MAX_TICKS;
                }
            }
        }

        // Get the velocity used for this note from the staff
        // This allows correct playback of tremolos even without SND enabled.
        Fraction nonUnwoundTick = Fraction::fromTicks(on - noteParams.tickOffset);
        int velo = staff->velocities().val(nonUnwoundTick) * noteParams.velocityMultiplier * e.velocityMultiplier();
        if (e.play()) {
            PlayNoteParams playParams;
            MidiInstrumentEffect eventEffect = noteEffect;
            int eventChannel = noteChannel;
            playParams.pitch = p;
            playParams.velo = std::clamp(velo, 1, 127);
            playParams.onTime = std::max(0, on - noteParams.graceOffsetOn);
            playParams.offTime = std::max(0, off - noteParams.graceOffsetOff);

            if (eventEffect == MidiInstrumentEffect::NONE) {
                if (e.slide()) {
                    eventEffect = MidiInstrumentEffect::SLIDE;
                } else if (e.hammerPull()) {
                    eventEffect = MidiInstrumentEffect::HAMMER_PULL;
                }

                eventChannel = getChannel(instr, note, eventEffect, context);
            }

            playParams.effect = eventEffect;
            playParams.channel = eventChannel;

            if (noteParams.graceOffsetOn == 0) {
                playParams.offset = ticks * e.offset() / 1000;
            }

            playParams.staffIdx = static_cast<int>(staff->idx());
            playParams.callAllSoundOff = noteParams.callAllSoundOff;
            playNote(events, note, playParams, pitchWheelRenderer);
        }
    }

    if (instr->singleNoteDynamics()) {
        renderSnd(events, chord, noteChannel, noteParams.tickOffset, context.sndController);
    }

    // Bends
    for (EngravingItem* e : note->el()) {
        if (!e || (e->type() != ElementType::BEND)) {
            continue;
        }

        Bend* bend = toBend(e);
        if (!bend->playBend()) {
            break;
        }

        collectBend(bend->points(), bend->staffIdx(), noteChannel, tick1, tick1 + getPlayTicksForBend(
                        note).ticks(), pitchWheelRenderer, noteEffect);
    }

    if (StretchedBend* stretchedBend = note->stretchedBend()) {
        collectBend(stretchedBend->pitchValues(), stretchedBend->staffIdx(), noteChannel, tick1, tick1 + getPlayTicksForBend(
                        note).ticks(), pitchWheelRenderer, noteEffect);
    }
}

//---------------------------------------------------------
//   aeolusSetStop
//---------------------------------------------------------

static void aeolusSetStop(int tick, int channel, int i, int k, bool val, EventsHolder& events)
{
    NPlayEvent event;
    event.setType(ME_CONTROLLER);
    event.setController(98);
    if (val) {
        event.setValue(0x40 + 0x20 + i);
    } else {
        event.setValue(0x40 + 0x10 + i);
    }

    event.setChannel(static_cast<uint8_t>(channel));
    events[channel].insert(std::pair<int, NPlayEvent>(tick, event));

    event.setValue(k);
    events[channel].insert(std::pair<int, NPlayEvent>(tick, event));
//      event.setValue(0x40 + i);
//      events->insert(std::pair<int,NPlayEvent>(tick, event));
}

//---------------------------------------------------------
//   collectProgramChanges
//---------------------------------------------------------

static void collectProgramChanges(EventsHolder& events, Measure const* m, const Staff* staff, int tickOffset)
{
    int firstStaffIdx = static_cast<int>(staff->idx());
    int nextStaffIdx  = firstStaffIdx + 1;

    //
    // collect program changes and controller
    //
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        for (EngravingItem* e : s->annotations()) {
            if (!e->isStaffTextBase() || static_cast<int>(e->staffIdx()) < firstStaffIdx
                || static_cast<int>(e->staffIdx()) >= nextStaffIdx) {
                continue;
            }
            const StaffTextBase* st1 = toStaffTextBase(e);
            Fraction tick = s->tick() + Fraction::fromTicks(tickOffset);

            Instrument* instr = e->part()->instrument(tick);
            for (const ChannelActions& ca : st1->channelActions()) {
                int channel = instr->channel().at(ca.channel)->channel();
                for (const String& ma : ca.midiActionNames) {
                    NamedEventList* nel = instr->midiAction(ma, ca.channel);
                    if (!nel) {
                        continue;
                    }
                    for (MidiCoreEvent event : nel->events) {
                        event.setChannel(channel);
                        NPlayEvent e1(event);
                        e1.setOriginatingStaff(firstStaffIdx);
                        if (e1.dataA() == CTRL_PROGRAM) {
                            events[channel].insert(std::pair<int, NPlayEvent>(tick.ticks() - 1, e1));
                        } else {
                            events[channel].insert(std::pair<int, NPlayEvent>(tick.ticks(), e1));
                        }
                    }
                }
            }
            if (st1->setAeolusStops()) {
                Staff* s1 = st1->staff();
                int voice   = 0;
                int channel = s1->channel(tick, voice);

                for (int i = 0; i < 4; ++i) {
                    static int num[4] = { 12, 13, 16, 16 };
                    for (int k = 0; k < num[i]; ++k) {
                        aeolusSetStop(tick.ticks(), channel, i, k, st1->getAeolusStop(i, k), events);
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------
//    renderHarmony
///    renders chord symbols
//---------------------------------------------------------
static void renderHarmony(EventsHolder& events, Measure const* m, Harmony* h, int tickOffset)
{
    if (!h->isRealizable()) {
        return;
    }
    Staff* staff = m->score()->staff(h->track() / VOICES);
    const InstrChannel* channel = staff->part()->harmonyChannel();
    IF_ASSERT_FAILED(channel) {
        return;
    }

    if (!staff->isPrimaryStaff()) {
        return;
    }

    int staffIdx = static_cast<int>(staff->idx());
    int velocity = staff->velocities().val(h->tick());

    RealizedHarmony r = h->getRealizedHarmony();
    std::vector<int> pitches = r.pitches();

    NPlayEvent ev(ME_NOTEON, static_cast<uint8_t>(channel->channel()), 0, velocity);
    ev.setHarmony(h);
    Fraction duration = r.getActualDuration(h->tick().ticks() + tickOffset);

    int onTime = h->tick().ticks() + tickOffset;
    int offTime = onTime + duration.ticks();

    ev.setOriginatingStaff(staffIdx);
    ev.setTuning(0.0);

    //add play events
    for (int p : pitches) {
        ev.setPitch(p);
        ev.setVelo(velocity);
        events[channel->channel()].insert(std::pair<int, NPlayEvent>(onTime, ev));
        ev.setVelo(0);
        events[channel->channel()].insert(std::pair<int, NPlayEvent>(offTime, ev));
    }
}

void CompatMidiRendererInternal::collectGraceBeforeChordEvents(Chord* chord, Chord* prevChord, EventsHolder& events, double veloMultiplier,
                                                               Staff* st,
                                                               int tickOffset,
                                                               PitchWheelRenderer& pitchWheelRenderer, MidiInstrumentEffect effect)
{
    // calculate offset for grace notes here
    const auto& grChords = chord->graceNotesBefore();
    std::vector<Chord*> graceNotesBeforeBar;
    std::copy_if(grChords.begin(), grChords.end(), std::back_inserter(graceNotesBeforeBar), [](Chord* ch) {
        return ch->noteType() == NoteType::ACCIACCATURA;
    });

    int graceTickSum = 0;
    int graceTickOffset = 0;

    size_t acciacaturaGraceSize = graceNotesBeforeBar.size();
    if (acciacaturaGraceSize > 0) {
        graceTickSum = graceNotesBeforeBar[0]->ticks().ticks();
        if (prevChord) {
            graceTickSum = std::min(prevChord->ticks().ticks() / 2, graceTickSum);
        }

        graceTickOffset = graceTickSum / static_cast<int>(acciacaturaGraceSize);
    }

    if (!graceNotesMerged(chord)) {
        int currentBeaforeBeatNote = 0;
        for (Chord* c : grChords) {
            for (const Note* note : c->notes()) {
                CollectNoteParams params;
                params.effect = effect;
                params.velocityMultiplier = veloMultiplier;
                params.tickOffset = tickOffset;

                if (note->noteType() == NoteType::ACCIACCATURA) {
                    params.graceOffsetOn = graceTickSum - graceTickOffset * currentBeaforeBeatNote;
                    params.graceOffsetOff = graceTickSum - graceTickOffset * (currentBeaforeBeatNote + 1);

                    collectNote(events, note, params, st, pitchWheelRenderer, _context);
                } else {
                    collectNote(events, note, params, st, pitchWheelRenderer, _context);
                }
            }

            currentBeaforeBeatNote++;
        }
    }
}

CompatMidiRendererInternal::ChordParams CompatMidiRendererInternal::collectChordParams(const Chord* chord, int tickOffset) const
{
    ChordParams chordParams;

    int currentTick = chord->tick().ticks();
    for (auto it : score->spannerMap().findOverlapping(currentTick + 1, currentTick + 2)) {
        Spanner* spanner = it.value;
        if (spanner->track() != chord->track()) {
            continue;
        }

        if (spanner->isLetRing()) {
            LetRing* letRing = toLetRing(spanner);
            chordParams.letRing = true;
            chordParams.endLetRingTick = letRing->endCR()->tick().ticks() + letRing->endCR()->ticks().ticks() + tickOffset;
        } else if (spanner->isPalmMute()) {
            chordParams.palmMute = true;
        }
    }

    return chordParams;
}

//---------------------------------------------------------
//   doCollectMeasureEvents
//---------------------------------------------------------

void CompatMidiRendererInternal::doCollectMeasureEvents(EventsHolder& events, Measure const* m, const Staff* staff, int tickOffset,
                                                        PitchWheelRenderer& pitchWheelRenderer, std::array<Chord*, VOICES>& prevChords)
{
    staff_idx_t firstStaffIdx = staff->idx();
    for (Staff* st : staff->masterScore()->staves()) {
        if (staff->id() == st->id()) {
            firstStaffIdx = st->idx();
        }
    }
    staff_idx_t nextStaffIdx  = firstStaffIdx + 1;

    SegmentType st = SegmentType::ChordRest;
    track_idx_t strack = firstStaffIdx * VOICES;
    track_idx_t etrack = nextStaffIdx * VOICES;
    for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
        Fraction tick = seg->tick();

        //render harmony
        for (EngravingItem* e : seg->annotations()) {
            if (!e || (e->track() < strack) || (e->track() >= etrack)) {
                continue;
            }
            Harmony* h = nullptr;
            if (e->isHarmony()) {
                h = toHarmony(e);
            } else if (e->isFretDiagram()) {
                h = toFretDiagram(e)->harmony();
            }
            if (!h || !h->play()) {
                continue;
            }
            renderHarmony(events, m, h, tickOffset);
        }

        for (track_idx_t track = strack; track < etrack; ++track) {
            // Skip linked staves, except primary
            Staff* st1 = m->score()->staff(track / VOICES);
            if (!st1->isPrimaryStaff()) {
                track += VOICES - 1;
                continue;
            }

            size_t voice = track % VOICES;
            EngravingItem* cr = seg->element(track);
            if (!cr || !cr->isChord()) {
                prevChords[voice] = nullptr;
                continue;
            }

            Chord* chord = toChord(cr);

            Instrument* instr = st1->part()->instrument(tick);

            // Get a velocity multiplier
            double veloMultiplier = NoteEvent::DEFAULT_VELOCITY_MULTIPLIER;
            for (Articulation* a : chord->articulations()) {
                if (a->playArticulation()) {
                    veloMultiplier *= instr->getVelocityMultiplier(a->articulationName());
                }
            }

            //
            // Add normal note events
            //
            ChordParams chordParams = collectChordParams(chord, tickOffset);

            MidiInstrumentEffect effect = MidiInstrumentEffect::NONE;
            if (chordParams.palmMute) {
                effect = MidiInstrumentEffect::PALM_MUTE;
            }

            collectGraceBeforeChordEvents(chord, prevChords[voice], events, veloMultiplier, st1, tickOffset, pitchWheelRenderer, effect);

            for (const Note* note : chord->notes()) {
                CollectNoteParams params;
                params.velocityMultiplier = veloMultiplier;
                params.tickOffset = tickOffset;
                params.letRingNote = chordParams.letRing;
                params.endLetRingTick = chordParams.endLetRingTick;
                if (_context.instrumentsHaveEffects) {
                    params.effect = effect;
                }

                if (_context.eachStringHasChannel && instr->hasStrings()) {
                    params.callAllSoundOff = true;
                }

                collectNote(events, note, params, st1, pitchWheelRenderer, _context);
            }

            if (!graceNotesMerged(chord)) {
                for (Chord* c : chord->graceNotesAfter()) {
                    for (const Note* note : c->notes()) {
                        CollectNoteParams params;
                        params.velocityMultiplier = veloMultiplier;
                        params.tickOffset = tickOffset;
                        params.effect = effect;
                        collectNote(events, note, params, st1, pitchWheelRenderer, _context);
                    }
                }
            }

            prevChords[voice] = chord;
        }
    }
}

CompatMidiRendererInternal::CompatMidiRendererInternal(Score* s)
    : score(s)
{
}

//---------------------------------------------------------
//   collectMeasureEvents
//    redirects to the correct function based on the passed method
//---------------------------------------------------------

void CompatMidiRendererInternal::collectMeasureEvents(EventsHolder& events, Measure const* m, const Staff* staff, int tickOffset,
                                                      PitchWheelRenderer& pitchWheelRenderer, std::array<Chord*, VOICES>& prevChords)
{
    doCollectMeasureEvents(events, m, staff, tickOffset, pitchWheelRenderer, prevChords);

    collectProgramChanges(events, m, staff, tickOffset);
}

//---------------------------------------------------------
//   renderStaff
//---------------------------------------------------------

void CompatMidiRendererInternal::renderStaff(EventsHolder& events, const Staff* staff, PitchWheelRenderer& pitchWheelRenderer)
{
    Measure const* lastMeasure = nullptr;

    const RepeatList& repeatList = score->repeatList();
    std::array<Chord*, VOICES> prevChords = { nullptr };

    for (const RepeatSegment* rs : repeatList) {
        const int tickOffset = rs->utick - rs->tick;

        Measure const* const start = rs->firstMeasure();

        for (Measure const* m = start; m; m = m->nextMeasure()) {
            staff_idx_t staffIdx = staff->idx();
            if (m->isMeasureRepeatGroup(staffIdx)) {
                MeasureRepeat* mr = m->measureRepeatElement(staffIdx);
                Measure const* playMeasure = lastMeasure;
                if (!playMeasure || !mr) {
                    continue;
                }

                for (int i = m->measureRepeatCount(staffIdx); i < mr->numMeasures() && playMeasure->prevMeasure(); ++i) {
                    playMeasure = playMeasure->prevMeasure();
                }

                int offset = (m->tick() - playMeasure->tick()).ticks();
                collectMeasureEvents(events, playMeasure, staff, tickOffset + offset, pitchWheelRenderer, prevChords);
            } else {
                lastMeasure = m;
                collectMeasureEvents(events, lastMeasure, staff, tickOffset, pitchWheelRenderer, prevChords);
            }

            if (m == rs->lastMeasure()) {
                break;
            }
        }
    }
}

//---------------------------------------------------------
//   renderSpanners
//---------------------------------------------------------

void CompatMidiRendererInternal::renderSpanners(EventsHolder& events, PitchWheelRenderer& pitchWheelRenderer)
{
    for (const auto& sp : score->spannerMap().map()) {
        Spanner* s = sp.second;

        if (!s->staff()->isPrimaryStaff()) {
            continue;
        }

        int idx = s->staff()->channel(s->tick(), 0);
        int channel = s->part()->instrument(s->tick())->channel(idx)->channel();
        const auto& channels = _context.channels->channelsMap[channel];
        if (channels.empty()) {
            doRenderSpanners(events, s, channel, pitchWheelRenderer, MidiInstrumentEffect::NONE);
        } else {
            for (const auto& channel : channels) {
                doRenderSpanners(events, s, channel.second, pitchWheelRenderer, channel.first.effect);
            }
        }
    }
}

static std::vector<std::pair<int, int> > collectTicksForEffect(const Score* const score, track_idx_t track, int stick, int etick,
                                                               MidiInstrumentEffect effect)
{
    std::vector<std::pair<int, int> > ticksForEffect;
    int curTick = stick;

    for (auto it : score->spannerMap().findOverlapping(stick, etick)) {
        Spanner* spanner = it.value;

        if (spanner->track() != track) {
            continue;
        }

        if (spanner->isPalmMute()) {
            int palmMuteStartTick = spanner->tick().ticks();
            int palmMuteEndTick = spanner->tick2().ticks();

            if (curTick < palmMuteStartTick) {
                int nextTick = std::min(palmMuteStartTick, etick);
                if (effect == MidiInstrumentEffect::NONE) {
                    ticksForEffect.push_back({ curTick, nextTick - 1 });
                }

                curTick = nextTick;
            }

            if (palmMuteStartTick <= curTick) {
                int nextTick = std::min(palmMuteEndTick, etick);
                if (effect == MidiInstrumentEffect::PALM_MUTE) {
                    ticksForEffect.push_back({ curTick, nextTick - 1 });
                }

                curTick = nextTick;
            }
        }
    }

    if (curTick < etick && effect == MidiInstrumentEffect::NONE) {
        ticksForEffect.push_back({ curTick, etick - 1 });
    }

    return ticksForEffect;
}

static VibratoParams getVibratoParams(VibratoType type)
{
    VibratoParams params;

    switch (type) {
    case VibratoType::GUITAR_VIBRATO:
        // guitar vibrato, up only
        params.lowPitch = 0;
        params.highPitch = 10;
        params.period = Constants::DIVISION / 3;
        break;

    case VibratoType::GUITAR_VIBRATO_WIDE:
        params.lowPitch = 0;         // 100 is a semitone
        params.highPitch = 20;
        params.period = Constants::DIVISION / 2.5;
        break;

    case VibratoType::VIBRATO_SAWTOOTH_WIDE:
        // vibrato with whammy bar up and down
        params.lowPitch = -25;         // 1/16
        params.highPitch = 25;
        params.period = Constants::DIVISION / 2;
        break;

    case VibratoType::VIBRATO_SAWTOOTH:
        params.lowPitch = -12;
        params.highPitch = 12;
        params.period = Constants::DIVISION / 2;
        break;

    default:
        LOGE() << "vibrato type is not handled in midi renderer";
        break;
    }

    return params;
}

void CompatMidiRendererInternal::doRenderSpanners(EventsHolder& events, Spanner* s, uint32_t channel,
                                                  PitchWheelRenderer& pitchWheelRenderer,
                                                  MidiInstrumentEffect effect)
{
    std::vector<std::pair<int, std::pair<bool, int> > > pedalEventList;

    int staff = static_cast<int>(s->staffIdx());

    if (s->isPedal()) {
        std::pair<int, std::pair<bool, int> > lastEvent;

        if (!pedalEventList.empty()) {
            lastEvent = pedalEventList.back();
        } else {
            lastEvent = std::pair<int, std::pair<bool, int> >(0, std::pair<bool, int>(true, staff));
        }

        int st = s->tick().ticks();

        if (lastEvent.second.first == false && lastEvent.first >= (st + 2)) {
            pedalEventList.pop_back();
            pedalEventList.push_back(std::pair<int,
                                               std::pair<bool,
                                                         int> >(st + (2 - MScore::pedalEventsMinTicks),
                                                                std::pair<bool, int>(false, staff)));
        }
        int a = st + 2;
        pedalEventList.push_back(std::pair<int, std::pair<bool, int> >(a, std::pair<bool, int>(true, staff)));

        int t = s->tick2().ticks() + (2 - MScore::pedalEventsMinTicks);
        const RepeatSegment& lastRepeat = *score->repeatList().back();
        if (t > lastRepeat.utick + lastRepeat.len()) {
            t = lastRepeat.utick + lastRepeat.len();
        }
        pedalEventList.push_back(std::pair<int, std::pair<bool, int> >(t, std::pair<bool, int>(false, staff)));
    } else if (s->isVibrato()) {
        int stick = s->tick().ticks();
        int etick = s->tick2().ticks();

        // from start to end of trill, send bend events at regular interval
        Vibrato* t = toVibrato(s);
        VibratoParams vibratoParams = getVibratoParams(t->vibratoType());

        std::vector<std::pair<int, int> > vibratoTicksForEffect = collectTicksForEffect(score, s->track(), stick, etick, effect);

        for (const auto& [tickStart, tickEnd] : vibratoTicksForEffect) {
            collectVibrato(channel, tickStart, tickEnd, vibratoParams, pitchWheelRenderer, effect, s->staffIdx());
        }
    }

    for (const auto& pe : pedalEventList) {
        NPlayEvent event;
        if (pe.second.first == true) {
            event = NPlayEvent(ME_CONTROLLER, static_cast<uint8_t>(channel), CTRL_SUSTAIN, 127);
        } else {
            event = NPlayEvent(ME_CONTROLLER, static_cast<uint8_t>(channel), CTRL_SUSTAIN, 0);
        }
        event.setOriginatingStaff(pe.second.second);
        event.setEffect(effect);
        events[channel].insert(std::pair<int, NPlayEvent>(pe.first, event));
    }
}

//---------------------------------------------------------
// findFirstTrill
//  search the spanners in the score, finding the first one
//  which overlaps this chord and is of type ElementType::TRILL
//---------------------------------------------------------

static Trill* findFirstTrill(Chord* chord)
{
    auto spanners = chord->score()->spannerMap().findOverlapping(1 + chord->tick().ticks(),
                                                                 chord->tick().ticks() + chord->actualTicks().ticks() - 1);
    for (auto i : spanners) {
        if (i.value->type() != ElementType::TRILL) {
            continue;
        }
        if (i.value->track() != chord->track()) {
            continue;
        }
        Trill* trill = toTrill(i.value);
        if (trill->playArticulation() == false) {
            continue;
        }
        return trill;
    }
    return nullptr;
}

//---------------------------------------------------------
//   renderMetronome
///   add metronome tick events
//---------------------------------------------------------

void CompatMidiRendererInternal::renderMetronome(EventsHolder& events)
{
    Measure const* const start = score->firstMeasure();
    Measure const* const end = score->lastMeasure();

    for (Measure const* m = start; m != end; m = m->nextMeasure()) {
        renderMetronome(events, m);
    }
}

//---------------------------------------------------------
//   renderMetronome
///   add metronome tick events
//---------------------------------------------------------

void CompatMidiRendererInternal::renderMetronome(EventsHolder& events, Measure const* m)
{
    int msrTick         = m->tick().ticks();
    BeatsPerSecond tempo = score->tempomap()->tempo(msrTick);
    TimeSigFrac timeSig = score->sigmap()->timesig(msrTick).nominal();

    int clickTicks      = timeSig.isBeatedCompound(tempo.val) ? timeSig.beatTicks() : timeSig.dUnitTicks();
    int endTick         = m->endTick().ticks();

    int rtick;

    if (m->isAnacrusis()) {
        int rem = m->ticks().ticks() % clickTicks;
        msrTick += rem;
        rtick = rem + timeSig.ticksPerMeasure() - m->ticks().ticks();
    } else {
        rtick = 0;
    }

    for (int tick = msrTick; tick < endTick; tick += clickTicks, rtick += clickTicks) {
        events[0].insert(std::pair<int, NPlayEvent>(tick, NPlayEvent(timeSig.rtick2beatType(rtick))));
    }
}

void CompatMidiRendererInternal::renderScore(EventsHolder& events, const Context& ctx, bool expandRepeats)
{
    _context = ctx;
    PitchWheelRenderer pitchWheelRender(wheelSpec);

    score->updateSwing();
    score->updateCapo();

    CompatMidiRender::createPlayEvents(score, score->firstMeasure(), nullptr);

    score->updateChannel();
    score->updateVelo();

    // create note & other events
    for (const Staff* st : score->staves()) {
        renderStaff(events, st, pitchWheelRender);
    }
    events.fixupMIDI();

    // create sustain pedal events
    renderSpanners(events, pitchWheelRender);

    EventsHolder pitchWheelEvents = pitchWheelRender.renderPitchWheel();
    events.mergePitchWheelEvents(pitchWheelEvents);
//    events->merge(pitchWheelEvents);

    if (ctx.metronome) {
        renderMetronome(events);
    }
}

/* static */
uint32_t getChannel(const Instrument* instr, const Note* note, MidiInstrumentEffect effect,
                    const CompatMidiRendererInternal::Context& context)
{
    int subchannel = note->subchannel();
    int channel = instr->channel(subchannel)->channel();

    if (!context.instrumentsHaveEffects && !context.eachStringHasChannel) {
        return channel;
    }

    CompatMidiRendererInternal::ChannelLookup::LookupData lookupData;

    if (context.instrumentsHaveEffects) {
        lookupData.effect = effect;
    }

    if (context.eachStringHasChannel && instr->hasStrings()) {
        lookupData.string = note->string();
        lookupData.staffIdx = note->staffIdx();
    }

    return context.channels->getChannel(channel, lookupData);
}

uint32_t CompatMidiRendererInternal::ChannelLookup::getChannel(uint32_t instrumentChannel, const LookupData& lookupData)
{
    auto& channelsForInstrument = channelsMap[instrumentChannel];

    auto channelIt = channelsForInstrument.find(lookupData);
    if (channelIt != channelsForInstrument.end()) {
        return channelIt->second;
    }

    channelsForInstrument.insert({ lookupData, maxChannel });
    return maxChannel++;
}

bool CompatMidiRendererInternal::ChannelLookup::LookupData::operator<(const CompatMidiRendererInternal::ChannelLookup::LookupData& other)
const
{
    return std::tie(string, staffIdx, effect) < std::tie(other.string, other.staffIdx, other.effect);
}

// In the case that graceNotesBefore or graceNotesAfter are attached to a note
// with an articulation such as a trill, then the grace notes are/will-be/have-been
// already merged into the articulation.
// So this predicate, graceNotesMerged, checks for this condition to avoid calling
// functions which would re-emit the grace notes by a different algorithm.

bool CompatMidiRendererInternal::graceNotesMerged(Chord* chord)
{
    if (findFirstTrill(chord)) {
        return true;
    }
    for (Articulation* a : chord->articulations()) {
        for (auto& oe : excursions) {
            if (oe.atype == a->symId()) {
                return true;
            }
        }
    }
    return false;
}
}
