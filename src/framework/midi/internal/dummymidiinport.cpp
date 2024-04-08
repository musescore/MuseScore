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
#include "dummymidiinport.h"

using namespace muse;
using namespace muse::midi;

void DummyMidiInPort::init()
{
}

std::vector<MidiDevice> DummyMidiInPort::availableDevices() const
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

async::Channel<tick_t, Event> DummyMidiInPort::eventReceived() const
{
    return m_eventReceived;
}
