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

#include "global/async/processevents.h"

#include "internal/audioconfiguration.h"
#include "audio/common/audiosanitizer.h"
#include "audio/common/audiothreadsecurer.h"
#include "internal/audiooutputdevicecontroller.h"

#include "audio/common/rpc/platform/general/generalrpcchannel.h"
#include "internal/playback.h"
#include "internal/soundfontcontroller.h"

#include "diagnostics/idiagnosticspathsregister.h"
#include "devtools/inputlag.h"

// Temporarily for compatibility
#include "audio/worker/audioworkermodule.h"
#include "audio/worker/internal/audioengine.h"
#include "audio/worker/internal/audiobuffer.h"
#include "audio/worker/internal/synthesizers/synthresolver.h"
#include "audio/worker/internal/audiothread.h"

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

static void measureInputLag(const float* buf, const size_t size)
{
    if (INPUT_LAG_TIMER_STARTED) {
        for (size_t i = 0; i < size; ++i) {
            if (!RealIsNull(buf[i])) {
                STOP_INPUT_LAG_TIMER;
                return;
            }
        }
    }
}

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
    m_audioOutputController = std::make_shared<AudioOutputDeviceController>(iocContext());
    m_mainPlayback = std::make_shared<Playback>(iocContext());
    m_rpcChannel = std::make_shared<rpc::GeneralRpcChannel>();
    m_soundFontController = std::make_shared<SoundFontController>();

    //! NOTE Temporarily for compatibility
    m_workerModule = std::make_shared<worker::AudioWorkerModule>();
    m_audioThread = std::make_shared<worker::AudioThread>();

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
    ioc()->registerExport<IAudioThreadSecurer>(moduleName(), std::make_shared<AudioThreadSecurer>());
    ioc()->registerExport<IAudioDriver>(moduleName(), m_audioDriver);
    ioc()->registerExport<IPlayback>(moduleName(), m_mainPlayback);
    ioc()->registerExport<rpc::IRpcChannel>(moduleName(), m_rpcChannel);
    ioc()->registerExport<ISoundFontController>(moduleName(), m_soundFontController);

    m_workerModule->registerExports();
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
    m_workerModule->resolveImports();
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

    m_workerModule->audioBuffer()->init(m_configuration->audioChannelsCount());

    m_audioOutputController->init();

    m_soundFontController->init();

    // rpc
    m_rpcTimer.setInterval(16); // corresponding to 60 fps
    QObject::connect(&m_rpcTimer, &QTimer::timeout, [this]() {
        m_rpcChannel->process();
    });
    m_rpcTimer.start();

    m_mainPlayback->init();

    // Setup audio driver
    setupAudioDriver(mode);

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

    if (m_audioDriver->isOpened()) {
        m_audioDriver->close();
    }
}

void AudioModule::onDestroy()
{
    //! NOTE During deinitialization, objects that process events are destroyed,
    //! it is better to destroy them on onDestroy, when no events should come anymore
    if (m_audioThread->isRunning()) {
        m_audioThread->stop([this]() {
            ONLY_AUDIO_WORKER_THREAD;
            m_workerModule->onDestroy();
        });
    }
}

void AudioModule::setupAudioDriver(const IApplication::RunMode& mode)
{
    IAudioDriver::Spec requiredSpec;
    requiredSpec.sampleRate = m_configuration->sampleRate();
    requiredSpec.format = IAudioDriver::Format::AudioF32;
    requiredSpec.channels = m_configuration->audioChannelsCount();
    requiredSpec.samples = m_configuration->driverBufferSize();

    if (m_configuration->shouldMeasureInputLag()) {
        requiredSpec.callback = [this](void* /*userdata*/, uint8_t* stream, int byteCount) {
            auto samplesPerChannel = byteCount / (2 * sizeof(float));  // 2 == m_configuration->audioChannelsCount()
            float* dest = reinterpret_cast<float*>(stream);
            const std::shared_ptr<worker::AudioBuffer>& audioBuffer = m_workerModule->audioBuffer();
            audioBuffer->pop(dest, samplesPerChannel);
            measureInputLag(dest, samplesPerChannel * audioBuffer->audioChannelCount());
        };
    } else {
        requiredSpec.callback = [this](void* /*userdata*/, uint8_t* stream, int byteCount) {
            auto samplesPerChannel = byteCount / (2 * sizeof(float));
            const std::shared_ptr<worker::AudioBuffer>& audioBuffer = m_workerModule->audioBuffer();
            audioBuffer->pop(reinterpret_cast<float*>(stream), samplesPerChannel);
        };
    }

    if (mode == IApplication::RunMode::GuiApp) {
        m_audioDriver->init();

        IAudioDriver::Spec activeSpec;
        if (m_audioDriver->open(requiredSpec, &activeSpec)) {
            setupAudioWorker(activeSpec);
            return;
        }

        LOGE() << "audio output open failed";
    }

    setupAudioWorker(requiredSpec);
}

void AudioModule::setupAudioWorker(const IAudioDriver::Spec& activeSpec)
{
    worker::AudioEngine::RenderConstraints consts;
    consts.minSamplesToReserveWhenIdle = m_configuration->minSamplesToReserve(RenderMode::IdleMode);
    consts.minSamplesToReserveInRealtime = m_configuration->minSamplesToReserve(RenderMode::RealTimeMode);
    consts.desiredAudioThreadNumber = m_configuration->desiredAudioThreadNumber();
    consts.minTrackCountForMultithreading = m_configuration->minTrackCountForMultithreading();

    auto workerSetup = [this, activeSpec, consts]() {
        AudioSanitizer::setupWorkerThread();
        ONLY_AUDIO_WORKER_THREAD;

        //! NOTE It should be as early as possible
        m_rpcChannel->initOnWorker();

        m_workerModule->onPreInit(IApplication::RunMode::GuiApp);

        // Setup audio engine
        std::shared_ptr<worker::AudioBuffer> audioBuffer = m_workerModule->audioBuffer();
        std::shared_ptr<worker::AudioEngine> audioEngine = m_workerModule->audioEngine();
        audioEngine->init(audioBuffer, consts);
        audioEngine->setAudioChannelsCount(m_configuration->audioChannelsCount());
        audioEngine->setSampleRate(activeSpec.sampleRate);
        audioEngine->setReadBufferSize(activeSpec.samples);

        audioEngine->setOnReadBufferChanged([this](const samples_t samples, const sample_rate_t rate) {
            msecs_t interval = m_configuration->audioWorkerInterval(samples, rate);
            m_audioThread->setInterval(interval);
        });

        m_workerModule->synthResolver()->init(m_configuration->defaultAudioInputParams());

        m_workerModule->onInit(IApplication::RunMode::GuiApp);
    };

    auto workerLoopBody = [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        async::processEvents();
        m_rpcChannel->process();
        m_workerModule->audioBuffer()->forward();
    };

    msecs_t interval = m_configuration->audioWorkerInterval(activeSpec.samples, activeSpec.sampleRate);
    m_audioThread->run(workerSetup, workerLoopBody, interval);
}
