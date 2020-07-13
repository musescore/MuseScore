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
#ifndef MU_AUDIO_AUDIOENGINE_H
#define MU_AUDIO_AUDIOENGINE_H

#include <memory>
#include <map>
#include <set>

#include "../iaudioengine.h"
#include "modularity/ioc.h"
#include "../iaudiodriver.h"

namespace mu {
namespace audio {
namespace engine {
class IAudioDriver;
class AudioEngine : public IAudioEngine
{
    INJECT(audio, IAudioDriver, driver)

public:
    AudioEngine();
    ~AudioEngine() override;

    bool init() override;
    bool isInited() const override;

    float samplerate() const override;

    handle play(IAudioSource* s, float volume = -1, float pan = 0, bool paused = false) override;
    void seek(time sec) override;
    void stop(handle h) override;
    void pause(handle h, bool paused) override;

    void syncAll(time sec);
    void stopAll();

    time position(handle h) const override;
    bool isEnded(handle h) const override;

    void setVolume(handle h, float volume) override;
    void setPan(handle h, float val) override;
    void setPlaySpeed(handle h, float speed) override;

private:

    struct SL;
    std::unique_ptr<SL> _sl;
    bool _inited{ false };

    struct Source {
        handle handel;
        IAudioSource* source{ nullptr };
        bool playing{ false };
    };

    std::map<handle, Source> _sources;

    mutable float _syncPlaybackPosition{ 0.0f };
};
}
}
}

#endif // MU_AUDIO_AUDIOENGINE_H
