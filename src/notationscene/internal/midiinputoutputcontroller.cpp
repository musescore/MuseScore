/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "midiinputoutputcontroller.h"

#include "midi/miditypes.h"

#include "log.h"

using namespace mu::notation;
using namespace muse;
using namespace muse::midi;

void MidiInputOutputController::init()
{
    if (!midiConfiguration()->midiPortIsAvalaible()) {
        return;
    }

    checkInputConnection();
    checkOutputConnection();

    midiInPort()->availableDevicesChanged().onNotify(this, [this]() {
        checkInputConnection();
    });

    midiConfiguration()->midiInputDeviceIdChanged().onNotify(this, [this]() {
        MidiDeviceID deviceId = midiConfiguration()->midiInputDeviceId();
        Ret ret = midiInPort()->connect(deviceId);
        if (!ret) {
            LOGW() << "failed connect to input device, deviceID: " << deviceId << ", err: " << ret.text();
        }
    });

    midiOutPort()->availableDevicesChanged().onNotify(this, [this]() {
        checkOutputConnection();
    });

    midiConfiguration()->midiOutputDeviceIdChanged().onNotify(this, [this]() {
        MidiDeviceID deviceId = midiConfiguration()->midiOutputDeviceId();
        Ret ret = midiOutPort()->connect(deviceId);
        if (!ret) {
            LOGW() << "failed connect to output device, deviceID: " << deviceId << ", err: " << ret.text();
        }
    });

    midiInPort()->eventReceived().onReceive(this, [this](const muse::midi::tick_t tick, const muse::midi::Event& event) {
        if (!configuration()->isMidiInputEnabled()) {
            return;
        }

        onMidiEventReceived(tick, event);
    });
}

void MidiInputOutputController::checkInputConnection()
{
    checkConnection(midiConfiguration()->midiInputDeviceId(),
                    midiInPort()->deviceID(),
                    midiInPort()->availableDevices(),
                    [this](const muse::midi::MidiDeviceID& deviceId) {
        return midiInPort()->connect(deviceId);
    });
}

void MidiInputOutputController::checkOutputConnection()
{
    checkConnection(midiConfiguration()->midiOutputDeviceId(),
                    midiOutPort()->deviceID(),
                    midiOutPort()->availableDevices(),
                    [this](const muse::midi::MidiDeviceID& deviceId) {
        return midiOutPort()->connect(deviceId);
    });
}

void MidiInputOutputController::checkConnection(const muse::midi::MidiDeviceID& preferredDeviceId,
                                                const muse::midi::MidiDeviceID& currentDeviceId,
                                                const muse::midi::MidiDeviceList& availableDevices,
                                                const std::function<Ret(const muse::midi::MidiDeviceID&)>& connectCallback)
{
    auto containsDevice = [](const MidiDeviceList& devices, const MidiDeviceID& deviceId) {
        for (const MidiDevice& device : devices) {
            if (device.id == deviceId) {
                return true;
            }
        }

        return false;
    };

    if (!preferredDeviceId.empty() && preferredDeviceId != currentDeviceId && containsDevice(availableDevices, preferredDeviceId)) {
        Ret ret = connectCallback(preferredDeviceId);
        if (!ret) {
            LOGW() << "failed connect to device, deviceID: " << preferredDeviceId << ", err: " << ret.text();
        }

        return;
    }

    if (containsDevice(availableDevices, currentDeviceId)) {
        return;
    }

    MidiDeviceID deviceId = firstAvailableDeviceId(availableDevices);
    if (deviceId.empty()) {
        deviceId = NONE_DEVICE_ID;
    }

    Ret ret = connectCallback(deviceId);
    if (!ret) {
        LOGW() << "failed connect to device, deviceID: " << deviceId << ", err: " << ret.text();
    }
}

void MidiInputOutputController::onMidiEventReceived(const muse::midi::tick_t tick, const muse::midi::Event& event)
{
    UNUSED(tick)

    Ret ret = midiRemote()->process(event);
    if (check_ret(ret, Ret::Code::Undefined)) {
        //! NOTE Event not command, pass further
        // pass
    } else if (!ret) {
        LOGE() << "failed midi remote process, err: " << ret.toString();
        // pass
    } else {
        //! NOTE Success midi remote process (the event is command)
        return;
    }

    auto notation = globalContext()->currentNotation();
    if (notation) {
        notation->midiInput()->onMidiEventReceived(event);
    }
}

muse::midi::MidiDeviceID MidiInputOutputController::firstAvailableDeviceId(const muse::midi::MidiDeviceList& devices) const
{
    for (const muse::midi::MidiDevice& device : devices) {
        if (device.id == muse::midi::NONE_DEVICE_ID) {
            continue;
        }

        return device.id;
    }

    return muse::midi::MidiDeviceID();
}
