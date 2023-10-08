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
#include "linuxmidiinport.h"
#include "framework/audio/audiomodule.h"

#include "midierrors.h"
#include "stringutils.h"
#include "translation.h"
#include "defer.h"
#include "log.h"

using namespace muse::midi;

void LinuxMidiInPort::init(std::shared_ptr<muse::audio::AudioModule> am)
{
    //m_alsa = std::make_shared<Linux>();
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    LOGI("-- init using audiomodule: %lx", am);

    //std::shared_ptr<muse::audio::IAudioDriver> ad = am->getDriver();
    //LOGI("-- init got audiodriver: %lx", ad);
    //LOGI("-- audiohandle: %lx", ad->getAudioDriverHandle());
#endif
}

void LinuxMidiInPort::deinit()
{
    if (isConnected()) {
        disconnect();
    }
}

std::vector<MidiDevice> LinuxMidiInPort::availableDevices() const
{
// FIX: this is compile-time, change so that we call availableMidiDevices if jack is selected
#if defined(JACK_AUDIO)
    return audioDriver()->availableMidiDevices();
#else
    std::lock_guard lock(m_devicesMutex);
    std::vector<MidiDevice> ret;
    ret.push_back({ NONE_DEVICE_ID, muse::trc("midi", "No device") });
    // return concatenation of alsa + jack devices
    std::shared_ptr<IMidiInPort> m_midiInPortJack;
    return ret;
#endif
}

muse::async::Notification LinuxMidiInPort::availableDevicesChanged() const
{
    return m_availableDevicesChanged;
}

muse::Ret LinuxMidiInPort::connect(const MidiDeviceID& deviceID)
{
    if (!deviceExists(deviceID)) {
        return make_ret(Err::MidiFailedConnect, "not found device, id: " + deviceID);
    }

    if (isConnected()) {
        disconnect();
    }

    DEFER {
        m_deviceChanged.notify();
    };

    muse::Ret ret = muse::make_ok();

    if (!deviceID.empty() && deviceID != NONE_DEVICE_ID) {
        std::vector<int> deviceParams = splitDeviceId(deviceID);
        IF_ASSERT_FAILED(deviceParams.size() == 3) {
            return make_ret(Err::MidiInvalidDeviceID, "invalid device id: " + deviceID);
        }

        //m_alsa->client = deviceParams.at(1);
        //m_alsa->port = deviceParams.at(2);

        m_deviceID = deviceID;
        ret = run();
    } else {
        m_deviceID = deviceID;
    }

    if (ret) {
        LOGD() << "Connected to " << m_deviceID;
    }

    return ret;
}

void LinuxMidiInPort::disconnect()
{
    if (!isConnected()) {
        return;
    }

    stop();

    LOGD() << "Disconnected from " << m_deviceID;

    //m_alsa->client = -1;
    //m_alsa->port = -1;
    //m_alsa->midiIn = nullptr;
    m_deviceID.clear();
}

bool LinuxMidiInPort::isConnected() const
{
    return /* m_alsa && m_alsa->midiIn && */ !m_deviceID.empty();
}

MidiDeviceID LinuxMidiInPort::deviceID() const
{
    return m_deviceID;
}

muse::async::Notification LinuxMidiInPort::deviceChanged() const
{
    return m_deviceChanged;
}

muse::async::Channel<tick_t, Event> LinuxMidiInPort::eventReceived() const
{
    return m_eventReceived;
}

muse::Ret LinuxMidiInPort::run()
{
    if (!isConnected()) {
        return make_ret(Err::MidiNotConnected);
    }

    if (m_thread) {
        LOGW() << "already started";
        return muse::Ret(true);
    }

    m_running.store(true);
    m_thread = std::make_shared<std::thread>(process, this);
    return muse::Ret(true);
}

void LinuxMidiInPort::stop()
{
    if (!m_thread) {
        LOGW() << "already stopped";
        return;
    }

    m_running.store(false);
    m_thread->join();
    m_thread = nullptr;
}

void LinuxMidiInPort::process(LinuxMidiInPort* self)
{
    self->doProcess();
}

void LinuxMidiInPort::doProcess()
{
}

bool LinuxMidiInPort::deviceExists(const MidiDeviceID& deviceId) const
{
    for (const MidiDevice& device : availableDevices()) {
        if (device.id == deviceId) {
            return true;
        }
    }

    return false;
}
