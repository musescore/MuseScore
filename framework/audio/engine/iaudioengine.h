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
#include <functional>

#include "modularity/imoduleexport.h"

#include "iaudiosource.h"

namespace mu {
namespace audio {
namespace engine {

class IAudioEngine : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioEngine)

public:
    virtual ~IAudioEngine() = default;

    using handle = unsigned int;
    using time = float;

    virtual bool init() = 0;
    virtual bool isInited() const = 0;

    virtual float samplerate() const = 0;

    virtual handle play(IAudioSource *s, float volume = -1, float pan = 0, bool paused = false) = 0;
    virtual void seek(time sec) = 0;
    virtual void stop(handle h) = 0;
    virtual void pause(handle h, bool paused) = 0;

    virtual time position(handle h) const = 0;
    virtual bool isEnded(handle h) const = 0;

    virtual void setVolume(handle h, float volume) = 0; // 0. - 1.
    virtual void setPan(handle h, float val) = 0; // -1 only left, 0 center, 1 only right
    virtual void setPlaySpeed(handle h, float speed) = 0;
};

}
}
}
#endif // MU_AUDIO_IAUDIOENGINE_H
