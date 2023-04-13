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

static std::shared_ptr<AudioConfiguration> s_audioConfiguration = {};
static std::shared_ptr<AudioThread> s_audioWorker = {};
static std::shared_ptr<AudioBuffer> s_audioBuffer = {};
static std::shared_ptr<AudioOutputDeviceController> s_audioOutputController = {};

static std::shared_ptr<FxResolver> s_fxResolver = {};
static std::shared_ptr<SynthResolver> s_synthResolver = {};

static std::shared_ptr<Playback> s_playbackFacade = {};

static std::shared_ptr<SoundFontRepository> s_soundFontRepository = {};

static std::shared_ptr<KnownAudioPluginsRegister> s_knownAudioPluginsRegister = {};
static std::shared_ptr<RegisterAudioPluginsScenario> s_registerAudioPluginsScenario = {};

#ifdef Q_OS_LINUX
#include "internal/platform/lin/linuxaudiodriver.h"
static std::shared_ptr<IAudioDriver> s_audioDriver = std::shared_ptr<IAudioDriver>(new LinuxAudioDriver());
#endif

#ifdef Q_OS_WIN
//#include "internal/platform/win/winmmdriver.h"
//static std::shared_ptr<IAudioDriver> s_audioDriver = std::shared_ptr<IAudioDriver>(new WinmmDriver());
//#include "internal/platform/win/wincoreaudiodriver.h"
//static std::shared_ptr<IAudioDriver> s_audioDriver = std::shared_ptr<IAudioDriver>(new CoreAudioDriver());
#include "internal/platform/win/wasapiaudiodriver.h"
static std::shared_ptr<IAudioDriver> s_audioDriver = std::shared_ptr<IAudioDriver>(new WasapiAudioDriver());
#endif

#ifdef Q_OS_MACOS
#include "internal/platform/osx/osxaudiodriver.h"
static std::shared_ptr<IAudioDriver> s_audioDriver = std::shared_ptr<IAudioDriver>(new OSXAudioDriver());
#endif

#ifdef Q_OS_WASM
#include "internal/platform/web/webaudiodriver.h"
static std::shared_ptr<IAudioDriver> s_audioDriver = std::shared_ptr<IAudioDriver>(new WebAudioDriver());
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
    s_audioConfiguration = std::make_shared<AudioConfiguration>();
    s_audioWorker = std::make_shared<AudioThread>();
    s_audioBuffer = std::make_shared<AudioBuffer>();
    s_audioOutputController = std::make_shared<AudioOutputDeviceController>();
    s_fxResolver = std::make_shared<FxResolver>();
    s_synthResolver = std::make_shared<SynthResolver>();
    s_playbackFacade = std::make_shared<Playback>();
    s_soundFontRepository = std::make_shared<SoundFontRepository>();
    s_registerAudioPluginsScenario = std::make_shared<RegisterAudioPluginsScenario>();

    ioc()->registerExport<IAudioConfiguration>(moduleName(), s_audioConfiguration);
    ioc()->registerExport<IAudioThreadSecurer>(moduleName(), std::make_shared<AudioThreadSecurer>());
    ioc()->registerExport<IAudioDriver>(moduleName(), s_audioDriver);
    ioc()->registerExport<IPlayback>(moduleName(), s_playbackFacade);

    ioc()->registerExport<ISynthResolver>(moduleName(), s_synthResolver);
    ioc()->registerExport<IFxResolver>(moduleName(), s_fxResolver);

    ioc()->registerExport<ISoundFontRepository>(moduleName(), s_soundFontRepository);

    ioc()->registerExport<IKnownAudioPluginsRegister>(moduleName(), std::make_shared<KnownAudioPluginsRegister>());
    ioc()->registerExport<IAudioPluginsScannerRegister>(moduleName(), std::make_shared<AudioPluginsScannerRegister>());
    ioc()->registerExport<IAudioPluginMetaReaderRegister>(moduleName(), std::make_shared<AudioPluginMetaReaderRegister>());
    ioc()->registerExport<IRegisterAudioPluginsScenario>(moduleName(), s_registerAudioPluginsScenario);
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
    s_fxResolver->registerResolver(AudioFxType::MuseFx, std::make_shared<MuseFxResolver>());
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
    s_audioConfiguration->init();
    s_soundFontRepository->init();
    s_registerAudioPluginsScenario->init();

    s_audioBuffer->init(s_audioConfiguration->audioChannelsCount(),
                        s_audioConfiguration->renderStep());

    s_audioOutputController->init();

    // Setup audio driver
    setupAudioDriver(mode);

    //! --- Diagnostics ---
    auto pr = ioc()->resolve<diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        std::vector<io::path_t> paths = s_audioConfiguration->soundFontDirectories();
        for (const io::path_t& p : paths) {
            pr->reg("soundfonts", p);
        }
        pr->reg("audio_plugins", s_audioConfiguration->knownAudioPluginsDir());
    }
}

