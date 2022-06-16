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
#include "audiooutputcontroller.h"

#include "log.h"

using namespace mu::audio;

void AudioOutputController::init()
{
    audioDriver()->selectOutputDevice(configuration()->audioOutputDeviceName());

    audioDriver()->availableOutputDevicesChanged().onNotify(this, [this]() {
        checkConnection();
    });

    configuration()->audioOutputDeviceNameChanged().onNotify(this, [this]() {
        std::string deviceName = configuration()->audioOutputDeviceName();
        audioDriver()->selectOutputDevice(deviceName);
    });
}

void AudioOutputController::checkConnection()
{
    std::string currentDeviceName = configuration()->audioOutputDeviceName();
    std::vector<std::string> devices = audioDriver()->availableOutputDevices();

    if (!contains(devices, currentDeviceName)) {
        audioDriver()->resetToDefaultOutputDevice();
    }
}

void AudioOutputController::connectCurrentOutputDevice()
{
    std::string deviceName = configuration()->audioOutputDeviceName();
    if (deviceName.empty()) {
        return;
    }

    bool ok = audioDriver()->selectOutputDevice(deviceName);
    if (!ok) {
        LOGW() << "failed connect to output device, deviceName: " << deviceName;
    }
}
