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
#ifndef MUSE_AUDIO_IAUDIODRIVER_H
#define MUSE_AUDIO_IAUDIODRIVER_H

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "global/async/notification.h"
#include "modularity/imoduleinterface.h"

#include "audiotypes.h"
#include "framework/midi/miditypes.h"
#include "framework/audio/midiqueue.h"

namespace muse::audio {
class AudioDriverState;

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
        unsigned int sampleRate;      // frequency -- samples per second
        Format format;                // Audio data format
        uint8_t channels;             // Number of channels: 1 mono, 2 stereo
        uint16_t samples;             // Audio buffer size in sample FRAMES (total samples divided by channel count)
        Callback callback;            // Callback that feeds the audio device
        void* userdata;               // Userdata passed to callback (ignored for NULL callbacks).
    };

    virtual void init() = 0;

    virtual std::string name() const = 0;
    virtual bool open(const Spec& spec, Spec* activeSpec) = 0;
    virtual void close() = 0;
    virtual bool isOpened() const = 0;

    virtual const Spec& activeSpec() const = 0;

    virtual AudioDeviceID outputDevice() const = 0;
    virtual bool selectOutputDevice(const AudioDeviceID& id) = 0;
    virtual bool resetToDefaultOutputDevice() = 0;
    virtual async::Notification outputDeviceChanged() const = 0;

    virtual AudioDeviceList availableOutputDevices() const = 0;
    virtual async::Notification availableOutputDevicesChanged() const = 0;

    virtual unsigned int outputDeviceBufferSize() const = 0;
    virtual bool setOutputDeviceBufferSize(unsigned int bufferSize) = 0;
    virtual async::Notification outputDeviceBufferSizeChanged() const = 0;

    virtual std::vector<unsigned int> availableOutputDeviceBufferSizes() const = 0;

    virtual unsigned int outputDeviceSampleRate() const = 0;
    virtual bool setOutputDeviceSampleRate(unsigned int bufferSize) = 0;
    virtual async::Notification outputDeviceSampleRateChanged() const = 0;

    virtual std::vector<unsigned int> availableOutputDeviceSampleRates() const = 0;
    virtual bool pushMidiEvent(muse::midi::Event&) = 0;
    virtual std::vector<muse::midi::MidiDevice> availableMidiDevices() const = 0;

    virtual void resume() = 0;
    virtual void suspend() = 0;
};

class AudioDriverState
{
public:
    virtual std::string name() const = 0;
    virtual bool open(const IAudioDriver::Spec& spec, IAudioDriver::Spec* activeSpec) = 0;
    virtual void close() = 0;
    virtual bool isOpened() const = 0;
    virtual bool pushMidiEvent(muse::midi::Event&) = 0;
    virtual void registerMidiInputQueue(async::Channel<muse::midi::tick_t, muse::midi::Event >) = 0;
    virtual std::vector<muse::midi::MidiDevice> availableMidiDevices() const = 0;

    IAudioDriver::Spec m_spec; // current running spec
    std::string m_deviceId;
};
}

#endif // MUSE_AUDIO_IAUDIODRIVER_H
