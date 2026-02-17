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
#include "audiomodule.h"

#include "ui/iuiactionsregister.h"
#include "global/modularity/ioc.h"

#include "audio/common/audiosanitizer.h"
#include "audio/common/audiothreadsecurer.h"
#ifdef Q_OS_WASM
#include "audio/common/rpc/platform/web/webrpcchannel.h"
#include "platform/web/websoundfontcontroller.h"
#else
#include "audio/common/rpc/platform/general/generalrpcchannel.h"
#include "platform/general/generalsoundfontcontroller.h"
#endif

#include "internal/audioconfiguration.h"
#include "internal/audioactionscontroller.h"
#include "internal/audiouiactions.h"
#include "internal/startaudiocontroller.h"
#include "internal/playback.h"
#include "internal/audiodrivercontroller.h"

#include "diagnostics/idiagnosticspathsregister.h"

#include "log.h"

using namespace muse;
using namespace muse::modularity;
using namespace muse::audio;

AudioModule::AudioModule()
{
    AudioSanitizer::setupMainThread();
}

static const std::string mname("audio");

std::string AudioModule::moduleName() const
{
    return mname;
}

void AudioModule::registerExports()
{
    m_configuration = std::make_shared<AudioConfiguration>(globalCtx());

#ifdef Q_OS_WASM
    m_rpcChannel = std::make_shared<rpc::WebRpcChannel>();
#else
    m_rpcChannel = std::make_shared<rpc::GeneralRpcChannel>();
#endif

    m_audioDriverController = std::make_shared<AudioDriverController>(globalCtx());

#ifdef Q_OS_WASM
    m_soundFontController = std::make_shared<WebSoundFontController>();
#else
    m_soundFontController = std::make_shared<GeneralSoundFontController>(globalCtx());
#endif

    m_startAudioController = std::make_shared<StartAudioController>(m_rpcChannel, globalCtx());

    globalIoc()->registerExport<IAudioConfiguration>(mname, m_configuration);
    globalIoc()->registerExport<IAudioThreadSecurer>(mname, std::make_shared<AudioThreadSecurer>());
    globalIoc()->registerExport<IAudioDriverController>(mname, m_audioDriverController);
    globalIoc()->registerExport<ISoundFontController>(mname, m_soundFontController);
    globalIoc()->registerExport<IStartAudioController>(mname, m_startAudioController);
    globalIoc()->registerExport<rpc::IRpcChannel>(mname, m_rpcChannel);

    m_startAudioController->registerExports();
}

void AudioModule::onInit(const IApplication::RunMode& mode)
{
    m_configuration->init();

    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    // rpc
    m_rpcChannel->setupOnMain();
#ifndef Q_OS_WASM
    m_rpcTicker.start(1, [this]() {
        m_rpcChannel->process();
    }, Ticker::Mode::Repeat);
#endif

    m_startAudioController->init();

#ifndef Q_OS_WASM
    m_startAudioController->startAudioProcessing(mode);
#endif

    m_audioInited = true;
}

void AudioModule::onDeinit()
{
    if (!m_audioInited) {
        return;
    }

    m_rpcTicker.stop();
    m_startAudioController->stopAudioProcessing();
}

modularity::IContextSetup* AudioModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new AudioContext(ctx);
}

// Context

void AudioContext::registerExports()
{
    m_actionsController = std::make_shared<AudioActionsController>(iocContext());
    m_mainPlayback = std::make_shared<Playback>(iocContext());
    ioc()->registerExport<IPlayback>(mname, m_mainPlayback);

#ifdef MUSE_MULTICONTEXT_WIP
    // Forward global services to context
    auto audioDriverController = globalIoc()->resolve<IAudioDriverController>(mname);
    ioc()->registerExport<IAudioDriverController>(mname, audioDriverController);

    auto soundFontController = globalIoc()->resolve<ISoundFontController>(mname);
    ioc()->registerExport<ISoundFontController>(mname, soundFontController);

    auto startAudioController = globalIoc()->resolve<IStartAudioController>(mname);
    ioc()->registerExport<IStartAudioController>(mname, startAudioController);

    auto rpcChannel = globalIoc()->resolve<rpc::IRpcChannel>(mname);
    ioc()->registerExport<rpc::IRpcChannel>(mname, rpcChannel);
#endif
}

void AudioContext::resolveImports()
{
    auto ar = ioc()->resolve<ui::IUiActionsRegister>(mname);
    if (ar) {
        ar->reg(std::make_shared<AudioUiActions>(m_actionsController));
    }
}

void AudioContext::onInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_actionsController->init();
    m_mainPlayback->init();

    //! --- Diagnostics ---
    auto pr = ioc()->resolve<muse::diagnostics::IDiagnosticsPathsRegister>(mname);
    if (pr) {
        auto configuration = globalIoc()->resolve<IAudioConfiguration>(mname);
        std::vector<io::path_t> paths = configuration->soundFontDirectories();
        for (const io::path_t& p : paths) {
            pr->reg("soundfonts", p);
        }
    }
}

void AudioContext::onDeinit()
{
    m_mainPlayback->deinit();
}
