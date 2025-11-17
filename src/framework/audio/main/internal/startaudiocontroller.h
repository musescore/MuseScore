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
#include "../../iaudiodrivercontroller.h"
#include "../isoundfontcontroller.h"

namespace muse::audio::engine {
class EngineController;
class GeneralAudioWorker;
}

namespace muse::audio {
class AlignmentBuffer;
class StartAudioController : public IStartAudioController, public async::Asyncable
{
    Inject<IAudioConfiguration> configuration;
    Inject<IAudioDriverController> audioDriverController;
    Inject<ISoundFontController> soundFontController;

public:
    StartAudioController(std::shared_ptr<rpc::IRpcChannel> rpcChannel);

    void registerExports();
    void init();

    bool isAudioStarted() const override;
    async::Channel<bool> isAudioStartedChanged() const override;

    void startAudioProcessing(const IApplication::RunMode& mode) override;
    void stopAudioProcessing() override;

private:

    void th_setupEngine();

    std::shared_ptr<rpc::IRpcChannel> m_rpcChannel;
    std::shared_ptr<engine::EngineController> m_engineController;
    std::shared_ptr<engine::GeneralAudioWorker> m_worker;
    std::shared_ptr<AlignmentBuffer> m_alignmentBuffer;
    size_t m_requiredSamplesTotal = 0;

    ValCh<bool> m_isEngineRunning;
    ValCh<bool> m_isAudioStarted;
};
}
