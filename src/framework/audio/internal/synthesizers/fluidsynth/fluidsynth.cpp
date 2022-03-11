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

#include "fluidsynth.h"

#include <thread>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <fluidsynth.h>

#include "log.h"
#include "realfn.h"

#include "audioerrors.h"
#include "audiotypes.h"

using namespace mu;
using namespace mu::midi;
using namespace mu::audio;
using namespace mu::audio::synth;
using namespace mu::mpe;

static constexpr double FLUID_GLOBAL_VOLUME_GAIN = 4.0;

static std::vector<double> FLUID_STANDARD_TUNING(12, -150.0);

/// @note
///  Fluid does not support MONO, so they start counting audio channels from 1, which means "1 pair of audio channels"
/// @see https://www.fluidsynth.org/api/settings_synth.html
static const audioch_t FLUID_AUDIO_CHANNELS_PAIR = 1;

struct mu::audio::synth::Fluid {
    fluid_settings_t* settings = nullptr;
    fluid_synth_t* synth = nullptr;

    ~Fluid()
    {
        delete_fluid_synth(synth);
        delete_fluid_settings(settings);
    }
};

FluidSynth::FluidSynth(const AudioSourceParams& params)
    : AbstractSynthesizer(params)
{
    m_fluid = std::make_shared<Fluid>();

    init();
}

bool FluidSynth::isValid() const
{
    return !m_soundFonts.empty();
}

SoundFontFormats FluidSynth::soundFontFormats() const
{
    return { SoundFontFormat::SF2, SoundFontFormat::SF3 };
}

Ret FluidSynth::init()
{
    auto fluid_log_out = [](int level, const char* message, void*) {
        switch (level) {
        case FLUID_PANIC:
        case FLUID_ERR:  {
            LOGE() << message;
        } break;
        case FLUID_WARN: {
            LOGW() << message;
        } break;
        case FLUID_INFO: {
            LOGI() << message;
        } break;
        case FLUID_DBG:  {
            LOGD() << message;
        } break;
        }

        if (level < FLUID_DBG) {
            bool debugme = true;
            (void)debugme;
        }
    };

    fluid_set_log_function(FLUID_PANIC, fluid_log_out, nullptr);
    fluid_set_log_function(FLUID_ERR, fluid_log_out, nullptr);
    fluid_set_log_function(FLUID_WARN, fluid_log_out, nullptr);
    fluid_set_log_function(FLUID_INFO, fluid_log_out, nullptr);
    fluid_set_log_function(FLUID_DBG, fluid_log_out, nullptr);

    m_fluid->settings = new_fluid_settings();
    fluid_settings_setnum(m_fluid->settings, "synth.gain", FLUID_GLOBAL_VOLUME_GAIN);
    fluid_settings_setint(m_fluid->settings, "synth.audio-channels", FLUID_AUDIO_CHANNELS_PAIR); // 1 pair of audio channels
    fluid_settings_setint(m_fluid->settings, "synth.lock-memory", 0);
    fluid_settings_setint(m_fluid->settings, "synth.threadsafe-api", 0);
    fluid_settings_setint(m_fluid->settings, "synth.midi-channels", 16);
    fluid_settings_setint(m_fluid->settings, "synth.dynamic-sample-loading", 1);
    fluid_settings_setint(m_fluid->settings, "synth.polyphony", 512);

    if (m_sampleRate > 0) {
        fluid_settings_setnum(m_fluid->settings, "synth.sample-rate", static_cast<double>(m_sampleRate));
    }

    fluid_settings_setint(m_fluid->settings, "synth.min-note-length", MIN_NOTE_LENGTH);

    fluid_settings_setint(m_fluid->settings, "synth.chorus.active", 0);
    fluid_settings_setnum(m_fluid->settings, "synth.chorus.depth", 8);
    fluid_settings_setnum(m_fluid->settings, "synth.chorus.level", 10);
    fluid_settings_setint(m_fluid->settings, "synth.chorus.nr", 4);
    fluid_settings_setnum(m_fluid->settings, "synth.chorus.speed", 1);

    fluid_settings_setint(m_fluid->settings, "synth.reverb.active", 1);
    fluid_settings_setnum(m_fluid->settings, "synth.reverb.room-size", 0.8);
    fluid_settings_setnum(m_fluid->settings, "synth.reverb.damp", 1.0);
    fluid_settings_setnum(m_fluid->settings, "synth.reverb.width", 0.5);
    fluid_settings_setnum(m_fluid->settings, "synth.reverb.level", 0.5);

    fluid_settings_setstr(m_fluid->settings, "audio.sample-format", "float");

    m_fluid->synth = new_fluid_synth(m_fluid->settings);

    LOGD() << "synth inited\n";
    return true;
}

