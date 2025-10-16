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

#include <functional>
#include <thread>
#include <vector>
#include <cassert>
#include <algorithm>
#include <atomic>

#include "../asyncable.h"
#include "queuepool.h"

namespace muse::audio::comm {
enum class SendMode {
    Auto = 0,
    Direct,
    Queue
};

template<typename ... T>
class ChannelImpl : public Asyncable::IConnectable
{
public:
    using Callback = std::function<void (const T&...)>;

private:

    struct Receiver {
        std::atomic<bool> enabled = true;
        Asyncable* receiver = nullptr;
        Callback callback;
    };

    struct QueueData {
        std::thread::id receiveTh;
        Queue queue;
        QueueData()
            : queue(QUEUE_CAPACITY) {}
    };

    struct ThreadData {
        std::thread::id threadId;
        std::vector<Receiver*> receivers;
        std::vector<QueueData*> queues;
    };

    std::vector<ThreadData*> m_threads { MAX_THREADS, nullptr };
    std::atomic<bool> m_sending = false;

    ThreadData& threadData(const std::thread::id& thId)
    {
        for (size_t i = 0; i < m_threads.size(); ++i) {
            ThreadData* thdata = m_threads.at(i);
            if (!thdata) {
                thdata = new ThreadData();
                thdata->threadId = thId;
                m_threads[i] = thdata;
                return *m_threads[i];
            } else if (thdata->threadId == thId) {
                return *m_threads[i];
            }
        }

        assert(false && "thread pool exhausted");
        static ThreadData dummy;
        return dummy;
    }

    inline auto findReceiver(const std::vector<Receiver*>& receivers, const Asyncable* a) const
    {
        auto it = std::find_if(receivers.begin(), receivers.end(), [a](const Receiver* r) {
            return a == r->receiver;
        });
        return it;
    }

    bool isReceiverConnected(const std::thread::id& receiveTh, const Asyncable* a) const
    {
        for (ThreadData* thdata : m_threads) {
            if (!thdata) {
                break;
            }

            if (thdata->threadId != receiveTh) {
                continue;
            }

            auto it = findReceiver(thdata->receivers, a);
            if (it != thdata->receivers.end()) {
                const Receiver* r = *it;
                bool val = r->enabled.load(std::memory_order_acquire);
                return val;
            }
        }
        return false;
    }

    void sendToQueue(ThreadData& sendThdata, const std::thread::id& receiveTh, const CommMsg& msg)
    {
        // we are looking queue for the receiver.
        QueueData* qdata = nullptr;
        for (QueueData* qd : sendThdata.queues) {
            if (qd->receiveTh == receiveTh) {
                qdata = qd;
                break;
            }
        }

        // we'll create a new one if we didn't find one.
        if (!qdata) {
            qdata = new QueueData();
            qdata->receiveTh = receiveTh;
            qdata->queue.port2()->onMessage([this](const CommMsg& m) {
                // while the message was being transmited,
                // the receiver could have been removed.
                if (isReceiverConnected(std::this_thread::get_id(), m.receiver)) {
                    m.func();
                }
            });

            QueuePool::instance()->regPort(sendThdata.threadId, qdata->queue.port1());  // send
            QueuePool::instance()->regPort(receiveTh, qdata->queue.port2());            // receive

            sendThdata.queues.push_back(qdata);
        }

        qdata->queue.port1()->send(msg);
    }

    void unregAllQueue()
    {
        QueuePool* pool = QueuePool::instance();
        for (ThreadData* thdata : m_threads) {
            if (!thdata) {
                break;
            }

            for (QueueData* qdata : thdata->queues) {
                pool->unregPort(thdata->threadId, qdata->queue.port1()); // send
                pool->unregPort(qdata->receiveTh, qdata->queue.port2()); // receive
            }
        }
    }

