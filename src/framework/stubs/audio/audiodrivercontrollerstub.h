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

#include "audio/iaudiodrivercontroller.h"

namespace muse::audio {
class AudioDriverControllerStub : public IAudioDriverController
{
public:

    // Api
    std::vector<std::string> availableAudioApiList() const override;

    std::string currentAudioApi() const override;
    void changeCurrentAudioApi(const std::string& name)  override;
    async::Notification currentAudioApiChanged() const override;

    // Current driver operation
    AudioDeviceList availableOutputDevices() const override;
    async::Notification availableOutputDevicesChanged() const override;

    bool open(const IAudioDriver::Spec& spec, IAudioDriver::Spec* activeSpec)  override;
    void close()  override;
    bool isOpened() const override;

    const IAudioDriver::Spec& activeSpec() const override;
    async::Channel<IAudioDriver::Spec> activeSpecChanged() const override;

    AudioDeviceID outputDevice() const override;
    bool selectOutputDevice(const AudioDeviceID& deviceId)  override;
    async::Notification outputDeviceChanged() const override;

    std::vector<samples_t> availableOutputDeviceBufferSizes() const override;
    void changeBufferSize(samples_t samples)  override;
    async::Notification outputDeviceBufferSizeChanged() const override;

    std::vector<sample_rate_t> availableOutputDeviceSampleRates() const override;
    void changeSampleRate(sample_rate_t sampleRate)  override;
    async::Notification outputDeviceSampleRateChanged() const override;

private:
    mutable IAudioDriverPtr m_audioDriver;
};
}
