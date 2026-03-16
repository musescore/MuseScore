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

#include <thread>
#include <vector>
#include <memory>
#include <mutex>

#include "rpcqueue.h"

#include "../asyncable.h"

namespace kors::async {
struct ICallable {
    virtual ~ICallable() = default;
    virtual void call(const void*) = 0;
    virtual bool tryLock() { return false; }
    virtual void unlock() {}
};

struct CallMsg {
    Asyncable* receiver = nullptr;
    std::shared_ptr<ICallable> func;
};

using Queue = RpcQueue<CallMsg>;
using Port = RpcPort<CallMsg>;

class QueuePool
{
public:

    static QueuePool* instance();

    void regPort(const std::thread::id& th, const std::shared_ptr<Port>& port);
    void unregPort(const std::thread::id& th, const std::shared_ptr<Port>& port);

    void processMessages();
    void processMessages(const std::thread::id& th);

private:

    QueuePool();
    ~QueuePool();

    struct ThreadData {
        std::thread::id threadId;
        std::vector<std::shared_ptr<Port> > ports;
        std::atomic<bool> locked = false;

        bool tryLock();
        void lock();
        void unlock();
    };

    ThreadData* threadData(const std::thread::id& threadId, bool create);

    std::mutex m_mutex;
    std::vector<ThreadData*> m_threads;
    std::atomic<size_t> m_count = 0;
};
}
