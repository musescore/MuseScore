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

#include "midirender.h"

#include <set>
#include <cmath>

#include "compat/midi/event.h"
#include "style/style.h"
#include "types/constants.h"

#include "libmscore/arpeggio.h"
#include "libmscore/articulation.h"
#include "libmscore/bend.h"
#include "libmscore/changeMap.h"
#include "libmscore/chord.h"
#include "libmscore/durationtype.h"
#include "libmscore/dynamic.h"
#include "libmscore/easeInOut.h"
#include "libmscore/glissando.h"
#include "libmscore/hairpin.h"
#include "libmscore/instrument.h"
#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/measurerepeat.h"
#include "libmscore/note.h"
#include "libmscore/noteevent.h"
#include "libmscore/palmmute.h"
#include "libmscore/part.h"
#include "libmscore/repeatlist.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/sig.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/stafftextbase.h"
#include "libmscore/stretchedbend.h"
#include "libmscore/swing.h"
#include "libmscore/synthesizerstate.h"
#include "libmscore/tempo.h"
#include "libmscore/tie.h"
#include "libmscore/tremolo.h"
#include "libmscore/trill.h"
#include "libmscore/undo.h"
#include "libmscore/utils.h"
#include "libmscore/vibrato.h"
#include "libmscore/volta.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
static constexpr int MIN_CHUNK_SIZE(10); // measure
static PitchWheelSpecs wheelSpec;

static bool graceNotesMerged(Chord* chord);

struct SndConfig {
    bool useSND = false;
    int controller = -1;
    DynamicsRenderMethod method = DynamicsRenderMethod::SEG_START;

    SndConfig() {}
    SndConfig(bool use, int c, DynamicsRenderMethod me)
        : useSND(use), controller(c), method(me) {}
};

//---------------------------------------------------------
//   Converts midi time (noteoff - noteon) to milliseconds
//---------------------------------------------------------
int toMilliseconds(float tempo, float midiTime)
{
    float ticksPerSecond = (float)Constants::division * tempo;
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

static void collectGlissando(int channel,
                             int onTime, int offTime,
                             int pitchDelta,
                             PitchWheelRenderer& pitchWheelRenderer)
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

    pitchWheelRenderer.addPitchWheelFunction(func, channel);
}

//---------------------------------------------------------
//   playNote
//---------------------------------------------------------
static void playNote(EventMap* events, const Note* note, int channel, int pitch,
                     int velo, int onTime, int offTime, int staffIdx, PitchWheelRenderer& pitchWheelRenderer)
{
    if (!note->play()) {
        return;
    }

    if (note->userVelocity() != 0) {
        velo = note->customizeVelocity(velo);
    }

    NPlayEvent ev(ME_NOTEON, channel, pitch, velo);
    ev.setOriginatingStaff(staffIdx);
    ev.setTuning(note->tuning());
    ev.setNote(note);
    if (offTime < onTime) {
        offTime = onTime;
    }
    events->insert(std::pair<int, NPlayEvent>(onTime, ev));
    // adds portamento for continuous glissando
    for (Spanner* spanner : note->spannerFor()) {
        if (spanner->type() == ElementType::GLISSANDO) {
            Glissando* glissando = toGlissando(spanner);
            if (glissando->glissandoStyle() == GlissandoStyle::PORTAMENTO) {
                Note* nextNote = toNote(spanner->endElement());
                double pitchDelta = nextNote->ppitch() - pitch;
                int timeDelta = offTime - onTime;
                if (pitchDelta != 0 && timeDelta != 0) {
                    collectGlissando(channel, onTime, offTime, pitchDelta, pitchWheelRenderer);
                }
            }
        }
    }

    ev.setVelo(0);
    if (!note->part()->instrument(note->tick())->useDrumset()) {
        events->insert(std::pair<int, NPlayEvent>(offTime, ev));
    }
}

