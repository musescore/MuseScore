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

#ifndef MUSE_AUDIO_JACKAUDIODRIVER_H
#define MUSE_AUDIO_JACKAUDIODRIVER_H

#include <jack/jack.h>

#include "async/asyncable.h"
#include "iaudiodriver.h"
#include "../lin/audiodeviceslistener.h"

namespace muse::audio {
struct JackDriverState {
    float* buffer = nullptr;
    void* jackDeviceHandle = nullptr;
    unsigned long samples = 0;
    int channels = 0;
    std::vector<jack_port_t*> outputPorts;
    IAudioDriver::Callback callback;
    void* userdata = nullptr;
    IAudioDriver::Spec format;
};

class JackAudioDriver : public IAudioDriver, public async::Asyncable
{
public:
    JackAudioDriver();
    ~JackAudioDriver();

    void init() override;

    std::string name() const override;
    bool open(const Spec& spec, Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;

    const Spec& activeSpec() const override;

    AudioDeviceID outputDevice() const override;
    bool selectOutputDevice(const AudioDeviceID& deviceId) override;
    bool resetToDefaultOutputDevice() override;
    async::Notification outputDeviceChanged() const override;

    AudioDeviceList availableOutputDevices() const override;
    async::Notification availableOutputDevicesChanged() const override;

    unsigned int outputDeviceBufferSize() const override;
    bool setOutputDeviceBufferSize(unsigned int bufferSize) override;
    async::Notification outputDeviceBufferSizeChanged() const override;

    std::vector<unsigned int> availableOutputDeviceBufferSizes() const override;

    unsigned int outputDeviceSampleRate() const override;
    bool setOutputDeviceSampleRate(unsigned int bufferSize) override;
    async::Notification outputDeviceSampleRateChanged() const override;

    std::vector<unsigned int> availableOutputDeviceSampleRates() const override;

    void resume() override;
    void suspend() override;

private:
    async::Notification m_outputDeviceChanged;

    mutable std::mutex m_devicesMutex;
    AudioDevicesListener m_devicesListener;
    async::Notification m_availableOutputDevicesChanged;

    std::string m_deviceName;

    async::Notification m_bufferSizeChanged;
    async::Notification m_sampleRateChanged;

    std::unique_ptr<JackDriverState> m_jackDriverState;
};
}

#endif // MUSE_AUDIO_JACKAUDIODRIVER_H