void FluidSynth::setSampleRate(unsigned int sampleRate)
{
    m_sampleRate = sampleRate;
    if (m_fluid->settings) {
        fluid_settings_setnum(m_fluid->settings, "synth.sample-rate", static_cast<double>(m_sampleRate));
    }
}

Ret FluidSynth::addSoundFonts(const std::vector<io::path>& sfonts)
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return make_ret(Err::SynthNotInited);
    }

    bool ok = true;
    for (const io::path& sfont : sfonts) {
        SoundFont sf;
        sf.id = fluid_synth_sfload(m_fluid->synth, sfont.c_str(), 0);
        if (sf.id == FLUID_FAILED) {
            LOGE() << "failed load soundfont: " << sfont;
            ok = false;
            continue;
        }

        sf.path = sfont;
        m_soundFonts.push_back(std::move(sf));

        LOGI() << "success load soundfont: " << sfont;
    }

    return ok ? make_ret(Err::NoError) : make_ret(Err::SoundFontFailedLoad);
}

Ret FluidSynth::removeSoundFonts()
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return make_ret(Err::SynthNotInited);
    }

    if (m_soundFonts.empty()) {
        return make_ret(Err::NoError);
    }

    bool ok = true;
    for (const SoundFont& sf : m_soundFonts) {
        int ret = fluid_synth_sfunload(m_fluid->synth, sf.id, true);
        if (ret == FLUID_FAILED) {
            LOGE() << "failed remove soundfont id: " << sf.id << ", path: " << sf.path;
            ok = false;
        }
    }

    m_soundFonts.clear();

    return ok ? make_ret(Err::NoError) : make_ret(Err::SoundFontFailedUnload);
}

std::string FluidSynth::name() const
{
    return "Fluid";
}

AudioSourceType FluidSynth::type() const
{
    return AudioSourceType::Fluid;
}

void FluidSynth::setupSound(const PlaybackSetupData& setupData)
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return;
    }

    if (m_soundFonts.empty()) {
        LOGE() << "sound fonts not loaded";
        return;
    }

    m_channels.clear();
    m_articulationMapping.clear();

    const Programs& programs = findPrograms(setupData);
    for (const Program& program : programs) {
        m_channels.emplace(m_channels.size(), program);
    }

    m_articulationMapping = articulationSounds(setupData);
    for (const auto& pair : m_articulationMapping) {
        m_channels.emplace(m_channels.size(), pair.second);
    }

    fluid_synth_activate_octave_tuning(m_fluid->synth, 0, 0, "standard", FLUID_STANDARD_TUNING.data(), 0);

    for (const auto& pair : m_channels) {
        fluid_synth_set_interp_method(m_fluid->synth, pair.first, FLUID_INTERP_DEFAULT);
        fluid_synth_pitch_wheel_sens(m_fluid->synth, pair.first, 2);
        fluid_synth_bank_select(m_fluid->synth, pair.first, pair.second.bank);
        fluid_synth_program_change(m_fluid->synth, pair.first, pair.second.program);
        fluid_synth_cc(m_fluid->synth, pair.first, 7, 127);
        fluid_synth_cc(m_fluid->synth, pair.first, 74, 0);
        fluid_synth_set_portamento_mode(m_fluid->synth, pair.first, FLUID_CHANNEL_PORTAMENTO_MODE_EACH_NOTE);
        fluid_synth_set_legato_mode(m_fluid->synth, pair.first, FLUID_CHANNEL_LEGATO_MODE_RETRIGGER);
        fluid_synth_activate_tuning(m_fluid->synth, pair.first, 0, 0, 0);
    }
}

