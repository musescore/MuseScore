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

#include "log.h"

using namespace muse;
using namespace muse::midi;

MidiDevicesListener::~MidiDevicesListener()
{
    stop();
}

void MidiDevicesListener::startWithCallback(const ActualDevicesCallback& callback)
{
    IF_ASSERT_FAILED(!m_isRunning) {
        LOGE() << "Cannot set callback while already running.";
        return;
    }

    m_actualDevicesCallback = callback;
    m_isRunning = true;
    m_devicesUpdateThread = std::make_shared<std::thread>(&MidiDevicesListener::th_updateDevices, this);
}

void MidiDevicesListener::stop()
{
    if (!m_devicesUpdateThread) {
        return;
    }

    m_isRunning = false;
    m_runningCv.notify_all();

    m_devicesUpdateThread->join();
    m_devicesUpdateThread = nullptr;
}

async::Notification MidiDevicesListener::devicesChanged() const
{
    return m_devicesChanged;
}

void MidiDevicesListener::th_updateDevices()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    while (m_isRunning) {
        MidiDeviceList devices = m_actualDevicesCallback();

        th_setDevices(devices);

        m_runningCv.wait_for(lock, std::chrono::milliseconds(5000));
    }
}

void MidiDevicesListener::th_setDevices(const MidiDeviceList& devices)
{
    if (devices == m_devices) {
        return;
    }

    m_devices = devices;
    m_devicesChanged.notify();
}
