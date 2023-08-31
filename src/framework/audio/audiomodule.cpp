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

#include <QQmlEngine>

#include "ui/iuiengine.h"
#include "modularity/ioc.h"
#include "log.h"

#include "internal/audioconfiguration.h"
#include "internal/audiosanitizer.h"
#include "internal/audiothread.h"
#include "internal/audiobuffer.h"
#include "internal/audiothreadsecurer.h"
#include "internal/audiooutputdevicecontroller.h"

#include "internal/plugins/knownaudiopluginsregister.h"
#include "internal/plugins/audiopluginsscannerregister.h"
#include "internal/plugins/audiopluginmetareaderregister.h"
#include "internal/plugins/registeraudiopluginsscenario.h"

#include "internal/worker/audioengine.h"
#include "internal/worker/playback.h"

#include "internal/soundfontrepository.h"

// synthesizers
#include "internal/synthesizers/fluidsynth/fluidresolver.h"
#include "internal/synthesizers/synthresolver.h"

#include "internal/fx/fxresolver.h"
#include "internal/fx/musefxresolver.h"

#include "diagnostics/idiagnosticspathsregister.h"

#include "log.h"

using namespace mu::modularity;
using namespace mu::audio;
using namespace mu::audio::synth;
using namespace mu::audio::fx;

#ifdef JACK_AUDIO
#include "internal/platform/jack/jackaudiodriver.h"
#endif

#ifdef Q_OS_LINUX
#include "internal/platform/lin/linuxaudiodriver.h"
#endif

#ifdef Q_OS_FREEBSD
#include "internal/platform/lin/linuxaudiodriver.h"
#endif
#ifdef Q_OS_WIN
//#include "internal/platform/win/winmmdriver.h"
//#include "internal/platform/win/wincoreaudiodriver.h"
#include "internal/platform/win/wasapiaudiodriver.h"
#endif

#ifdef Q_OS_MACOS
#include "internal/platform/osx/osxaudiodriver.h"
#endif

#ifdef Q_OS_WASM
#include "internal/platform/web/webaudiodriver.h"
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
    return "audio_engine";
}

void AudioModule::registerExports()
{
    m_configuration = std::make_shared<AudioConfiguration>();
    m_audioWorker = std::make_shared<AudioThread>();
    m_audioBuffer = std::make_shared<AudioBuffer>();
    m_audioOutputController = std::make_shared<AudioOutputDeviceController>();
    m_fxResolver = std::make_shared<FxResolver>();
    m_synthResolver = std::make_shared<SynthResolver>();
    m_playbackFacade = std::make_shared<Playback>();
    m_soundFontRepository = std::make_shared<SoundFontRepository>();
    m_registerAudioPluginsScenario = std::make_shared<RegisterAudioPluginsScenario>();

#if defined(JACK_AUDIO)
    m_audioDriver = std::shared_ptr<IAudioDriver>(new JackAudioDriver());
#else

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    m_audioDriver = std::shared_ptr<IAudioDriver>(new LinuxAudioDriver());
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

#endif // JACK_AUDIO

    ioc()->registerExport<IAudioConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IAudioThreadSecurer>(moduleName(), std::make_shared<AudioThreadSecurer>());
    ioc()->registerExport<IAudioDriver>(moduleName(), m_audioDriver);
    ioc()->registerExport<IPlayback>(moduleName(), m_playbackFacade);

    ioc()->registerExport<ISynthResolver>(moduleName(), m_synthResolver);
    ioc()->registerExport<IFxResolver>(moduleName(), m_fxResolver);

    ioc()->registerExport<ISoundFontRepository>(moduleName(), m_soundFontRepository);

    ioc()->registerExport<IKnownAudioPluginsRegister>(moduleName(), std::make_shared<KnownAudioPluginsRegister>());
    ioc()->registerExport<IAudioPluginsScannerRegister>(moduleName(), std::make_shared<AudioPluginsScannerRegister>());
    ioc()->registerExport<IAudioPluginMetaReaderRegister>(moduleName(), std::make_shared<AudioPluginMetaReaderRegister>());
    ioc()->registerExport<IRegisterAudioPluginsScenario>(moduleName(), m_registerAudioPluginsScenario);
}

