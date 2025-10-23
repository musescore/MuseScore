/*
MIT License

Copyright (c) Igor Korsukov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <memory>
#include <queue>
#include <functional>
#include <cassert>

#include "ringqueue.h"

namespace kors::queue {
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

    ~RpcQueue()
    {
        m_port1->connect(nullptr);
        m_port2->connect(nullptr);
    }

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

        assert(m_connPort);

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

        // if there are no more pending ones, we send them to the queue
        if (ok) {
            ok = m_queue.tryPush(item);
        }

        // If the queue is full, add to the pending
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