bool FluidSynth::hasAnythingToPlayback(const msecs_t from, const msecs_t to) const
{
    if (!m_offStreamEvents.empty() || !m_playingEvents.empty()) {
        return true;
    }

    if (!m_isActive || m_mainStreamEvents.empty()) {
        return false;
    }

    msecs_t startMsec = m_mainStreamEvents.from;
    msecs_t endMsec = m_mainStreamEvents.to;

    return from >= startMsec && to <= endMsec;
}

void FluidSynth::revokePlayingNotes()
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return;
    }

    for (const PlaybackEvent& event : m_playingEvents) {
        if (!std::holds_alternative<NoteEvent>(event)) {
            continue;
        }

        const NoteEvent& noteEvent = std::get<NoteEvent>(event);

        handleNoteOffEvents(event, noteEvent.arrangementCtx().actualTimestamp,
                            noteEvent.arrangementCtx().actualTimestamp + noteEvent.arrangementCtx().actualDuration);
    }

    m_playingEvents.clear();
    turnOffAllControllerModes();
}

void FluidSynth::flushSound()
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return;
    }

    revokePlayingNotes();

    for (const auto& pair : m_channels) {
        fluid_synth_all_sounds_off(m_fluid->synth, pair.first);
    }
}

void FluidSynth::midiChannelSoundsOff(channel_t chan)
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return;
    }

    fluid_synth_all_sounds_off(m_fluid->synth, chan);
}

bool FluidSynth::midiChannelVolume(channel_t chan, float volume)
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return false;
    }

    int val = static_cast<int>(volume * 100.f);
    val = std::clamp(val, 0, 127);

    int ret = fluid_synth_cc(m_fluid->synth, chan, VOLUME_MSB, val);
    return ret == FLUID_OK;
}

bool FluidSynth::midiChannelBalance(channel_t chan, float balance)
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return false;
    }

    balance = std::clamp(balance, -1.f, 1.f);
    float normalized = (balance < 0 ? 63 : 64) + 63 * balance;
    int val = static_cast<int>(std::lround(normalized));
    val = std::clamp(val, 0, 127);

    int ret = fluid_synth_cc(m_fluid->synth, chan, PAN_MSB, val);
    return ret == FLUID_OK;
}

bool FluidSynth::midiChannelPitch(channel_t chan, int16_t pitch)
{
    // 0-16383 with 8192 being center

    IF_ASSERT_FAILED(m_fluid->synth) {
        return false;
    }

    pitch = std::clamp(pitch, static_cast<int16_t>(-12), static_cast<int16_t>(12));

    int32_t val = (8192 * pitch) / 12;
    val = 8192 + val;
    val = std::clamp(val, 0, 16383);

    int ret = fluid_synth_pitch_bend(m_fluid->synth, chan, val);
    return ret == FLUID_OK;
}

unsigned int FluidSynth::audioChannelsCount() const
{
    return FLUID_AUDIO_CHANNELS_PAIR * 2;
}

samples_t FluidSynth::process(float* buffer, samples_t samplesPerChannel)
{
    IF_ASSERT_FAILED(samplesPerChannel > 0) {
        return 0;
    }

    msecs_t nextMsecs = samplesToMsecs(samplesPerChannel, m_sampleRate);

    if (!hasAnythingToPlayback(m_playbackPosition, m_playbackPosition + nextMsecs)) {
        return 0;
    }

    if (isActive()) {
        handleMainStreamEvents(nextMsecs);
    } else {
        handleOffStreamEvents(nextMsecs);
    }

    int result = fluid_synth_write_float(m_fluid->synth, samplesPerChannel,
                                         buffer, 0, audioChannelsCount(),
                                         buffer, 1, audioChannelsCount());

    if (result != FLUID_OK) {
        return 0;
    }

    return samplesPerChannel;
}

