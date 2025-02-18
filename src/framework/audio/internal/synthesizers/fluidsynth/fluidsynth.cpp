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

#include <fluidsynth.h>

#include "sfcachedloader.h"
#include "audioerrors.h"
#include "audiotypes.h"

#include "log.h"

using namespace muse;
using namespace muse::midi;
using namespace muse::audio;
using namespace muse::audio::synth;
using namespace muse::mpe;

static constexpr double FLUID_GLOBAL_VOLUME_GAIN = 4.8;
static constexpr int DEFAULT_MIDI_VOLUME = 100;
static constexpr msecs_t MIN_NOTE_LENGTH = 10;

/// @note
///  Fluid does not support MONO, so they start counting audio channels from 1, which means "1 pair of audio channels"
/// @see https://www.fluidsynth.org/api/settings_synth.html
static constexpr unsigned int FLUID_AUDIO_CHANNELS_PAIR = 1;
static constexpr unsigned int FLUID_AUDIO_CHANNELS_COUNT = FLUID_AUDIO_CHANNELS_PAIR * 2;

static constexpr bool STAFF_TO_MIDIOUT_CHANNEL = true;

struct muse::audio::synth::Fluid {
    fluid_settings_t* settings = nullptr;
    fluid_synth_t* synth = nullptr;

    ~Fluid()
    {
        delete_fluid_synth(synth);
        delete_fluid_settings(settings);
    }
};

FluidSynth::FluidSynth(const AudioSourceParams& params, const modularity::ContextPtr& iocCtx)
    : AbstractSynthesizer(params, iocCtx)
{
    m_fluid = std::make_shared<Fluid>();

    init();
}

bool FluidSynth::isValid() const
{
    return m_fluid->synth != nullptr;
}

Ret FluidSynth::init()
{
    auto fluid_log_out = [](int level, const char* message, void*) {
#undef LOG_TAG
#define LOG_TAG "FluidSynth"

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

#undef LOG_TAG
#define LOG_TAG CLASSFUNC

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
    fluid_settings_setint(m_fluid->settings, "synth.reverb.active", 0);

    fluid_settings_setstr(m_fluid->settings, "audio.sample-format", "float");

    createFluidInstance();

    m_sequencer.setOnOffStreamFlushed([this]() {
        m_allNotesOffRequested = true;
    });

    LOGD() << "synth inited\n";
    return true;
}

void FluidSynth::createFluidInstance()
{
    m_fluid->synth = new_fluid_synth(m_fluid->settings);

    fluid_sfloader_t* sfloader = new_fluid_sfloader(loadSoundFont, delete_fluid_sfloader);

    fluid_sfloader_set_data(sfloader, m_fluid->settings);
    fluid_synth_add_sfloader(m_fluid->synth, sfloader);
}

void FluidSynth::allNotesOff()
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return;
    }

    fluid_synth_all_notes_off(m_fluid->synth, -1);

    int lastChannelIdx = static_cast<int>(m_sequencer.channels().lastIndex());
    for (int i = 0; i < lastChannelIdx; ++i) {
        setControllerValue(i, midi::SUSTAIN_PEDAL_CONTROLLER, 0);
        setControllerValue(i, midi::SOSTENUTO_PEDAL_CONTROLLER, 0);
        setPitchBend(i, 8192);
    }

    auto port = midiOutPort();
    if (port->isConnected()) {
        // Send all notes off to connected midi ports.
        // Room for improvement:
        // - We could record which groups/channels we sent something or which channels were scheduled in the sequencer.
        // - We could send all events at once.
        int lowerBound = 0;
        int upperBound = 0;
        if (STAFF_TO_MIDIOUT_CHANNEL) {
            int channel = m_sequencer.lastStaff();
            if (channel < 0) {
                return;
            }
            channel = channel % 16;
            lowerBound = channel;
            upperBound = channel + 1;
        } else {
            upperBound = min(lastChannelIdx, 16);
        }
        for (int i = lowerBound; i < upperBound; i++) {
            muse::midi::Event e(muse::midi::Event::Opcode::ControlChange, muse::midi::Event::MessageType::ChannelVoice20);
            e.setChannel(i);
            e.setIndex(123); // CC#123 = All notes off
            port->sendEvent(e);
        }
        for (int i = lowerBound; i < upperBound; i++) {
            muse::midi::Event e(muse::midi::Event::Opcode::ControlChange, muse::midi::Event::MessageType::ChannelVoice20);
            e.setChannel(i);
            e.setIndex(midi::SUSTAIN_PEDAL_CONTROLLER);
            e.setData(0);
            port->sendEvent(e);
        }
        for (int i = lowerBound; i < upperBound; i++) {
            muse::midi::Event e(muse::midi::Event::Opcode::PitchBend, muse::midi::Event::MessageType::ChannelVoice20);
            e.setChannel(i);
            e.setData(0x80000000);
            port->sendEvent(e);
        }
    }
}

