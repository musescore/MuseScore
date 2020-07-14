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

#include "audioengine.h"

#include <soloud.h>

#include "log.h"
#include "ptrutils.h"
#include "../audioerrors.h"

using namespace mu::audio::engine;

//! Defines the buffer size for the audio driver.
//! If the value is too small, there may be stuttering sound
//! If the value is too large, there will be a big latency
//! before the start of playback and after the end of playback

constexpr int BUF_SIZE{ 1024 };

struct AudioEngine::SL {
    SoLoud::Soloud engine;
};

AudioEngine::AudioEngine()
{
    m_sl = std::shared_ptr<SL>(new SL);
}

bool AudioEngine::isInited() const
{
    std::shared_ptr<IAudioDriver> drv = driver();
    if (!drv) {
        return false;
    }

    if (!drv->isOpened()) {
        return false;
    }

    if (!m_inited) {
        return false;
    }

    return true;
}

mu::Ret AudioEngine::init()
{
    if (isInited()) {
        return make_ret(Ret::Code::Ok);
    }

    int res = m_sl->engine.init(SoLoud::Soloud::CLIP_ROUNDOFF,
                                SoLoud::Soloud::MUAUDIO,
                                SoLoud::Soloud::AUTO,
                                BUF_SIZE,
                                2);

    if (res == SoLoud::SO_NO_ERROR) {
        LOGI() << "success inited audio engine";
        m_inited = true;
        return make_ret(Ret::Code::Ok);
    }

    m_inited = false;

    Err err = Err::UnknownError;
    if (SoLoud::INVALID_PARAMETER == res) {
        err = Err::EngineInvalidParameter;
    } else if (int(Err::DriverNotFound) == res) {
        err = Err::DriverNotFound;
    } else if (int(Err::DriverOpenFailed) == res) {
        err = Err::DriverOpenFailed;
    }

    LOGE() << "failed inited audio engine, err: " << int(err);
    return make_ret(err);
}

void AudioEngine::deinit()
{
    m_sl->engine.deinit();
    m_inited = false;
}

float AudioEngine::sampleRate() const
{
    return m_sl->engine.getBackendSamplerate();
}

IAudioEngine::handle AudioEngine::play(IAudioSource* s, float volume, float pan, bool paused)
{
    IF_ASSERT_FAILED(s) {
        return 0;
    }

    IF_ASSERT_FAILED(isInited()) {
        return 0;
    }

    s->setSampleRate(sampleRate());

    SoLoud::AudioSource* sa = s->source();
    IF_ASSERT_FAILED(sa) {
        return 0;
    }

    handle h = m_sl->engine.play(*sa, volume, pan, paused);

    Source ss;
    ss.handel = h;
    ss.source = s;
    ss.playing = !paused;
    m_sources.insert({ h, ss });

    return h;
}

void AudioEngine::seek(time sec)
{
    LOGD() << "seek to " << sec;
    syncAll(sec);
}

void AudioEngine::stop(handle h)
{
    LOGD() << "stop";
    m_sl->engine.stop(h);
    m_sources.erase(h);
}

void AudioEngine::pause(handle h, bool paused)
{
    LOGI() << (paused ? "pause" : "resume");

    auto it = m_sources.find(h);
    if (it != m_sources.end()) {
        it->second.playing = !paused;
    }

    m_sl->engine.setPause(h, paused);
}

void AudioEngine::syncAll(time sec)
{
    for (auto it = m_sources.begin(); it != m_sources.end(); ++it) {
        if (it->second.playing) {
            it->second.source->sync(sec);
        }
    }
}

void AudioEngine::stopAll()
{
    m_sl->engine.stopAll();
}

IAudioEngine::time AudioEngine::position(handle h) const
{
    return m_sl->engine.getStreamPosition(h);
}

bool AudioEngine::isEnded(handle h) const
{
    //! NOTE When does the source end
    //! Soloud deletes voice, i.e. handle becomes invalid
    return !m_sl->engine.isValidVoiceHandle(h);
}

void AudioEngine::setVolume(handle h, float volume)
{
    m_sl->engine.setVolume(h, volume);
}

void AudioEngine::setPan(handle h, float val)
{
    m_sl->engine.setPan(h, val);
}

void AudioEngine::setPlaySpeed(handle h, float speed)
{
    m_sl->engine.setRelativePlaySpeed(h, speed);
}
