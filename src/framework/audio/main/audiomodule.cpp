/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "global/modularity/ioc.h"

#include "audio/common/audiosanitizer.h"
#include "audio/common/audiothreadsecurer.h"
#ifdef Q_OS_WASM
#include "audio/common/rpc/platform/web/webrpcchannel.h"
#include "audio/worker/platform/web/webaudioworker.h"
#else
#include "audio/common/rpc/platform/general/generalrpcchannel.h"
#include "audio/worker/platform/general/generalaudioworker.h"
#endif

#include "internal/audioconfiguration.h"
#include "internal/startaudiocontroller.h"
#include "internal/playback.h"
#include "internal/soundfontcontroller.h"
#include "internal/audiooutputdevicecontroller.h"

#include "diagnostics/idiagnosticspathsregister.h"

#include "log.h"

using namespace muse;
using namespace muse::modularity;
using namespace muse::audio;

#ifdef MUSE_MODULE_AUDIO_JACK
#include "audio/driver/platform/jack/jackaudiodriver.h"
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
#include <QtEnvironmentVariables>
#include "audio/driver/platform/lin/alsaaudiodriver.h"
#ifdef MUSE_PIPEWIRE_AUDIO_DRIVER
#include "audio/driver/platform/lin/pwaudiodriver.h"
#endif
#endif

#ifdef Q_OS_WIN
//#include "audio/driver/platform/win/winmmdriver.h"
//#include "audio/driver/platform/win/wincoreaudiodriver.h"
#include "audio/driver/platform/win/wasapiaudiodriver.h"
#endif

#ifdef Q_OS_MACOS
#include "audio/driver/platform/osx/osxaudiodriver.h"
#endif

#ifdef Q_OS_WASM
#include "audio/driver/platform/web/webaudiodriver.h"
#endif

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

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
std::shared_ptr<IAudioDriver> makeLinuxAudioDriver()
{
#if defined(MUSE_PIPEWIRE_AUDIO_DRIVER)
    if (!qEnvironmentVariableIsSet("MUSESCORE_FORCE_ALSA")) {
        auto driver = std::make_shared<PwAudioDriver>();
        if (driver->connectedToPwServer()) {
            LOGI() << "Using audio driver: Pipewire";
            return driver;
        }
    }
#endif // MUSE_PIPEWIRE_AUDIO_DRIVER
    LOGI() << "Using audio driver: ALSA";
    return std::make_shared<AlsaAudioDriver>();
}

#endif // Q_OS_LINUX || Q_OS_FREEBSD

void AudioModule::registerExports()
{
    m_configuration = std::make_shared<AudioConfiguration>(iocContext());
    m_startAudioController = std::make_shared<StartAudioController>();
    m_audioOutputController = std::make_shared<AudioOutputDeviceController>(iocContext());
    m_mainPlayback = std::make_shared<Playback>(iocContext());

    m_soundFontController = std::make_shared<SoundFontController>();

#ifdef Q_OS_WASM
    m_rpcChannel = std::make_shared<rpc::WebRpcChannel>();
    m_audioWorker = std::make_shared<worker::WebAudioWorker>(m_rpcChannel);
#else
    m_rpcChannel = std::make_shared<rpc::GeneralRpcChannel>();
    m_audioWorker = std::make_shared<worker::GeneralAudioWorker>(m_rpcChannel);
    m_audioWorker->registerExports();
#endif

#if defined(MUSE_MODULE_AUDIO_JACK)
    m_audioDriver = std::shared_ptr<IAudioDriver>(new JackAudioDriver());
#else

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    m_audioDriver = makeLinuxAudioDriver();
#endif

#ifdef Q_OS_WIN
    //m_audioDriver = std::shared_ptr<IAudioDriver>(new WinmmDriver());
    //m_audioDriver = std::shared_ptr<IAudioDriver>(new CoreAudioDriver());
    m_audioDriver = std::shared_ptr<IAudioDriver>(new WasapiAudioDriver());
#endif

#ifdef Q_OS_MACOS
    m_audioDriver = std::shared_ptr<IAudioDriver>(new OSXAudioDriver());
#endif

#ifdef Q_OS_WASM
    m_audioDriver = std::shared_ptr<IAudioDriver>(new WebAudioDriver());
#endif

#endif // MUSE_MODULE_AUDIO_JACK

    ioc()->registerExport<IAudioConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IStartAudioController>(moduleName(), m_startAudioController);
    ioc()->registerExport<IAudioThreadSecurer>(moduleName(), std::make_shared<AudioThreadSecurer>());
    ioc()->registerExport<rpc::IRpcChannel>(moduleName(), m_rpcChannel);
    ioc()->registerExport<worker::IAudioWorker>(moduleName(), m_audioWorker);
    ioc()->registerExport<IAudioDriver>(moduleName(), m_audioDriver);
    ioc()->registerExport<ISoundFontController>(moduleName(), m_soundFontController);
    ioc()->registerExport<IPlayback>(moduleName(), m_mainPlayback);
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
}

void AudioModule::onInit(const IApplication::RunMode& mode)
{
    /** We have three layers
        ------------------------
        Main (main thread) - public client interface
            see registerExports
        ------------------------
        Worker (worker thread) - generate and mix audio data
            * AudioEngine
            * Sequencer
            * Players
            * Synthesizers
            * Audio decode (.ogg ...)
            * Mixer
        ------------------------
        Driver (driver thread) - request audio data to play
        ------------------------

        All layers work in separate threads.
        We need to make sure that each part of the system works only in its thread and,
        ideally, there is no access to the same object from different threads,
        in order to avoid problems associated with access data thread safety.

        Objects from different layers (threads) must interact only through:
            * Asynchronous API (@see thirdparty/deto) - controls and pass midi data
            * AudioBuffer - pass audio data from worker to driver for play

        AudioEngine is in the worker and operates only with the buffer,
        in fact, it knows nothing about the data consumer, about the audio driver.

    **/

    // Init configuration
    m_configuration->init();

    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    // rpc
    m_rpcChannel->setupOnMain();
#ifndef Q_OS_WASM
    m_rpcTimer.setInterval(16); // corresponding to 60 fps
    QObject::connect(&m_rpcTimer, &QTimer::timeout, [this]() {
        m_rpcChannel->process();
    });
    m_rpcTimer.start();
#endif

    m_audioOutputController->init();
    m_soundFontController->init();
    m_mainPlayback->init();

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
    m_rpcTimer.stop();

    m_startAudioController->stopAudioProcessing();
}
