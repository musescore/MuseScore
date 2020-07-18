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

struct AudioEngine::SL {
    SoLoud::Soloud engine;
};

AudioEngine::AudioEngine()
{
    m_sl = std::shared_ptr<SL>(new SL);
    m_sl->engine.mBackendData = this;
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
                                SAMPLE_GRANULARITY, // 1024
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

IAudioEngine::handle AudioEngine::play(std::shared_ptr<IAudioSource> s, float volume, float pan, bool paused)
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
    return h;
}

void AudioEngine::seek(handle h, time sec)
{
    m_sl->engine.seek(h, sec);
}

void AudioEngine::stop(handle h)
{
    m_sl->engine.stop(h);
}

void AudioEngine::setPause(handle h, bool paused)
{
    m_sl->engine.setPause(h, paused);
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

mu::async::Notification AudioEngine::playCallbackCalled() const
{
    return m_playCallbackCalled;
}

void AudioEngine::onPlayCallbackCalled()
{
    m_playCallbackCalled.notify();
}
