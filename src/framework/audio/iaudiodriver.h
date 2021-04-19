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
#ifndef MU_AUDIO_IAUDIODRIVER_H
#define MU_AUDIO_IAUDIODRIVER_H

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "modularity/imoduleexport.h"
#include "async/notification.h"

namespace mu::audio {
class IAudioDriver : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioDriver)

public:
    virtual ~IAudioDriver() = default;

    enum class Format {
        AudioF32, // float 32 bit
        AudioS16  // short 16 bit
    };

    using Callback = std::function<void (void* userdata, uint8_t* stream, int len)>;

    struct Spec
    {
        int sampleRate;               // frequency -- samples per second
        Format format;                // Audio data format
        uint8_t channels;             // Number of channels: 1 mono, 2 stereo
        uint16_t samples;             // Audio buffer size in sample FRAMES (total samples divided by channel count)
        Callback callback;            // Callback that feeds the audio device
        void* userdata;               // Userdata passed to callback (ignored for NULL callbacks).
    };

    virtual std::string name() const = 0;
    virtual bool open(const Spec& spec, Spec* activeSpec) = 0;
    virtual void close() = 0;
    virtual bool isOpened() const = 0;

    virtual std::string outputDevice() const = 0;
    virtual bool selectOutputDevice(const std::string& name) = 0;
    virtual std::vector<std::string> availableOutputDevices() const = 0;
    virtual async::Notification availableOutputDevicesChanged() const = 0;

    virtual void resume() = 0;
    virtual void suspend() = 0;
};
using IAudioDriverPtr = std::shared_ptr<IAudioDriver>;
}

#endif // MU_AUDIO_IAUDIODRIVER_H
