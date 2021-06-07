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
#include "midideviceslistener.h"

#include <memory>

using namespace mu::midi;

MidiDevicesListener::MidiDevicesListener()
{
    m_devicesUpdateThread = std::make_shared<std::thread>(updateDevices, this);
}

MidiDevicesListener::~MidiDevicesListener()
{
    if (!m_devicesUpdateThread) {
        return;
    }

    m_devicesUpdateThread->join();
    m_devicesUpdateThread = nullptr;
}

void MidiDevicesListener::reg(const ActualDevicesCallback& callback)
{
    m_actualDevicesCallback = callback;
}

mu::async::Notification MidiDevicesListener::devicesChanged() const
{
    return m_devicesChanged;
}

void MidiDevicesListener::updateDevices(MidiDevicesListener* self)
{
    self->doUpdateDevices();
}

void MidiDevicesListener::doUpdateDevices()
{
    while (true) {
        MidiDeviceList devices = m_actualDevicesCallback();

        setDevices(devices);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void MidiDevicesListener::setDevices(const MidiDeviceList& devices)
{
    if (devices == m_devices) {
        return;
    }

    m_devices = devices;
    m_devicesChanged.notify();
}
