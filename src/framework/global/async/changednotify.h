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
    void onChanged(Asyncable* receiver, Call f, Asyncable::Mode mode = Asyncable::Mode::SetOnce)
    {
        m_data->changed.onReceive(receiver, f, mode);
    }

    template<typename Call>
    void onItemChanged(Asyncable* receiver, Call f, Asyncable::Mode mode = Asyncable::Mode::SetOnce)
    {
        m_data->itemChanged.onReceive(receiver, f, mode);
    }

    template<typename Call>
    void onItemAdded(Asyncable* receiver, Call f, Asyncable::Mode mode = Asyncable::Mode::SetOnce)
    {
        m_data->itemAdded.onReceive(receiver, f, mode);
    }

    template<typename Call>
    void onItemRemoved(Asyncable* receiver, Call f, Asyncable::Mode mode = Asyncable::Mode::SetOnce)
    {
        m_data->itemRemoved.onReceive(receiver, f, mode);
    }

    template<typename Call>
    void onItemReplaced(Asyncable* receiver, Call f, Asyncable::Mode mode = Asyncable::Mode::SetOnce)
    {
        m_data->itemReplaced.onReceive(receiver, f, mode);
    }

    void disconnect(Asyncable* receiver)
    {
        m_data->changed.disconnect(receiver);
        m_data->itemChanged.disconnect(receiver);
        m_data->itemAdded.disconnect(receiver);
        m_data->itemRemoved.disconnect(receiver);
        m_data->itemReplaced.disconnect(receiver);
    }

private:
    friend class ChangedNotifier<T>;

    struct Data {
        Channel<> changed;
        Channel<const T&> itemChanged;
        Channel<const T&> itemAdded;
        Channel<const T&> itemRemoved;
        Channel<const T&, const T&> itemReplaced;
    };

    std::shared_ptr<Data> m_data;
};

template<typename T>
class ChangedNotifier
{
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
        m_notify->m_data->changed.send();
    }

    void itemChanged(const T& item)
    {
        m_notify->m_data->itemChanged.send(item);
    }

    void itemAdded(const T& item)
    {
        m_notify->m_data->itemAdded.send(item);
    }

    void itemRemoved(const T& item)
    {
        m_notify->m_data->itemRemoved.send(item);
    }

    void itemReplaced(const T& oldItem, const T& newItem)
    {
        m_notify->m_data->itemReplaced.send(oldItem, newItem);
    }

private:
    std::shared_ptr<ChangedNotify<T> > m_notify;
};
}
