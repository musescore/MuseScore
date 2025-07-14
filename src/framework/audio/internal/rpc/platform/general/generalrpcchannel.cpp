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

using namespace muse::audio::rpc;

void GeneralRpcChannel::initOnWorker()
{
    m_workerThreadID = std::this_thread::get_id();
}

bool GeneralRpcChannel::isWorkerThread() const
{
    return std::this_thread::get_id() == m_workerThreadID;
}

void GeneralRpcChannel::process()
{
    if (isWorkerThread()) {
        doProcessRPC(m_mainTh, m_workerTh);
    } else {
        doProcessRPC(m_workerTh, m_mainTh);
    }
}

void GeneralRpcChannel::doProcessRPC(RpcData& from, RpcData& to) const
{
    MQ fromMQ;
    {
        std::scoped_lock<std::mutex> lock(from.mutex);
        fromMQ.swap(from.queue);
    }

    while (!fromMQ.empty()) {
        const Msg& m = fromMQ.front();

        // all
        if (to.listenerAll) {
            to.listenerAll(m);
        }

        // by method
        {
            auto it = to.onMethods.find(m.method);
            if (it != to.onMethods.end()) {
                it->second(m);
            }
        }

        // by callId (response)
        if (m.type == MsgType::Response) {
            auto it = to.onResponses.find(m.callId);
            if (it != to.onResponses.end()) {
                it->second(m);
                to.onResponses.erase(it);
            }
        }

        fromMQ.pop();
    }
}

void GeneralRpcChannel::send(const Msg& msg, const Handler& onResponse)
{
    if (isWorkerThread()) {
        std::scoped_lock<std::mutex> lock(m_workerTh.mutex);
        m_workerTh.queue.push(msg);

        if (onResponse) {
            m_workerTh.onResponses[msg.callId] = onResponse;
        }
    } else {
        std::scoped_lock<std::mutex> lock(m_mainTh.mutex);
        m_mainTh.queue.push(msg);

        if (onResponse) {
            m_mainTh.onResponses[msg.callId] = onResponse;
        }
    }
}

void GeneralRpcChannel::onMethod(Method method, Handler h)
{
    if (isWorkerThread()) {
        m_workerTh.onMethods[method] = h;
    } else {
        m_mainTh.onMethods[method] = h;
    }
}

void GeneralRpcChannel::listenAll(Handler h)
{
    if (isWorkerThread()) {
        m_workerTh.listenerAll = h;
    } else {
        m_mainTh.listenerAll = h;
    }
}
