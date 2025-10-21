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

#include "channel.h"

namespace kors::async {
class Notification
{
public:

    using Callback = std::function<void ()>;

private:

    std::shared_ptr<Channel<> > m_ch;

public:

    Notification()
        : m_ch(std::make_shared<Channel<> >())
    {
    }

    Notification(const Notification& ch)
        : m_ch(ch.m_ch)
    {
    }

    ~Notification() = default;

    Notification& operator=(const Notification& ch)
    {
        m_ch = ch.m_ch;
        return *this;
    }

    void notify()
    {
        m_ch->send();
    }

    template<typename Func>
    void onNotify(const Asyncable* receiver, Func f, Asyncable::Mode mode = Asyncable::Mode::SetOnce)
    {
        m_ch->onReceive(receiver, f, mode);
    }

    void close()
    {
        m_ch->close();
    }

    template<typename Func>
    void onClose(const Asyncable* receiver, Func f, Asyncable::Mode mode = Asyncable::Mode::SetOnce)
    {
        m_ch->onClose(receiver, f, mode);
    }

    bool isConnected() const
    {
        return m_ch->isConnected();
    }

    void disconnect(const Asyncable* receiver)
    {
        m_ch->disconnect(receiver);
    }

    uint64_t key() const { return m_ch->key(); }
};
}