async::Channel<unsigned int> FluidSynth::audioChannelsCountChanged() const
{
    return m_streamsCountChanged;
}

void FluidSynth::handleMainStreamEvents(const msecs_t nextMsecs)
{
    msecs_t from = m_playbackPosition;
    msecs_t to = from + nextMsecs;

    EventsMapIteratorList range = m_mainStreamEvents.findEventsRange(from, to);

    for (const auto& it : range) {
        for (const PlaybackEvent& event : it->second) {
            if (handleNoteOnEvents(event, from, from + nextMsecs)) {
                m_playingEvents.emplace_back(event);
            }
        }
    }

    handleAlreadyPlayingEvents(from, from + nextMsecs);

    setPlaybackPosition(to);
}

void FluidSynth::handleOffStreamEvents(const msecs_t nextMsecs)
{
    msecs_t from = m_offStreamEvents.from;
    msecs_t to = m_offStreamEvents.to;

    EventsMapIteratorList range = m_offStreamEvents.findEventsRange(from, to);

    for (const auto& it : range) {
        for (const PlaybackEvent& event : it->second) {
            if (handleNoteOnEvents(event, from, from + nextMsecs)) {
                m_playingEvents.emplace_back(event);
            }
        }
    }

    handleAlreadyPlayingEvents(from, from + nextMsecs);

    m_offStreamEvents.from += nextMsecs;
    if (m_offStreamEvents.from >= m_offStreamEvents.to) {
        m_offStreamEvents.clear();
    }
}

void FluidSynth::handleAlreadyPlayingEvents(const msecs_t from, const msecs_t to)
{
    auto it = m_playingEvents.cbegin();
    while (it != m_playingEvents.cend()) {
        if (handleNoteOffEvents(*it, from, to)) {
            it = m_playingEvents.erase(it);
        } else {
            ++it;
        }
    }
}

bool FluidSynth::handleNoteOnEvents(const mpe::PlaybackEvent& event, const msecs_t from, const msecs_t to)
{
    if (!std::holds_alternative<NoteEvent>(event)) {
        return false;
    }

    const NoteEvent& noteEvent = std::get<NoteEvent>(event);

    timestamp_t timestampFrom = noteEvent.arrangementCtx().actualTimestamp;

    channel_t channelIdx = channel(noteEvent);

    handlePitchBendControl(noteEvent, channelIdx, from, to);
    handleAftertouch(noteEvent, channelIdx, from, to);

    if (timestampFrom < from || timestampFrom >= to) {
        return false;
    }

    note_idx_t noteIdx = noteIndex(noteEvent.pitchCtx().nominalPitchLevel);

    if (isLegatoModeApplicable(noteEvent)) {
        enableLegatoMode(channelIdx);
    }

    if (isPedalModeApplicable(noteEvent)) {
        enablePedalMode(channelIdx);
    }

    if (isPortamentoModeApplicable(noteEvent)) {
        enablePortamentoMode(noteEvent, channelIdx);
        noteIdx = noteIndex(noteEvent.pitchCtx().nominalPitchLevel + noteEvent.pitchCtx().pitchCurve.maxAmplitudeLevel());
    }

    fluid_synth_noteon(m_fluid->synth,
                       channelIdx,
                       noteIdx,
                       noteVelocity(noteEvent.expressionCtx().expressionCurve.maxAmplitudeLevel()));

    return true;
}

