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

#ifndef MU_AUDIO_RPC_RPCSTREAMCONTROLLER_H
#define MU_AUDIO_RPC_RPCSTREAMCONTROLLER_H

#include <memory>
#include <string>
#include <cstdint>
#include <map>
#include <functional>
#include <utility>

#include "modularity/ioc.h"
#include "irpcaudiostreamchannel.h"

#include "midistreamcontroller.h"
#include "wavstreamcontroller.h"
#include "irpcaudiostreamchannel.h"
#include "workertypes.h"

namespace mu {
namespace audio {
namespace worker {
class RpcStreamController
{
    INJECT(audio_engine, IRpcAudioStreamChannel, channel)

public:
    RpcStreamController() = default;
    ~RpcStreamController();

    void setup();

    void callRpc(const StreamID& id, CallID method, const Args& args);

    void getAudio(const StreamID& id, float* buf, uint32_t samples, uint32_t bufSize, Context* ctx);

private:

    using Call = std::function<void (const StreamID& id, const Args& args)>;
    mutable std::map<CallID, Call> m_calls;

    std::shared_ptr<MidiStreamController> m_midi;
    std::shared_ptr<WavStreamController> m_wav;
};
}
}
}

#endif//MU_AUDIO_RPC_RPCSTREAMCONTROLLER_H
