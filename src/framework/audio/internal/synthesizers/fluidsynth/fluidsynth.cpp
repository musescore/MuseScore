//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "fluidsynth.h"

#include <thread>
#include <sstream>
#include <algorithm>
#include <cmath>

#include <fluidsynth.h>

#include "log.h"
#include "audioerrors.h"

static const double FLUID_GLOBAL_VOLUME_GAIN{ 1.8 };

using namespace mu;
using namespace mu::midi;
using namespace mu::audio::synth;

struct mu::audio::synth::Fluid {
    fluid_settings_t* settings = nullptr;
    fluid_synth_t* synth = nullptr;

    ~Fluid()
    {
        delete_fluid_synth(synth);
        delete_fluid_settings(settings);
    }
};

FluidSynth::FluidSynth()
{
    m_fluid = std::make_shared<Fluid>();
}

bool FluidSynth::isValid() const
{
    return !m_soundFonts.empty();
}

std::string FluidSynth::name() const
{
    return "Fluid";
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
    fluid_settings_setint(m_fluid->settings, "synth.audio-channels", 1);
    fluid_settings_setint(m_fluid->settings, "synth.lock-memory", 0);
    fluid_settings_setint(m_fluid->settings, "synth.threadsafe-api", 0);
    fluid_settings_setint(m_fluid->settings, "synth.midi-channels", 80);

    if (m_sampleRate > 0) {
        fluid_settings_setnum(m_fluid->settings, "synth.sample-rate", static_cast<double>(m_sampleRate));
    }

    //fluid_settings_setint(_fluid->settings, "synth.min-note-length", 50);
    //fluid_settings_setint(_fluid->settings, "synth.polyphony", conf.polyphony);

//    fluid_settings_setstr(m_fluid->settings, "synth.chorus.active", 0);
//    fluid_settings_setnum(m_fluid->settings, "synth.chorus.depth", 8);
//    fluid_settings_setnum(m_fluid->settings, "synth.chorus.level", 10);
//    fluid_settings_setint(m_fluid->settings, "synth.chorus.nr", 4);
//    fluid_settings_setnum(m_fluid->settings, "synth.chorus.speed", 1);

    /*
 https://github.com/FluidSynth/fluidsynth/wiki/UserManual
 rev_preset
        num:0 roomsize:0.2 damp:0.0 width:0.5 level:0.9
        num:1 roomsize:0.4 damp:0.2 width:0.5 level:0.8
        num:2 roomsize:0.6 damp:0.4 width:0.5 level:0.7
        num:3 roomsize:0.8 damp:0.7 width:0.5 level:0.6
        num:4 roomsize:0.8 damp:1.0 width:0.5 level:0.5
*/

//    fluid_settings_setstr(m_fluid->settings, "synth.reverb.active", 0);
//    fluid_settings_setnum(m_fluid->settings, "synth.reverb.room-size", 0.8);
//    fluid_settings_setnum(m_fluid->settings, "synth.reverb.damp", 1.0);
//    fluid_settings_setnum(m_fluid->settings, "synth.reverb.width", 0.5);
//    fluid_settings_setnum(m_fluid->settings, "synth.reverb.level", 0.5);

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
        m_preallocated.resize(int(m_sampleRate) * 2);
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

Ret FluidSynth::setupChannels(const std::vector<Event>& events)
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return make_ret(Err::SynthNotInited);
    }

    if (m_soundFonts.empty()) {
        LOGE() << "sound fonts not loaded";
        return make_ret(Err::SoundFontNotLoaded);
    }

    fluid_synth_program_reset(m_fluid->synth);
    fluid_synth_system_reset(m_fluid->synth);

    std::set<channel_t> channels;
    for (const Event& e: events) {
        channels.insert(e.channel());
    }

    for (channel_t ch : channels) {
        fluid_synth_set_interp_method(m_fluid->synth, ch, FLUID_INTERP_DEFAULT);
        fluid_synth_pitch_wheel_sens(m_fluid->synth, ch, 12);
    }

    for (const Event& e: events) {
        handleEvent(e);
    }

    return make_ret(Err::NoError);
}

