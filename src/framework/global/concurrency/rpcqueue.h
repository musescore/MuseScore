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

#include <memory>
#include <queue>

#include "ringqueue.h"

namespace muse {
//! NOTE Single Producer/Single Consumer

template<typename T>
class RpcPort;

template<typename T>
class RpcQueue
{
private:
    std::shared_ptr<RpcPort<T> > m_port1;
    std::shared_ptr<RpcPort<T> > m_port2;

public:
    explicit RpcQueue(size_t capacity = 128)
        : m_port1(std::make_shared<RpcPort<T> >(capacity))
        , m_port2(std::make_shared<RpcPort<T> >(capacity))
    {
        m_port1->connect(m_port2);
        m_port2->connect(m_port1);
    }

    ~RpcQueue() = default;

    RpcQueue(const RpcQueue&) = delete;
    RpcQueue& operator=(const RpcQueue&) = delete;

    std::shared_ptr<RpcPort<T> > port1() const { return m_port1; }
    std::shared_ptr<RpcPort<T> > port2() const { return m_port2; }
};

template<typename T>
class RpcPort
{
private:
    std::shared_ptr<RpcPort<T> > m_connPort;
    RingQueue<T> m_queue;
    std::vector<T> m_buffer;
    std::queue<T> m_pending;
    std::function<void(const T&)> m_handler;

public:

    explicit RpcPort(size_t capacity)
        : m_queue(capacity), m_buffer(capacity)
    {
    }

    void connect(const std::shared_ptr<RpcPort<T> >& port)
    {
        m_connPort = port;
    }

    void process()
    {
        // try send pending
        sendPending();

        // receive messages
        m_buffer.clear();
        bool ok = m_connPort->m_queue.tryPopAll(m_buffer);
        if (ok && m_handler) {
            for (const T& item : m_buffer) {
                m_handler(item);
            }
        }
    }

    bool sendPending()
    {
        while (!m_pending.empty()) {
            const T& item = m_pending.front();
            bool ok = m_queue.tryPush(item);
            if (ok) {
                m_pending.pop();
            } else {
                return false;
            }
        }
        return true;
    }

    void send(const T& item)
    {
        // try send pending first
        bool ok = sendPending();
        if (ok) {
            ok = m_queue.tryPush(item);
        }

        if (!ok) {
            m_pending.push(item);
        }
    }

    void onMessage(const std::function<void(const T&)>& handler)
    {
        m_handler = handler;
    }
};
}
