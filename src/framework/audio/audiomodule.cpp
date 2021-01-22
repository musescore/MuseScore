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
#include "internal/audioengine.h"
#include "internal/audioconfiguration.h"
#include "ui/iuiengine.h"
#include "devtools/audioenginedevtools.h"

#include "internal/rpc/queuedrpcchannel.h"
#include "internal/rpc/rpcsequencer.h"
#include "internal/audiosanitizer.h"
#include "internal/audiothread.h"

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
static std::shared_ptr<rpc::RpcSequencer> s_rpcSequencer = std::make_shared<rpc::RpcSequencer>();
static synth::SynthesizerController s_synthesizerController;

#ifdef Q_OS_LINUX
#include "internal/platform/lin/linuxaudiodriver.h"
static std::shared_ptr<IAudioDriver> s_audioDriver = std::shared_ptr<IAudioDriver>(new LinuxAudioDriver());
#endif

#ifdef Q_OS_WIN
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

    //! TODO Will be removed
    ioc()->registerExportNoDelete<IAudioEngine>(moduleName(), AudioEngine::instance());
}

void AudioModule::registerUiTypes()
{
    qmlRegisterType<AudioEngineDevTools>("MuseScore.Audio", 1, 0, "AudioEngineDevTools");
    qmlRegisterType<synth::SynthsSettingsModel>("MuseScore.Audio", 1, 0, "SynthsSettingsModel");

    //! NOTE No Qml, as it will be, need to uncomment
    //framework::ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(mu4_audio_QML_IMPORT);
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

    //! TODO It looks like the audio buffer should not be created in the audio engine,
    //! but should be created externally and set to the audio engine.

    // Init configuration
    s_audioConfiguration->init();

    // Setup rpc system and worker
    s_rpcSequencer->setup();
    s_audioWorker->channel()->setupMainThread();
    s_audioWorker->setAudioBuffer(AudioEngine::instance()->buffer());
    s_audioWorker->run([]() {
        AudioSanitizer::setupWorkerThread();
    });

    // Setup audio driver
    IAudioDriver::Spec requiredSpec;
    requiredSpec.sampleRate = 48000;
    requiredSpec.format = IAudioDriver::Format::AudioF32;
    requiredSpec.channels = 2; // stereo
    requiredSpec.samples = s_audioConfiguration->driverBufferSize();
    requiredSpec.callback = [](void* /*userdata*/, uint8_t* stream, int byteCount) {
        auto samples = byteCount / (2 * sizeof(float));
        AudioEngine::instance()->buffer()->pop(reinterpret_cast<float*>(stream), samples);
    };

    IAudioDriver::Spec activeSpec;
    bool driverOpened = s_audioDriver->open(requiredSpec, &activeSpec);
    if (!driverOpened) {
        LOGE() << "audio output open failed";
        return;
    }

    // Setup synthesizers
    s_synthesizerController.init();

    // Setup audio engine
    //! NOTE Send msg for init audio engine to worker
    s_audioWorker->channel()->send(
        rpc::Msg(
            rpc::TargetName::AudioEngine,
            "init",
            rpc::Args::make_arg2<int, uint16_t>(activeSpec.sampleRate, activeSpec.samples)
            ));
}

void AudioModule::onDeinit()
{
    s_audioWorker->stop();
    s_audioDriver->close();
    AudioEngine::instance()->deinit();
}
