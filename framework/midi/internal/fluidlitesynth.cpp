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

#include "fluidlitesynth.h"

#include <thread>
#include <sstream>
#include <algorithm>
#include <cmath>

#include <fluidlite.h>

#include "log.h"

namespace  {
static double GLOBAL_VOLUME_GAIN{ 1.8 };
}

enum fluid_status {
    FLUID_OK = 0,
    FLUID_FAILED = -1
};

template<class T>
static const T& clamp(const T& v, const T& lo, const T& hi)
{
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

using namespace mu;
using namespace mu::midi;

struct mu::midi::Fluid {
    fluid_settings_t* settings = nullptr;
    fluid_synth_t* synth = nullptr;
    int sfontID = FLUID_FAILED;

    ~Fluid()
    {
        delete_fluid_synth(synth);
        delete_fluid_settings(settings);
    }
};

FluidLiteSynth::FluidLiteSynth()
{
    m_fluid = std::make_shared<Fluid>();
}

std::string FluidLiteSynth::name() const
{
    return "fluidlite";
}

Ret FluidLiteSynth::init(float samplerate)
{
    auto fluid_log_out = [](int level, char* message, void*) {
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
    fluid_settings_setnum(m_fluid->settings, "synth.gain", GLOBAL_VOLUME_GAIN);
    fluid_settings_setint(m_fluid->settings, "synth.audio-channels", 1);
    fluid_settings_setint(m_fluid->settings, "synth.lock-memory", 0);
    fluid_settings_setint(m_fluid->settings, "synth.threadsafe-api", 0);
    fluid_settings_setnum(m_fluid->settings, "synth.sample-rate", static_cast<double>(samplerate));
    fluid_settings_setint(m_fluid->settings, "synth.midi-channels", 80);

    //fluid_settings_setint(_fluid->settings, "synth.min-note-length", 50);
    //fluid_settings_setint(_fluid->settings, "synth.polyphony", conf.polyphony);

    fluid_settings_setstr(m_fluid->settings, "synth.chorus.active", "no");
    fluid_settings_setnum(m_fluid->settings, "synth.chorus.depth", 8);
    fluid_settings_setnum(m_fluid->settings, "synth.chorus.level", 10);
    fluid_settings_setint(m_fluid->settings, "synth.chorus.nr", 4);
    fluid_settings_setnum(m_fluid->settings, "synth.chorus.speed", 1);

    /*
 https://github.com/FluidSynth/fluidsynth/wiki/UserManual
 rev_preset
        num:0 roomsize:0.2 damp:0.0 width:0.5 level:0.9
        num:1 roomsize:0.4 damp:0.2 width:0.5 level:0.8
        num:2 roomsize:0.6 damp:0.4 width:0.5 level:0.7
        num:3 roomsize:0.8 damp:0.7 width:0.5 level:0.6
        num:4 roomsize:0.8 damp:1.0 width:0.5 level:0.5
*/

    fluid_settings_setstr(m_fluid->settings, "synth.reverb.active", "no");
    fluid_settings_setnum(m_fluid->settings, "synth.reverb.room-size", 0.8);
    fluid_settings_setnum(m_fluid->settings, "synth.reverb.damp", 1.0);
    fluid_settings_setnum(m_fluid->settings, "synth.reverb.width", 0.5);
    fluid_settings_setnum(m_fluid->settings, "synth.reverb.level", 0.5);

    fluid_settings_setstr(m_fluid->settings, "audio.sample-format", "float");

    m_fluid->synth = new_fluid_synth(m_fluid->settings);

    m_sampleRate = samplerate;
    // preallocated buffer size must be at least (sample rate) * (channels number)
    m_preallocated.resize(int(m_sampleRate) * 2);

    LOGD() << "synth inited\n";
    return true;
}

Ret FluidLiteSynth::addSoundFont(const io::path& filePath)
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return false;
    }

    m_fluid->sfontID = fluid_synth_sfload(m_fluid->synth, filePath.c_str(), 0);
    if (m_fluid->sfontID == FLUID_FAILED) {
        LOGE() << "failed load soundfont: " << filePath;
        return false;
    }

    LOGI() << "success load soundfont: " << filePath;

    return true;
}

