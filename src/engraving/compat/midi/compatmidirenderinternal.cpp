/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include <cmath>

#include "compat/midi/event.h"
#include "types/constants.h"

#include "dom/accidental.h"
#include "dom/arpeggio.h"
#include "dom/articulation.h"
#include "dom/bend.h"
#include "dom/chord.h"
#include "dom/durationtype.h"
#include "dom/dynamic.h"
#include "dom/glissando.h"
#include "dom/guitarbend.h"
#include "dom/hairpin.h"
#include "dom/instrument.h"
#include "dom/letring.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/measurerepeat.h"
#include "dom/note.h"
#include "dom/noteevent.h"
#include "dom/part.h"
#include "dom/repeatlist.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/staff.h"
#include "dom/stafftextbase.h"
#include "dom/stretchedbend.h"
#include "dom/swing.h"
#include "dom/tie.h"
#include "dom/trill.h"
#include "dom/undo.h"
#include "dom/utils.h"
#include "dom/vibrato.h"
#include "dom/volta.h"

#include "log.h"

namespace mu::engraving {
static PitchWheelSpecs wheelSpec;
static int LET_RING_MAX_TICKS = Constants::DIVISION * 16;
std::unordered_map<String,
                   CompatMidiRendererInternal::Context::BuiltInArticulation> CompatMidiRendererInternal::Context::
s_builtInArticulationsValues = {
    { u"staccatissimo", { 1.0, 30 } },
    { u"staccato", { 1.0, 50 } },
    { u"portato", { 1.0, 67 } },
    { u"tenuto", { 1.0, 100 } },
    { u"accent", { 1.2, 100 } },
    { u"marcato", { 1.44, 100 } },
    { u"sforzato", { 1.69, 100 } },
};

struct CollectNoteParams {
    double velocityMultiplier = 1.;
    int tickOffset = 0;
    int graceOffsetOn = 0;
    int graceOffsetOff = 0;
    int endLetRingTick = 0;
    int previousChordTicks = -1;
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

struct BendPlaybackInfo {
    int startTick = 0;
    int endTick = 0;
    float startTimeFactor = 0.f;
    float endTimeFactor = 1.f;
};

static uint32_t getChannel(const Instrument* instr, const Note* note, MidiInstrumentEffect effect,
                           const CompatMidiRendererInternal::Context& context);

static void fillScoreVelocities(const Score* score, CompatMidiRendererInternal::Context& context);
static void fillHairpinVelocities(const Hairpin* h, std::unordered_map<staff_idx_t, VelocityMap>& velocitiesByStaff);
static void fillVoltaVelocities(const Volta* volta, VelocityMap& veloMap);

static double chordVelocityMultiplier(const Chord* chord, const CompatMidiRendererInternal::Context& context);
static double velocityMultiplierByInstrument(const Instrument* instrument, const String& articulationName,
                                             const CompatMidiRendererInternal::Context& context);
static int graceBendDuration(const Chord* chord);

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
    if (!tie || !tie->endNote()) {
        return note->chord()->actualTicks();
    }

    Fraction stick = note->chord()->tick();
    Note* nextNote = tie->endNote();
    while (tie && tie->endNote()) {
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
        events[params.channel].emplace(params.onTime - 1, ev1);
    }

    NPlayEvent ev(ME_NOTEON, params.channel, params.pitch, params.velo);
    ev.setOriginatingStaff(params.staffIdx);
    ev.setTuning(note->tuning());
    ev.setNote(note);
    ev.setEffect(params.effect);
    if (params.offTime > 0 && params.offTime < params.onTime) {
        return;
    }

