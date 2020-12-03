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
#ifndef MU_AUDIO_AUDIOTYPES_H
#define MU_AUDIO_AUDIOTYPES_H

#include <string>

#include "internal/audiointernaltypes.h"

namespace mu {
namespace audio {
enum class PlayStatus {
    UNDEFINED = 0,
    STOPED,
    PLAYING,
    PAUSED
};

struct LoopRegion
{
    float begin = 0.0f;
    float end = 0.0f;

    bool isValid() const { return !(begin < 0.f) && end > begin; }
};

enum class CtxKey {
    Silent = 0,
    HasEnded = 1,
    Position = 2,
    InstanceDestroyed = 3,

    // midi
    PlayTick = 12,
    FromTick = 13,
    ToTick = 14,
};

struct Context
{
    Args args;

    template<typename T>
    void set(CtxKey k, const T& t)
    {
        args.setArg<T>(int(k), t);
    }

    template<typename T>
    T get(CtxKey k, T def = T()) const
    {
        return args.arg<T>(int(k), def);
    }

    void clear()
    {
        args.clear();
    }

    void swap(Context& other)
    {
        args.swap(other.args);
    }

    bool hasVal(CtxKey k) const
    {
        return args.hasArg(int(k));
    }

    std::string dump() const
    {
        std::string str;
        if (hasVal(CtxKey::Silent)) {
            str += "Silent: " + std::to_string(get<bool>(CtxKey::Silent));
        }

        if (hasVal(CtxKey::HasEnded)) {
            str += " HasEnded: " + std::to_string(get<bool>(CtxKey::HasEnded));
        }

        if (hasVal(CtxKey::Position)) {
            str += " Position: " + std::to_string(get<double>(CtxKey::Position));
        }

        if (hasVal(CtxKey::PlayTick)) {
            str += " PlayTick: " + std::to_string(get<int>(CtxKey::PlayTick));
        }

        if (hasVal(CtxKey::FromTick)) {
            str += " FromTick: " + std::to_string(get<int>(CtxKey::FromTick));
        }

        if (hasVal(CtxKey::ToTick)) {
            str += " ToTick: " + std::to_string(get<int>(CtxKey::ToTick));
        }

        return str;
    }
};
}
}

#endif // MU_AUDIO_AUDIOTYPES_H
