/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "global/modularity/imoduleinterface.h"
#include "irpcchannel.h"

namespace muse::audio::rpc {
class IContextRpcChannelController : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(IContextRpcChannelController)
public:
    virtual ~IContextRpcChannelController() = default;

    virtual void send(const Msg& msg, const Handler& onResponse = nullptr) = 0;
    virtual void onRequest(CtxId ctxId, MsgCode code, Handler h) = 0;
    virtual void onNotification(CtxId ctxId, MsgCode code, Handler h) = 0;

    virtual void addStream(std::shared_ptr<IRpcStream> s) = 0;
    virtual void removeStream(StreamId id) = 0;
    virtual void sendStream(const StreamMsg& msg) = 0;
    virtual void onStream(StreamId id, StreamHandler h) = 0;
};

class IContextRpcChannel : MODULE_CONTEXT_INTERFACE, public IStreamRpcChannel
{
    INTERFACE_ID(IContextRpcChannel)
public:
    virtual ~IContextRpcChannel() = default;

    virtual void send(const Msg& msg, const Handler& onResponse = nullptr) = 0;
    virtual void onRequest(MsgCode code, Handler h) = 0;
    virtual void onNotification(MsgCode code, Handler h) = 0;

    // stream (async/channel)
    template<typename ... Types>
    StreamId addSendStream(StreamName name, const async::Channel<Types...>& ch)
    {
        StreamId id = new_stream_id();
        auto s = new RpcStream<Types...>(this, name, id, StreamType::Send, ch, nullptr);
        addStream(std::shared_ptr<IRpcStream>(s));
        return id;
    }

    template<typename ... Types>
    void addReceiveStream(StreamName name, rpc::StreamId id, const async::Channel<Types...>& ch, const RpcStreamExec& exec = nullptr)
    {
        auto s = new RpcStream<Types...>(this, name, id, StreamType::Receive, ch, exec);
        addStream(std::shared_ptr<IRpcStream>(s));
    }
};
}
