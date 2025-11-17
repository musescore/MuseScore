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

#include "ui/iuiengine.h"
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

static void audio_init_qrc()
{
    Q_INIT_RESOURCE(audio);
}

AudioModule::AudioModule()
{
    AudioSanitizer::setupMainThread();
}

std::string AudioModule::moduleName() const
{
    return "audio";
}

void AudioModule::registerExports()
{
    m_configuration = std::make_shared<AudioConfiguration>(iocContext());
    m_actionsController = std::make_shared<AudioActionsController>();
    m_mainPlayback = std::make_shared<Playback>(iocContext());
    m_audioDriverController = std::make_shared<AudioDriverController>(iocContext());

#ifdef Q_OS_WASM
    m_rpcChannel = std::make_shared<rpc::WebRpcChannel>();
    m_soundFontController = std::make_shared<WebSoundFontController>();
#else
    m_rpcChannel = std::make_shared<rpc::GeneralRpcChannel>();
    m_soundFontController = std::make_shared<GeneralSoundFontController>();
#endif

    m_startAudioController = std::make_shared<StartAudioController>(m_rpcChannel);

    ioc()->registerExport<IAudioConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IStartAudioController>(moduleName(), m_startAudioController);
    ioc()->registerExport<IAudioThreadSecurer>(moduleName(), std::make_shared<AudioThreadSecurer>());
    ioc()->registerExport<rpc::IRpcChannel>(moduleName(), m_rpcChannel);
    ioc()->registerExport<IAudioDriverController>(moduleName(), m_audioDriverController);
    ioc()->registerExport<ISoundFontController>(moduleName(), m_soundFontController);
    ioc()->registerExport<IPlayback>(moduleName(), m_mainPlayback);

    m_startAudioController->registerExports();
}

void AudioModule::registerResources()
{
    audio_init_qrc();
}

void AudioModule::registerUiTypes()
{
    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(muse_audio_QML_IMPORT);
}

void AudioModule::resolveImports()
{
    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<AudioUiActions>(m_actionsController));
    }
}

void AudioModule::onInit(const IApplication::RunMode& mode)
{
    m_configuration->init();

    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_actionsController->init();

    // rpc
    m_rpcChannel->setupOnMain();
#ifndef Q_OS_WASM
    m_rpcTicker.start(1, [this]() {
        m_rpcChannel->process();
    }, Ticker::Mode::Repeat);
#endif

    m_mainPlayback->init();

    m_startAudioController->init();

#ifndef Q_OS_WASM
    m_startAudioController->startAudioProcessing(mode);
#endif

    //! --- Diagnostics ---
    auto pr = ioc()->resolve<muse::diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        std::vector<io::path_t> paths = m_configuration->soundFontDirectories();
        for (const io::path_t& p : paths) {
            pr->reg("soundfonts", p);
        }
    }
}

void AudioModule::onDeinit()
{
    m_mainPlayback->deinit();
    m_rpcTicker.stop();

    m_startAudioController->stopAudioProcessing();
}
