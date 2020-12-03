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

#include "queuedrpcstreamchannel.h"

#include <cstring>

#include "log.h"

using namespace mu::audio::worker;

void QueuedRpcStreamChannel::setupWorkerThread()
{
    m_streamThreadID = std::this_thread::get_id();
}

bool QueuedRpcStreamChannel::isWorkerThread() const
{
    return std::this_thread::get_id() == m_streamThreadID;
}

// Rpc
void QueuedRpcStreamChannel::doSend(const StreamID& id, CallID method, const Args& args)
{
    if (isWorkerThread()) {
        std::lock_guard<std::mutex> lock(m_workerTh.mutex);
        m_workerTh.queue.push(Msg { id, method, args });
        if (m_workerQueueChanged) {
            m_workerQueueChanged();
        }
    } else {
        std::lock_guard<std::mutex> lock(m_mainTh.mutex);
        m_mainTh.queue.push(Msg { id, method, args });
    }
}

void QueuedRpcStreamChannel::doListen(const StreamID& id, Handler h)
{
    if (isWorkerThread()) {
        m_workerTh.listens[id] = h;
    } else {
        m_mainTh.listens[id] = h;
    }
}

void QueuedRpcStreamChannel::doUnlisten(const StreamID& id)
{
    if (isWorkerThread()) {
        m_workerTh.listens.erase(id);
    } else {
        m_mainTh.listens.erase(id);
    }
}

void QueuedRpcStreamChannel::doListenAll(HandlerAll h)
{
    if (isWorkerThread()) {
        m_workerTh.listenAll = h;
    } else {
        m_mainTh.listenAll = h;
    }
}

void QueuedRpcStreamChannel::doUnlistenAll()
{
    if (isWorkerThread()) {
        m_workerTh.listenAll = nullptr;
    } else {
        m_mainTh.listenAll = nullptr;
    }
}

// Audio

void QueuedRpcStreamChannel::process()
{
    // Rpc
    if (isWorkerThread()) {
        doProcessRPC(m_mainTh, m_workerTh);

        doRequestAudio();

        if (DIRECT_RECIEVE_AUDIO) {
            doRecieveAudio();
        }
    } else {
        doProcessRPC(m_workerTh, m_mainTh);

        if (!DIRECT_RECIEVE_AUDIO) {
            doRecieveAudio();
        }
    }
}

void QueuedRpcStreamChannel::doProcessRPC(RpcData& from, RpcData& to)
{
    MQ fromMQ;
    {
        std::lock_guard<std::mutex> lock(from.mutex);
        fromMQ.swap(from.queue);
    }

    while (!fromMQ.empty()) {
        const Msg& m = fromMQ.front();

        if (to.listenAll) {
            to.listenAll(m.streamID, m.method, m.args);
        }

        if (to.listens[m.streamID]) {
            to.listens[m.streamID](m.method, m.args);
        }

        fromMQ.pop();
    }
}

void QueuedRpcStreamChannel::doRequestAudio()
{
    IF_ASSERT_FAILED(m_getAudio) {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (!allStreamsInState(RequestState::REQUESTED)) {
        return;
    }

    for (auto it = m_streams.begin(); it != m_streams.end(); ++it) {
        std::shared_ptr<Stream>& s = it->second;
        if (s->state != RequestState::REQUESTED) {
            continue;
        }

        const StreamID& id = it->first;
        s->ctx.clear();
        m_getAudio(id, &s->buf[0], s->samples, s->bufSize, &s->ctx);

        //LOGI() << "QueuedRpcStreamChannel::doRequestAudio: " << s->ctx.dump();

        s->state = RequestState::WRITED;
    }
}

void QueuedRpcStreamChannel::doRecieveAudio()
{
    { //lock
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (!allStreamsInState(RequestState::WRITED)) {
            return;
        }

        for (auto it = m_streams.begin(); it != m_streams.end(); ++it) {
            std::shared_ptr<Stream>& s = it->second;
            if (s->state != RequestState::WRITED) {
                continue;
            }

            IF_ASSERT_FAILED(s->getBuffer) {
                continue;
            }

            float* dst = s->getBuffer(s->bufSize, s->ctx);
            std::memcpy(dst, &s->buf[0], s->bufSize * sizeof(float));

            s->state = RequestState::FREE;
        }
    } //unlock

    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        for (auto it = m_streams.begin(); it != m_streams.end(); ++it) {
            IF_ASSERT_FAILED(it->second->onRequestFinished) {
                continue;
            }
            it->second->onRequestFinished();
        }
    }
}

void QueuedRpcStreamChannel::onWorkerQueueChanged(std::function<void()> func)
{
    m_workerQueueChanged = func;
}
