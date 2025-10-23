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

#include <set>
#include <thread>
#include <mutex>
#include <cassert>
#include <algorithm>

namespace kors::async {
class Asyncable
{
public:

    enum class Mode {
        SetOnce = 0,
        SetReplace
    };

    virtual ~Asyncable()
    {
        async_disconnectAll();
    }

    struct IConnectable {
        std::set<Asyncable*> asyncables;
        virtual ~IConnectable()
        {
            assert(asyncables.empty());
        }

        virtual void disconnectAsyncable(Asyncable* a, const std::thread::id& connectThId) = 0;
    };

    bool async_isConnected() const
    {
        std::scoped_lock lock(m_async_mutex);
        return !m_async_connects.empty();
    }

    bool async_isConnected(IConnectable* c) const
    {
        return async_connectData(c).connection != nullptr;
    }

    std::thread::id async_connectThread(IConnectable* c) const
    {
        return async_connectData(c).threadId;
    }

    void async_connect(IConnectable* c)
    {
        assert(c);
        if (!c) {
            return;
        }

        const std::thread::id threadId = std::this_thread::get_id();
        std::scoped_lock lock(m_async_mutex);
        auto it = std::find_if(m_async_connects.begin(), m_async_connects.end(), [c](const ConnectData& d) {
            return d.connection == c;
        });

        if (it != m_async_connects.end()) {
            assert(it->threadId == threadId && "more than one connection in different threads");
            return;
        }

        m_async_connects.emplace(ConnectData { threadId, c });
        c->asyncables.insert(this);
    }

    void async_disconnect(IConnectable* c)
    {
        std::scoped_lock lock(m_async_mutex);
        auto it = std::find_if(m_async_connects.begin(), m_async_connects.end(), [c](const ConnectData& d) {
            return d.connection == c;
        });

        if (it != m_async_connects.end()) {
            m_async_connects.erase(it);
            c->asyncables.erase(this);
        }
    }

    void async_disconnectAll()
    {
        std::set<ConnectData> copy;
        {
            std::scoped_lock lock(m_async_mutex);
            m_async_connects.swap(copy);
        }

        for (const ConnectData& d : copy) {
            d.connection->disconnectAsyncable(this, d.threadId);
        }

        {
            std::scoped_lock lock(m_async_mutex);
            for (const ConnectData& d : copy) {
                d.connection->asyncables.erase(this);
            }
        }
    }

private:

    struct ConnectData {
        std::thread::id threadId;
        IConnectable* connection = nullptr;

        bool operator<(const ConnectData& other) const
        {
            return connection < other.connection;
        }
    };

    const ConnectData& async_connectData(IConnectable* c) const
    {
        std::scoped_lock lock(m_async_mutex);
        auto it = std::find_if(m_async_connects.begin(), m_async_connects.end(), [c](const ConnectData& d) {
            return d.connection == c;
        });

        if (it != m_async_connects.end()) {
            return *it;
        }

        static ConnectData dummy;
        return dummy;
    }

    mutable std::mutex m_async_mutex;
    std::set<ConnectData> m_async_connects;
};
}
