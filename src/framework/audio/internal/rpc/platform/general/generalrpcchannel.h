/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <queue>
#include <mutex>
#include <thread>

#include "../../irpcchannel.h"

namespace muse::audio::rpc {
class GeneralRpcChannel : public IRpcChannel
{
public:
    GeneralRpcChannel() = default;

    void initOnWorker();
    void process();

    // IRpcChannel
    void send(const Msg& msg, const Handler& onResponse = nullptr) override;
    void onMethod(Method method, Handler h) override;
    void listenAll(Handler h) override;

private:

    using MQ = std::queue<Msg>;

    struct RpcData {
        std::mutex mutex;
        MQ queue;
        Handler listenerAll;
        std::map<Method, Handler> onMethods;
        std::map<CallId, Handler> onResponses;
    };

    bool isWorkerThread() const;
    void doProcessRPC(RpcData& from, RpcData& to) const;

    std::thread::id m_workerThreadID;
    RpcData m_workerTh;
    RpcData m_mainTh;
};
}