bool FluidSynth::handleEvent(const midi::Event& event)
{
    int ret = FLUID_OK;
    switch (event.opcode()) {
    case Event::Opcode::NoteOn: {
        // fluid_synth_noteon expects 0...127
        ret = fluid_synth_noteon(m_fluid->synth, event.channel(), event.note(), event.velocity7());
        m_tuning.add(event.note(), event.pitchTuningCents());
    } break;
    case Event::Opcode::NoteOff: {
        ret = fluid_synth_noteoff(m_fluid->synth, event.channel(), event.note());
        m_tuning.add(event.note(), event.pitchTuningCents());
    } break;
    case Event::Opcode::ControlChange: {
        if (event.index() == muse::midi::EXPRESSION_CONTROLLER) {
            ret = setExpressionLevel(event.data7());
        } else {
            ret = fluid_synth_cc(m_fluid->synth, event.channel(), event.index(), event.data7());
        }
    } break;
    case Event::Opcode::ProgramChange: {
        fluid_synth_program_change(m_fluid->synth, event.channel(), event.program());
    } break;
    case Event::Opcode::PitchBend: {
        ret = setPitchBend(event.channel(), event.pitchBend14());
    } break;
    default: {
        LOGD() << "not supported event type: " << event.opcodeString();
        ret = FLUID_FAILED;
    }
    }

    if (STAFF_TO_MIDIOUT_CHANNEL && event.isChannelVoice()) {
        int staff = m_sequencer.lastStaff();
        if (staff >= 0) {
            int channel = staff % 16;
            midi::Event me(event);
            me.setChannel(channel);
            midiOutPort()->sendEvent(me);
        }
    } else {
        midiOutPort()->sendEvent(event);
    }

    return ret == FLUID_OK;
}

void FluidSynth::setSampleRate(unsigned int sampleRate)
{
    if (m_sampleRate == sampleRate) {
        return;
    }

    m_sampleRate = sampleRate;
    if (m_fluid->settings) {
        fluid_settings_setnum(m_fluid->settings, "synth.sample-rate", static_cast<double>(m_sampleRate));
    }

    if (m_fluid->synth) {
        delete_fluid_synth(m_fluid->synth);
    }

    createFluidInstance();
    addSoundFonts(std::vector<io::path_t>(m_sfontPaths.cbegin(), m_sfontPaths.cend()));

    if (m_setupData.isValid()) {
        setupSound(m_setupData);
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
        m_sfontPaths.insert(sfont);
    }

    return ok ? make_ret(Err::NoError) : make_ret(Err::SoundFontFailedLoad);
}

void FluidSynth::setPreset(const std::optional<midi::Program>& preset)
{
    m_preset = preset;
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

    fluid_synth_activate_key_tuning(m_fluid->synth, 0, 0, "standard", NULL, true);

    auto setupChannel = [this](const midi::channel_t channelIdx, const midi::Program& program) {
        fluid_synth_set_interp_method(m_fluid->synth, channelIdx, FLUID_INTERP_DEFAULT);
        fluid_synth_pitch_wheel_sens(m_fluid->synth, channelIdx, 24);
        fluid_synth_bank_select(m_fluid->synth, channelIdx, program.bank);
        fluid_synth_program_change(m_fluid->synth, channelIdx, program.program);
        fluid_synth_cc(m_fluid->synth, channelIdx, 7, DEFAULT_MIDI_VOLUME);
        fluid_synth_cc(m_fluid->synth, channelIdx, muse::midi::EXPRESSION_CONTROLLER, m_sequencer.naturalExpressionLevel());
        fluid_synth_cc(m_fluid->synth, channelIdx, 74, 0);
        fluid_synth_set_portamento_mode(m_fluid->synth, channelIdx, FLUID_CHANNEL_PORTAMENTO_MODE_EACH_NOTE);
        fluid_synth_set_legato_mode(m_fluid->synth, channelIdx, FLUID_CHANNEL_LEGATO_MODE_RETRIGGER);
        fluid_synth_activate_tuning(m_fluid->synth, channelIdx, 0, 0, 0);
    };

    m_sequencer.channelAdded().onReceive(this, setupChannel);
    m_sequencer.init(setupData, m_preset, setupData.supportsSingleNoteDynamics);

    for (const auto& voice : m_sequencer.channels().data()) {
        for (const auto& pair : voice.second) {
            const ChannelMap::ChannelMapping& channelMapping = pair.second;
            setupChannel(channelMapping.first, channelMapping.second);
        }
    }
}

