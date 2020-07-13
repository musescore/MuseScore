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

#include "modularity/ioc.h"
#include "iaudiodriver.h"

using namespace mu::audio::engine;

constexpr int BUF_SIZE{ 1024 };

struct AudioEngine::SL {
    SoLoud::Soloud engine;
};

AudioEngine::AudioEngine()
{
    _sl = std::unique_ptr<SL>(new SL);
}

AudioEngine::~AudioEngine()
{
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

    if (!_inited) {
        return false;
    }

    return true;
}

bool AudioEngine::init()
{
    if (isInited()) {
        return true;
    }

    auto res = _sl->engine.init(SoLoud::Soloud::CLIP_ROUNDOFF,
                                SoLoud::Soloud::MUAUDIO,
                                SoLoud::Soloud::AUTO,
                                BUF_SIZE,
                                2);

    if (res == SoLoud::SO_NO_ERROR) {
        LOGI() << "success inited audio engine";
        _inited = true;
    } else {
        LOGE() << "failed inited audio engine, err: " << res;
        _inited = false;
    }

    return _inited;
}

float AudioEngine::samplerate() const
{
    return _sl->engine.getBackendSamplerate();
}

IAudioEngine::handle AudioEngine::play(IAudioSource* s, float volume, float pan, bool paused)
{
    LOGI() << "play start at " << _syncPlaybackPosition;

    IF_ASSERT_FAILED(s) {
        return 0;
    }

    SoLoud::AudioSource* sa = s->source();
    IF_ASSERT_FAILED(sa) {
        return 0;
    }

    handle h = _sl->engine.play(*sa, volume, pan, paused);

    Source ss;
    ss.handel = h;
    ss.source = s;
    ss.playing = !paused;
    _sources.insert({ h, ss });

    if (!paused) {
        syncAll(_syncPlaybackPosition);
    }
    return h;
}

void AudioEngine::seek(time sec)
{
    LOGD() << "seek to " << sec;
    _syncPlaybackPosition = sec;
    syncAll(_syncPlaybackPosition);
}

void AudioEngine::stop(handle h)
{
    LOGD() << "stop";
    _sl->engine.stop(h);
    _sources.erase(h);
}

void AudioEngine::pause(handle h, bool paused)
{
    LOGI() << (paused ? "pause" : "resume");

    auto it = _sources.find(h);
    if (it != _sources.end()) {
        it->second.playing = !paused;
    }

    if (!paused) {
        syncAll(_syncPlaybackPosition);
    }

    _sl->engine.setPause(h, paused);
}

void AudioEngine::syncAll(time sec)
{
    for (auto it = _sources.begin(); it != _sources.end(); ++it) {
        if (it->second.playing) {
            it->second.source->sync(sec);
        }
    }
}

void AudioEngine::stopAll()
{
    _sl->engine.stopAll();
}

IAudioEngine::time AudioEngine::position(handle h) const
{
    return _sl->engine.getStreamPosition(h);
}

bool AudioEngine::isEnded(handle h) const
{
    //! NOTE When does the source end
    //! Soloud deletes voice, i.e. handle becomes invalid
    return !_sl->engine.isValidVoiceHandle(h);
}

void AudioEngine::setVolume(handle h, float volume)
{
    _sl->engine.setVolume(h, volume);
}

void AudioEngine::setPan(handle h, float val)
{
    _sl->engine.setPan(h, val);
}

void AudioEngine::setPlaySpeed(handle h, float speed)
{
    _sl->engine.setRelativePlaySpeed(h, speed);
}
