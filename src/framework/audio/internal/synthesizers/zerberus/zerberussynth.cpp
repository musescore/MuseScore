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
#include "zerberussynth.h"
#include <algorithm>

#include "log.h"
#include "io/path.h"
#include "internal/zerberus.h"
#include "internal/controllers.h"
#include "midi/miditypes.h"
#include "midi/midierrors.h"

using namespace mu;
using namespace mu::midi;
using namespace mu::audio::synth;

ZerberusSynth::ZerberusSynth()
{
    m_zerb = new zerberus::Zerberus();
}

ZerberusSynth::~ZerberusSynth()
{
    if (m_zerb) {
        delete m_zerb;
    }
}

bool ZerberusSynth::isValid() const
{
    return !m_zerb->soundFonts().empty();
}

std::string ZerberusSynth::name() const
{
    return "Zerberus";
}

SoundFontFormats ZerberusSynth::soundFontFormats() const
{
    return { SoundFontFormat::SFZ };
}

Ret ZerberusSynth::init()
{
    if (!m_zerb) {
        m_zerb = new zerberus::Zerberus();
        m_zerb->setSampleRate(m_sampleRate);
    }
    return true;
}

void ZerberusSynth::setSampleRate(unsigned int sampleRate)
{
    m_sampleRate = sampleRate;
    if (m_zerb) {
        m_zerb->setSampleRate(sampleRate);
        m_preallocated.resize(int(sampleRate) * AUDIO_CHANNELS, 0);
    }
}

Ret ZerberusSynth::addSoundFonts(const std::vector<io::path>& sfonts)
{
    IF_ASSERT_FAILED(m_zerb) {
        return make_ret(Err::SynthNotInited);
    }

    bool ok = true;
    for (const io::path& sfont : sfonts) {
        bool sok = m_zerb->addSoundFont(sfont.toQString());
        if (!sok) {
            LOGE() << "failed load soundfont: " << sfont;
            ok = false;
        } else {
            LOGI() << "success load soundfont: " << sfont;
        }
    }

    return ok ? make_ret(Err::NoError) : make_ret(Err::SoundFontFailedLoad);
}

Ret ZerberusSynth::removeSoundFonts()
{
    IF_ASSERT_FAILED(m_zerb) {
        return make_ret(Err::SynthNotInited);
    }

    QStringList paths = m_zerb->soundFonts();
    if (paths.empty()) {
        return make_ret(Err::NoError);
    }

    bool ok = true;
    for (const QString& path : paths) {
        ok &= m_zerb->removeSoundFont(path);
    }

    LOGI() << "sound fonts removed";

    return ok ? make_ret(Err::NoError) : make_ret(Err::SoundFontFailedUnload);
}

Ret ZerberusSynth::setupMidiChannels(const std::vector<Event>& events)
{
    IF_ASSERT_FAILED(m_zerb) {
        return make_ret(Err::SynthNotInited);
    }

    if (m_zerb->soundFonts().empty()) {
        LOGE() << "sound fonts not loaded";
        return make_ret(Err::SoundFontNotLoaded);
    }

    for (const Event& e : events) {
        handleEvent(e);
    }

    return make_ret(Err::NoError);
}

bool ZerberusSynth::handleEvent(const Event& e)
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

    IF_ASSERT_FAILED(m_zerb) {
        return false;
    }

    int ret = true;
    switch (e.type()) {
    case EventType::ME_NOTEON: {
        ret = m_zerb->noteOn(e.channel(), e.note(), e.velocity());
    } break;
    case EventType::ME_NOTEOFF: {
        ret = m_zerb->noteOff(e.channel(), e.note());
    } break;
    case EventType::ME_CONTROLLER: {
        ret = m_zerb->controller(e.channel(), e.index(), e.data());
    } break;
    case EventType::ME_PROGRAM: {
        ret = false;
        NOT_IMPLEMENTED;
    } break;
    case EventType::ME_PITCHBEND: {
        ret = false;
        NOT_IMPLEMENTED;
    } break;
    default: {
        NOT_SUPPORTED << " event type: " << static_cast<int>(e.type());
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

    m_zerb->process(samples, m_preallocated.data(), nullptr, nullptr);
}

void ZerberusSynth::midiChannelSoundsOff(channel_t chan)
{
    IF_ASSERT_FAILED(m_zerb) {
        return;
    }

    m_zerb->allSoundsOff(chan);
}

bool ZerberusSynth::midiChannelVolume(channel_t chan, float volume)
{
    int val = static_cast<int>(volume * 100.f);
    val = std::clamp(val, 0, 127);

    m_zerb->controller(chan, zerberus::CTRL_VOLUME, val);
    return false;
}

bool ZerberusSynth::midiChannelBalance(channel_t chan, float val)
{
    UNUSED(chan);
    UNUSED(val);
    NOT_IMPLEMENTED;
    return false;
}

bool ZerberusSynth::midiChannelPitch(channel_t chan, int16_t pitch)
{
    UNUSED(chan);
    UNUSED(pitch);
    NOT_IMPLEMENTED;
    return false;
}

void ZerberusSynth::setIsActive(bool arg)
{
    m_isActive = arg;
}

bool ZerberusSynth::isActive() const
{
    return m_isActive;
}

void ZerberusSynth::writeBuf(float* stream, unsigned int samples)
{
    IF_ASSERT_FAILED(m_zerb) {
        return;
    }

    IF_ASSERT_FAILED(samples > 0) {
        return;
    }

    if (!isActive()) {
        return;
    }

    m_zerb->process(samples, stream, nullptr, nullptr);
}

unsigned int ZerberusSynth::audioChannelsCount() const
{
    return synth::AUDIO_CHANNELS;
}

void ZerberusSynth::process(float* buffer, unsigned int sampleCount)
{
    writeBuf(buffer, sampleCount);
}

async::Channel<unsigned int> ZerberusSynth::audioChannelsCountChanged() const
{
    return m_streamsCountChanged;
}
