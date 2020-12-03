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

#include "../../audiotypes.h"

namespace mu {
namespace audio {
namespace worker {
using StreamID = int;

enum class CallType {
    Midi = 1000,
    Wav =  2000
};

enum class CallMethod {
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
    LoadMidi =            31,

    // wav
    LoadTrack =           40,
    OnTrackLoaded =       41,
};

using CallID = int; //! NOTE CallID = CallType + CallMethod

inline CallID callID(CallType type, CallMethod method)
{
    return int(type) + int(method);
}

inline CallType callType(CallID cid)
{
    return CallType((int(float(cid) / 1000.0f)) * 1000);
}

inline CallMethod callMethod(CallID cid)
{
    return CallMethod(int(cid) - int(callType(cid)));
}
}
}
}

#endif // MU_AUDIO_WORKERTYPES_H