static void collectVibrato(int channel,
                           int onTime, int offTime,
                           int lowPitch, int highPitch,
                           PitchWheelRenderer& pitchWheelRenderer)
{
    const uint16_t vibratoPeriod = Constants::division / 2;
    const uint32_t duration = offTime - onTime;
    const float scale = 2 * (float)wheelSpec.mLimit / wheelSpec.mAmplitude / 100;

    if (duration < vibratoPeriod) {
        return;
    }

    const int pillarAmplitude = (highPitch - lowPitch);

    PitchWheelRenderer::PitchWheelFunction func;
    func.mStartTick = onTime;
    func.mEndTick = offTime - duration % vibratoPeriod;//removed last points to make more smooth of the end

    auto vibratoFunc = [startTick = onTime, pillarAmplitude, vibratoPeriod, lowPitch, scale] (uint32_t tick) {
        float x = (float)(tick - startTick) / vibratoPeriod;
        return (pillarAmplitude * 2 / M_PI * asin(sin(2 * M_PI * x)) + lowPitch) * scale;
    };
    func.func = vibratoFunc;

    pitchWheelRenderer.addPitchWheelFunction(func, channel);
}

static void collectBend(const Bend* bend,
                        int channel,
                        int onTime, int offTime,
                        PitchWheelRenderer& pitchWheelRenderer)
{
    const PitchValues& points = bend->points();
    size_t pitchSize = points.size();

    const float scale = 2 * (float)wheelSpec.mLimit / wheelSpec.mAmplitude / 100;
    uint32_t duration = offTime - onTime;

    for (size_t i = 0; i < pitchSize - 1; i++) {
        PitchValue curValue = points[i];
        PitchValue nextValue = points[i + 1];

        //! y = a x^2 + b - curve
        float curTick = (float)curValue.time * duration / 100;
        float nextTick = (float)nextValue.time * duration / 100;

        float a = (float)(nextValue.pitch - curValue.pitch) / ((curTick - nextTick) * (curTick - nextTick));
        float b = curValue.pitch;

        uint32_t x0 = curValue.time * duration / 100;

        PitchWheelRenderer::PitchWheelFunction func;
        func.mStartTick = onTime + x0;
        uint32_t startTimeNextPoint = nextValue.time * duration / 100;
        func.mEndTick = onTime + startTimeNextPoint;

        auto bendFunc = [ startTick = func.mStartTick, scale,
                          a, b] (uint32_t tick) {
            float x = (float)(tick - startTick);

            float y = a * x * x + b;

            return y * scale;
        };
        func.func = bendFunc;
        pitchWheelRenderer.addPitchWheelFunction(func, channel);
    }
    PitchWheelRenderer::PitchWheelFunction func;
    func.mStartTick = onTime + points[pitchSize - 1].time * duration / 100;
    func.mEndTick = offTime;

    if (func.mEndTick == func.mStartTick) {
        return;
    }

    //! y = releaseValue linear curve
    uint32_t releaseValue = points[pitchSize - 1].pitch * scale;
    auto bendFunc = [releaseValue] (uint32_t tick) {
        UNUSED(tick)
        return releaseValue;
    };
    func.func = bendFunc;
    pitchWheelRenderer.addPitchWheelFunction(func, channel);
}

//---------------------------------------------------------
//   collectNote
//---------------------------------------------------------