void AudioModule::registerResources()
{
    audio_init_qrc();
}

void AudioModule::registerUiTypes()
{
    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(audio_QML_IMPORT);
}

void AudioModule::resolveImports()
{
    m_fxResolver->registerResolver(AudioFxType::MuseFx, std::make_shared<MuseFxResolver>());
}

void AudioModule::onInit(const framework::IApplication::RunMode& mode)
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
    m_registerAudioPluginsScenario->init();

    if (mode == framework::IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_soundFontRepository->init();

    m_audioBuffer->init(m_configuration->audioChannelsCount(),
                        m_configuration->renderStep());

    m_audioOutputController->init();

    // Setup audio driver
    setupAudioDriver(mode);

    //! --- Diagnostics ---
    auto pr = ioc()->resolve<diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        std::vector<io::path_t> paths = m_configuration->soundFontDirectories();
        for (const io::path_t& p : paths) {
            pr->reg("soundfonts", p);
        }
        pr->reg("known_audio_plugins", m_configuration->knownAudioPluginsFilePath());
    }
}

void AudioModule::onDelayedInit()
{
    Ret ret = m_registerAudioPluginsScenario->registerNewPlugins();
    if (!ret) {
        LOGE() << ret.toString();
    }
}

void AudioModule::onDeinit()
{
    if (m_audioDriver->isOpened()) {
        m_audioDriver->close();
    }
}

void AudioModule::onDestroy()
{
    //! NOTE During deinitialization, objects that process events are destroyed,
    //! it is better to destroy them on onDestroy, when no events should come anymore
    if (m_audioWorker->isRunning()) {
        m_audioWorker->stop([this]() {
            ONLY_AUDIO_WORKER_THREAD;
            m_playbackFacade->deinit();
            AudioEngine::instance()->deinit();
        });
    }
}

void AudioModule::setupAudioDriver(const framework::IApplication::RunMode& mode)
{
    IAudioDriver::Spec requiredSpec;
    requiredSpec.sampleRate = m_configuration->sampleRate();
    requiredSpec.format = IAudioDriver::Format::AudioF32;
    requiredSpec.channels = m_configuration->audioChannelsCount();
    requiredSpec.samples = m_configuration->driverBufferSize();
    requiredSpec.callback = [this](void* /*userdata*/, uint8_t* stream, int byteCount) {
        auto samplesPerChannel = byteCount / (2 * sizeof(float));
        m_audioBuffer->pop(reinterpret_cast<float*>(stream), samplesPerChannel);
    };

    if (mode == framework::IApplication::RunMode::GuiApp) {
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
    auto workerSetup = [this, activeSpec]() {
        AudioSanitizer::setupWorkerThread();
        ONLY_AUDIO_WORKER_THREAD;

        // Setup audio engine
        AudioEngine::instance()->init(m_audioBuffer);
        AudioEngine::instance()->setAudioChannelsCount(activeSpec.channels);
        AudioEngine::instance()->setSampleRate(activeSpec.sampleRate);
        AudioEngine::instance()->setReadBufferSize(activeSpec.samples);

        auto fluidResolver = std::make_shared<FluidResolver>();
        m_synthResolver->registerResolver(AudioSourceType::Fluid, fluidResolver);
        m_synthResolver->init(m_configuration->defaultAudioInputParams());

        // Initialize IPlayback facade and make sure that it's initialized after the audio-engine
        m_playbackFacade->init();
    };

    auto workerLoopBody = [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        m_audioBuffer->forward();
    };

    m_audioWorker->run(workerSetup, workerLoopBody);
}
