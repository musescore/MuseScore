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
#ifndef MU_AUDIO_IAUDIOENGINE_H
#define MU_AUDIO_IAUDIOENGINE_H

#include <string>

#include "modularity/imoduleexport.h"

#include "ret.h"
#include "async/channel.h"
#include "iaudiosource.h"
#include "audiotypes.h"

namespace mu {
namespace audio {
class IAudioEngine : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioEngine)

public:
    virtual ~IAudioEngine() = default;

    using handle = unsigned int;
    using time = float;

    enum class Status {
        Undefined = 0,
        Created,
        Playing,
        Paused,
        Stoped
    };

    virtual Ret init() = 0;
    virtual void deinit() = 0;
    virtual bool isInited() const = 0;
    virtual async::Channel<bool> initChanged() const = 0;

    virtual float sampleRate() const = 0;

    virtual handle play(std::shared_ptr<IAudioSource> src, float volume = -1, float pan = 0, bool paused = false) = 0;
    virtual void seek(handle h, time sec) = 0;
    virtual void setPause(handle h, bool paused) = 0;
    virtual void stop(handle h) = 0;

    virtual Status status(handle h) const = 0;
    virtual async::Channel<Status> statusChanged(handle h) const = 0;

    virtual time position(handle h) const = 0;

    virtual void setVolume(handle h, float volume) = 0; // 0. - 1.
    virtual void setPan(handle h, float val) = 0; // -1 only left, 0 center, 1 only right
    virtual void setPlaySpeed(handle h, float speed) = 0;

    // actions on driver callback
    virtual void swapPlayContext(handle h, Context& ctx) = 0;
    virtual async::Channel<Context> playContextChanged(handle h) const = 0;
};
}
}
#endif // MU_AUDIO_IAUDIOENGINE_H
