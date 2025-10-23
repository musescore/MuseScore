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
#include "generalrpcchannel.h"

#include "../../../audiosanitizer.h"

#include "log.h"

//#define RPC_LOGGING_ENABLED

#ifdef RPC_LOGGING_ENABLED
#define RPCLOG() LOGDA()
#else
#define RPCLOG() LOGN()
#endif

using namespace muse::audio::rpc;

static thread_local bool s_isMainThread = false;

GeneralRpcChannel::~GeneralRpcChannel()
{
    m_engineRpcData.streams.clear();
    m_mainRpcData.streams.clear();
}

void GeneralRpcChannel::setupOnMain()
{
    ONLY_AUDIO_MAIN_THREAD;
    s_isMainThread = true;

    m_msgQueue.port1()->onMessage([this](const Msg& m) {
        receive(m_mainRpcData, m);
    });

    m_streamQueue.port1()->onMessage([this](const StreamMsg& m) {
        receive(m_mainRpcData, m);
    });
}

void GeneralRpcChannel::setupOnEngine()
{
    m_msgQueue.port2()->onMessage([this](const Msg& m) {
        receive(m_engineRpcData, m);
    });

    m_streamQueue.port2()->onMessage([this](const StreamMsg& m) {
        receive(m_engineRpcData, m);
    });
}

void GeneralRpcChannel::process()
{
    if (s_isMainThread) {
        m_msgQueue.port1()->process();
        m_streamQueue.port1()->process();
    } else {
        m_msgQueue.port2()->process();
        m_streamQueue.port2()->process();
    }
}

void GeneralRpcChannel::send(const Msg& msg, const Handler& onResponse)
{
    RPCLOG() << "callId: " << msg.callId
             << ", method: " << to_string(msg.method)
             << ", type: " << to_string(msg.type)
             << ", data.size: " << msg.data.size();

    if (s_isMainThread) {
        if (onResponse) {
            m_mainRpcData.onResponses[msg.callId] = onResponse;
        }

        m_msgQueue.port1()->send(msg);
    } else {
        if (onResponse) {
            m_engineRpcData.onResponses[msg.callId] = onResponse;
        }

        m_msgQueue.port2()->send(msg);
    }
}

void GeneralRpcChannel::receive(RpcData& to, const Msg& m) const
{
    RPCLOG() << "callId: " << m.callId
             << ", method: " << to_string(m.method)
             << ", type: " << to_string(m.type)
             << ", data.size: " << m.data.size();

    // all
    if (to.listenerAll) {
        to.listenerAll(m);
    }

    // by method
    {
        auto it = to.onMethods.find(m.method);
        if (it != to.onMethods.end() && it->second) {
            it->second(m);
        }
    }

    // by callId (response)
    if (m.type == MsgType::Response) {
        auto it = to.onResponses.find(m.callId);
        if (it != to.onResponses.end() && it->second) {
            it->second(m);
            to.onResponses.erase(it);
        }
    }
}

void GeneralRpcChannel::onMethod(Method method, Handler h)
{
    if (s_isMainThread) {
        m_mainRpcData.onMethods[method] = h;
    } else {
        m_engineRpcData.onMethods[method] = h;
    }
}

void GeneralRpcChannel::listenAll(Handler h)
{
    if (s_isMainThread) {
        m_mainRpcData.listenerAll = h;
    } else {
        m_engineRpcData.listenerAll = h;
    }
}

void GeneralRpcChannel::sendStream(const StreamMsg& msg)
{
    RPCLOG() << "stream: " << to_string(msg.name)
             << ", streamId: " << msg.streamId
             << ", data.size: " << msg.data.size();

    if (s_isMainThread) {
        m_streamQueue.port1()->send(msg);
    } else {
        m_streamQueue.port2()->send(msg);
    }
}

void GeneralRpcChannel::receive(RpcData& to, const StreamMsg& m) const
{
    RPCLOG() << "received stream: " << to_string(m.name)
             << ", streamId: " << m.streamId
             << ", data.size: " << m.data.size();

    auto it = to.onStreams.find(m.streamId);
    if (it != to.onStreams.end() && it->second) {
        it->second(m);
    }
}

void GeneralRpcChannel::addStream(std::shared_ptr<IRpcStream> s)
{
    s->init();

    if (s_isMainThread) {
        m_mainRpcData.streams.insert({ s->streamId(), s });
    } else {
        m_engineRpcData.streams.insert({ s->streamId(), s });
    }
}

void GeneralRpcChannel::removeStream(StreamId id)
{
    auto removeStrm = [](std::map<StreamId, std::shared_ptr<IRpcStream> >& streams, StreamId id) {
        auto it = streams.find(id);
        if (it != streams.end()) {
            streams.erase(it);
        }
    };

    if (s_isMainThread) {
        removeStrm(m_mainRpcData.streams, id);
    } else {
        removeStrm(m_engineRpcData.streams, id);
    }
}

void GeneralRpcChannel::onStream(StreamId id, StreamHandler h)
{
    if (s_isMainThread) {
        m_mainRpcData.onStreams[id] = h;
    } else {
        m_engineRpcData.onStreams[id] = h;
    }
}
