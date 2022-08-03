/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "abstractmidiinport.h"

#include "async/async.h"

using namespace mu;
using namespace mu::midi;

static constexpr int PROCESS_EVENTS_INTERVAL = 20;

void AbstractMidiInPort::init()
{
    m_mainThreadID = std::this_thread::get_id();

    QObject::connect(&m_processTimer, &QTimer::timeout, [this]() { doProcessEvents(); });
}

async::Channel<std::vector<std::pair<tick_t, Event> > > AbstractMidiInPort::eventsReceived() const
{
    return m_eventReceived;
}

void AbstractMidiInPort::doEventsRecived(const std::vector<std::pair<tick_t, Event> >& events)
{
    async::Async::call(this, [this, events]() {
        for (auto it : events) {
            m_eventsQueue.push_back(it);
        }

        if (m_eventsQueue.empty()) {
            return;
        }

        if (!m_processTimer.isActive()) {
            m_processTimer.start(PROCESS_EVENTS_INTERVAL);
        }
    }, m_mainThreadID);
}

void AbstractMidiInPort::doProcessEvents()
{
    if (!m_eventsQueue.empty()) {
        m_eventReceived.send(m_eventsQueue);
    }

    m_eventsQueue.clear();
    m_processTimer.stop();
}