void AudioModule::onDelayedInit()
{
    Ret ret = s_registerAudioPluginsScenario->registerNewPlugins();
    if (!ret) {
        LOGE() << ret.toString();
    }
}

void AudioModule::onDeinit()
{
    if (s_audioDriver->isOpened()) {
        s_audioDriver->close();
    }
}

void AudioModule::onDestroy()
{
    //! NOTE During deinitialization, objects that process events are destroyed,
    //! it is better to destroy them on onDestroy, when no events should come anymore
    if (s_audioWorker->isRunning()) {
        s_audioWorker->stop([]() {
            ONLY_AUDIO_WORKER_THREAD;
            s_playbackFacade->deinit();
            AudioEngine::instance()->deinit();
        });
    }
}

void AudioModule::setupAudioDriver(const framework::IApplication::RunMode& mode)
{
    IAudioDriver::Spec requiredSpec;
    requiredSpec.sampleRate = s_audioConfiguration->sampleRate();
    requiredSpec.format = IAudioDriver::Format::AudioF32;
    requiredSpec.channels = s_audioConfiguration->audioChannelsCount();
    requiredSpec.samples = s_audioConfiguration->driverBufferSize();
    requiredSpec.callback = [](void* /*userdata*/, uint8_t* stream, int byteCount) {
        auto samplesPerChannel = byteCount / (2 * sizeof(float));
        s_audioBuffer->pop(reinterpret_cast<float*>(stream), samplesPerChannel);
    };

    if (mode == framework::IApplication::RunMode::GuiApp) {
        s_audioDriver->init();

        IAudioDriver::Spec activeSpec;
        if (s_audioDriver->open(requiredSpec, &activeSpec)) {
            setupAudioWorker(activeSpec);
            return;
        }

        LOGE() << "audio output open failed";
    }

    setupAudioWorker(requiredSpec);
}

void AudioModule::setupAudioWorker(const IAudioDriver::Spec& activeSpec)
{
    auto workerSetup = [activeSpec]() {
        AudioSanitizer::setupWorkerThread();
        ONLY_AUDIO_WORKER_THREAD;

        // Setup audio engine
        AudioEngine::instance()->init(s_audioBuffer);
        AudioEngine::instance()->setAudioChannelsCount(activeSpec.channels);
        AudioEngine::instance()->setSampleRate(activeSpec.sampleRate);
        AudioEngine::instance()->setReadBufferSize(activeSpec.samples);

        auto fluidResolver = std::make_shared<FluidResolver>();
        s_synthResolver->registerResolver(AudioSourceType::Fluid, fluidResolver);
        s_synthResolver->init(s_audioConfiguration->defaultAudioInputParams());

        // Initialize IPlayback facade and make sure that it's initialized after the audio-engine
        s_playbackFacade->init();
    };

    auto workerLoopBody = []() {
        ONLY_AUDIO_WORKER_THREAD;
        s_audioBuffer->forward();
    };

    s_audioWorker->run(workerSetup, workerLoopBody);
}
