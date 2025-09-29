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

#include "../istartaudiocontroller.h"

#include "global/types/retval.h"
#include "global/async/asyncable.h"

#include "global/modularity/ioc.h"
#include "../iaudioconfiguration.h"
#include "audio/common/rpc/irpcchannel.h"
#include "audio/engine/iaudioworker.h"
#include "../../iaudiodrivercontroller.h"

namespace muse::audio {
class StartAudioController : public IStartAudioController, public async::Asyncable
{
    Inject<IAudioConfiguration> configuration;
    Inject<engine::IAudioWorker> audioWorker;
    Inject<IAudioDriverController> audioDriverController;
    Inject<rpc::IRpcChannel> rpcChannel;

public:
    StartAudioController() = default;

    bool isAudioStarted() const override;
    async::Channel<bool> isAudioStartedChanged() const override;

    void startAudioProcessing(const IApplication::RunMode& mode) override;
    void stopAudioProcessing() override;

private:
    IAudioDriverPtr audioDriver() const;

    ValCh<bool> m_isAudioStarted;
};
}
