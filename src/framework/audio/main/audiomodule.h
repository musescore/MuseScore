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

#pragma once

#include <memory>

#include "modularity/imodulesetup.h"
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
class AudioModule : public modularity::IModuleSetup
{
public:
    AudioModule();

    std::string moduleName() const override;

    void registerExports() override;

    void onInit(const IApplication::RunMode& mode) override;
    void onDeinit() override;

    modularity::IContextSetup* newContext(const muse::modularity::ContextPtr& ctx) const override;

private:
    std::shared_ptr<AudioConfiguration> m_configuration;
    std::shared_ptr<AudioDriverController> m_audioDriverController;
    std::shared_ptr<ISoundFontController> m_soundFontController;
    std::shared_ptr<StartAudioController> m_startAudioController;
    std::shared_ptr<rpc::IRpcChannel> m_rpcChannel;
    Ticker m_rpcTicker;
    bool m_audioInited = false;
};

class AudioContext : public modularity::IContextSetup
{
public:
    AudioContext(const muse::modularity::ContextPtr& ctx)
        : modularity::IContextSetup(ctx) {}

    void registerExports() override;
    void resolveImports() override;
    void onInit(const IApplication::RunMode& mode) override;
    void onDeinit() override;

private:
    std::shared_ptr<AudioActionsController> m_actionsController;
    std::shared_ptr<Playback> m_mainPlayback;
};
}
