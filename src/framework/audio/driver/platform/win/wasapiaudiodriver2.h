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

#include <memory>
#include <thread>
#include <atomic>

#include "iaudiodriver.h"

namespace muse::audio {
class WasapiAudioDriver2 : public IAudioDriver
{
public:
    WasapiAudioDriver2();
    ~WasapiAudioDriver2();

    void init() override;

    std::string name() const override;

    AudioDeviceID defaultDevice() const override;
    bool open(const Spec& spec, Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;

    const Spec& activeSpec() const override;
    async::Channel<Spec> activeSpecChanged() const override;

    AudioDeviceList availableOutputDevices() const override;
    async::Notification availableOutputDevicesChanged() const override;
    std::vector<samples_t> availableOutputDeviceBufferSizes() const override;
    std::vector<sample_rate_t> availableOutputDeviceSampleRates() const override;

private:

    void updateAudioDeviceList();

    void th_audioThread();
    bool th_audioInitialize();
    void th_processAudioData();

    struct Data;
    std::shared_ptr<Data> m_data;

    AudioDeviceID m_defaultDeviceId;
    async::Notification m_outputDeviceChanged;

    struct DeviceListener;
    std::shared_ptr<DeviceListener> m_deviceListener;
    AudioDeviceList m_deviceList;
    async::Notification m_deviceListChanged;

    std::thread m_audioThread;
    IAudioDriver::Spec m_activeSpec;
    async::Channel<Spec> m_activeSpecChanged;
    uint32_t m_bufferFrames = 0;
    samples_t m_defaultPeriod = 0;
    samples_t m_minimumPeriod = 0;
    std::vector<uint8_t> m_surroundAudioBuffer; //! NOTE: See #17648
    std::atomic<bool> m_opened;
};
}
