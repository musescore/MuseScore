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
#ifndef MU_AUDIO_RPCCONTROLLERS_H
#define MU_AUDIO_RPCCONTROLLERS_H

#include <functional>

#include "irpcchannel.h"
#include "irpccontroller.h"
#include "internal/worker/audioengine.h"

namespace mu::audio::rpc {
class RpcControllers : public async::Asyncable
{
public:
    RpcControllers() = default;

    void reg(const IRpcControllerPtr& controller);

    void init(const IRpcChannelPtr& channel);
    void deinit();

private:

    using Call = std::function<void (const Args& args)>;
    using Calls = std::map<Method, Call>;

    void bindMethod(Calls& calls, const Method& method, const Call& call);
    void doCall(const Calls& calls, const Msg& msg);

    AudioEngine* audioEngine() const;
    ISequencerPtr sequencer() const;

    void audioEngineHandle(const Msg& msg);
    void sequencerHandle(const Msg& msg);

    std::vector<IRpcControllerPtr> m_controllers;
    IRpcChannelPtr m_channel;
    IRpcChannel::ListenID m_listenID = -1;
};
}

#endif // MU_AUDIO_RPCCONTROLLERS_H
