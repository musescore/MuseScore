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
#include "generalrpcchannel.h"

#include <thread>

#include "../../../audiosanitizer.h"

#include "log.h"

//#define RPC_LOGGING_ENABLED

#ifdef RPC_LOGGING_ENABLED
#define RPCLOG() LOGDA()
#else
#define RPCLOG() LOGN()
#endif

using namespace muse::audio::rpc;

void GeneralRpcChannel::initOnWorker()
{
    ONLY_AUDIO_WORKER_THREAD;
    m_workerThreadID = std::this_thread::get_id();
}

bool GeneralRpcChannel::isWorkerThread() const
{
    return std::this_thread::get_id() == m_workerThreadID;
}

void GeneralRpcChannel::process()
{
    if (isWorkerThread()) {
        receive(m_mainRpcData, m_workerRpcData);
    } else {
        receive(m_workerRpcData, m_mainRpcData);
    }
}

void GeneralRpcChannel::receive(RpcData& from, RpcData& to) const
{
    MsgQueue msgQueue;
    StreamMsgQueue streamMsgQueue;
    {
        std::scoped_lock<std::mutex> lock(from.mutex);
        msgQueue.swap(from.queue);
        streamMsgQueue.swap(from.streamQueue);
    }

    // msgs
    while (!msgQueue.empty()) {
        const Msg& m = msgQueue.front();

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

        msgQueue.pop();
    }

    // streams
    while (!streamMsgQueue.empty()) {
        const StreamMsg& m = streamMsgQueue.front();

        RPCLOG() << "received stream: " << to_string(m.name)
                 << ", streamId: " << m.streamId
                 << ", data.size: " << m.data.size();

        auto it = to.onStreams.find(m.streamId);
        if (it != to.onStreams.end() && it->second) {
            it->second(m);
        }

        streamMsgQueue.pop();
    }
}

void GeneralRpcChannel::send(const Msg& msg, const Handler& onResponse)
{
    RPCLOG() << "callId: " << msg.callId
             << ", method: " << to_string(msg.method)
             << ", type: " << to_string(msg.type)
             << ", data.size: " << msg.data.size();

    if (isWorkerThread()) {
        std::scoped_lock<std::mutex> lock(m_workerRpcData.mutex);
        m_workerRpcData.queue.push(msg);

        if (onResponse) {
            m_workerRpcData.onResponses[msg.callId] = onResponse;
        }
    } else {
        std::scoped_lock<std::mutex> lock(m_mainRpcData.mutex);
        m_mainRpcData.queue.push(msg);

        if (onResponse) {
            m_mainRpcData.onResponses[msg.callId] = onResponse;
        }
    }
}

void GeneralRpcChannel::onMethod(Method method, Handler h)
{
    if (isWorkerThread()) {
        m_workerRpcData.onMethods[method] = h;
    } else {
        m_mainRpcData.onMethods[method] = h;
    }
}

void GeneralRpcChannel::listenAll(Handler h)
{
    if (isWorkerThread()) {
        m_workerRpcData.listenerAll = h;
    } else {
        m_mainRpcData.listenerAll = h;
    }
}

void GeneralRpcChannel::addStream(std::shared_ptr<IRpcStream> s)
{
    s->init();

    if (isWorkerThread()) {
        m_workerRpcData.streams.insert({ s->streamId(), s });
    } else {
        m_mainRpcData.streams.insert({ s->streamId(), s });
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

    if (isWorkerThread()) {
        removeStrm(m_workerRpcData.streams, id);
    } else {
        removeStrm(m_mainRpcData.streams, id);
    }
}

void GeneralRpcChannel::sendStream(const StreamMsg& msg)
{
    RPCLOG() << "stream: " << to_string(msg.name)
             << ", streamId: " << msg.streamId
             << ", data.size: " << msg.data.size();

    if (isWorkerThread()) {
        std::scoped_lock<std::mutex> lock(m_workerRpcData.mutex);
        m_workerRpcData.streamQueue.push(msg);
    } else {
        std::scoped_lock<std::mutex> lock(m_mainRpcData.mutex);
        m_mainRpcData.streamQueue.push(msg);
    }
}

void GeneralRpcChannel::onStream(StreamId id, StreamHandler h)
{
    if (isWorkerThread()) {
        m_workerRpcData.onStreams[id] = h;
    } else {
        m_mainRpcData.onStreams[id] = h;
    }
}
