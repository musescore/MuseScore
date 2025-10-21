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
#include <type_traits>

#include "asyncable.h"
#include "internal/channelimpl.h"

namespace kors::async {
template<typename ... T>
class Channel
{
public:

    using Callback = std::function<void (const T&...)>;

private:

    struct Data {
        ChannelImpl<T...> mainCh;
        std::unique_ptr<ChannelImpl<> > closeCh;

        Data(size_t max_threads)
            : mainCh(max_threads) {}
    };

    std::shared_ptr<Data> m_data;

public:
    Channel(size_t max_threads = conf::MAX_THREADS_PER_CHANNEL)
        : m_data(std::make_shared<Data>(max_threads))
    {
    }

    Channel(const Channel& ch)
        : m_data(ch.m_data)
    {
    }

    ~Channel() = default;

    Channel& operator=(const Channel& ch)
    {
        m_data = ch.m_data;
        return *this;
    }

    void send(const T&... args)
    {
        m_data->mainCh.send(SendMode::Auto, args ...);
    }

    template<typename Func>
    void onReceive(const Asyncable* receiver, Func f, Asyncable::Mode mode = Asyncable::Mode::SetOnce)
    {
        if constexpr (std::is_convertible_v<Func, Callback>) {
            m_data->mainCh.onReceive(receiver, f, mode);
        } else {
            Callback c = [f](const T&... args) {
                f(args ...);
            };
            m_data->mainCh.onReceive(receiver, c, mode);
        }
    }

    void close()
    {
        if (m_data->closeCh) {
            m_data->closeCh->send(SendMode::Auto);
        }
    }

    template<typename Func>
    void onClose(const Asyncable* receiver, Func f, Asyncable::Mode mode = Asyncable::Mode::SetOnce)
    {
        if (!m_data->closeCh) {
            m_data->closeCh = std::make_unique<ChannelImpl<> >(m_data->mainCh.maxThreads());
        }

        using CloseCall = std::function<void ()>;

        if constexpr (std::is_convertible_v<Func, CloseCall>) {
            m_data->closeCh->onReceive(receiver, f, mode);
        } else {
            CloseCall c = [f]() {
                f();
            };
            m_data->closeCh->onReceive(receiver, c, mode);
        }
    }

    bool isConnected() const
    {
        return m_data->mainCh.isConnected();
    }

    void disconnect(const Asyncable* a)
    {
        m_data->mainCh.disconnect(a);
        if (m_data->closeCh) {
            m_data->closeCh->disconnect(a);
        }
    }

    uint64_t key() const { return reinterpret_cast<uint64_t>(m_data.get()); }
};
}
