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
#ifndef MU_AUDIO_WORKERTYPES_H
#define MU_AUDIO_WORKERTYPES_H

#include <vector>
#include <memory>

namespace mu {
namespace audio {
namespace engine {
using StreamID = int;

enum class CallType : uint32_t {
    Midi = 0x00000000,
    Wav =  0x10000000
};

enum class CallMethod : uint32_t {
    // common
    Create =              1,
    Destroy =             2,
    SetSamplerate =       3,
    SetLoopRegion =       4,

    InstanceCreate =      10,
    InstanceDestroy =     11,
    InstanceInit =        12,
    InstanceSeekFrame =   13,
    InstaneOnSeek =       14,

    SetPlaybackSpeed =    20,
    SetIsTrackMuted =     21,
    SetTrackVolume =      22,
    SetTrackBalance =     23,

    // midi
    InitMidi =            30,
    LoadMidi =            31,

    // wav
    LoadTrack =           40,
    OnTrackLoaded =       41,

};

using CallID = uint32_t; //! NOTE CallID = CallType | CallMetthod

inline CallID callID(CallType type, CallMethod method)
{
    return uint32_t(type) | uint32_t(method);
}

class Args
{
public:

    template<typename T>
    static Args make_arg1(const T& val)
    {
        Args d;
        d.setArg<T>(0, val);
        return d;
    }

    template<typename T1, typename T2>
    static Args make_arg2(const T1& val1, const T2& val2)
    {
        Args d;
        d.setArg<T1>(0, val1);
        d.setArg<T2>(1, val2);
        return d;
    }

    template<typename T>
    void setArg(int i, const T& val)
    {
        IArg* p = new Arg<T>(val);
        m_args.insert(m_args.begin() + i, std::shared_ptr<IArg>(p));
    }

    template<typename T>
    T arg(int i = 0) const
    {
        IArg* p = m_args.at(i).get();
        if (!p) {
            return T();
        }
        Arg<T>* d = reinterpret_cast<Arg<T>*>(p);
        return d->val;
    }

    int count() const
    {
        return int(m_args.size());
    }

    struct IArg {
        virtual ~IArg() = default;
    };

    template<typename T>
    struct Arg : public IArg {
        T val;
        Arg(const T& v)
            : IArg(), val(v) {}
    };

private:
    std::vector<std::shared_ptr<IArg> > m_args;
};
}
}
}

#endif // MU_AUDIO_WORKERTYPES_H
