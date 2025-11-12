/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#ifndef MUSE_AUDIO_AUDIOMODULE_H
#define MUSE_AUDIO_AUDIOMODULE_H

#include <memory>

#include "modularity/imodulesetup.h"
#include "global/async/asyncable.h"
#include "global/ticker.h"

namespace muse::audio::rpc  {
class IRpcChannel;
}

namespace muse::audio {
class AudioConfiguration;
class AudioActionsController;
class StartAudioController;
class Playback;
class ISoundFontController;
class AudioDriverController;
class AudioModule : public modularity::IModuleSetup, public async::Asyncable
{
public:
    AudioModule();

    std::string moduleName() const override;

    void registerExports() override;
    void registerResources() override;
    void registerUiTypes() override;
    void resolveImports() override;
    void onInit(const IApplication::RunMode& mode) override;
    void onDeinit() override;

private:
    std::shared_ptr<AudioConfiguration> m_configuration;
    std::shared_ptr<AudioActionsController> m_actionsController;
    std::shared_ptr<StartAudioController> m_startAudioController;
    std::shared_ptr<Playback> m_mainPlayback;
    std::shared_ptr<ISoundFontController> m_soundFontController;

    Ticker m_rpcTicker;
    std::shared_ptr<rpc::IRpcChannel> m_rpcChannel;

    std::shared_ptr<AudioDriverController> m_audioDriverController;
};
}

#endif // MUSE_AUDIO_AUDIOMODULE_H
