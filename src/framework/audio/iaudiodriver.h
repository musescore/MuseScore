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

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "global/async/notification.h"

#include "audio/common/audiotypes.h"

namespace muse::audio {
class IAudioDriver
{
public:
    virtual ~IAudioDriver() = default;

    using Callback = std::function<void (uint8_t* stream, int len)>;

    struct Spec
    {
        AudioDeviceID deviceId;
        OutputSpec output;
        Callback callback;
        inline bool isValid() const { return output.isValid() && !deviceId.empty() && callback != nullptr; }
    };

    virtual void init() = 0;

    virtual std::string name() const = 0;

    virtual AudioDeviceID defaultDevice() const = 0;

    virtual bool open(const Spec& spec, Spec* activeSpec) = 0;
    virtual void close() = 0;
    virtual bool isOpened() const = 0;

    virtual const Spec& activeSpec() const = 0;
    virtual async::Channel<Spec> activeSpecChanged() const = 0;

    virtual std::vector<samples_t> availableOutputDeviceBufferSizes() const = 0;
    virtual std::vector<sample_rate_t> availableOutputDeviceSampleRates() const = 0;
    virtual AudioDeviceList availableOutputDevices() const = 0;
    virtual async::Notification availableOutputDevicesChanged() const = 0;
};
using IAudioDriverPtr = std::shared_ptr<IAudioDriver>;
}