static void collectNote(EventMap* events, int channel, const Note* note, double velocityMultiplier, int tickOffset, Staff* staff,
                        SndConfig config, PitchWheelRenderer& pitchWheelRenderer, int graceOffsetOn = 0, int graceOffsetOff = 0)
{
    if (!note->play() || note->hidden()) {      // do not play overlapping notes
        return;
    }
    Chord* chord = note->chord();

    int staffIdx = static_cast<int>(staff->idx());
    int ticks;
    int tieLen = 0;
    if (chord->isGrace()) {
        assert(!graceNotesMerged(chord));      // this function should not be called on a grace note if grace notes are merged
        chord = toChord(chord->explicitParent());
    }

    ticks = chord->actualTicks().ticks();   // ticks of the actual note
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
            } else {
                // recurse
                collectNote(events, channel, n, velocityMultiplier, tickOffset, staff, config, pitchWheelRenderer);
                break;
            }
            if (n->tieFor() && n != n->tieFor()->endNote()) {
                n = n->tieFor()->endNote();
            } else {
                break;
            }
        }
    }

    int tick1    = chord->tick().ticks() + tickOffset;
    bool tieFor  = note->tieFor();
    bool tieBack = note->tieBack();

    NoteEventList nel = note->playEvents();
    size_t nels = nel.size();
    for (int i = 0, pitch = note->ppitch(); i < static_cast<int>(nels); ++i) {
        const NoteEvent& e = nel[i];     // we make an explicit const ref, not a const copy.  no need to copy as we won't change the original object.

        // skip if note has a tie into it and only one NoteEvent
        // its length was already added to previous note
        // if we wish to suppress first note of ornament
        // then change "nels == 1" to "i == 0", and change "break" to "continue"
        if (tieBack && nels == 1 && !isGlissandoFor(note)) {
            break;
        }
        int p = pitch + e.pitch();
        if (p < 0) {
            p = 0;
        } else if (p > 127) {
            p = 127;
        }
        int on  = tick1 + (ticks * e.ontime()) / 1000;
        int off = on + (ticks * e.len()) / 1000 - 1;
        if (tieFor && i == static_cast<int>(nels) - 1) {
            off += tieLen;
        }

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
        } else {
            velo = staff->velocities().val(nonUnwoundTick);
        }

        velo *= velocityMultiplier;
        playNote(events, note, channel, p, std::clamp(velo, 1, 127), std::max(0, on - graceOffsetOn), std::max(0,
                                                                                                               off - graceOffsetOff),
                 staffIdx, pitchWheelRenderer);
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
                if (velo == lastVal) {
                    continue;
                }
                lastVal = velo;

                velocityMap[t] = velo;
            }
        }

        double CONVERSION_FACTOR = MidiRenderer::ARTICULATION_CONV_FACTOR;
        for (auto& change : multChanges) {
            // Ignore fix events: they are available as cached ramp starts
            // and considering them ends up with multiplying twice effectively
            if (change.first == change.second) {
                continue;
            }

            int lastVal = MidiRenderer::ARTICULATION_CONV_FACTOR;
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
            NPlayEvent event = NPlayEvent(ME_CONTROLLER, channel, config.controller, std::clamp(point->second, 0, 127));
            event.setOriginatingStaff(staffIdx);
            events->insert(std::make_pair(point->first + tickOffset, event));
        }
    }

    // Bends
    for (EngravingItem* e : note->el()) {
        if (e == 0 || (e->type() != ElementType::BEND && e->type() != ElementType::STRETCHED_BEND)) {
            continue;
        }
        Bend* bend = toBend(e);
        if (!bend->playBend()) {
            break;
        }
        collectBend(bend, channel, tick1, tick1 + note->playTicks(), pitchWheelRenderer);
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
    if (val) {
        event.setValue(0x40 + 0x20 + i);
    } else {
        event.setValue(0x40 + 0x10 + i);
    }

    event.setChannel(static_cast<uint8_t>(channel));
    events->insert(std::pair<int, NPlayEvent>(tick, event));

    event.setValue(k);
    events->insert(std::pair<int, NPlayEvent>(tick, event));
//      event.setValue(0x40 + i);
//      events->insert(std::pair<int,NPlayEvent>(tick, event));
}

//---------------------------------------------------------
//   collectProgramChanges
//---------------------------------------------------------

static void collectProgramChanges(EventMap* events, Measure const* m, Staff* staff, int tickOffset)
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
                            events->insert(std::pair<int, NPlayEvent>(tick.ticks() - 1, e1));
                        } else {
                            events->insert(std::pair<int, NPlayEvent>(tick.ticks(), e1));
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
static void renderHarmony(EventMap* events, Measure const* m, Harmony* h, int tickOffset)
{
    if (!h->isRealizable()) {
        return;
    }
    Staff* staff = m->score()->staff(h->track() / VOICES);
    const InstrChannel* channel = staff->part()->harmonyChannel();
    IF_ASSERT_FAILED(channel) {
        return;
    }

    events->registerChannel(channel->channel());
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
        events->insert(std::pair<int, NPlayEvent>(onTime, ev));
        ev.setVelo(0);
        events->insert(std::pair<int, NPlayEvent>(offTime, ev));
    }
}

