/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) MuseScore BVBA and others
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
//#include <thread>
//#include <queue>
//#include <mutex>
#include "linuxmidioutport.h"
#include "framework/audio/audiomodule.h"

#include "midierrors.h"
#include "translation.h"
#include "defer.h"
#include "log.h"

using namespace muse::midi;

void LinuxMidiOutPort::init()
{
#if JACK_AUDIO
    m_midiOutPortJack = std::make_unique<JackMidiOutPort>();
    m_midiOutPortJack->init();
#endif
    m_midiOutPortAlsa = std::make_unique<AlsaMidiOutPort>();
    m_midiOutPortAlsa->init();

    m_devicesListener.startWithCallback([this]() {
        return availableDevices();
    });

    m_devicesListener.devicesChanged().onNotify(this, [this]() {
        bool connectedDeviceRemoved = true;
        for (const MidiDevice& device: availableDevices()) {
            if (m_deviceID == device.id) {
                connectedDeviceRemoved = false;
            }
        }

        if (connectedDeviceRemoved) {
            disconnect();
        }

        m_availableDevicesChanged.notify();
    });
}

void LinuxMidiOutPort::deinit()
{
    if (isConnected()) {
        disconnect();
    }
}

std::vector<MidiDevice> LinuxMidiOutPort::availableDevices() const
{
    std::lock_guard lock(m_devicesMutex);

    std::vector<MidiDevice> ret;
#if JACK_AUDIO
    auto vj = m_midiOutPortJack->availableDevices();
    ret.insert(ret.end(), vj.begin(), vj.end());
#endif
    auto va = m_midiOutPortAlsa->availableDevices();
    ret.insert(ret.end(), va.begin(), va.end());

    // FIX: this should be done by gui/caller
    ret.push_back({ NONE_DEVICE_ID, muse::trc("midi", "No device") });
    return ret;
}

muse::async::Notification LinuxMidiOutPort::availableDevicesChanged() const
{
    return m_availableDevicesChanged;
}

muse::Ret LinuxMidiOutPort::connect(const MidiDeviceID& deviceID)
{
    if (!deviceExists(deviceID)) {
        return make_ret(Err::MidiFailedConnect, "not found device, id: " + deviceID);
    }

    DEFER {
        m_deviceChanged.notify();
    };

    if (!deviceID.empty() && deviceID != NONE_DEVICE_ID) {
        std::vector<int> deviceParams = splitDeviceId(deviceID);
        IF_ASSERT_FAILED(deviceParams.size() == 3) {
            return make_ret(Err::MidiInvalidDeviceID, "invalid device id: " + deviceID);
        }

        if (deviceParams.at(1) == 9999) { // This is an jack device
#if JACK_AUDIO
            m_midiOutPortCurrent = /* JackMidiOutPort */ m_midiOutPortJack.get();
#endif
        } else {
            m_midiOutPortCurrent = /* AlsaMidiOutPort */ m_midiOutPortAlsa.get();
        }

        if (m_midiOutPortCurrent->isConnected()) {
            m_midiOutPortCurrent->disconnect();
        }
        LOGD() << "Connected to " << deviceID; // FIX: let caller log instead (has return state of connect)
        m_deviceID = deviceID;
        // continue the connect in the driver
        return m_midiOutPortCurrent->connect(deviceID);
    }

    return Ret(true);
}

void LinuxMidiOutPort::disconnect()
{
    if (!isConnected()) {
        return;
    }

    LOGD() << "Disconnected from " << m_deviceID;

    m_deviceID.clear();
}

bool LinuxMidiOutPort::isConnected() const
{
    return m_midiOutPortCurrent && !m_deviceID.empty();
}

MidiDeviceID LinuxMidiOutPort::deviceID() const
{
    return m_deviceID;
}

muse::async::Notification LinuxMidiOutPort::deviceChanged() const
{
    return m_deviceChanged;
}

bool LinuxMidiOutPort::supportsMIDI20Output() const
{
    return false;
}

muse::Ret LinuxMidiOutPort::sendEvent(const Event& e)
{
    if (!isConnected()) {
        return make_ret(Err::MidiNotConnected);
    }

    if (e.isChannelVoice20()) {
        auto events = e.toMIDI10();
        for (auto& event : events) {
            muse::Ret ret = sendEvent(event);
            if (!ret) {
                return ret;
            }
        }
        return muse::Ret(true);
    }

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    Event e2(e);
    return muse::Ret(audioDriver()->pushMidiEvent(e2));
#else // alsa
    return m_midiOutPortCurrent->sendEvent(e);
#endif
}

bool LinuxMidiOutPort::deviceExists(const MidiDeviceID& deviceId) const
{
    for (const MidiDevice& device : availableDevices()) {
        if (device.id == deviceId) {
            return true;
        }
    }
    return false;
}