void FluidSynth::setupEvents(const mpe::PlaybackData& playbackData)
{
    m_sequencer.load(playbackData);
}

const mpe::PlaybackData& FluidSynth::playbackData() const
{
    return m_sequencer.playbackData();
}

void FluidSynth::revokePlayingNotes()
{
    m_allNotesOffRequested = true;
}

void FluidSynth::flushSound()
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return;
    }

    allNotesOff();

    fluid_synth_all_sounds_off(m_fluid->synth, -1);
    fluid_synth_cc(m_fluid->synth, -1, 121, 127);
}

bool FluidSynth::isActive() const
{
    return m_sequencer.isActive();
}

void FluidSynth::setIsActive(const bool isActive)
{
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

    if (isActive()) {
        setExpressionLevel(m_sequencer.currentExpressionLevel());
    }
}

unsigned int FluidSynth::audioChannelsCount() const
{
    return FLUID_AUDIO_CHANNELS_COUNT;
}

samples_t FluidSynth::process(float* buffer, samples_t samplesPerChannel)
{
    IF_ASSERT_FAILED(samplesPerChannel > 0) {
        return 0;
    }

    if (m_allNotesOffRequested) {
        allNotesOff();
        m_allNotesOffRequested = false;
    }

    const msecs_t nextMsecs = samplesToMsecs(samplesPerChannel, m_sampleRate);
    const FluidSequencer::EventSequenceMap sequences = m_sequencer.movePlaybackForward(nextMsecs);
    samples_t sampleOffset = 0;

    for (auto it = sequences.cbegin(); it != sequences.cend(); ++it) {
        samples_t durationInSamples = samplesPerChannel - sampleOffset;

        auto nextIt = std::next(it);
        if (nextIt != sequences.cend()) {
            msecs_t duration = nextIt->first - it->first;
            durationInSamples = microSecsToSamples(duration, m_sampleRate);
        }

        IF_ASSERT_FAILED(sampleOffset + durationInSamples <= samplesPerChannel) {
            break;
        }

        if (!processSequence(it->second, durationInSamples, buffer + sampleOffset * FLUID_AUDIO_CHANNELS_COUNT)) {
            return 0;
        }

        sampleOffset += durationInSamples;
    }

    return samplesPerChannel;
}

bool FluidSynth::processSequence(const FluidSequencer::EventSequence& sequence, const samples_t samples, float* buffer)
{
    if (!sequence.empty()) {
        m_tuning.reset();
    }

    for (const FluidSequencer::EventType& event : sequence) {
        handleEvent(std::get<midi::Event>(event));
    }

    fluid_synth_tune_notes(m_fluid->synth, 0, 0, m_tuning.size(), m_tuning.keys.data(), m_tuning.pitches.data(), true);

    if (samples == 0) {
        return true;
    }

    int result = fluid_synth_write_float(m_fluid->synth, samples,
                                         buffer, 0, FLUID_AUDIO_CHANNELS_COUNT,
                                         buffer, 1, FLUID_AUDIO_CHANNELS_COUNT);

    return result == FLUID_OK;
}

async::Channel<unsigned int> FluidSynth::audioChannelsCountChanged() const
{
    return m_streamsCountChanged;
}

void FluidSynth::toggleExpressionController()
{
    if (isActive()) {
        setExpressionLevel(m_sequencer.currentExpressionLevel());
    } else {
        setExpressionLevel(m_sequencer.naturalExpressionLevel());
    }
}

int FluidSynth::setExpressionLevel(int level)
{
    midi::channel_t lastChannelIdx = m_sequencer.channels().lastIndex();

    for (midi::channel_t i = 0; i < lastChannelIdx; ++i) {
        fluid_synth_cc(m_fluid->synth, i, muse::midi::EXPRESSION_CONTROLLER, level);
    }

    return FLUID_OK;
}

int FluidSynth::setControllerValue(int channel, int ctrl, int value)
{
    int currentValue = 0;
    fluid_synth_get_cc(m_fluid->synth, channel, ctrl, &currentValue);

    if (value == currentValue) {
        return FLUID_OK;
    }

    return fluid_synth_cc(m_fluid->synth, channel, ctrl, value);
}

int FluidSynth::setPitchBend(int channel, int pitchBend)
{
    int currentValue = 0;
    fluid_synth_get_pitch_bend(m_fluid->synth, channel, &currentValue);

    if (pitchBend == currentValue) {
        return FLUID_OK;
    }

    return fluid_synth_pitch_bend(m_fluid->synth, channel, pitchBend);
}
