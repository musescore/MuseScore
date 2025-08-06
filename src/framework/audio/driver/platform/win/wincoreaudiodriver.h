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
#pragma once

#include <thread>
#include <atomic>

#include "async/asyncable.h"

#include "iaudiodriver.h"
#include "internal/audiodeviceslistener.h"

#include "audioclient.h"

namespace muse::audio {
class CoreAudioDriver : public IAudioDriver, public async::Asyncable
{
public:
    CoreAudioDriver();
    ~CoreAudioDriver();

    void init() override;

    std::string name() const override;
    bool open(const Spec& spec, Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;

    AudioDeviceID outputDevice() const override;
    bool selectOutputDevice(const AudioDeviceID& id) override;
    bool resetToDefaultOutputDevice() override;
    async::Notification outputDeviceChanged() const override;

    AudioDeviceList availableOutputDevices() const override;
    async::Notification availableOutputDevicesChanged() const override;

    unsigned int outputDeviceBufferSize() const override;
    bool setOutputDeviceBufferSize(unsigned int bufferSize) override;
    async::Notification outputDeviceBufferSizeChanged() const override;

    std::vector<unsigned int> availableOutputDeviceBufferSizes() const override;

    void resume() override;
    void suspend() override;

private:
    void clean();

    std::string defaultDeviceId() const;

    std::vector<unsigned int> resolveBufferSizes(unsigned int minBufferSize);

    std::atomic<bool> m_active { false };
    std::thread m_thread;

    async::Notification m_outputDeviceChanged;

    mutable std::mutex m_devicesMutex;
    AudioDevicesListener m_devicesListener;
    async::Notification m_availableOutputDevicesChanged;
    std::string m_deviceId;
    async::Notification m_bufferSizeChanged;
    async::Notification m_sampleRateChanged;
    std::vector<unsigned int> m_bufferSizes;
};
}