bool FluidSynth::handleNoteOffEvents(const mpe::PlaybackEvent& event, const msecs_t from, const msecs_t to)
{
    if (!std::holds_alternative<NoteEvent>(event)) {
        return false;
    }

    const NoteEvent& noteEvent = std::get<NoteEvent>(event);

    timestamp_t timestampFrom = noteEvent.arrangementCtx().actualTimestamp;
    timestamp_t timestampTo = timestampFrom + noteEvent.arrangementCtx().actualDuration;

    channel_t channelIdx = channel(noteEvent);

    handlePitchBendControl(noteEvent, channelIdx, from, to);
    handleAftertouch(noteEvent, channelIdx, from, to);

    if (timestampTo <= from || timestampTo > to) {
        return false;
    }

    note_idx_t noteIdx = noteIndex(noteEvent.pitchCtx().nominalPitchLevel);

    if (!isLegatoModeApplicable(noteEvent)) {
        disableLegatoMode(channelIdx);
    }

    if (!isPedalModeApplicable(noteEvent)) {
        disablePedalMode(channelIdx);
    }

    if (hasToDisablePortamentoMode(noteEvent, channelIdx, from, to)) {
        disablePortamentoMode(channelIdx);
        noteIdx = noteIndex(noteEvent.pitchCtx().nominalPitchLevel + noteEvent.pitchCtx().pitchCurve.maxAmplitudeLevel());
    }

    fluid_synth_noteoff(m_fluid->synth,
                        channelIdx,
                        noteIdx);

    return true;
}

channel_t FluidSynth::channel(const mpe::NoteEvent& noteEvent) const
{
    for (const auto& pair : m_articulationMapping) {
        if (noteEvent.expressionCtx().articulations.contains(pair.first)) {
            return findChannelByProgram(pair.second);
        }
    }

    return 0;
}

channel_t FluidSynth::findChannelByProgram(const midi::Program& program) const
{
    for (const auto& pair : m_channels) {
        if (pair.second == program) {
            return pair.first;
        }
    }

    return 0;
}

note_idx_t FluidSynth::noteIndex(const mpe::pitch_level_t pitchLevel) const
{
    static constexpr mpe::pitch_level_t MIN_SUPPORTED_LEVEL = mpe::pitchLevel(PitchClass::C, 0);
    static constexpr note_idx_t MIN_SUPPORTED_NOTE = 12; // MIDI equivalent for C0
    static constexpr mpe::pitch_level_t MAX_SUPPORTED_LEVEL = mpe::pitchLevel(PitchClass::C, 8);
    static constexpr note_idx_t MAX_SUPPORTED_NOTE = 108; // MIDI equivalent for C8

    if (pitchLevel <= MIN_SUPPORTED_LEVEL) {
        return MIN_SUPPORTED_NOTE;
    }

    if (pitchLevel >= MAX_SUPPORTED_LEVEL) {
        return MAX_SUPPORTED_NOTE;
    }

    float stepCount = MIN_SUPPORTED_NOTE + ((pitchLevel - MIN_SUPPORTED_LEVEL) / static_cast<float>(mpe::PITCH_LEVEL_STEP));

    return RealRound(stepCount, 0);
}

velocity_t FluidSynth::noteVelocity(const mpe::dynamic_level_t dynamicLevel) const
{
    static constexpr mpe::dynamic_level_t MIN_SUPPORTED_LEVEL = mpe::dynamicLevelFromType(DynamicType::ppp);
    static constexpr mpe::dynamic_level_t MAX_SUPPORTED_LEVEL = mpe::dynamicLevelFromType(DynamicType::fff);
    static constexpr midi::velocity_t MIN_SUPPORTED_VELOCITY = 16; // MIDI equivalent for PPP
    static constexpr midi::velocity_t MAX_SUPPORTED_VELOCITY = 127; // MIDI equivalent for FFF
    static constexpr midi::velocity_t VELOCITY_STEP = 16;

    if (dynamicLevel <= MIN_SUPPORTED_LEVEL) {
        return MIN_SUPPORTED_VELOCITY;
    }

    if (dynamicLevel >= MAX_SUPPORTED_LEVEL) {
        return MAX_SUPPORTED_VELOCITY;
    }

    float stepCount = ((dynamicLevel - MIN_SUPPORTED_LEVEL) / static_cast<float>(mpe::DYNAMIC_LEVEL_STEP));

    if (dynamicLevel == mpe::dynamicLevelFromType(DynamicType::Natural)) {
        stepCount += 0.5;
    }

    if (dynamicLevel > mpe::dynamicLevelFromType(DynamicType::Natural)) {
        stepCount -= 1;
    }

    return RealRound(MIN_SUPPORTED_VELOCITY + (stepCount * VELOCITY_STEP), 0);
}

