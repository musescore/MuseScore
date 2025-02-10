/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include "runloop.h"

#include <QSocketNotifier>
#include <QTimer>

#include "log.h"

using namespace muse::vst;

IMPLEMENT_FUNKNOWN_METHODS(RunLoop, Steinberg::Linux::IRunLoop, Steinberg::Linux::IRunLoop::iid);

RunLoop::RunLoop()
{
    FUNKNOWN_CTOR;
}

RunLoop::~RunLoop()
{
    FUNKNOWN_DTOR;
}

Steinberg::tresult RunLoop::registerEventHandler(Steinberg::Linux::IEventHandler* handler,
                                                 Steinberg::Linux::FileDescriptor fd)
{
    LOGD() << "call handler: " << handler << ", FileDescriptor: " << fd;

    if (!handler) {
        return Steinberg::kInvalidArgument;
    }

    auto it = std::find_if(m_handlers.begin(), m_handlers.end(), [fd](const Handler* h) {
        return h->fd == fd;
    });

    if (it != m_handlers.end()) {
        return Steinberg::kInvalidArgument;
    }

    Handler* h = new Handler();
    h->fd = fd;
    h->handler = handler;
    h->readSN = new QSocketNotifier(fd, QSocketNotifier::Read);
    h->writeSN = new QSocketNotifier(fd, QSocketNotifier::Write);

    QObject::connect(h->readSN, &QSocketNotifier::activated, [h](QSocketDescriptor, QSocketNotifier::Type) {
        h->handler->onFDIsSet(h->fd);
    });

    QObject::connect(h->writeSN, &QSocketNotifier::activated, [h](QSocketDescriptor, QSocketNotifier::Type) {
        h->handler->onFDIsSet(h->fd);
    });

    m_handlers.push_back(h);

    return Steinberg::kResultTrue;
}

RunLoop::Handler::~Handler()
{
    readSN->disconnect();
    writeSN->disconnect();

    delete readSN;
    delete writeSN;
}

Steinberg::tresult RunLoop::unregisterEventHandler(Steinberg::Linux::IEventHandler* handler)
{
    LOGD() << "call handler: " << handler;

    for (auto it = m_handlers.begin(); it != m_handlers.end();) {
        Handler* h = *it;
        if (h->handler != handler) {
            ++it; // next
            continue;
        }

        m_handlers.erase(it);

        delete h;
    }

    return Steinberg::kResultTrue;
}

Steinberg::tresult RunLoop::registerTimer(Steinberg::Linux::ITimerHandler* handler,
                                          Steinberg::Linux::TimerInterval milliseconds)
{
    LOGD() << "call handler: " << handler << ", interval: " << milliseconds;

    auto it = std::find_if(m_timers.begin(), m_timers.end(), [handler](const Timer* t) {
        return t->handler == handler;
    });

    if (it != m_timers.end()) {
        return Steinberg::kInvalidArgument;
    }

    Timer* t = new Timer();
    t->interval = milliseconds;
    t->handler = handler;
    t->timer = new QTimer();
    t->timer->setSingleShot(false);
    t->timer->start(milliseconds);

    QObject::connect(t->timer, &QTimer::timeout, [t]() {
        t->handler->onTimer();
    });

    m_timers.push_back(t);

    return Steinberg::kResultTrue;
}

RunLoop::Timer::~Timer()
{
    timer->disconnect();
    delete timer;
}

Steinberg::tresult RunLoop::unregisterTimer(Steinberg::Linux::ITimerHandler* handler)
{
    LOGD() << "call handler: " << handler;

    for (auto it = m_timers.begin(); it != m_timers.end();) {
        Timer* t = *it;
        if (t->handler != handler) {
            ++it; // next
            continue;
        }

        m_timers.erase(it);
        delete t;
    }

    return Steinberg::kResultTrue;
}

void RunLoop::stop()
{
    for (auto it = m_handlers.begin(); it != m_handlers.end(); ++it) {
        Handler* h = *it;
        delete h;
    }

    m_handlers.clear();

    for (auto it = m_timers.begin(); it != m_timers.end(); ++it) {
        Timer* t = *it;
        delete t;
    }

    m_timers.clear();
}
