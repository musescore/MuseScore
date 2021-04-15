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
#include "queuedrpcchannel.h"

#include "log.h"

using namespace mu::audio::rpc;

bool QueuedRpcChannel::isSerialized() const
{
    return false;
}

void QueuedRpcChannel::send(const Msg& msg)
{
    if (isWorkerThread()) {
        std::lock_guard<std::mutex> lock(m_workerTh.mutex);
        m_workerTh.queue.push(msg);

        //! NOTE Calls the `process` method on the main thread
        m_mainThreadInvoker->invoke([this]() { process(); });
    } else {
        std::lock_guard<std::mutex> lock(m_mainTh.mutex);
        m_mainTh.queue.push(msg);
    }
}

IRpcChannel::ListenID QueuedRpcChannel::listen(Handler h)
{
    if (isWorkerThread()) {
        std::lock_guard<std::mutex> lock(m_workerTh.mutex);
        m_workerTh.lastID++;
        m_workerTh.listens[m_workerTh.lastID] = h;
        return m_workerTh.lastID;
    } else {
        std::lock_guard<std::mutex> lock(m_mainTh.mutex);
        m_mainTh.lastID++;
        m_mainTh.listens[m_mainTh.lastID] = h;
        return m_mainTh.lastID;
    }
}

void QueuedRpcChannel::unlisten(ListenID id)
{
    if (isWorkerThread()) {
        std::lock_guard<std::mutex> lock(m_workerTh.mutex);
        m_workerTh.listens.erase(id);
    } else {
        std::lock_guard<std::mutex> lock(m_mainTh.mutex);
        m_mainTh.listens.erase(id);
    }
}

bool QueuedRpcChannel::isWorkerThread() const
{
    return std::this_thread::get_id() == m_streamThreadID;
}

void QueuedRpcChannel::setupWorkerThread()
{
    m_streamThreadID = std::this_thread::get_id();
}

void QueuedRpcChannel::setupMainThread()
{
    m_mainThreadInvoker = std::make_shared<framework::Invoker>();
}

void QueuedRpcChannel::process()
{
    if (isWorkerThread()) {
        doProcess(m_mainTh, m_workerTh);
    } else {
        doProcess(m_workerTh, m_mainTh);
    }
}

void QueuedRpcChannel::doProcess(RpcData& from, RpcData& to)
{
    MQ fromMQ;
    {
        std::lock_guard<std::mutex> lock(from.mutex);
        fromMQ.swap(from.queue);
    }

    if (fromMQ.empty()) {
        return;
    }

    while (!fromMQ.empty()) {
        const Msg& m = fromMQ.front();
        for (auto it = to.listens.begin(); it != to.listens.end(); ++it) {
            it->second(m);
        }
        fromMQ.pop();
    }
}
