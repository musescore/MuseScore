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

#include "asyncable.h"
#include "internal/channelimpl.h"

namespace muse::audio::comm {
template<typename ... T>
class Channel
{
public:

    using Callback = typename ChannelImpl<T...>::Callback;

private:

    std::shared_ptr<ChannelImpl<T...> > m_mainCh;
    std::shared_ptr<ChannelImpl<bool> > m_closeCh;
    std::shared_ptr<ChannelImpl<const Asyncable*> > m_disconnectCh;

public:
    Channel()
        : m_mainCh(std::make_shared<ChannelImpl<T...> >())
    {
    }

    Channel(const Channel& ch)
        : m_mainCh(ch.m_mainCh), m_closeCh(ch.m_closeCh), m_disconnectCh(ch.m_disconnectCh)
    {
    }

    ~Channel() = default;

    Channel& operator=(const Channel& ch)
    {
        m_mainCh = ch.m_mainCh;
        m_closeCh = ch.m_closeCh;
        m_disconnectCh = ch.m_disconnectCh;
        return *this;
    }

    void send(const T&... args)
    {
        m_mainCh->send(SendMode::Auto, args ...);
    }

    void onReceive(const Asyncable* receiver, const Callback& f)
    {
        m_mainCh->onReceive(receiver, f);
    }

    template<typename Func>
    void onReceive(const Asyncable* receiver, Func f)
    {
        Callback callback = [f](const T&... args) {
            f(args ...);
        };
        onReceive(receiver, callback);
    }

    void disconnect(const Asyncable* a)
    {
        if (m_mainCh->isSending()) {
            m_mainCh->disableReceiver(a);
            if (!m_disconnectCh) {
                m_disconnectCh = std::make_shared<ChannelImpl<const Asyncable*> >();
                m_disconnectCh->onReceive(nullptr, [this](const Asyncable* a) {
                    disconnect(a);
                });
            }

            m_disconnectCh->send(SendMode::Queue, a);
        } else {
            m_mainCh->disconnectReceiver(a);
        }
    }

    void close()
    {
        if (m_closeCh) {
            m_closeCh->send(SendMode::Auto, true);
        }
    }

    void onClose(const Asyncable* receiver, const Callback& f)
    {
        if (!m_closeCh) {
            m_closeCh = std::make_shared<ChannelImpl<bool> >();
        }
        m_closeCh->onReceive(receiver, f);
    }

    template<typename Func>
    void onClose(const Asyncable* receiver, Func f)
    {
        Callback callback = [f](const T&... args) {
            f(args ...);
        };
        onClose(receiver, callback);
    }

    bool isConnected() const
    {
        return m_mainCh->isConnected();
    }

    uint64_t key() const { return reinterpret_cast<uint64_t>(m_mainCh.get()); }
};
}