static void collectGraceBeforeChordEvents(Chord* chord, EventMap* events, const SndConfig& config, int channel, double veloMultiplier,
                                          Staff* st, int tickOffset, PitchWheelRenderer& pitchWheelRenderer)
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
        graceTickOffset = graceTickSum / acciacaturaGraceSize;
    }

    if (!graceNotesMerged(chord)) {
        int currentBeaforeBeatNote = 0;
        for (Chord* c : chord->graceNotesBefore()) {
            for (const Note* note : c->notes()) {
                if (note->noteType() == NoteType::ACCIACCATURA) {
                    collectNote(events, channel, note, veloMultiplier, tickOffset, st, config,
                                pitchWheelRenderer,
                                graceTickSum - graceTickOffset * currentBeaforeBeatNote,
                                graceTickSum - graceTickOffset * (currentBeaforeBeatNote + 1) + 1);
                    currentBeaforeBeatNote++;
                } else {
                    collectNote(events, channel, note, veloMultiplier, tickOffset, st, config, pitchWheelRenderer);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   collectMeasureEventsSimple
//    the original, velocity-only method of collecting events.
//---------------------------------------------------------

void MidiRenderer::collectMeasureEventsSimple(EventMap* events, Measure const* m, const StaffContext& sctx, int tickOffset,
                                              PitchWheelRenderer& pitchWheelRenderer)
{
    staff_idx_t firstStaffIdx = sctx.staff->idx();
    staff_idx_t nextStaffIdx  = firstStaffIdx + 1;

    SegmentType st = SegmentType::ChordRest;
    track_idx_t strack = firstStaffIdx * VOICES;
    track_idx_t etrack = nextStaffIdx * VOICES;

    for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
        int tick = seg->tick().ticks();

        //render harmony
        if (sctx.renderHarmony) {
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
        }

        for (track_idx_t track = strack; track < etrack; ++track) {
            // skip linked staves, except primary
            if (!m->score()->staff(track / VOICES)->isPrimaryStaff()) {
                track += VOICES - 1;
                continue;
            }
            EngravingItem* cr = seg->element(track);
            if (cr == 0 || cr->type() != ElementType::CHORD) {
                continue;
            }

            Chord* chord = toChord(cr);
            Staff* st1   = chord->staff();
            Instrument* instr = chord->part()->instrument(Fraction::fromTicks(tick));
            int channel = instr->channel(chord->upNote()->subchannel())->channel();
            events->registerChannel(channel);

            double veloMultiplier = 1;
            for (Articulation* a : chord->articulations()) {
                if (a->playArticulation()) {
                    veloMultiplier *= instr->getVelocityMultiplier(a->articulationName());
                }
            }

            SndConfig config;             // dummy

            collectGraceBeforeChordEvents(chord, events, config, channel, veloMultiplier, st1, tickOffset, pitchWheelRenderer);

            for (const Note* note : chord->notes()) {
                collectNote(events, channel, note, veloMultiplier, tickOffset, st1, config, pitchWheelRenderer);
            }

            if (!graceNotesMerged(chord)) {
                for (Chord* c : chord->graceNotesAfter()) {
                    for (const Note* note : c->notes()) {
                        collectNote(events, channel, note, veloMultiplier, tickOffset, st1, config, pitchWheelRenderer);
                    }
                }
            }
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

void MidiRenderer::collectMeasureEventsDefault(EventMap* events, Measure const* m, const StaffContext& sctx, int tickOffset,
                                               PitchWheelRenderer& pitchWheelRenderer)
{
    int controller = getControllerFromCC(sctx.cc);

    if (controller == -1) {
        LOGW("controller for CC %d not valid", sctx.cc);
        return;
    }

    staff_idx_t firstStaffIdx = sctx.staff->idx();
    staff_idx_t nextStaffIdx  = firstStaffIdx + 1;

    SegmentType st = SegmentType::ChordRest;
    track_idx_t strack = firstStaffIdx * VOICES;
    track_idx_t etrack = nextStaffIdx * VOICES;
    for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
        Fraction tick = seg->tick();

        //render harmony
        if (sctx.renderHarmony) {
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
        }

        for (track_idx_t track = strack; track < etrack; ++track) {
            // Skip linked staves, except primary
            Staff* st1 = m->score()->staff(track / VOICES);
            if (!st1->isPrimaryStaff()) {
                track += VOICES - 1;
                continue;
            }

            EngravingItem* cr = seg->element(track);
            if (!cr) {
                continue;
            }

            if (!cr->isChord()) {
                continue;
            }

            Chord* chord = toChord(cr);

            Instrument* instr = st1->part()->instrument(tick);
            int subchannel = chord->upNote()->subchannel();
            int channel = instr->channel(subchannel)->channel();

            events->registerChannel(channel);

            // Get a velocity multiplier
            double veloMultiplier = 1;
            for (Articulation* a : chord->articulations()) {
                if (a->playArticulation()) {
                    veloMultiplier *= instr->getVelocityMultiplier(a->articulationName());
                }
            }

            bool useSND = instr->singleNoteDynamics();
            SndConfig config = SndConfig(useSND, controller, sctx.method);

            //
            // Add normal note events
            //
            collectGraceBeforeChordEvents(chord, events, config, channel, veloMultiplier, st1, tickOffset, pitchWheelRenderer);

            for (const Note* note : chord->notes()) {
                collectNote(events, channel, note, veloMultiplier, tickOffset, st1, config, pitchWheelRenderer);
            }

            if (!graceNotesMerged(chord)) {
                for (Chord* c : chord->graceNotesAfter()) {
                    for (const Note* note : c->notes()) {
                        collectNote(events, channel, note, veloMultiplier, tickOffset, st1, config, pitchWheelRenderer);
                    }
                }
            }
        }
    }
}

MidiRenderer::MidiRenderer(Score* s)
    : score(s)
{
    setMinChunkSize(MIN_CHUNK_SIZE);
}

//---------------------------------------------------------
//   collectMeasureEvents
//    redirects to the correct function based on the passed method
//---------------------------------------------------------

void MidiRenderer::collectMeasureEvents(EventMap* events, Measure const* m, const StaffContext& sctx, int tickOffset,
                                        PitchWheelRenderer& pitchWheelRenderer)
{
    switch (sctx.method) {
    case DynamicsRenderMethod::SIMPLE:
        collectMeasureEventsSimple(events, m, sctx, tickOffset, pitchWheelRenderer);
        break;
    case DynamicsRenderMethod::SEG_START:
    case DynamicsRenderMethod::FIXED_MAX:
        collectMeasureEventsDefault(events, m, sctx, tickOffset, pitchWheelRenderer);
        break;
    default:
        LOGW("Unrecognized dynamics method: %d", int(sctx.method));
        break;
    }

    collectProgramChanges(events, m, sctx.staff, tickOffset);
}

//---------------------------------------------------------
//   renderStaffChunk
//---------------------------------------------------------

void MidiRenderer::renderStaffChunk(const Chunk& chunk, EventMap* events, const StaffContext& sctx, PitchWheelRenderer& pitchWheelRenderer)
{
    Measure const* const start = chunk.startMeasure();
    Measure const* const end = chunk.endMeasure();
    const int tickOffset = chunk.tickOffset();

    Measure const* lastMeasure = start->prevMeasure();

    for (Measure const* m = start; m != end; m = m->nextMeasure()) {
        staff_idx_t staffIdx = sctx.staff->idx();
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
            collectMeasureEvents(events, playMeasure, sctx, tickOffset + offset, pitchWheelRenderer);
        } else {
            lastMeasure = m;
            collectMeasureEvents(events, lastMeasure, sctx, tickOffset, pitchWheelRenderer);
        }
    }
}

//---------------------------------------------------------
//   renderSpanners
//---------------------------------------------------------

void MidiRenderer::renderSpanners(const Chunk& chunk, EventMap* events, PitchWheelRenderer& pitchWheelRenderer)
{
    const int tickOffset = chunk.tickOffset();
    const int tick1 = chunk.tick1();
    const int tick2 = chunk.tick2();

    std::map<int, std::vector<std::pair<int, std::pair<bool, int> > > > channelPedalEvents;
    for (const auto& sp : score->spannerMap().map()) {
        Spanner* s = sp.second;

        int staff = static_cast<int>(s->staffIdx());
        int idx = s->staff()->channel(s->tick(), 0);
        int channel = s->part()->instrument(s->tick())->channel(idx)->channel();

        if (s->isPedal()) {
            channelPedalEvents.insert({ channel, std::vector<std::pair<int, std::pair<bool, int> > >() });
            std::vector<std::pair<int, std::pair<bool, int> > > pedalEventList = channelPedalEvents.at(channel);
            std::pair<int, std::pair<bool, int> > lastEvent;

            if (!pedalEventList.empty()) {
                lastEvent = pedalEventList.back();
            } else {
                lastEvent = std::pair<int, std::pair<bool, int> >(0, std::pair<bool, int>(true, staff));
            }

            int st = s->tick().ticks();
            if (st >= tick1 && st < tick2) {
                // Handle "overlapping" pedal segments (usual case for connected pedal line)
                if (lastEvent.second.first == false && lastEvent.first >= (st + tickOffset + 2)) {
                    channelPedalEvents.at(channel).pop_back();
                    channelPedalEvents.at(channel).push_back(std::pair<int,
                                                                       std::pair<bool,
                                                                                 int> >(st + tickOffset + (2 - MScore::pedalEventsMinTicks),
                                                                                        std::pair<bool, int>(false, staff)));
                }
                int a = st + tickOffset + 2;
                channelPedalEvents.at(channel).push_back(std::pair<int, std::pair<bool, int> >(a, std::pair<bool, int>(true, staff)));
            }
            if (s->tick2().ticks() >= tick1 && s->tick2().ticks() <= tick2) {
                int t = s->tick2().ticks() + tickOffset + (2 - MScore::pedalEventsMinTicks);
                const RepeatSegment& lastRepeat = *score->repeatList().back();
                if (t > lastRepeat.utick + lastRepeat.len()) {
                    t = lastRepeat.utick + lastRepeat.len();
                }
                channelPedalEvents.at(channel).push_back(std::pair<int, std::pair<bool, int> >(t, std::pair<bool, int>(false, staff)));
            }
        } else if (s->isVibrato()) {
            int stick = s->tick().ticks();
            int etick = s->tick2().ticks();
            if (stick >= tick2 || etick < tick1) {
                continue;
            }

            if (stick < tick1) {
                stick = tick1;
            }
            if (etick > tick2) {
                etick = tick2;
            }

            // from start to end of trill, send bend events at regular interval
            Vibrato* t = toVibrato(s);
            // guitar vibrato, up only
            int spitch = 0;       // 1/8 (100 is a semitone)
            int epitch = 12;
            if (t->vibratoType() == VibratoType::GUITAR_VIBRATO_WIDE) {
                spitch = 0;         // 1/4
                epitch = 25;
            }
            // vibrato with whammy bar up and down
            else if (t->vibratoType() == VibratoType::VIBRATO_SAWTOOTH_WIDE) {
                spitch = -25;         // 1/16
                epitch = 25;
            } else if (t->vibratoType() == VibratoType::VIBRATO_SAWTOOTH) {
                spitch = -12;
                epitch = 12;
            }

            collectVibrato(channel, stick, etick, spitch, epitch, pitchWheelRenderer);
        } else {
            continue;
        }
    }

    for (const auto& pedalEvents : channelPedalEvents) {
        int channel = pedalEvents.first;
        for (const auto& pe : pedalEvents.second) {
            NPlayEvent event;
            if (pe.second.first == true) {
                event = NPlayEvent(ME_CONTROLLER, static_cast<uint8_t>(channel), CTRL_SUSTAIN, 127);
            } else {
                event = NPlayEvent(ME_CONTROLLER, static_cast<uint8_t>(channel), CTRL_SUSTAIN, 0);
            }
            event.setOriginatingStaff(pe.second.second);
            events->insert(std::pair<int, NPlayEvent>(pe.first, event));
        }
    }
}

// This struct specifies how to render an articulation.
//   atype - the articulation type to implement, such as SymId::ornamentTurn
//   ostyles - the actual ornament has a property called ornamentStyle whose value is
//             a value of type OrnamentStyle.  This ostyles field indicates the
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
//            * if there is sufficient duration in the principle note, AND repeatp is true, then body
//               will be rendered multiple times, as the duration allows.
//            * to avoid a time gap (or rest) in rendering the articulation, if sustainp is true,
//               then the final note of the body will be sustained to fill the left-over time.
//    suffix - similar to prefix but played once at the end of the rendered ornament.
//    repeatp  - whether the body is repeatable in its entirety.
//    sustainp - whether the final note of the body should be sustained to fill the remaining duration.

struct OrnamentExcursion {
    SymId atype;
    std::set<OrnamentStyle> ostyles;
    int duration;
    std::vector<int> prefix;
    std::vector<int> body;
    bool repeatp;
    bool sustainp;
    std::vector<int> suffix;
};

std::set<OrnamentStyle> baroque  = { OrnamentStyle::BAROQUE };
std::set<OrnamentStyle> defstyle = { OrnamentStyle::DEFAULT };
std::set<OrnamentStyle> any; // empty set has the special meaning of any-style, rather than no-styles.
int _16th = Constants::division / 4;
int _32nd = _16th / 2;

std::vector<OrnamentExcursion> excursions = {
    //  articulation type            set of  duration       body         repeatp      suffix
    //                               styles          prefix                    sustainp
    { SymId::ornamentTurn,                any, _32nd, {},    { 1, 0, -1, 0 },   false, true, {} },
    { SymId::ornamentTurnInverted,        any, _32nd, {},    { -1, 0, 1, 0 },   false, true, {} },
    { SymId::ornamentTurnSlash,           any, _32nd, {},    { -1, 0, 1, 0 },   false, true, {} },
    { SymId::ornamentTrill,           baroque, _32nd, { 1, 0 }, { 1, 0 },        true,  true, {} },
    { SymId::ornamentTrill,          defstyle, _32nd, { 0, 1 }, { 0, 1 },        true,  true, {} },
    { SymId::brassMuteClosed,         baroque, _32nd, { 0, -1 }, { 0, -1 },      true,  true, {} },
    { SymId::ornamentMordent,             any, _32nd, {},    { 0, -1, 0 },     false, true, {} },
    { SymId::ornamentShortTrill,     defstyle, _32nd, {},    { 0, 1, 0 },      false, true, {} },// inverted mordent
    { SymId::ornamentShortTrill,      baroque, _32nd, { 1, 0, 1 }, { 0 },         false, true, {} },// short trill
    { SymId::ornamentTremblement,         any, _32nd, { 1, 0 }, { 1, 0 },        false, true, {} },
    { SymId::brassMuteClosed,        defstyle, _32nd, {},    { 0 },             false, true, {} },// regular hand-stopped brass
    { SymId::ornamentPrallMordent,        any, _32nd, {},    { 1, 0, -1, 0 },   false, true, {} },
    { SymId::ornamentLinePrall,           any, _32nd, { 2, 2, 2 }, { 1, 0 },       true,  true, {} },
    { SymId::ornamentUpPrall,             any, _16th, { -1, 0 }, { 1, 0 },        true,  true, { 1, 0 } },// p 144 Ex 152 [1]
    { SymId::ornamentUpMordent,           any, _16th, { -1, 0 }, { 1, 0 },        true,  true, { -1, 0 } },// p 144 Ex 152 [1]
    { SymId::ornamentPrecompMordentUpperPrefix, any, _16th, { 1, 1, 1, 0 }, { 1, 0 },    true,  true, {} },// p136 Cadence Appuyee [1] [2]
    { SymId::ornamentDownMordent,         any, _16th, { 1, 1, 1, 0 }, { 1, 0 },    true,  true, { -1, 0 } },// p136 Cadence Appuyee + mordent [1] [2]
    { SymId::ornamentPrallUp,             any, _16th, { 1, 0 }, { 1, 0 },        true,  true, { -1, 0 } },// p136 Double Cadence [1]
    { SymId::ornamentPrallDown,           any, _16th, { 1, 0 }, { 1, 0 },        true,  true, { -1, 0, 0, 0 } },// p144 ex 153 [1]
    { SymId::ornamentPrecompSlide,        any, _32nd, {},    { 0 },          false, true, {} }

    // [1] Some of the articulations/ornaments in the excursions table above come from
    // Baroque Music, Style and Performance A Handbook, by Robert Donington,(c) 1982
    // ISBN 0-393-30052-8, W. W. Norton & Company, Inc.

    // [2] In some cases, the example from [1] does not preserve the timing.
    // For example, illustrates 2+1/4 counts per half note.
};

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

// In the case that graceNotesBefore or graceNotesAfter are attached to a note
// with an articulation such as a trill, then the grace notes are/will-be/have-been
// already merged into the articulation.
// So this predicate, graceNotesMerged, checks for this condition to avoid calling
// functions which would re-emit the grace notes by a different algorithm.

static bool graceNotesMerged(Chord* chord)
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

//---------------------------------------------------------
//   renderMetronome
///   add metronome tick events
//---------------------------------------------------------

void MidiRenderer::renderMetronome(const Chunk& chunk, EventMap* events)
{
    const int tickOffset = chunk.tickOffset();
    Measure const* const start = chunk.startMeasure();
    Measure const* const end = chunk.endMeasure();

    for (Measure const* m = start; m != end; m = m->nextMeasure()) {
        renderMetronome(events, m, Fraction::fromTicks(tickOffset));
    }
}

//---------------------------------------------------------
//   renderMetronome
///   add metronome tick events
//---------------------------------------------------------

void MidiRenderer::renderMetronome(EventMap* events, Measure const* m, const Fraction& tickOffset)
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
        events->insert(std::pair<int, NPlayEvent>(tick + tickOffset.ticks(), NPlayEvent(timeSig.rtick2beatType(rtick))));
    }
}

void MidiRenderer::renderScore(EventMap* events, const Context& ctx)
{
    PitchWheelRenderer pitchWheelRender(wheelSpec);

    updateState();
    for (const Chunk& chunk : chunks) {
        renderChunk(chunk, events, ctx, pitchWheelRender);
    }
}

void MidiRenderer::renderChunk(const Chunk& chunk, EventMap* events, const Context& ctx, PitchWheelRenderer& pitchWheelRenderer)
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
        LOGW("Unrecognized dynamics method: %d", method);
        break;
    }

    // create note & other events
    for (Staff* st : score->staves()) {
        StaffContext sctx;
        sctx.staff = st;
        sctx.method = renderMethod;
        sctx.cc = cc;
        sctx.renderHarmony = ctx.renderHarmony;
        renderStaffChunk(chunk, events, sctx, pitchWheelRenderer);
    }
    events->fixupMIDI();

    // create sustain pedal events
    renderSpanners(chunk, events, pitchWheelRenderer);

    EventMap pitchWheelEvents = pitchWheelRenderer.renderPitchWheel();
    events->merge(pitchWheelEvents);

    if (ctx.metronome) {
        renderMetronome(chunk, events);
    }

    // NOTE:JT this is a temporary fix for duplicate events until polyphonic aftertouch support
    // can be implemented. This removes duplicate SND events.
    int lastChannel = -1;
    int lastController = -1;
    int lastValue = -1;
    for (auto i = events->begin(); i != events->end();) {
        if (i->second.type() == ME_CONTROLLER) {
            auto& event = i->second;
            if (event.channel() == lastChannel
                && event.controller() == lastController
                && event.value() == lastValue) {
                i = events->erase(i);
            } else {
                lastChannel = event.channel();
                lastController = event.controller();
                lastValue = event.value();
                i++;
            }
        } else {
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
        if (sp->isHairpin() && sp->tick2().ticks() > endTick) {
            return false;
        }
    }

    // Repeat measures rely on the previous measure
    // being properly rendered, disallow breaking
    // chunk at repeat measure.
    if (const Measure* next = last->nextMeasure()) {
        for (const Staff* staff : score->staves()) {
            if (next->isMeasureRepeatGroup(staff->idx())) {
                return false;
            }
        }
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

        Measure const* const end = rs->lastMeasure()->nextMeasure();
        int count = 0;
        bool needBreak = false;
        Measure const* chunkStart = nullptr;
        for (Measure const* m = rs->firstMeasure(); m != end; m = m->nextMeasure()) {
            if (!chunkStart) {
                chunkStart = m;
            }
            if ((++count) >= minChunkSize) {
                needBreak = true;
            }
            if (needBreak && canBreakChunk(m)) {
                chunks.emplace_back(tickOffset, chunkStart, m);
                chunkStart = nullptr;
                needBreak = false;
                count = 0;
            }
        }
        if (chunkStart) {   // last measures did not get added to chunk list
            chunks.emplace_back(tickOffset, chunkStart, rs->lastMeasure());
        }
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

std::vector<MidiRenderer::Chunk> MidiRenderer::chunksFromRange(const int fromTick, const int toTick)
{
    std::vector<Chunk> result;

    updateState();

    for (const Chunk& chunk : chunks) {
        if (chunk.utick2() >= fromTick && chunk.utick1() <= toTick) {
            result.push_back(chunk);
        }
    }

    return result;
}

//---------------------------------------------------------
//   RangeMap::setOccupied
//---------------------------------------------------------

void RangeMap::setOccupied(int tick1, int tick2)
{
    auto it1 = status.upper_bound(tick1);
    const bool beforeBegin = (it1 == status.begin());
    if (beforeBegin || (--it1)->second != Range::BEGIN) {
        if (!beforeBegin && it1->first == tick1) {
            status.erase(it1);
        } else {
            status.insert(std::make_pair(tick1, Range::BEGIN));
        }
    }

    const auto it2 = status.lower_bound(tick2);
    const bool afterEnd = (it2 == status.end());
    if (afterEnd || it2->second != Range::END) {
        if (!afterEnd && it2->first == tick2) {
            status.erase(it2);
        } else {
            status.insert(std::make_pair(tick2, Range::END));
        }
    }
}

//---------------------------------------------------------
//   RangeMap::occupiedRangeEnd
//---------------------------------------------------------

int RangeMap::occupiedRangeEnd(int tick) const
{
    const auto it = status.upper_bound(tick);
    if (it == status.begin()) {
        return tick;
    }
    const int rangeEnd = (it == status.end()) ? tick : it->first;
    if (it->second == Range::END) {
        return rangeEnd;
    }
    return tick;
}
}
