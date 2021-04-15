/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#ifndef MU_AUDIO_QUEUEDRPCCHANNEL_H
#define MU_AUDIO_QUEUEDRPCCHANNEL_H

#include <thread>
#include <mutex>
#include <queue>
#include <memory>

#include "irpcchannel.h"
#include "invoker.h"

namespace mu::audio::rpc {
class QueuedRpcChannel : public IRpcChannel
{
public:
    QueuedRpcChannel() = default;

    bool isSerialized() const override;

    void send(const Msg& msg) override;
    ListenID listen(Handler h) override;
    void unlisten(ListenID id) override;

    bool isWorkerThread() const;
    void setupWorkerThread(); //! NOTE Must called from worker thread

    void setupMainThread(); //! NOTE Must called from main thread

    void process();

private:

    using MQ = std::queue<Msg>;

    struct RpcData {
        std::mutex mutex;
        MQ queue;
        ListenID lastID = 0;
        std::map<ListenID, Handler> listens;
    };

    void doProcess(RpcData& from, RpcData& to);

    std::shared_ptr<framework::Invoker> m_mainThreadInvoker;
    std::thread::id m_streamThreadID;
    RpcData m_workerTh;
    RpcData m_mainTh;
};

using QueuedRpcChannelPtr = std::shared_ptr<QueuedRpcChannel>;
}

#endif // MU_AUDIO_QUEUEDRPCCHANNEL_H