Ret FluidLiteSynth::setupChannels(const Programs& programs)
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return false;
    }

    IF_ASSERT_FAILED(m_fluid->sfontID != FLUID_FAILED) {
        return false;
    }

    m_programs = programs;

    fluid_synth_program_reset(m_fluid->synth);
    fluid_synth_system_reset(m_fluid->synth);

    for (const Program& prog : m_programs) {
        fluid_synth_reset_tuning(m_fluid->synth, prog.channel);

        fluid_synth_bank_select(m_fluid->synth, prog.channel, prog.bank);
        fluid_synth_program_change(m_fluid->synth, prog.channel, prog.program);

        fluid_synth_set_interp_method(m_fluid->synth, prog.channel, FLUID_INTERP_DEFAULT);
        fluid_synth_pitch_wheel_sens(m_fluid->synth, prog.channel, 12);

        //LOGD() << "ch: " << prog.ch << ", prog: " << prog.prog << ", bank: " << prog.bank << "\n";
    }

    return make_ret(Ret::Code::Ok);
}

const Program& FluidLiteSynth::program(uint16_t chan) const
{
    for (const Program& p : m_programs) {
        if (p.channel == chan) {
            return p;
        }
    }
    static Program dummy;
    return dummy;
}

bool FluidLiteSynth::handleEvent(const Event& e)
{
    if (m_isLoggingSynthEvents) {
        const Program& p = program(e.channel);
        LOGD() << " bank: " << p.bank << " program: " << p.program << " " << e.to_string();
    }

    int ret = FLUID_OK;
    switch (e.type) {
    case ME_NOTEON: {
        ret = fluid_synth_noteon(m_fluid->synth, e.channel, e.a, e.b);
    } break;
    case ME_NOTEOFF: {
        ret = fluid_synth_noteoff(m_fluid->synth, e.channel, e.a);
    } break;
    case ME_CONTROLLER: {
        ret = fluid_synth_cc(m_fluid->synth, e.channel, e.a, e.b);
    } break;
    case ME_PROGRAMCHANGE: {
        fluid_synth_program_change(m_fluid->synth, e.channel, e.b);
    } break;
    case ME_PITCHBEND: {
        int pitch = e.b << 7 | e.a;
        ret = fluid_synth_pitch_bend(m_fluid->synth, e.channel, pitch);
    } break;
    case META_TEMPO: {
        // noop
    } break;
    default: {
        LOGW() << "not supported event type: " << static_cast<int>(e.type);
        ret = FLUID_FAILED;
    }
    }

    return ret == FLUID_OK;
}

void FluidLiteSynth::allSoundsOff()
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return;
    }

    fluid_synth_all_notes_off(m_fluid->synth, -1);
    fluid_synth_all_sounds_off(m_fluid->synth, -1);
}

void FluidLiteSynth::flushSound()
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return;
    }

    fluid_synth_all_notes_off(m_fluid->synth, -1);
    fluid_synth_all_sounds_off(m_fluid->synth, -1);

    int size = int(m_sampleRate);

    fluid_synth_write_float(m_fluid->synth, size, &m_preallocated[0], 0, 1, &m_preallocated[0], size, 1);
}

void FluidLiteSynth::channelSoundsOff(channel_t chan)
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return;
    }

    fluid_synth_all_sounds_off(m_fluid->synth, chan);
}

bool FluidLiteSynth::channelVolume(channel_t chan, float volume)
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return false;
    }

    int val = static_cast<int>(volume * 100.f);
    val = clamp(val, 0, 127);

    int ret = fluid_synth_cc(m_fluid->synth, chan, VOLUME_MSB, val);
    return ret == FLUID_OK;
}

bool FluidLiteSynth::channelBalance(channel_t chan, float balance)
{
    IF_ASSERT_FAILED(m_fluid->synth) {
        return false;
    }

    balance = clamp(balance, -1.f, 1.f);
    float normalized = (balance < 0 ? 63 : 64) + 63 * balance;
    int val = static_cast<int>(std::lround(normalized));
    val = clamp(val, 0, 127);

    int ret = fluid_synth_cc(m_fluid->synth, chan, PAN_MSB, val);
    return ret == FLUID_OK;
}

bool FluidLiteSynth::channelPitch(channel_t chan, int16_t pitch)
{
    // 0-16383 with 8192 being center

    IF_ASSERT_FAILED(m_fluid->synth) {
        return false;
    }

    pitch = clamp(pitch, static_cast<int16_t>(-12), static_cast<int16_t>(12));

    int32_t val = (8192 * pitch) / 12;
    val = 8192 + val;
    val = clamp(val, 0, 16383);

    int ret = fluid_synth_pitch_bend(m_fluid->synth, chan, val);
    return ret == FLUID_OK;
}

void FluidLiteSynth::writeBuf(float* stream, unsigned int samples)
{
    IF_ASSERT_FAILED(samples > 0) {
        return;
    }

    fluid_synth_write_float(m_fluid->synth, static_cast<int>(samples), stream, 0, 1, stream, static_cast<int>(samples), 1);
}