bool FluidSynth::handleEvent(const Event& e)
{
    if (e.isChannelVoice20()) {
        auto events = e.toMIDI10();
        bool ret = true;
        for (auto& event : events) {
            ret &= handleEvent(event);
        }
        return ret;
    }

    if (m_isLoggingSynthEvents) {
        LOGD() << e.to_string();
    }

    int ret = FLUID_OK;
    switch (e.type()) {
    case EventType::ME_NOTEON: {
        ret = fluid_synth_noteon(m_fluid->synth, e.channel(), e.note(), e.velocity());
    } break;
    case EventType::ME_NOTEOFF: {
        ret = fluid_synth_noteoff(m_fluid->synth, e.channel(), e.note());
    } break;
    case EventType::ME_CONTROLLER: {
        if (e.index() == CntrType::CTRL_PROGRAM) {
            ret = fluid_synth_program_change(m_fluid->synth, e.channel(), e.data());
        } else {
            ret = fluid_synth_cc(m_fluid->synth, e.channel(), e.index(), e.data());
        }
    } break;
    case EventType::ME_PROGRAM: {
        fluid_synth_program_change(m_fluid->synth, e.channel(), e.program());
    } break;
    case EventType::ME_PITCHBEND: {
        ret = fluid_synth_pitch_bend(m_fluid->synth, e.channel(), e.data());
    } break;
    default: {
        LOGW() << "not supported event type: " << static_cast<int>(e.type());
        ret = FLUID_FAILED;
    }
    }

    return ret == FLUID_OK;
}

void FluidSynth::allSoundsOff()
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return;
    }

    fluid_synth_all_notes_off(m_fluid->synth, -1);
    fluid_synth_all_sounds_off(m_fluid->synth, -1);
}

void FluidSynth::flushSound()
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return;
    }

    fluid_synth_all_notes_off(m_fluid->synth, -1);
    fluid_synth_all_sounds_off(m_fluid->synth, -1);

    int size = int(m_sampleRate);

    fluid_synth_write_float(m_fluid->synth, size, &m_preallocated[0], 0, 1, &m_preallocated[0], size, 1);
}

void FluidSynth::channelSoundsOff(channel_t chan)
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return;
    }

    fluid_synth_all_sounds_off(m_fluid->synth, chan);
}

bool FluidSynth::channelVolume(channel_t chan, float volume)
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return false;
    }

    int val = static_cast<int>(volume * 100.f);
    val = std::clamp(val, 0, 127);

    int ret = fluid_synth_cc(m_fluid->synth, chan, VOLUME_MSB, val);
    return ret == FLUID_OK;
}

bool FluidSynth::channelBalance(channel_t chan, float balance)
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

bool FluidSynth::channelPitch(channel_t chan, int16_t pitch)
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

bool FluidSynth::isActive() const
{
    return m_isActive;
}

void FluidSynth::setIsActive(bool arg)
{
    m_isActive = arg;
}

void FluidSynth::writeBuf(float* stream, unsigned int samples)
{
    IF_ASSERT_FAILED(samples > 0) {
        return;
    }

    fluid_synth_write_float(m_fluid->synth, static_cast<int>(samples),
                            stream, 0, AUDIO_CHANNELS,
                            stream, 1, AUDIO_CHANNELS);
}

unsigned int FluidSynth::streamCount() const
{
    return synth::AUDIO_CHANNELS;
}

void FluidSynth::forward(unsigned int sampleCount)
{
    writeBuf(m_buffer.data(), sampleCount);
}

async::Channel<unsigned int> FluidSynth::streamsCountChanged() const
{
    return m_streamsCountChanged;
}

const float* FluidSynth::data() const
{
    return m_buffer.data();
}

void FluidSynth::setBufferSize(unsigned int samples)
{
    auto sc = streamCount();
    auto targetSize = samples * sc;
    if (targetSize > 0 && m_buffer.size() < targetSize) {
        m_buffer.resize(samples * streamCount());
    }
}
