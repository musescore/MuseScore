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
#include "dummymidioutport.h"

#include "log.h"
#include "midierrors.h"

using namespace muse;
using namespace muse::midi;

void DummyMidiOutPort::init()
{
}

MidiDeviceList DummyMidiOutPort::availableDevices() const
{
    MidiDevice d;
    d.id = "dummy";
    d.name = "Dummy";
    return { d };
}

async::Notification DummyMidiOutPort::availableDevicesChanged() const
{
    return {};
}

Ret DummyMidiOutPort::connect(const MidiDeviceID& deviceID)
{
    LOGI() << "deviceID: " << deviceID;
    m_connectedDeviceID = deviceID;
    return Ret(true);
}

void DummyMidiOutPort::disconnect()
{
    LOGI() << "disconnect";
    m_connectedDeviceID.clear();
}

bool DummyMidiOutPort::isConnected() const
{
    return !m_connectedDeviceID.empty();
}

MidiDeviceID DummyMidiOutPort::deviceID() const
{
    return m_connectedDeviceID;
}

bool DummyMidiOutPort::supportsMIDI20Output() const
{
    return false;
}

Ret DummyMidiOutPort::sendEvent(const Event& e)
{
    if (!isConnected()) {
        return make_ret(Err::MidiNotConnected);
    }
    LOGI() << e.to_string();
    return Ret(true);
}
