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

#include <thread>
#include <atomic>

#include "../../../../iaudiodriver.h"

#include "global/async/asyncable.h"

namespace muse::audio {
class AsioAudioDriver : public IAudioDriver, public async::Asyncable
{
public:
    AsioAudioDriver();

    void init() override;

    std::string name() const override;
    bool open(const Spec& spec, Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;

    const Spec& activeSpec() const override;
    async::Channel<Spec> activeSpecChanged() const override;

    bool setOutputDeviceBufferSize(unsigned int bufferSize) override;
    async::Notification outputDeviceBufferSizeChanged() const override;
    bool setOutputDeviceSampleRate(unsigned int sampleRate) override;
    async::Notification outputDeviceSampleRateChanged() const override;
    std::vector<unsigned int> availableOutputDeviceBufferSizes() const override;
    std::vector<unsigned int> availableOutputDeviceSampleRates() const override;

    AudioDeviceID outputDevice() const override;
    bool selectOutputDevice(const AudioDeviceID& id) override;
    bool resetToDefaultOutputDevice() override;
    async::Notification outputDeviceChanged() const override;

    AudioDeviceList availableOutputDevices() const override;
    async::Notification availableOutputDevicesChanged() const override;

    void resume() override;
    void suspend() override;

private:

    bool doOpen(const AudioDeviceID& device, const Spec& spec, Spec* activeSpec);
    void reset();

    std::thread m_thread;
    std::atomic<bool> m_running = false;

    AudioDeviceID m_deviceId;
    async::Notification m_outputDeviceChanged;
    async::Notification m_availableOutputDevicesChanged;

    async::Channel<Spec> m_activeSpecChanged;

    async::Notification m_outputDeviceBufferSizeChanged;
    async::Notification m_outputDeviceSampleRateChanged;
};
}
