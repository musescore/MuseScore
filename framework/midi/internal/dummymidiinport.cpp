//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "dummymidiinport.h"

using namespace mu;
using namespace mu::midi;

std::vector<MidiDevice> DummyMidiInPort::devices() const
{
    MidiDevice d;
    d.id = "dummy";
    d.name = "Dummy";
    return { d };
}

Ret DummyMidiInPort::connect(const MidiDeviceID& deviceID)
{
    m_deviceID = deviceID;
    return true;
}

void DummyMidiInPort::disconnect()
{
    m_deviceID.clear();
}

bool DummyMidiInPort::isConnected() const
{
    return !m_deviceID.empty();
}

MidiDeviceID DummyMidiInPort::deviceID() const
{
    return m_deviceID;
}

Ret DummyMidiInPort::run()
{
    m_running = true;
    return true;
}

void DummyMidiInPort::stop()
{
    m_running = false;
}

bool DummyMidiInPort::isRunning() const
{
    return m_running;
}

async::Channel<std::pair<tick_t, Event> > DummyMidiInPort::eventReceived() const
{
    return m_eventReceived;
}
