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
#include "winmidioutport.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include "midierrors.h"
#include "stringutils.h"
#include "translation.h"
#include "defer.h"
#include "log.h"

struct muse::midi::WinMidiOutPort::Win {
    HMIDIOUT midiOut;
    int deviceID;
};

using namespace muse;
using namespace muse::midi;

static std::string errorString(MMRESULT ret)
{
    switch (ret) {
    case MMSYSERR_NOERROR: return "MMSYSERR_NOERROR";
    case MIDIERR_NODEVICE: return "MIDIERR_NODEVICE";
    case MMSYSERR_ALLOCATED: return "MMSYSERR_ALLOCATED";
    case MMSYSERR_BADDEVICEID: return "MMSYSERR_BADDEVICEID";
    case MMSYSERR_INVALPARAM: return "MMSYSERR_INVALPARAM";
    case MMSYSERR_NOMEM: return "MMSYSERR_NOMEM";
    }

    return "UNKNOWN";
}

void WinMidiOutPort::init()
{
    m_win = std::make_shared<Win>();

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

void WinMidiOutPort::deinit()
{
    if (isConnected()) {
        disconnect();
    }
}

MidiDeviceList WinMidiOutPort::availableDevices() const
{
    std::lock_guard lock(m_devicesMutex);
    MidiDeviceList ret;

    ret.push_back({ NONE_DEVICE_ID, muse::trc("midi", "No device") });

    int numDevs = midiOutGetNumDevs();
    if (numDevs == 0) {
        return ret;
    }

    for (int i = 0; i < numDevs; i++) {
        MIDIOUTCAPSW devCaps;
        midiOutGetDevCapsW(i, &devCaps, sizeof(MIDIOUTCAPSW));

        if (devCaps.wTechnology == MOD_SWSYNTH) {
            continue;
        }

        std::wstring wstr(devCaps.szPname);
        std::string str(wstr.begin(), wstr.end());

        MidiDevice dev;
        dev.id = makeUniqueDeviceId(i, devCaps.wMid, devCaps.wPid);
        dev.name = str;

        ret.push_back(std::move(dev));
    }

    return ret;
}

async::Notification WinMidiOutPort::availableDevicesChanged() const
{
    return m_availableDevicesChanged;
}

Ret WinMidiOutPort::connect(const MidiDeviceID& deviceID)
{
    DEFER {
        m_deviceChanged.notify();
    };

    if (isConnected()) {
        disconnect();
    }

    if (!deviceID.empty() && deviceID != NONE_DEVICE_ID) {
        std::vector<int> deviceParams = splitDeviceId(deviceID);
        IF_ASSERT_FAILED(deviceParams.size() == 3) {
            return make_ret(Err::MidiInvalidDeviceID, "invalid device id: " + deviceID);
        }

        m_win->deviceID = deviceParams.at(0);
        MMRESULT ret = midiOutOpen(&m_win->midiOut, m_win->deviceID, 0, 0, CALLBACK_NULL);
        if (ret != MMSYSERR_NOERROR) {
            return make_ret(Err::MidiFailedConnect, "failed open port, error: " + errorString(ret));
        }
    }

    m_deviceID = deviceID;

    LOGD() << "Connected to " << m_deviceID;

    return muse::make_ok();
}

void WinMidiOutPort::disconnect()
{
    if (!isConnected()) {
        return;
    }

    LOGD() << "Disconnected from " << m_deviceID;

    midiOutClose(m_win->midiOut);
    m_win->deviceID = -1;
    m_deviceID.clear();
}

bool WinMidiOutPort::isConnected() const
{
    return !m_deviceID.empty();
}

MidiDeviceID WinMidiOutPort::deviceID() const
{
    return m_deviceID;
}

async::Notification WinMidiOutPort::deviceChanged() const
{
    return m_deviceChanged;
}

bool WinMidiOutPort::supportsMIDI20Output() const
{
    return false;
}

Ret WinMidiOutPort::sendEvent(const Event& e)
{
    if (!isConnected()) {
        return make_ret(Err::MidiNotConnected);
    }

    auto events = e.toMIDI10();
    for (auto& event : events) {
        uint32_t msg = event.to_MIDI10Package();
        MMRESULT ret = midiOutShortMsg(m_win->midiOut, (DWORD)msg);
        if (ret != MMSYSERR_NOERROR) {
            return make_ret(Err::MidiFailedConnect, "failed send event, error: " + errorString(ret));
        }
    }

    return Ret(true);
}
