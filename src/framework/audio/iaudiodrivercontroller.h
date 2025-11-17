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

#pragma once

#include "iaudiodriver.h"

#include "modularity/imoduleinterface.h"

namespace muse::audio {
class IAudioDriverController : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioDriverController)

public:
    virtual ~IAudioDriverController() = default;

    // Api
    virtual std::vector<std::string> availableAudioApiList() const = 0;

    virtual std::string currentAudioApi() const = 0;
    virtual void changeCurrentAudioApi(const std::string& name) = 0;
    virtual async::Notification currentAudioApiChanged() const = 0;

    // Current driver operation
    virtual AudioDeviceList availableOutputDevices() const = 0;
    virtual async::Notification availableOutputDevicesChanged() const = 0;

    virtual bool open(const IAudioDriver::Spec& spec, IAudioDriver::Spec* activeSpec) = 0;
    virtual void close() = 0;
    virtual bool isOpened() const = 0;

    virtual const IAudioDriver::Spec& activeSpec() const = 0;
    virtual async::Channel<IAudioDriver::Spec> activeSpecChanged() const = 0;

    virtual AudioDeviceID outputDevice() const = 0;
    virtual bool selectOutputDevice(const std::string& deviceId) = 0;
    virtual async::Notification outputDeviceChanged() const = 0;

    virtual std::vector<unsigned int> availableOutputDeviceBufferSizes() const = 0;
    virtual void changeBufferSize(samples_t samples) = 0;
    virtual async::Notification outputDeviceBufferSizeChanged() const = 0;

    virtual std::vector<unsigned int> availableOutputDeviceSampleRates() const = 0;
    virtual void changeSampleRate(sample_rate_t sampleRate) = 0;
    virtual async::Notification outputDeviceSampleRateChanged() const = 0;
};
}
