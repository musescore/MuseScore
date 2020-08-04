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

#include "iaudioengine.h"
#include "modularity/ioc.h"
#include "iaudiodriver.h"

#include "ret.h"
#include "retval.h"
#include "invoker.h"

namespace mu {
namespace audio {
class AudioEngine : public IAudioEngine
{
    INJECT(audio, IAudioDriver, driver)

public:
    AudioEngine();

    Ret init() override;
    void deinit() override;
    bool isInited() const override;
    async::Channel<bool> initChanged() const override;

    float sampleRate() const override;

    handle play(std::shared_ptr<IAudioSource> s, float volume = -1, float pan = 0, bool paused = false) override;
    void seek(handle h, time sec) override;
    void setPause(handle h, bool paused) override;
    void stop(handle h) override;

    Status status(handle h) const override;
    async::Channel<Status> statusChanged(handle h) const override;

    time position(handle h) const override;

    void setVolume(handle h, float volume) override;
    void setPan(handle h, float val) override;
    void setPlaySpeed(handle h, float speed) override;

    void swapPlayContext(handle h, Context& ctx) override;
    async::Channel<Context> playContextChanged(handle h) const override;

private:

    void onPlay(handle h);
    void onSeek(handle h);
    void onPause(handle h);
    void onStop(handle h);

    struct HandleMeta {
        Status status = Status::Undefined;
        async::Channel<Status> statusChanged;
        Context playContext;
        bool playContextRequested = false;
        async::Channel<Context> playContextChanged;

        bool isValid() const { return status != Status::Undefined; }
    };

    HandleMeta& pushMeta(handle h);
    void popMeta(handle h);
    HandleMeta& meta(handle h);
    const HandleMeta& meta(handle h) const;

    struct SL;
    std::shared_ptr<SL> m_sl;
    bool m_inited = false;
    mu::async::Channel<bool> m_initChanged;

    mutable std::mutex m_metasMutex;
    std::map<handle, HandleMeta> m_handleMetas;
    framework::Invoker m_popMetaInvoker;
};
}
}

#endif // MU_AUDIO_AUDIOENGINE_H
