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

    std::string currentAudioApi() const override;
    std::vector<std::string> availableAudioApiList() const override;

    IAudioDriverPtr audioDriver() const override;
    void changeAudioDriver(const std::string& name) override;
    async::Notification audioDriverChanged() const override;

    void selectOutputDevice(const std::string& deviceId) override;
    void changeBufferSize(samples_t samples) override;
    void changeSampleRate(sample_rate_t sampleRate) override;

private:
    IAudioDriverPtr createDriver(const std::string& name) const;
    void subscribeOnDriver();

    void checkOutputDevice();
    void updateOutputSpec();

    IAudioDriverPtr m_audioDriver;
    async::Notification m_audioDriverChanged;
};
}