    void sendAuto(const T&... args)
    {
        const std::thread::id threadId = std::this_thread::get_id();

        ThreadData& sendThdata = threadData(threadId);

        // the sender's thread is the same as the receiver's thread
        for (const Receiver* r : sendThdata.receivers) {
            if (!r->enabled.load(std::memory_order_acquire)) {
                continue;
            }
            r->callback(args ...);
        }

        // we are looking for receivers from other threads
        for (ThreadData* receiveThdata : m_threads) {
            if (!receiveThdata) {
                // there is no one further
                break;
            }

            if (receiveThdata->threadId == threadId) {
                // skip this thread
                continue;
            }

            for (const Receiver* r : receiveThdata->receivers) {
                if (!r->enabled.load(std::memory_order_acquire)) {
                    continue;
                }
                CommMsg msg;
                msg.receiver = r->receiver;
                const auto& call = r->callback;
                std::tuple<T...> vargs(args ...);
                msg.func = [call, vargs]() {
                    std::apply([call](const auto&... args) {
                        call(args ...);
                    }, vargs);
                };
                sendToQueue(sendThdata, receiveThdata->threadId, msg);
            }
        }
    }

    void sendDirect(const T&... args)
    {
        for (ThreadData* receiveThdata : m_threads) {
            if (!receiveThdata) {
                // there is no one further
                break;
            }

            for (const Receiver* r : receiveThdata->receivers) {
                if (!r->enabled.load(std::memory_order_acquire)) {
                    continue;
                }
                r->callback(args ...);
            }
        }
    }

    void sendQueue(const T&... args)
    {
        const std::thread::id threadId = std::this_thread::get_id();

        ThreadData& sendThdata = threadData(threadId);

        for (ThreadData* receiveThdata : m_threads) {
            if (!receiveThdata) {
                // there is no one further
                break;
            }

            for (const Receiver* r : receiveThdata->receivers) {
                if (!r->enabled.load(std::memory_order_acquire)) {
                    continue;
                }
                CommMsg msg;
                msg.receiver = r->receiver;
                const auto& call = r->callback;
                std::tuple<T...> vargs(args ...);
                msg.func = [call, vargs]() {
                    std::apply([call](const auto&... args) {
                        call(args ...);
                    }, vargs);
                };
                sendToQueue(sendThdata, receiveThdata->threadId, msg);
            }
        }
    }

public:

    ChannelImpl() = default;

    ~ChannelImpl()
    {
        unregAllQueue();

        for (ThreadData* thdata : m_threads) {
            if (!thdata) {
                break;
            }

            for (Receiver* r : thdata->receivers) {
                if (r->receiver) {
                    r->receiver->disconnectFromAsync(this);
                }
                delete r;
            }

            for (QueueData* qdata : thdata->queues) {
                delete qdata;
            }
        }
    }

    // IConnectable
    void disconnectAsync(Asyncable* a)
    {
        assert(a);

        bool sending = isSending();
        assert(sending);
        if (sending) {
            return;
        }

        // remove receiver
        for (ThreadData* thdata : m_threads) {
            if (!thdata) {
                break;
            }

            auto it = findReceiver(thdata->receivers, a);
            if (it != thdata->receivers.end()) {
                delete *it;
                thdata->receivers.erase(it);
            }
        }

        a->disconnectFromAsync(this);
    }

    void send(SendMode mode, const T&... args)
    {
        if (!isConnected()) {
            // there is no one to send to
            return;
        }

        m_sending.store(true, std::memory_order_release);

        switch (mode) {
        case SendMode::Auto: {
            sendAuto(args ...);
        } break;
        case SendMode::Direct: {
            sendDirect(args ...);
        } break;
        case SendMode::Queue: {
            sendQueue(args ...);
        } break;
        }

        m_sending.store(false, std::memory_order_release);
    }

    bool isSending() const
    {
        bool val = m_sending.load(std::memory_order_acquire);
        return val;
    }

    void onReceive(const Asyncable* receiver, const Callback& f)
    {
        const std::thread::id threadId = std::this_thread::get_id();
        ThreadData& data = threadData(threadId);

        Receiver* r = new Receiver();
        r->receiver = const_cast<Asyncable*>(receiver);
        if (r->receiver) {
            r->receiver->connectToAsync(this);
        }
        r->callback = f;

        data.receivers.push_back(r);
    }

    void disconnectReceiver(const Asyncable* a)
    {
        disconnectAsync(const_cast<Asyncable*>(a));
    }

    void disableReceiver(const Asyncable* a)
    {
        for (ThreadData* thdata : m_threads) {
            if (!thdata) {
                break;
            }

            auto it = findReceiver(thdata->receivers, a);
            if (it != thdata->receivers.end()) {
                it->enabled.store(false, std::memory_order_release);
                break;
            }
        }
    }

    bool isConnected() const
    {
        assert(m_threads.size() > 0);
        return m_threads.at(0) != nullptr;
    }
};
}
