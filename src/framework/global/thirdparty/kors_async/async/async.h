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

#include <functional>
#include <mutex>
#include <type_traits>

#include "internal/queuepool.h"
#include "conf.h"

namespace kors::async {
class Async
{
private:
    using Call = std::function<void ()>;

    struct QueueData : public Asyncable::IConnectable {
        std::thread::id sendTh;
        std::thread::id receiveTh;
        Queue queue;
        mutable std::mutex mutex;
        std::set<Asyncable*> callers;

        QueueData()
            : queue(conf::QUEUE_CAPACITY) {}

        void connect(Asyncable* a)
        {
            if (!a) {
                return;
            }

            if (isConnected(a)) {
                return;
            }

            a->async_connect(this);

            std::scoped_lock lock(mutex);
            callers.insert(a);
        }

        bool isConnected(Asyncable* a) const
        {
            std::scoped_lock lock(mutex);
            return callers.find(a) != callers.end();
        }

        void disconnect(Asyncable* a)
        {
            std::scoped_lock lock(mutex);
            callers.erase(a);
        }

        void disconnectAsyncable(Asyncable* a, const std::thread::id&) override
        {
            disconnect(a);
        }
    };

    std::mutex m_mutex;
    std::vector<QueueData*> m_queues;

    ~Async()
    {
        for (QueueData* d : m_queues) {
            QueuePool::instance()->unregPort(d->sendTh, d->queue.port1());           // send
            QueuePool::instance()->unregPort(d->receiveTh, d->queue.port2());        // receive

            for (Asyncable* a : d->callers) {
                a->async_disconnect(d);
            }

            delete d;
        }
    }

    QueueData* queueData(const std::thread::id& sendTh, const std::thread::id& receiveTh)
    {
        std::scoped_lock lock(m_mutex);
        for (QueueData* d : m_queues) {
            if (d->sendTh == sendTh && d->receiveTh == receiveTh) {
                return d;
            }
        }

        QueueData* d = new QueueData();
        d->sendTh = sendTh;
        d->receiveTh = receiveTh;
        m_queues.push_back(d);

        d->queue.port2()->onMessage([d](const CallMsg& m) {
            if (m.receiver) {
                if (d->isConnected(m.receiver)) {
                    m.func->call(nullptr);
                }
            } else {
                m.func->call(nullptr);
            }
        });

        QueuePool::instance()->regPort(sendTh, d->queue.port1());           // send
        QueuePool::instance()->regPort(receiveTh, d->queue.port2());        // receive

        return d;
    }

    void callQueue(const Asyncable* caller_, const Call& func, const std::thread::id& th)
    {
        Asyncable* caller = const_cast<Asyncable*>(caller_);

        struct Func : public ICallable
        {
            Call func;
            Func(const Call& f = nullptr)
                : func(f) {}

            void call(const void*) override
            {
                func();
            }
        };

        CallMsg m;
        m.receiver = caller;
        m.func = std::make_shared<Func>(func);

        const std::thread::id sendTh = std::this_thread::get_id();
        QueueData* qdata = instance()->queueData(sendTh, th);
        assert(qdata);
        if (!qdata) {
            return;
        }

        qdata->connect(caller);
        qdata->queue.port1()->send(m);
    }

    static inline void do_call(const Asyncable* caller, const Call& func, const std::thread::id& th = std::this_thread::get_id())
    {
        instance()->callQueue(caller, func, th);
    }

public:

    static Async* instance()
    {
        // needs to be created earlier
        QueuePool::instance();

        static Async a;
        return &a;
    }

    template<typename Func>
    static void call(const Asyncable* caller, Func f, const std::thread::id& th = std::this_thread::get_id())
    {
        if constexpr (std::is_convertible_v<Func, Call>) {
            do_call(caller, f, th);
        } else {
            Call c = [f]() mutable { f(); };
            do_call(caller, c, th);
        }
    }

    template<typename F, typename Arg1>
    static void call(const Asyncable* caller, F f, Arg1 a1, const std::thread::id& th = std::this_thread::get_id())
    {
        Call c = [f, a1]() mutable { f(a1); };
        do_call(caller, c, th);
    }
};
}
