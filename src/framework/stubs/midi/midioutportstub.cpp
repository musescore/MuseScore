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
 #include "midioutportstub.h"

using namespace muse;
using namespace muse::midi;

MidiDeviceList MidiOutPortStub::availableDevices() const
{
    return {};
}

async::Notification MidiOutPortStub::availableDevicesChanged() const
{
    return {};
}

Ret MidiOutPortStub::connect(const MidiDeviceID&)
{
    return Ret(Ret::Code::NotImplemented);
}

void MidiOutPortStub::disconnect()
{
}

bool MidiOutPortStub::isConnected() const
{
    return false;
}

MidiDeviceID MidiOutPortStub::deviceID() const
{
    return {};
}

async::Notification MidiOutPortStub::deviceChanged() const
{
    return {};
}

bool MidiOutPortStub::supportsMIDI20Output() const
{
    return false;
}

Ret MidiOutPortStub::sendEvent(const Event&)
{
    return Ret(Ret::Code::NotImplemented);
}
