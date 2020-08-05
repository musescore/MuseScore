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
#include "zerberussynth.h"

#include "log.h"
#include "zerberus/zerberus.h"
#include "audio/midi/event.h"

using namespace mu;
using namespace mu::midi;

template<class T>
static const T& clamp(const T& v, const T& lo, const T& hi)
{
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

static unsigned int AUDIO_CHANNELS = 2;

ZerberusSynth::ZerberusSynth()
{
    m_zerb = new zerberus::Zerberus();
}

std::string ZerberusSynth::name() const
{
    return "zerberus";
}

Ret ZerberusSynth::init(float samplerate)
{
    if (m_zerb) {
        delete m_zerb;
    }

    m_zerb = new zerberus::Zerberus();
    m_zerb->setSampleRate(samplerate);

    // preallocated buffer size must be at least (sample rate) * (channels number)
    m_preallocated.resize(int(samplerate) * AUDIO_CHANNELS);

    return true;
}

Ret ZerberusSynth::addSoundFont(const io::path& filePath)
{
    IF_ASSERT_FAILED(m_zerb) {
        return false;
    }

    bool ok = m_zerb->addSoundFont(io::pathToQString(filePath));
    if (!ok) {
        return false;
    }

    return true;
}

Ret ZerberusSynth::setupChannels(const Programs& programs)
{
    IF_ASSERT_FAILED(m_zerb) {
        return false;
    }

    m_programs = programs;

    for (const Program& prog : m_programs) {
        m_zerb->controller(prog.channel, Ms::CTRL_PROGRAM, prog.program);
    }

    return make_ret(Ret::Code::Ok);
}

bool ZerberusSynth::handleEvent(const Event& e)
{
//    if (m_isLoggingSynthEvents) {
//        const Program& p = program(e.channel);
//        LOGD() << " bank: " << p.bank << " program: " << p.program << " " << e.to_string();
//    }

    IF_ASSERT_FAILED(m_zerb) {
        return false;
    }

    int ret = true;
    switch (e.type) {
    case ME_NOTEON: {
        ret = m_zerb->noteOn(e.channel, e.a, e.b);
    } break;
    case ME_NOTEOFF: { //msb << 7 | lsb
        ret = m_zerb->noteOff(e.channel, e.a);
    } break;
    case ME_CONTROLLER: {
        ret = m_zerb->controller(e.channel, e.a, e.b);
    } break;
    case ME_PROGRAMCHANGE: {
        ret = false;
        NOT_IMPLEMENTED;
    } break;
    case ME_PITCHBEND: {
        ret = false;
        NOT_IMPLEMENTED;
    } break;
    case META_TEMPO: {
        // noop
    } break;
    default: {
        NOT_SUPPORTED << " event type: " << static_cast<int>(e.type);
        ret = false;
    }
    }

    return ret;
}

void ZerberusSynth::allSoundsOff()
{
    IF_ASSERT_FAILED(m_zerb) {
        return;
    }

    m_zerb->allSoundsOff(-1);
}

void ZerberusSynth::flushSound()
{
    IF_ASSERT_FAILED(m_zerb) {
        return;
    }

    m_zerb->allSoundsOff(-1);

    int samples = int(m_zerb->sampleRate());

    m_zerb->process(samples, &m_preallocated[0], nullptr, nullptr);
}

void ZerberusSynth::channelSoundsOff(channel_t chan)
{
    IF_ASSERT_FAILED(m_zerb) {
        return;
    }

    m_zerb->allSoundsOff(chan);
}

bool ZerberusSynth::channelVolume(channel_t chan, float volume)
{
    int val = static_cast<int>(volume * 100.f);
    val = clamp(val, 0, 127);

    m_zerb->controller(chan, Ms::CTRL_VOLUME, val);
    return false;
}

bool ZerberusSynth::channelBalance(channel_t chan, float val)
{
    UNUSED(chan);
    UNUSED(val);
    NOT_IMPLEMENTED;
    return false;
}

bool ZerberusSynth::channelPitch(channel_t chan, int16_t pitch)
{
    UNUSED(chan);
    UNUSED(pitch);
    NOT_IMPLEMENTED;
    return false;
}

void ZerberusSynth::writeBuf(float* stream, unsigned int samples)
{
    IF_ASSERT_FAILED(m_zerb) {
        return;
    }

    IF_ASSERT_FAILED(samples > 0) {
        return;
    }

    for (unsigned int i = 0; i < (samples * AUDIO_CHANNELS); ++i) {
        stream[i] = 0.0f;
    }

    m_zerb->process(samples, stream, nullptr, nullptr);
}
