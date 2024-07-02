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
#include "audiooutputdevicecontroller.h"

#include "global/async/async.h"

#include "audiothread.h"

#include "log.h"

using namespace muse::audio;

void AudioOutputDeviceController::init()
{
    checkConnection();

    audioDriver()->availableOutputDevicesChanged().onNotify(this, [this]() {
        checkConnection();
    });

    configuration()->audioOutputDeviceIdChanged().onNotify(this, [this]() {
        AudioDeviceID deviceId = configuration()->audioOutputDeviceId();
        audioDriver()->selectOutputDevice(deviceId);
    });

    configuration()->driverBufferSizeChanged().onNotify(this, [this]() {
        unsigned int bufferSize = configuration()->driverBufferSize();
        bool ok = audioDriver()->setOutputDeviceBufferSize(bufferSize);
        if (ok) {
            async::Async::call(this, [this, bufferSize](){
                audioEngine()->setReadBufferSize(bufferSize);
            }, AudioThread::ID);
        }
    });
}

void AudioOutputDeviceController::checkConnection()
{
    auto containsDevice = [](const AudioDeviceList& devices, const AudioDeviceID& deviceId) {
        for (const AudioDevice& device : devices) {
            if (device.id == deviceId) {
                return true;
            }
        }

        return false;
    };

    AudioDeviceID preferredDeviceId = configuration()->audioOutputDeviceId();
    AudioDeviceID currentDeviceId = audioDriver()->outputDevice();
    AudioDeviceList devices = audioDriver()->availableOutputDevices();

    if (!preferredDeviceId.empty() && preferredDeviceId != currentDeviceId && containsDevice(devices, preferredDeviceId)) {
        audioDriver()->selectOutputDevice(preferredDeviceId);
        return;
    }

    if (!containsDevice(devices, currentDeviceId)) {
        audioDriver()->resetToDefaultOutputDevice();
    }
}
