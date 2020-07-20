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

#ifndef MU_AUDIO_QUEUEDRPCSTREAMCHANNEL_H
#define MU_AUDIO_QUEUEDRPCSTREAMCHANNEL_H

#include <queue>
#include <vector>
#include <mutex>
#include <thread>
#include <map>

#include "rpcstreamchannelbase.h"

namespace mu {
namespace audio {
namespace engine {

class QueuedRpcStreamChannel : public RpcStreamChannelBase
{
public:
    QueuedRpcStreamChannel();
    ~QueuedRpcStreamChannel() override;

    const static bool DIRECT_RECIEVE_AUDIO{true};

    void setupWorkerThread();

    void process();

private:
    // Rpc
    void doSend(const StreamID& id, CallID method, const Args& args) override;
    void doListen(const StreamID& id, Handler h) override;
    void doUnlisten(const StreamID& id) override;
    void doListenAll(HandlerAll h) override;
    void doUnlistenAll() override;

    struct Msg {
        StreamID streamID;
        CallID method;
        Args args;
    };

    using MQ = std::queue<Msg>;

    struct RpcData {
        std::mutex mutex;
        MQ queue;
        std::map<StreamID, Handler> listens;
        HandlerAll listenAll;
    };

    bool isWorkerThread() const;

    void doProcessRPC(RpcData &from, RpcData &to);
    void doRequestAudio();
    void doRecieveAudio();

    // Rpc
    std::thread::id m_streamThreadID;
    RpcData m_workerTh;
    RpcData m_mainTh;     //! NOTE Can be accessed from the main thread and from the audio driver thread
};

}
}
}

#endif // MU_AUDIO_QUEUEDRPCSTREAMCHANNEL_H
