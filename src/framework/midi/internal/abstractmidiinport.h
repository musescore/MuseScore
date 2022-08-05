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
#ifndef MU_MIDI_ABSTRACTMIDIINPORT_H
#define MU_MIDI_ABSTRACTMIDIINPORT_H

#include <QTimer>

#include "async/asyncable.h"

#include "imidiinport.h"

namespace mu::midi {
class AbstractMidiInPort : public IMidiInPort, public async::Asyncable
{
public:
    virtual void init();

    async::Channel<std::vector<std::pair<tick_t, Event> > > eventsReceived() const override;

protected:
    void doEventsRecived(const std::vector<std::pair<tick_t, Event> >& events);

private:
    void doProcessEvents();

    async::Channel<std::vector<std::pair<tick_t, Event> > > m_eventReceived;

    QTimer m_processTimer;
    std::vector<std::pair<tick_t, Event> > m_eventsQueue;

    std::thread::id m_mainThreadID;
};
}

#endif // MU_MIDI_ABSTRACTMIDIINPORT_H
