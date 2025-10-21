/*
MIT License

Copyright (c) 2020 Igor Korsukov

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

#include "asyncable.h"
#include "channel.h"

namespace muse::async {
template<typename T>
class ChangedNotifier;

template<typename T>
class ChangedNotify
{
public:
    ChangedNotify()
        : m_data(std::make_shared<Data>()) {}

    ChangedNotify(const ChangedNotify& notify)
        : m_data(notify.m_data) {}

    ~ChangedNotify() = default;

    template<typename Call>
    void onChanged(Asyncable* receiver, Call f, Asyncable::AsyncMode mode = Asyncable::AsyncMode::AsyncSetOnce)
    {
        m_data->ch.onReceive(receiver, [f](CallType type, const T&, const T&) {
            if (type == CallType::Changed) {
                f();
            }
        }, mode);
    }

    template<typename Call>
    void onItemChanged(Asyncable* receiver, Call f, Asyncable::AsyncMode mode = Asyncable::AsyncMode::AsyncSetOnce)
    {
        m_data->ch.onReceive(receiver, [f](CallType type, const T& item, const T&) {
            if (type == CallType::ItemChanged) {
                f(item);
            }
        }, mode);
    }

    template<typename Call>
    void onItemAdded(Asyncable* receiver, Call f, Asyncable::AsyncMode mode = Asyncable::AsyncMode::AsyncSetOnce)
    {
        m_data->ch.onReceive(receiver, [f](CallType type, const T& item, const T&) {
            if (type == CallType::ItemAdded) {
                f(item);
            }
        }, mode);
    }

    template<typename Call>
    void onItemRemoved(Asyncable* receiver, Call f, Asyncable::AsyncMode mode = Asyncable::AsyncMode::AsyncSetOnce)
    {
        m_data->ch.onReceive(receiver, [f](CallType type, const T& item, const T&) {
            if (type == CallType::ItemRemoved) {
                f(item);
            }
        }, mode);
    }

    template<typename Call>
    void onItemReplaced(Asyncable* receiver, Call f, Asyncable::AsyncMode mode = Asyncable::AsyncMode::AsyncSetOnce)
    {
        m_data->ch.onReceive(receiver, [f](CallType type, const T& oldItem, const T& newItem) {
            if (type == CallType::ItemRemoved) {
                f(oldItem, newItem);
            }
        }, mode);
    }

    void disconnect(Asyncable* receiver)
    {
        m_data->ch.resetOnReceive(receiver);
    }

private:
    friend class ChangedNotifier<T>;

    enum class CallType {
        Undefined = 0,
        Changed,
        ItemChanged,
        ItemAdded,
        ItemRemoved,
        ItemReplaced
    };

    void send(CallType type, const T& a1, const T& a2)
    {
        m_data->ch.send(type, a1, a2);
    }

    void send(CallType type, const T& a1)
    {
        static T dummy;
        m_data->ch.send(type, a1, dummy);
    }

    void send(CallType type)
    {
        static T dummy;
        m_data->ch.send(type, dummy, dummy);
    }

    struct Data {
        Channel<CallType, const T&, const T&> ch;
    };

    std::shared_ptr<Data> m_data;
};

template<typename T>
class ChangedNotifier
{
    using CallType = typename ChangedNotify<T>::CallType;

public:
    ChangedNotifier()
        : m_notify(std::make_shared<ChangedNotify<T> >()) {}
    ~ChangedNotifier() {}

    std::shared_ptr<ChangedNotify<T> > notify() const
    {
        return m_notify;
    }

    void changed()
    {
        m_notify->send(CallType::Changed);
    }

    void itemChanged(const T& item)
    {
        m_notify->send(CallType::ItemChanged, item);
    }

    void itemAdded(const T& item)
    {
        m_notify->send(CallType::ItemAdded, item);
    }

    void itemRemoved(const T& item)
    {
        m_notify->send(CallType::ItemRemoved, item);
    }

    void itemReplaced(const T& oldItem, const T& newItem)
    {
        m_notify->send(CallType::ItemReplaced, oldItem, newItem);
    }

private:
    std::shared_ptr<ChangedNotify<T> > m_notify;
};
}