    events[params.channel].emplace(std::max(0, params.onTime - params.offset), ev);
    Accidental* acc = note->accidental();
    if (acc) {
        AccidentalType type = acc->accidentalType();
        double cents = Accidental::subtype2centOffset(type);
        if (!muse::RealIsNull(cents)) {
            double pwValue = cents / 100.0 * (double)wheelSpec.mLimit / (double)wheelSpec.mAmplitude;
            PitchWheelRenderer::PitchWheelFunction func;
            func.mStartTick = params.onTime - params.offset;
            func.mEndTick = params.offTime - params.offset;
            auto microtonalPW = [pwValue](uint32_t tick) {
                UNUSED(tick);
                return static_cast<int>(std::round(pwValue));
            };
            func.func = microtonalPW;
            pitchWheelRenderer.addPitchWheelFunction(func, params.channel, params.staffIdx, MidiInstrumentEffect::NONE);
        }
    }
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
    if (params.offTime != -1) {
        events[params.channel].emplace(std::max(0, params.offTime - params.offset), ev);
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

static void addConstPitchWheel(int tick, float value, PitchWheelRenderer& pitchWheelRenderer, int channel, staff_idx_t staffIdx,
                               MidiInstrumentEffect effect)
{
    const float scale = (float)wheelSpec.mLimit / wheelSpec.mAmplitude;

    PitchWheelRenderer::PitchWheelFunction pitchWheelConstFunc;
    auto constFunc = [value, scale] (uint32_t tick) {
        UNUSED(tick)
        return value * scale;
    };

    pitchWheelConstFunc.func = constFunc;
    pitchWheelConstFunc.mStartTick = tick;
    pitchWheelConstFunc.mEndTick = tick + wheelSpec.mStep;
    pitchWheelRenderer.addPitchWheelFunction(pitchWheelConstFunc, channel, staffIdx, effect);
}

static bool shouldProceedBend(const Note* note)
{
    const GuitarBend* bendFor = note->bendFor();
    const Note* baseNote = bendFor->startNoteOfChain();

    const GuitarBend* firstBend = baseNote->bendFor();
    if (firstBend && firstBend->type() == GuitarBendType::PRE_BEND) {
        const Note* nextNote = firstBend->endNote();
        if (nextNote) {
            baseNote = nextNote;
        }
    }

    return baseNote->lastTiedNote(false) == note;
}

static BendPlaybackInfo getBendPlaybackInfo(const GuitarBend* bend, int bendStart, int bendDuration)
{
    BendPlaybackInfo bendInfo;

    // currently ignoring diagram for grace bends
    if (bend->type() != GuitarBendType::GRACE_NOTE_BEND) {
        bendInfo.startTimeFactor = bend->startTimeFactor();
        bendInfo.endTimeFactor = bend->endTimeFactor();
    }

    bendInfo.startTick = bendStart + bendDuration * bendInfo.startTimeFactor;
    bendInfo.endTick = bendStart + bendDuration * bendInfo.endTimeFactor;

    return bendInfo;
}

/*
 * All consecutive tie and bend combinations are processed in a single pass, adding pitch bends where needed to ensure continuity between notes.
 *
 * When processing the first bend in a series, the duration of any preceding ties is also included,
 * allowing for an accurate total duration calculation.
 *
 * Additional calls (for notes that have already been processed) are filtered out by the function shouldProceedBend(Note*),
 * preventing redundant processing.
*/
static void collectGuitarBend(const Note* note,
                              int channel,
                              int onTime, int graceOffset, int previousChordTicks,
                              PitchWheelRenderer& pitchWheelRenderer, MidiInstrumentEffect effect)
{
    if (!shouldProceedBend(note)) {
        return;
    }

    int curPitchBendSegmentStart = onTime;
    int curPitchBendSegmentEnd = 0;

    int quarterOffsetFromStartNote = 0;
    int currentQuarterTones = 0;

    if (note->bendFor()->type() == GuitarBendType::GRACE_NOTE_BEND) {
        curPitchBendSegmentStart -= graceOffset;
    }

    const float scale = (float)wheelSpec.mLimit / wheelSpec.mAmplitude;

    while (note->bendFor() || note->tieFor()) {
        const GuitarBend* bendFor = note->bendFor();
        int duration = 0;
        if (bendFor && bendFor->type() == GuitarBendType::GRACE_NOTE_BEND) {
            duration = (previousChordTicks == -1) ? GRACE_BEND_DURATION : std::min(previousChordTicks / 2, GRACE_BEND_DURATION);
        } else {
            duration = note->chord()->actualTicks().ticks();
        }
        curPitchBendSegmentEnd = curPitchBendSegmentStart + duration;

        if (bendFor) {
            const Note* endNote = bendFor->endNote();

            if (!endNote) {
                return;
            }

            BendPlaybackInfo bendPlaybackInfo = getBendPlaybackInfo(bendFor, curPitchBendSegmentStart, duration);
            double initialPitchBendValue = quarterOffsetFromStartNote / 2.0;

            if (bendPlaybackInfo.startTick > curPitchBendSegmentStart) {
                addConstPitchWheel(curPitchBendSegmentStart, initialPitchBendValue, pitchWheelRenderer, channel, note->staffIdx(), effect);
            }

            currentQuarterTones = bendFor->bendAmountInQuarterTones();

            double tickDelta = duration * (bendPlaybackInfo.endTimeFactor - bendPlaybackInfo.startTimeFactor);
            double a = currentQuarterTones / 2.0 / (tickDelta * tickDelta);
            double b = initialPitchBendValue;
            auto bendFunc = [startTick = bendPlaybackInfo.startTick, scale, a, b] (uint32_t tick) {
                float x = (float)(tick - startTick);
                float y = a * x * x + b;
                return y * scale;
            };

            PitchWheelRenderer::PitchWheelFunction pitchWheelSquareFunc;

            pitchWheelSquareFunc.func = bendFunc;

            pitchWheelSquareFunc.mStartTick = bendPlaybackInfo.startTick;
            pitchWheelSquareFunc.mEndTick = bendPlaybackInfo.endTick;

            pitchWheelRenderer.addPitchWheelFunction(pitchWheelSquareFunc, channel, note->staffIdx(), effect);
            quarterOffsetFromStartNote += currentQuarterTones;

            if (bendPlaybackInfo.endTick < curPitchBendSegmentEnd) {
                addConstPitchWheel(bendPlaybackInfo.endTick, quarterOffsetFromStartNote / 2.0, pitchWheelRenderer, channel,
                                   note->staffIdx(),
                                   effect);
            }

            if (note == endNote) {
                break;
            }

            note = endNote;
        } else {
            if (note->bendBack()) {
                addConstPitchWheel(note->tick().ticks(), quarterOffsetFromStartNote / 2.0, pitchWheelRenderer, channel,
                                   note->staffIdx(), effect);
            }

            const Tie* tie = note->tieFor();
            note = tie->endNote();
            if (!note) {
                break;
            }
        }

        curPitchBendSegmentStart = curPitchBendSegmentEnd;
    }

    if (note->bendBack()) {
        addConstPitchWheel(note->tick().ticks(), quarterOffsetFromStartNote / 2.0, pitchWheelRenderer, channel, note->staffIdx(), effect);
    }
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

static void renderSnd(EventsHolder& events, const Chord* chord, int noteChannel, int tickOffset,
                      const CompatMidiRendererInternal::Context& context)
{
    Fraction stick = chord->tick();
    Fraction etick = stick + chord->ticks();
    const Staff* staff = chord->staff();
    const VelocityMap& veloEvents = context.velocitiesByStaff.at(staff->idx());
    const VelocityMap& multEvents = context.velocityMultiplicationsByStaff.at(staff->idx());
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
        int lastVelocity = 0;
        auto lastValocityIt = velocityMap.upper_bound(change.first.ticks());
        if (lastValocityIt != velocityMap.end()) {
            lastVelocity = lastValocityIt->second;
        } else if (!velocityMap.empty()) {
            lastVelocity = velocityMap.cbegin()->second;
        }

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
        NPlayEvent event = NPlayEvent(ME_CONTROLLER, noteChannel, context.sndController, std::clamp(point->second, 0, 127));
        event.setOriginatingStaff(chord->staffIdx());
        events[noteChannel].insert(std::make_pair(point->first + tickOffset, event));
    }
}

int graceBendDuration(const Chord* chord)
{
    int graceDuration = GRACE_BEND_DURATION;
    if (chord) {
        graceDuration = std::min(chord->ticks().ticks() / 2, graceDuration);
    }

    return graceDuration;
}

static int calculateTieLength(const Note* note)
{
    int tieLen = 0;

    const Note* n = note;
    while (n) {
        // Process ties or bends
        Tie* tieFor = n->tieFor();
        GuitarBend* bendFor = n->bendFor();

        if (tieFor && tieFor->endNote() != n) {
            n = tieFor->endNote();
        } else if (bendFor && bendFor->endNote() != n) {
            n = bendFor->endNote();
        } else {
            break;
        }

        NoteEventList nel = n->playEvents();

        if (!nel.empty()) {
            tieLen += n->playEvents()[0].len() * n->chord()->actualTicks().ticks() / NoteEvent::NOTE_LENGTH;
        }
    }

    return tieLen;
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
    const Instrument* instr = chord->part()->instrument(chord->tick());
    MidiInstrumentEffect noteEffect = noteParams.effect;

    int noteChannel = getChannel(instr, note, noteEffect, context);

    int tieLen = calculateTieLength(note);
    if (chord->isGrace()) {
        assert(!CompatMidiRendererInternal::graceNotesMerged(chord));      // this function should not be called on a grace note if grace notes are merged
        chord = toChord(chord->explicitParent());
    }

    int ticks = chord->actualTicks().ticks();   // ticks of the actual note
    // calculate additional length due to ties forward
    // taking NoteEvent length adjustments into account

    int tick1    = note->tick().ticks() + noteParams.tickOffset;
    const GuitarBend* bendFor = note->bendFor();
    const GuitarBend* bendBack = note->bendBack();

    NoteEventList nel = note->playEvents();
    size_t nels = nel.size();
    for (size_t i = 0; i < nels; ++i) {
        const NoteEvent& e = nel[i];     // we make an explicit const ref, not a const copy.  no need to copy as we won't change the original object.

        // skip if note has a tie into it and only one NoteEvent
        // its length was already added to previous note
        // if we wish to suppress first note of ornament
        // then change "nels == 1" to "i == 0", and change "break" to "continue"
        if (note->tieBack() && nels == 1 && !isGlissandoFor(note)) {
            break;
        }

        // skipping the notes which are connected by bends
        if (bendBack && bendBack->type() != GuitarBendType::PRE_BEND && i == 0) {
            continue;
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
            if ((note->tieFor() || bendFor) && i == nels - 1) {
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
        int velo = context.velocitiesByStaff.at(staff->idx()).val(nonUnwoundTick) * noteParams.velocityMultiplier * e.velocityMultiplier();
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

            if (instr->singleNoteDynamics()) {
                renderSnd(events, chord, noteChannel, noteParams.tickOffset, context);
            }
        }
    }

    // Bends
    if (bendFor) {
        collectGuitarBend(note, noteChannel, tick1, noteParams.graceOffsetOn, noteParams.previousChordTicks, pitchWheelRenderer,
                          noteEffect);
    } else {
        // old bends implementation
        for (const EngravingItem* e : note->el()) {
            if (!e || (e->type() != ElementType::BEND)) {
                continue;
            }

            const Bend* bend = toBend(e);
            if (!bend->playBend()) {
                break;
            }

            collectBend(bend->points(), bend->staffIdx(), noteChannel, tick1, tick1 + getPlayTicksForBend(
                            note).ticks(), pitchWheelRenderer, noteEffect);
        }
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
    events[channel].emplace(tick, event);

    event.setValue(k);
    events[channel].emplace(tick, event);
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
                            events[channel].emplace(tick.ticks() - 1, e1);
                        } else {
                            events[channel].emplace(tick.ticks(), e1);
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
static void renderHarmony(EventsHolder& events, Measure const* m, Harmony* h, int tickOffset,
                          const CompatMidiRendererInternal::Context& context)
{
    if (!h->isRealizable() || context.harmonyChannelSetting == CompatMidiRendererInternal::HarmonyChannelSetting::DISABLED) {
        return;
    }

    if (context.partsWithMutedHarmony.find(h->part()->id().toStdString()) != context.partsWithMutedHarmony.end()) {
        return;
    }

    Staff* staff = m->score()->staff(h->track() / VOICES);
    const InstrChannel* instrChannel = staff->part()->harmonyChannel();
    IF_ASSERT_FAILED(instrChannel) {
        LOGE() << "channel for harmony isn't found for part " << staff->part()->partName();
        return;
    }

    if (!staff->isPrimaryStaff()) {
        return;
    }

    int channel = instrChannel->channel();
    int staffIdx = static_cast<int>(staff->idx());

    if (context.harmonyChannelSetting == CompatMidiRendererInternal::HarmonyChannelSetting::LOOKUP) {
        CompatMidiRendererInternal::ChannelLookup::LookupData lookupData;
        lookupData.harmony = true;
        channel = context.channels->getChannel(channel, lookupData);
    }

    int velocity = context.velocitiesByStaff.at(staff->idx()).val(h->tick());

    RealizedHarmony r = h->getRealizedHarmony();
    std::vector<int> pitches = r.pitches();

    NPlayEvent ev(ME_NOTEON, static_cast<uint8_t>(channel), 0, velocity);
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
        events[channel].emplace(onTime, ev);
        ev.setVelo(0);
        events[channel].emplace(offTime, ev);
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
    } else {
        bool hasGraceBend = std::any_of(grChords.begin(), grChords.end(), [](Chord* ch) {
            return std::any_of(ch->notes().begin(), ch->notes().end(), [](Note* n) {
                return n->isGraceBendStart();
            });
        });

        if (hasGraceBend) {
            graceTickSum = graceBendDuration(prevChord);
        }
    }

    if (!graceNotesMerged(chord)) {
        int currentBeaforeBeatNote = 0;
        for (Chord* c : grChords) {
            for (const Note* note : c->notes()) {
                GuitarBend* bendFor = note->bendFor();
                if (bendFor && bendFor->type() == GuitarBendType::PRE_BEND) {
                    continue;
                }

                CollectNoteParams params;
                params.effect = effect;
                params.velocityMultiplier = veloMultiplier;
                params.tickOffset = tickOffset;

                bool isGraceBend = (note->bendFor() && note->bendFor()->type() == GuitarBendType::GRACE_NOTE_BEND);
                if (prevChord) {
                    params.previousChordTicks = prevChord->actualTicks().ticks();
                }

                if (note->noteType() == NoteType::ACCIACCATURA) {
                    params.graceOffsetOn = graceTickSum - graceTickOffset * currentBeaforeBeatNote;
                    params.graceOffsetOff = graceTickSum - graceTickOffset * (currentBeaforeBeatNote + 1);

                    collectNote(events, note, params, st, pitchWheelRenderer, m_context);
                } else if (isGraceBend) {
                    params.graceOffsetOn = graceTickSum;
                    params.graceOffsetOff = 0;

                    collectNote(events, note, params, st, pitchWheelRenderer, m_context);
                } else {
                    collectNote(events, note, params, st, pitchWheelRenderer, m_context);
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
            ChordRest* endCR = letRing->endCR();
            chordParams.endLetRingTick = (endCR ? endCR->tick().ticks() + endCR->ticks().ticks() : letRing->tick2().ticks()) + tickOffset;
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
            renderHarmony(events, m, h, tickOffset, m_context);
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
            double veloMultiplier = NoteEvent::DEFAULT_VELOCITY_MULTIPLIER * chordVelocityMultiplier(chord, m_context);

            //
            // Add normal note events
            //
            ChordParams chordParams = collectChordParams(chord, tickOffset);

            MidiInstrumentEffect effect = MidiInstrumentEffect::NONE;
            if (chordParams.palmMute) {
                effect = MidiInstrumentEffect::PALM_MUTE;
            }

            collectGraceBeforeChordEvents(chord, prevChords[voice], events, veloMultiplier, st1, tickOffset, pitchWheelRenderer, effect);

            Instrument* instr = st1->part()->instrument(tick);
            for (const Note* note : chord->notes()) {
                CollectNoteParams params;
                params.velocityMultiplier = veloMultiplier;
                params.tickOffset = tickOffset;
                params.letRingNote = chordParams.letRing;
                params.endLetRingTick = chordParams.endLetRingTick;
                if (m_context.instrumentsHaveEffects) {
                    params.effect = effect;
                }

                if (m_context.eachStringHasChannel && instr->hasStrings()) {
                    params.callAllSoundOff = true;
                }

                collectNote(events, note, params, st1, pitchWheelRenderer, m_context);
            }

            if (!graceNotesMerged(chord)) {
                for (Chord* c : chord->graceNotesAfter()) {
                    for (const Note* note : c->notes()) {
                        CollectNoteParams params;
                        params.velocityMultiplier = veloMultiplier;
                        params.tickOffset = tickOffset;
                        params.effect = effect;
                        collectNote(events, note, params, st1, pitchWheelRenderer, m_context);
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
        const auto& channels = m_context.channels->channelsMap[channel];
        if (channels.empty()) {
            doRenderSpanners(events, s, channel, pitchWheelRenderer, MidiInstrumentEffect::NONE);
        } else {
            for (const auto& channel2 : channels) {
                doRenderSpanners(events, s, channel2.second, pitchWheelRenderer, channel2.first.effect);
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
    struct PedalEvent {
        int tick = 0;
        bool on = true;
        int staffIdx = 0;

        PedalEvent() = default;
        PedalEvent(int tick, bool on, int staffIdx)
            : tick(tick), on(on), staffIdx(staffIdx)
        {
        }
    };

    std::vector<PedalEvent> pedalEventList;

    int staffIdx = static_cast<int>(s->staffIdx());

    if (s->isPedal()) {
        PedalEvent lastEvent;

        if (!pedalEventList.empty()) {
            lastEvent = pedalEventList.back();
        } else {
            lastEvent = { 0, true, staffIdx };
        }

        int st = s->tick().ticks();

        if (!lastEvent.on && lastEvent.tick >= (st + 2)) {
            pedalEventList.emplace(pedalEventList.cend() - 1,
                                   st + (2 - MScore::pedalEventsMinTicks), false, staffIdx);
        }
        int a = st + 2;
        pedalEventList.emplace_back(a, true, staffIdx);

        int t = s->tick2().ticks() + (2 - MScore::pedalEventsMinTicks);
        const RepeatSegment& lastRepeat = *score->repeatList().back();
        if (t > lastRepeat.utick + lastRepeat.len()) {
            t = lastRepeat.utick + lastRepeat.len();
        }
        pedalEventList.emplace_back(t, false, staffIdx);
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
        if (pe.on) {
            event = NPlayEvent(ME_CONTROLLER, static_cast<uint8_t>(channel), CTRL_SUSTAIN, 127);
        } else {
            event = NPlayEvent(ME_CONTROLLER, static_cast<uint8_t>(channel), CTRL_SUSTAIN, 0);
        }
        event.setOriginatingStaff(pe.staffIdx);
        event.setEffect(effect);
        events[channel].emplace(pe.tick, event);
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
        if (!trill->playSpanner()) {
            continue;
        }
        return trill;
    }
    return nullptr;
}

void CompatMidiRendererInternal::renderScore(EventsHolder& events, const Context& context, bool expandRepeats)
{
    UNUSED(expandRepeats);

    m_context = context;
    PitchWheelRenderer pitchWheelRender(wheelSpec);

    score->updateSwing();
    score->updateCapo();

    if (!m_context.useDefaultArticulations) {
        fillArticulationsInfo();
    }

    CompatMidiRender::createPlayEvents(score, score->firstMeasure(), nullptr, m_context);

    score->updateChannel();
    fillScoreVelocities(score, m_context);

    // create note & other events
    for (const Staff* st : score->staves()) {
        renderStaff(events, st, pitchWheelRender);
    }
    events.fixupMIDI();

    // create sustain pedal events
    renderSpanners(events, pitchWheelRender);

    EventsHolder pitchWheelEvents = pitchWheelRender.renderPitchWheel();
    events.mergePitchWheelEvents(pitchWheelEvents);
    if (m_context.applyCaesuras) {
        m_context.pauseMap->calculate(score);
    }
}

void CompatMidiRendererInternal::fillArticulationsInfo()
{
    for (const Part* part : score->parts()) {
        for (const auto& [tick, instr] : part->instruments()) {
            String instrId = instr->id();
            for (auto it = Context::s_builtInArticulationsValues.cbegin(); it != Context::s_builtInArticulationsValues.cend(); it++) {
                const String& articulationName = it->first;
                const std::vector<MidiArticulation>& instrArticulations = instr->articulation();
                bool instrHasArticulation
                    = std::any_of(instrArticulations.begin(),
                                  instrArticulations.end(), [articulationName](const MidiArticulation& instrArticulation) {
                    return instrArticulation.name == articulationName;
                });

                if (!instrHasArticulation) {
                    m_context.articulationsWithoutValuesByInstrument[instrId].insert(articulationName);
                }
            }
        }
    }
}

double chordVelocityMultiplier(const Chord* chord, const CompatMidiRendererInternal::Context& context)
{
    double veloMultiplier = 1.0;
    Instrument* instr = chord->part()->instrument();
    for (Articulation* a : chord->articulations()) {
        if (a->playArticulation()) {
            veloMultiplier *= velocityMultiplierByInstrument(instr, a->articulationName(), context);
        }
    }

    return veloMultiplier;
}

double velocityMultiplierByInstrument(const Instrument* instrument, const String& articulationName,
                                      const CompatMidiRendererInternal::Context& context)
{
    using Ctx = CompatMidiRendererInternal::Context;
    if (context.useDefaultArticulations) {
        auto it = Ctx::s_builtInArticulationsValues.find(articulationName);
        if (it != Ctx::s_builtInArticulationsValues.end()) {
            return it->second.velocityMultiplier;
        }
    } else {
        auto articulationsForInstrumentIt = context.articulationsWithoutValuesByInstrument.find(instrument->id());
        if (articulationsForInstrumentIt != context.articulationsWithoutValuesByInstrument.end()) {
            const auto& articulationsForInstrument = articulationsForInstrumentIt->second;
            if (articulationsForInstrument.find(articulationName) != articulationsForInstrument.end()) {
                return Ctx::s_builtInArticulationsValues[articulationName].velocityMultiplier;
            }
        }
    }

    return instrument->getVelocityMultiplier(articulationName);
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
        if (note->string() >= 0) {
            lookupData.string = note->string();
        } else {
            int string = 0;
            int fret = 0;
            const StringData* stringData = instr->stringData();
            IF_ASSERT_FAILED(stringData && stringData->convertPitch(note->pitch(), note->staff(), &string, &fret)) {
                LOGE() << "channel isn't calculated for instrument " << instr->nameAsPlainText();
                return channel;
            }

            lookupData.string = string;
        }

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
    if (harmony && !other.harmony) {
        return true;
    }

    if (!harmony && other.harmony) {
        return false;
    }

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

/* static */
void fillHairpinVelocities(const Hairpin* h, std::unordered_map<staff_idx_t, VelocityMap>& velocitiesByStaff)
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
    case DynamicRange::STAFF:
        if (st->isPrimaryStaff()) {
            velocitiesByStaff[st->idx()].addHairpin(tick, tick2, veloChange, method, direction);
        }
        break;
    case DynamicRange::PART:
        for (Staff* s : st->part()->staves()) {
            if (s->isPrimaryStaff()) {
                velocitiesByStaff[s->idx()].addHairpin(tick, tick2, veloChange, method, direction);
            }
        }
        break;
    case DynamicRange::SYSTEM:
        for (Staff* s : st->score()->staves()) {
            if (s->isPrimaryStaff()) {
                velocitiesByStaff[s->idx()].addHairpin(tick, tick2, veloChange, method, direction);
            }
        }
        break;
    }
}

void fillScoreVelocities(const Score* score, CompatMidiRendererInternal::Context& context)
{
    Score* mainScore = score->masterScore();

    if (!mainScore->firstMeasure()) {
        return;
    }

    for (size_t staffIdx = 0; staffIdx < mainScore->nstaves(); staffIdx++) {
        Staff* st = mainScore->staff(staffIdx);
        if (!st->isPrimaryStaff()) {
            continue;
        }

        VelocityMap& velo = context.velocitiesByStaff[st->idx()];
        VelocityMap& mult = context.velocityMultiplicationsByStaff[st->idx()];
        Part* prt = st->part();
        size_t partStaves = prt->nstaves();
        staff_idx_t partStaff = mainScore->staffIdx(prt);

        for (Segment* s = mainScore->firstMeasure()->first(); s; s = s->next1()) {
            Fraction tick = s->tick();
            for (const EngravingItem* e : s->annotations()) {
                if (e->staffIdx() != staffIdx) {
                    continue;
                }

                if (e->type() != ElementType::DYNAMIC) {
                    continue;
                }

                const Dynamic* d = toDynamic(e);
                int v = d->velocity();

                // treat an invalid dynamic as no change, i.e. a dynamic set to 0
                if (v < 1) {
                    continue;
                }

                v = std::clamp(v, 1, 127);                 //  illegal values

                // If a dynamic has 'velocity change' update its ending
                int change = d->changeInVelocity();
                ChangeDirection direction = ChangeDirection::INCREASING;
                if (change < 0) {
                    direction = ChangeDirection::DECREASING;
                }

                staff_idx_t dStaffIdx = d->staffIdx();
                switch (d->dynRange()) {
                case DynamicRange::STAFF:
                    if (dStaffIdx == staffIdx) {
                        velo.addDynamic(tick, v);
                        if (change != 0) {
                            Fraction etick = tick + d->velocityChangeLength();
                            ChangeMethod method = ChangeMethod::NORMAL;
                            velo.addHairpin(tick, etick, change, method, direction);
                        }
                    }
                    break;
                case DynamicRange::PART:
                    if (dStaffIdx >= partStaff && dStaffIdx < partStaff + partStaves) {
                        for (staff_idx_t i = partStaff; i < partStaff + partStaves; ++i) {
                            Staff* stp = mainScore->staff(i);
                            if (!stp->isPrimaryStaff()) {
                                continue;
                            }

                            VelocityMap& stVelo = context.velocitiesByStaff[stp->idx()];
                            stVelo.addDynamic(tick, v);
                            if (change != 0) {
                                Fraction etick = tick + d->velocityChangeLength();
                                ChangeMethod method = ChangeMethod::NORMAL;
                                stVelo.addHairpin(tick, etick, change, method, direction);
                            }
                        }
                    }
                    break;
                case DynamicRange::SYSTEM:
                    for (size_t i = 0; i < mainScore->nstaves(); ++i) {
                        Staff* sts = mainScore->staff(i);
                        if (!sts->isPrimaryStaff()) {
                            continue;
                        }

                        VelocityMap& stVelo = context.velocitiesByStaff[mainScore->staff(i)->idx()];
                        stVelo.addDynamic(tick, v);
                        if (change != 0) {
                            Fraction etick = tick + d->velocityChangeLength();
                            ChangeMethod method = ChangeMethod::NORMAL;
                            stVelo.addHairpin(tick, etick, change, method, direction);
                        }
                    }
                    break;
                }
            }

            if (s->isChordRestType()) {
                for (size_t i = staffIdx * VOICES; i < (staffIdx + 1) * VOICES; ++i) {
                    EngravingItem* el = s->element(i);
                    if (!el || !el->isChord()) {
                        continue;
                    }

                    Chord* chord = toChord(el);

                    double veloMultiplier = chordVelocityMultiplier(chord, context);

                    if (muse::RealIsEqual(veloMultiplier, 1.0)) {
                        continue;
                    }

                    // TODO this should be a (configurable?) constant somewhere
                    static Fraction ARTICULATION_CHANGE_TIME_MAX = Fraction(1, 16);
                    Fraction ARTICULATION_CHANGE_TIME = std::min(s->ticks(), ARTICULATION_CHANGE_TIME_MAX);
                    int start = veloMultiplier * engraving::CompatMidiRendererInternal::ARTICULATION_CONV_FACTOR;
                    int change = (veloMultiplier - 1) * engraving::CompatMidiRendererInternal::ARTICULATION_CONV_FACTOR;
                    mult.addDynamic(chord->tick(), start);
                    mult.addHairpin(chord->tick(),
                                    chord->tick() + ARTICULATION_CHANGE_TIME, change, ChangeMethod::NORMAL, ChangeDirection::DECREASING);
                }
            }
        }

        for (const auto& sp : mainScore->spannerMap().map()) {
            Spanner* s = sp.second;
            if (s->type() != ElementType::HAIRPIN || sp.second->staffIdx() != staffIdx) {
                continue;
            }

            fillHairpinVelocities(toHairpin(s), context.velocitiesByStaff);
        }
    }

    for (Staff* st : mainScore->staves()) {
        if (st->isPrimaryStaff()) {
            context.velocitiesByStaff[st->idx()].setup();
            context.velocityMultiplicationsByStaff[st->idx()].setup();
        }
    }

    for (auto it = mainScore->spanner().cbegin(); it != mainScore->spanner().cend(); ++it) {
        Spanner* spanner = (*it).second;
        if (!spanner->isVolta()) {
            continue;
        }

        Volta* volta = toVolta(spanner);
        Staff* st = volta->staff();
        if (st->isPrimaryStaff()) {
            fillVoltaVelocities(volta, context.velocitiesByStaff[st->idx()]);
        }
    }
}

/* static */
void fillVoltaVelocities(const Volta* volta, VelocityMap& veloMap)
{
    Measure* startMeasure = volta->startMeasure();
    Measure* endMeasure = volta->endMeasure();

    if (startMeasure && endMeasure) {
        if (!endMeasure->repeatEnd()) {
            return;
        }

        Fraction startTick = Fraction::fromTicks(startMeasure->tick().ticks() - 1);
        Fraction endTick = Fraction::fromTicks((endMeasure->tick() + endMeasure->ticks()).ticks() - 1);
        int prevVelo = veloMap.val(startTick);
        veloMap.addDynamic(endTick, prevVelo);
    }
}
}