void FluidSynth::handlePitchBendControl(const mpe::NoteEvent& noteEvent, const midi::channel_t channelIdx, const msecs_t from,
                                        const msecs_t to)
{
    if (!noteEvent.expressionCtx().articulations.containsAnyOf(BEND_SUPPORTED_TYPES.cbegin(),
                                                               BEND_SUPPORTED_TYPES.cend())) {
        return;
    }

    for (const auto& pair : noteEvent.pitchCtx().pitchCurve) {
        timestamp_t eventTimestamp = noteEvent.arrangementCtx().actualTimestamp;
        timestamp_t currentPoint = eventTimestamp + noteEvent.arrangementCtx().actualDuration * percentageToFactor(pair.first);

        if (currentPoint < from || currentPoint > to) {
            continue;
        }

        int pitchBendValue = 8192;

        if (pair.first == HUNDRED_PERCENT) {
            fluid_synth_channel_pressure(m_fluid->synth, channelIdx, pitchBendValue);
            return;
        }

        float ratio = pair.second / static_cast<float>(ONE_PERCENT);
        ratio = std::clamp(ratio, -2.f, 2.f);

        pitchBendValue = 8192 + RealRound(ratio * 4096, 0);
        pitchBendValue = std::clamp(pitchBendValue, 0, 16383);

        fluid_synth_pitch_bend(m_fluid->synth, channelIdx, pitchBendValue);
        return;
    }
}

void FluidSynth::handleAftertouch(const mpe::NoteEvent& noteEvent, const midi::channel_t channelIdx, const msecs_t from, const msecs_t to)
{
    if (!noteEvent.expressionCtx().articulations.containsAnyOf(AFTERTOUCH_SUPPORTED_TYPES.cbegin(),
                                                               AFTERTOUCH_SUPPORTED_TYPES.cend())) {
        return;
    }

    for (const auto& pair : noteEvent.pitchCtx().pitchCurve) {
        timestamp_t eventTimestamp = noteEvent.arrangementCtx().actualTimestamp;
        timestamp_t currentPoint = eventTimestamp + noteEvent.arrangementCtx().actualDuration * percentageToFactor(pair.first);

        if (currentPoint < from || currentPoint > to) {
            continue;
        }

        int value = 0;

        if (pair.first == HUNDRED_PERCENT) {
            fluid_synth_channel_pressure(m_fluid->synth, channelIdx, value);
            return;
        }

        float ratio = pair.second / static_cast<float>(ONE_PERCENT);
        ratio = std::clamp(ratio, -1.f, 1.f);

        value = 64 + RealRound(ratio * 16, 0);
        value = std::clamp(value, 0, 127);

        fluid_synth_channel_pressure(m_fluid->synth, channelIdx, value);
        return;
    }
}

void FluidSynth::enablePortamentoMode(const mpe::NoteEvent& noteEvent, const midi::channel_t channelIdx)
{
    ControllersModeContext& ctx = m_controllersModeMap[channelIdx];

    if (ctx.isPortamentoModeEnabled) {
        return;
    }

    ctx.isPortamentoModeEnabled = true;

    fluid_synth_cc(m_fluid->synth, channelIdx, 65, 127);
    fluid_synth_cc(m_fluid->synth, channelIdx, 5, 74);

    note_idx_t val = noteIndex(noteEvent.pitchCtx().nominalPitchLevel);

    fluid_synth_cc(m_fluid->synth, channelIdx, 84, val);

    return;
}

