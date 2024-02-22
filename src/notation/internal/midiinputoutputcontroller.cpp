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
#include "midiinputoutputcontroller.h"

#include "midi/miditypes.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::midi;

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

    midiInPort()->eventReceived().onReceive(this, [this](const midi::tick_t tick, const midi::Event& event) {
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
                    [this](const midi::MidiDeviceID& deviceId) {
        return midiInPort()->connect(deviceId);
    });
}

void MidiInputOutputController::checkOutputConnection()
{
    checkConnection(midiConfiguration()->midiOutputDeviceId(),
                    midiOutPort()->deviceID(),
                    midiOutPort()->availableDevices(),
                    [this](const midi::MidiDeviceID& deviceId) {
        return midiOutPort()->connect(deviceId);
    });
}

void MidiInputOutputController::checkConnection(const midi::MidiDeviceID& preferredDeviceId, const midi::MidiDeviceID& currentDeviceId,
                                                const midi::MidiDeviceList& availableDevices,
                                                const std::function<Ret(const midi::MidiDeviceID&)>& connectCallback)
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

void MidiInputOutputController::onMidiEventReceived(const midi::tick_t tick, const midi::Event& event)
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

mu::midi::MidiDeviceID MidiInputOutputController::firstAvailableDeviceId(const midi::MidiDeviceList& devices) const
{
    for (const midi::MidiDevice& device : devices) {
        if (device.id == midi::NONE_DEVICE_ID) {
            continue;
        }

        return device.id;
    }

    return midi::MidiDeviceID();
}
