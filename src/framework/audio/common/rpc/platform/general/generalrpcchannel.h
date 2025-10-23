/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "../../irpcchannel.h"
#include "global/concurrency/rpcqueue.h"

namespace muse::audio::rpc {
class GeneralRpcChannel : public IRpcChannel
{
public:
    GeneralRpcChannel() = default;
    ~GeneralRpcChannel() override;

    void setupOnMain() override;
    void setupOnEngine() override;

    void process() override;

    // IRpcChannel
    // msgs
    void send(const Msg& msg, const Handler& onResponse = nullptr) override;
    void onMethod(Method method, Handler h) override;
    void listenAll(Handler h) override;

    // stream
    void addStream(std::shared_ptr<IRpcStream> s) override;
    void removeStream(StreamId id) override;
    void sendStream(const StreamMsg& msg) override;
    void onStream(StreamId id, StreamHandler h) override;

private:

    struct RpcData {
        // msgs
        Handler listenerAll;
        std::map<Method, Handler> onMethods;
        std::map<CallId, Handler> onResponses;

        // stream
        std::map<StreamId, std::shared_ptr<IRpcStream> > streams;
        std::map<StreamId, StreamHandler> onStreams;
    };

    void receive(RpcData& to, const Msg& m) const;
    void receive(RpcData& to, const StreamMsg& m) const;

    RpcData m_engineRpcData;
    RpcData m_mainRpcData;

    // port1 - main thread
    // port2 - engine rpc thread;
    RpcQueue<Msg> m_msgQueue;
    RpcQueue<StreamMsg> m_streamQueue;
};
}