void FluidSynth::enableLegatoMode(const midi::channel_t channelIdx)
{
    ControllersModeContext& ctx = m_controllersModeMap[channelIdx];

    if (ctx.isLegatoModeEnabled) {
        return;
    }

    ctx.isLegatoModeEnabled = true;
    fluid_synth_cc(m_fluid->synth, channelIdx, 68, 127);

    return;
}

void FluidSynth::enablePedalMode(const midi::channel_t channelIdx)
{
    ControllersModeContext& ctx = m_controllersModeMap[channelIdx];

    if (ctx.isDamperPedalEnabled) {
        return;
    }

    ctx.isDamperPedalEnabled = true;
    fluid_synth_cc(m_fluid->synth, channelIdx, 64, 127);

    return;
}

bool FluidSynth::isPortamentoModeApplicable(const mpe::NoteEvent& noteEvent) const
{
    if (m_setupData.category == SoundCategory::Keyboards
        || m_setupData.category == SoundCategory::Percussions) {
        return false;
    }

    const ArticulationMap& articulations = noteEvent.expressionCtx().articulations;

    return articulations.containsAnyOf(PORTAMENTO_CC_SUPPORTED_TYPES.cbegin(),
                                       PORTAMENTO_CC_SUPPORTED_TYPES.cend());
}

bool FluidSynth::isLegatoModeApplicable(const mpe::NoteEvent& noteEvent) const
{
    const ArticulationMap& articulations = noteEvent.expressionCtx().articulations;

    return articulations.containsAnyOf(LEGATO_CC_SUPPORTED_TYPES.cbegin(),
                                       LEGATO_CC_SUPPORTED_TYPES.cend());
}

bool FluidSynth::isPedalModeApplicable(const mpe::NoteEvent& noteEvent) const
{
    const ArticulationMap& articulations = noteEvent.expressionCtx().articulations;

    return articulations.containsAnyOf(PEDAL_CC_SUPPORTED_TYPES.cbegin(),
                                       PEDAL_CC_SUPPORTED_TYPES.cend());
}

bool FluidSynth::hasToDisablePortamentoMode(const mpe::NoteEvent& noteEvent, const midi::channel_t channelIdx, const msecs_t from,
                                            const msecs_t to) const
{
    ControllersModeContext& ctx = m_controllersModeMap[channelIdx];

    if (!isPortamentoModeApplicable(noteEvent)) {
        return ctx.isPortamentoModeEnabled;
    }

    timestamp_t endPoint = noteEvent.arrangementCtx().actualTimestamp + noteEvent.arrangementCtx().actualDuration;

    if (endPoint < from || endPoint > to) {
        return false;
    }

    return true;
}

void FluidSynth::disablePortamentoMode(const midi::channel_t channelIdx)
{
    ControllersModeContext& ctx = m_controllersModeMap[channelIdx];

    if (ctx.isPortamentoModeEnabled) {
        fluid_synth_cc(m_fluid->synth, channelIdx, 65, 0);
        ctx.isPortamentoModeEnabled = false;
    }
}

void FluidSynth::disableLegatoMode(const midi::channel_t channelIdx)
{
    ControllersModeContext& ctx = m_controllersModeMap[channelIdx];

    if (ctx.isLegatoModeEnabled) {
        fluid_synth_cc(m_fluid->synth, channelIdx, 68, 0);
        ctx.isLegatoModeEnabled = false;
    }
}

void FluidSynth::disablePedalMode(const midi::channel_t channelIdx)
{
    ControllersModeContext& ctx = m_controllersModeMap[channelIdx];

    if (ctx.isDamperPedalEnabled) {
        fluid_synth_cc(m_fluid->synth, channelIdx, 64, 0);
        ctx.isDamperPedalEnabled = false;
    }
}

void FluidSynth::turnOffAllControllerModes()
{
    for (const auto& pair : m_controllersModeMap) {
        disablePortamentoMode(pair.first);
        disableLegatoMode(pair.first);
        disablePedalMode(pair.first);
    }
}
