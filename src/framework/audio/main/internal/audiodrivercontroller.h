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

#include "global/async/asyncable.h"

#include "audio/iaudiodrivercontroller.h"

#include "modularity/ioc.h"
#include "audio/main/iaudioconfiguration.h"
#include "audio/common/rpc/irpcchannel.h"

namespace muse::audio {
class AudioDriverController : public IAudioDriverController, public Injectable, public async::Asyncable
{
    Inject<IAudioConfiguration> configuration = { this };
    Inject<rpc::IRpcChannel> rpcChannel = { this };

public:
    AudioDriverController(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    // Api
    std::vector<std::string> availableAudioApiList() const override;

    std::string currentAudioApi() const override;
    void changeCurrentAudioApi(const std::string& name) override;
    async::Notification currentAudioApiChanged() const override;

    // Current driver operation
    AudioDeviceList availableOutputDevices() const override;
    async::Notification availableOutputDevicesChanged() const override;

    bool open(const IAudioDriver::Spec& spec, IAudioDriver::Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;

    const IAudioDriver::Spec& activeSpec() const override;
    async::Channel<IAudioDriver::Spec> activeSpecChanged() const override;

    AudioDeviceID outputDevice() const override;
    bool selectOutputDevice(const std::string& deviceId) override;
    async::Notification outputDeviceChanged() const override;

    std::vector<unsigned int> availableOutputDeviceBufferSizes() const override;
    void changeBufferSize(samples_t samples) override;
    async::Notification outputDeviceBufferSizeChanged() const override;

    std::vector<unsigned int> availableOutputDeviceSampleRates() const override;
    void changeSampleRate(sample_rate_t sampleRate) override;
    async::Notification outputDeviceSampleRateChanged() const override;

private:
    IAudioDriverPtr createDriver(const std::string& name) const;
    void setNewDriver(IAudioDriverPtr newDriver);

    void checkOutputDevice();
    void updateOutputSpec();

    IAudioDriverPtr m_audioDriver;
    async::Notification m_currentAudioApiChanged;
    async::Notification m_availableOutputDevicesChanged;
    async::Channel<IAudioDriver::Spec> m_activeSpecChanged;
    async::Notification m_outputDeviceChanged;
    async::Notification m_outputDeviceBufferSizeChanged;
    async::Notification m_outputDeviceSampleRateChanged;
};
}
