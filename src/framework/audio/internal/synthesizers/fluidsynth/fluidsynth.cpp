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

#include "sfcachedloader.h"
#include "audioerrors.h"
#include "audiotypes.h"

using namespace mu;
using namespace mu::midi;
using namespace mu::audio;
using namespace mu::audio::synth;
using namespace mu::mpe;

static constexpr double FLUID_GLOBAL_VOLUME_GAIN = 4.8;
static constexpr int DEFAULT_MIDI_VOLUME = 100;

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
    return m_fluid->synth != nullptr;
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
    fluid_settings_setnum(m_fluid->settings, "synth.reverb.room-size", 1.0);
    fluid_settings_setnum(m_fluid->settings, "synth.reverb.damp", 1.0);
    fluid_settings_setnum(m_fluid->settings, "synth.reverb.width", 1.5);
    fluid_settings_setnum(m_fluid->settings, "synth.reverb.level", 0.8);

    fluid_settings_setstr(m_fluid->settings, "audio.sample-format", "float");

    m_fluid->synth = new_fluid_synth(m_fluid->settings);

    fluid_sfloader_t* sfloader = new_fluid_sfloader(loadSoundFont, delete_fluid_sfloader);

    fluid_sfloader_set_data(sfloader, m_fluid->settings);
    fluid_synth_add_sfloader(m_fluid->synth, sfloader);

    m_currentExpressionLevel = DEFAULT_MIDI_VOLUME;

    LOGD() << "synth inited\n";
    return true;
}

bool FluidSynth::handleEvent(const midi::Event& event)
{
    int ret = FLUID_OK;
    switch (event.opcode()) {
    case Event::Opcode::NoteOn: {
        ret = fluid_synth_noteon(m_fluid->synth, event.channel(), event.note(), event.velocity());
    } break;
    case Event::Opcode::NoteOff: {
        ret = fluid_synth_noteoff(m_fluid->synth, event.channel(), event.note());
    } break;
    case Event::Opcode::ControlChange: {
        int currentValue = 0;
        fluid_synth_get_cc(m_fluid->synth, event.channel(), event.index(), &currentValue);

        if (event.data() == static_cast<uint32_t>(currentValue)) {
            break;
        }

        ret = fluid_synth_cc(m_fluid->synth, event.channel(), event.index(), event.data());
        updateCurrentExpressionLevel(event);

    } break;
    case Event::Opcode::ProgramChange: {
        fluid_synth_program_change(m_fluid->synth, event.channel(), event.program());
    } break;
    case Event::Opcode::PitchBend: {
        ret = fluid_synth_pitch_bend(m_fluid->synth, event.channel(), event.data());
    } break;
    default: {
        LOGD() << "not supported event type: " << event.opcodeString();
        ret = FLUID_FAILED;
    }
    }

    return ret == FLUID_OK;
}

void FluidSynth::updateCurrentExpressionLevel(const midi::Event& event)
{
    if (event.index() == 11) {
        m_currentExpressionLevel = event.data();
    }
}

void FluidSynth::setSampleRate(unsigned int sampleRate)
{
    m_sampleRate = sampleRate;
    if (m_fluid->settings) {
        fluid_settings_setnum(m_fluid->settings, "synth.sample-rate", static_cast<double>(m_sampleRate));
    }
}

Ret FluidSynth::addSoundFonts(const std::vector<io::path_t>& sfonts)
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return make_ret(Err::SynthNotInited);
    }

    bool ok = true;
    for (const io::path_t& sfont : sfonts) {
        if (fluid_synth_sfload(m_fluid->synth, sfont.c_str(), 0) == FLUID_FAILED) {
            LOGE() << "failed load soundfont: " << sfont;
            ok = false;
            continue;
        }

        LOGI() << "success load soundfont: " << sfont;
    }

    return ok ? make_ret(Err::NoError) : make_ret(Err::SoundFontFailedLoad);
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

    m_channels.clear();
    m_articulationMapping.clear();

    const Programs& programs = findPrograms(setupData);
    for (const Program& program : programs) {
        m_channels.emplace(static_cast<int>(m_channels.size()), program);
    }

    m_articulationMapping = articulationSounds(setupData);
    for (const auto& pair : m_articulationMapping) {
        m_channels.emplace(static_cast<int>(m_channels.size()), pair.second);
    }

    fluid_synth_activate_octave_tuning(m_fluid->synth, 0, 0, "standard", FLUID_STANDARD_TUNING.data(), 0);

    for (const auto& pair : m_channels) {
        fluid_synth_set_interp_method(m_fluid->synth, pair.first, FLUID_INTERP_DEFAULT);
        fluid_synth_pitch_wheel_sens(m_fluid->synth, pair.first, 2);
        fluid_synth_bank_select(m_fluid->synth, pair.first, pair.second.bank);
        fluid_synth_program_change(m_fluid->synth, pair.first, pair.second.program);
        fluid_synth_cc(m_fluid->synth, pair.first, 7, m_currentExpressionLevel);
        fluid_synth_cc(m_fluid->synth, pair.first, 74, 0);
        fluid_synth_set_portamento_mode(m_fluid->synth, pair.first, FLUID_CHANNEL_PORTAMENTO_MODE_EACH_NOTE);
        fluid_synth_set_legato_mode(m_fluid->synth, pair.first, FLUID_CHANNEL_LEGATO_MODE_RETRIGGER);
        fluid_synth_activate_tuning(m_fluid->synth, pair.first, 0, 0, 0);
    }

    m_sequencer.init(m_articulationMapping, m_channels);
}

void FluidSynth::setupEvents(const mpe::PlaybackData& playbackData)
{
    m_sequencer.load(playbackData);
}

bool FluidSynth::hasAnythingToPlayback(const msecs_t from, const msecs_t to) const
{
    if (!m_offStreamEvents.empty()) {
        return true;
    }

    if (!m_isActive || m_mainStreamEvents.empty()) {
        return false;
    }

    msecs_t startMsec = m_mainStreamEvents.from;
    msecs_t endMsec = m_mainStreamEvents.to;

    bool lowerBound = from >= startMsec && from < endMsec;
    bool upperBound = to > startMsec && to <= endMsec;

    return lowerBound || upperBound;
}

void FluidSynth::revokePlayingNotes()
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return;
    }

    fluid_synth_all_notes_off(m_fluid->synth, -1);
}

void FluidSynth::flushSound()
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return;
    }

    revokePlayingNotes();

    fluid_synth_all_sounds_off(m_fluid->synth, -1);
    fluid_synth_cc(m_fluid->synth, -1, 121, 127);
}

bool FluidSynth::isActive() const
{
    return m_sequencer.isActive();
}

void FluidSynth::setIsActive(const bool isActive)
{
    AbstractSynthesizer::setIsActive(isActive);

    m_sequencer.setActive(isActive);
    toggleExpressionController();
}

msecs_t FluidSynth::playbackPosition() const
{
    return m_sequencer.playbackPosition();
}

void FluidSynth::setPlaybackPosition(const msecs_t newPosition)
{
    m_sequencer.setPlaybackPosition(newPosition);
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

    const FluidSequencer::EventSequence& sequence = m_sequencer.eventsToBePlayed(nextMsecs);

    for (const midi::Event& event : sequence) {
        handleEvent(event);
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

void FluidSynth::toggleExpressionController()
{
    int volume = DEFAULT_MIDI_VOLUME;

    if (m_isActive) {
        volume = m_currentExpressionLevel;
    }

    for (const auto& pair : m_channels) {
        fluid_synth_cc(m_fluid->synth, pair.first, 11, volume);
    }
}
