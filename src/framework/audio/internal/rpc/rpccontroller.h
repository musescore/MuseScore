//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_AUDIO_RPCCONTROLLER_H
#define MU_AUDIO_RPCCONTROLLER_H

#include <functional>

#include "modularity/ioc.h"
#include "irpcchannel.h"
#include "internal/worker/audioengine.h"

namespace mu::audio::rpc {
class RpcController : public async::Asyncable
{
    INJECT(audio, IRpcChannel, rpcChannel)

public:
    RpcController() = default;

    void init();

private:

    using Call = std::function<void (const Args& args)>;
    using Calls = std::map<Method, Call>;

    void bindMethod(Calls& calls, const Method& method, const Call& call);
    void doCall(const Calls& calls, const Msg& msg);

    AudioEngine* audioEngine() const;
    ISequencerPtr sequencer() const;

    void audioEngineHandle(const Msg& msg);
    void sequencerHandle(const Msg& msg);
};
}

#endif // MU_AUDIO_RPCCONTROLLER_H
