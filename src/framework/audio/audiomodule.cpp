//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "audiomodule.h"

#include <QQmlEngine>
#include "modularity/ioc.h"
#include "internal/worker/audioengine.h"
#include "internal/audioconfiguration.h"
#include "ui/iuiengine.h"
#include "devtools/audioenginedevtools.h"

#include "internal/rpc/queuedrpcchannel.h"
#include "internal/rpc/rpccontrollers.h"
#include "internal/rpc/rpcaudioenginecontroller.h"
#include "internal/rpc/rpcsequencer.h"
#include "internal/rpc/rpcsequencercontroller.h"
#include "internal/rpc/rpcdevtoolscontroller.h"

#include "internal/audiosanitizer.h"
#include "internal/audiothread.h"
#include "internal/audiobuffer.h"

// synthesizers
#include "internal/synthesizers/fluidsynth/fluidsynth.h"
#include "internal/synthesizers/zerberus/zerberussynth.h"
#include "internal/synthesizers/soundfontsprovider.h"
#include "internal/synthesizers/synthesizercontroller.h"
#include "internal/synthesizers/synthesizersregister.h"
#include "view/synthssettingsmodel.h"

#include "log.h"

using namespace mu::framework;
using namespace mu::audio;

static std::shared_ptr<AudioConfiguration> s_audioConfiguration = std::make_shared<AudioConfiguration>();
static std::shared_ptr<AudioThread> s_audioWorker = std::make_shared<AudioThread>();
static std::shared_ptr<mu::audio::AudioBuffer> s_audioBuffer = std::make_shared<mu::audio::AudioBuffer>();

static std::shared_ptr<rpc::RpcControllers> s_rpcControllers = std::make_shared<rpc::RpcControllers>();
static std::shared_ptr<rpc::RpcSequencer> s_rpcSequencer = std::make_shared<rpc::RpcSequencer>();

#ifdef Q_OS_LINUX
#include "internal/platform/lin/linuxaudiodriver.h"
static std::shared_ptr<IAudioDriver> s_audioDriver = std::shared_ptr<IAudioDriver>(new LinuxAudioDriver());
#endif

#ifdef Q_OS_WIN
//#include "internal/platform/win/winmmdriver.h"
//static std::shared_ptr<IAudioDriver> s_audioDriver = std::shared_ptr<IAudioDriver>(new WinmmDriver());
#include "internal/platform/win/wincoreaudiodriver.h"
static std::shared_ptr<IAudioDriver> s_audioDriver = std::shared_ptr<IAudioDriver>(new CoreAudioDriver());
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
    ioc()->registerExport<IAudioConfiguration>(moduleName(), s_audioConfiguration);
    ioc()->registerExport<IAudioDriver>(moduleName(), s_audioDriver);
    ioc()->registerExport<ISequencer>(moduleName(), s_rpcSequencer);

    // synthesizers
    std::shared_ptr<synth::ISynthesizersRegister> sreg = std::make_shared<synth::SynthesizersRegister>();
    sreg->registerSynthesizer("Zerberus", std::make_shared<synth::ZerberusSynth>());
    sreg->registerSynthesizer("Fluid", std::make_shared<synth::FluidSynth>());
    sreg->setDefaultSynthesizer("Fluid");

    ioc()->registerExport<synth::ISynthesizersRegister>(moduleName(), sreg);
    ioc()->registerExport<synth::ISoundFontsProvider>(moduleName(), new synth::SoundFontsProvider());

    //! TODO maybe need remove
    ioc()->registerExport<rpc::IRpcChannel>(moduleName(), s_audioWorker->channel());
}

void AudioModule::registerResources()
{
    audio_init_qrc();
}

void AudioModule::registerUiTypes()
{
    qmlRegisterType<AudioEngineDevTools>("MuseScore.Audio", 1, 0, "AudioEngineDevTools");
    qmlRegisterType<synth::SynthsSettingsModel>("MuseScore.Audio", 1, 0, "SynthsSettingsModel");

    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(audio_QML_IMPORT);
}

void AudioModule::onInit(const framework::IApplication::RunMode&)
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
            * Rpc (remote call procedure) channel - controls and pass midi data
            * AudioBuffer - pass audio data from worker to driver for play

        AudioEngine is in the worker and operates only with the buffer,
        in fact, it knows nothing about the data consumer, about the audio driver.

    **/

    // Init configuration
    s_audioConfiguration->init();

    // Setup rpc system and worker
    s_rpcSequencer->setup();
    s_audioWorker->channel()->setupMainThread();
    s_audioWorker->setAudioBuffer(s_audioBuffer);
    s_audioWorker->run([]() {
        AudioSanitizer::setupWorkerThread();
        ONLY_AUDIO_WORKER_THREAD;

        AudioEngine::instance()->setAudioBuffer(s_audioBuffer);
        AudioEngine::instance()->init();

        s_rpcControllers->reg(std::make_shared<rpc::RpcAudioEngineController>());
        s_rpcControllers->reg(std::make_shared<rpc::RpcSequencerController>());
        s_rpcControllers->reg(std::make_shared<rpc::RpcDevToolsController>());
        s_rpcControllers->init(s_audioWorker->channel());
    });

    // Setup audio driver
    IAudioDriver::Spec requiredSpec;
    requiredSpec.sampleRate = 48000;
    requiredSpec.format = IAudioDriver::Format::AudioF32;
    requiredSpec.channels = 2; // stereo
    requiredSpec.samples = s_audioConfiguration->driverBufferSize();
    requiredSpec.callback = [](void* /*userdata*/, uint8_t* stream, int byteCount) {
        auto samples = byteCount / (2 * sizeof(float));
        s_audioBuffer->pop(reinterpret_cast<float*>(stream), samples);
    };

    IAudioDriver::Spec activeSpec;
    bool driverOpened = s_audioDriver->open(requiredSpec, &activeSpec);
    if (!driverOpened) {
        LOGE() << "audio output open failed";
        return;
    }

    // Setup audio engine
    //! NOTE Send msg for audio engine to worker
    s_audioWorker->channel()->send(
        rpc::Msg(
            rpc::TargetName::AudioEngine,
            "onDriverOpened",
            rpc::Args::make_arg2<int, uint16_t>(activeSpec.sampleRate, activeSpec.samples)
            ));
}

void AudioModule::onDeinit()
{
    s_audioDriver->close();
    s_audioWorker->stop([]() {
        ONLY_AUDIO_WORKER_THREAD;
        s_rpcControllers->deinit();
        AudioEngine::instance()->deinit();
    });
}
