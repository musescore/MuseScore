/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include "startworkercontroller.h"

#include "global/modularity/ioc.h"

#ifdef Q_OS_WIN
#include "global/platform/win/waitabletimer.h"
#endif

#include "audio/common/audioutils.h"
#include "audio/common/rpc/rpcpacker.h"

#include "audioworkerconfiguration.h"
#include "audioengine.h"
#include "workerplayback.h"
#include "workerchannelcontroller.h"

#include "fx/fxresolver.h"
#include "fx/musefxresolver.h"

#include "synthesizers/fluidsynth/fluidresolver.h"
#include "synthesizers/synthresolver.h"
#include "synthesizers/soundfontrepository.h"

#include "muse_framework_config.h"

#ifdef MUSE_MODULE_AUDIO_WORKER
#ifdef Q_OS_WASM
#include "audio/driver/platform/web/webaudiochannel.h"
#endif
#endif

#include "log.h"

using namespace muse;
using namespace muse::modularity;
using namespace muse::audio;
using namespace muse::audio::worker;
using namespace muse::audio::fx;
using namespace muse::audio::synth;

static std::string moduleName()
{
    return "audio_worker";
}

static muse::modularity::ModulesIoC* ioc()
{
    return muse::modularity::globalIoc();
}

StartWorkerController::StartWorkerController(std::shared_ptr<rpc::IRpcChannel> rpcChannel)
    : m_rpcChannel(rpcChannel)
{
    rpc::set_last_stream_id(100000);

    m_rpcChannel->onMethod(rpc::Method::WorkerInit, [this](const rpc::Msg& msg) {
        OutputSpec spec;
        AudioWorkerConfig conf;

        IF_ASSERT_FAILED(rpc::RpcPacker::unpack(msg.data, spec, conf)) {
            return;
        }

        init(spec, conf);
    });
}

void StartWorkerController::registerExports()
{
    //! NOTE Services registration
    //! At the moment, it is better to create services
    //! and register them in the IOC in the main thread,
    //! because the IOC thread is not safe.
    //! Therefore, we will need to make own IOC instance for the worker.

    m_configuration = std::make_shared<AudioWorkerConfiguration>();
    m_audioEngine = std::make_shared<AudioEngine>();
    m_workerPlayback = std::make_shared<WorkerPlayback>();
    m_workerChannelController  = std::make_shared<WorkerChannelController>();
    m_fxResolver = std::make_shared<FxResolver>();
    m_synthResolver = std::make_shared<SynthResolver>();
    m_soundFontRepository = std::make_shared<SoundFontRepository>();

    ioc()->registerExport<IAudioWorkerConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IAudioEngine>(moduleName(), m_audioEngine);
    ioc()->registerExport<IWorkerPlayback>(moduleName(), m_workerPlayback);
    ioc()->registerExport<IFxResolver>(moduleName(), m_fxResolver);
    ioc()->registerExport<ISynthResolver>(moduleName(), m_synthResolver);
    ioc()->registerExport<ISoundFontRepository>(moduleName(), m_soundFontRepository);
}

void StartWorkerController::init(const OutputSpec& outputSpec, const AudioWorkerConfig& conf)
{
    //! NOTE It should be as early as possible
    m_rpcChannel->setupOnWorker();

    m_configuration->init(conf);

    m_fxResolver->registerResolver(AudioFxType::MuseFx, std::make_shared<MuseFxResolver>());
    m_synthResolver->registerResolver(AudioSourceType::Fluid, std::make_shared<FluidResolver>());

    worker::AudioEngine::RenderConstraints consts;
    consts.minSamplesToReserveWhenIdle = minSamplesToReserve(RenderMode::IdleMode);
    consts.minSamplesToReserveInRealtime = minSamplesToReserve(RenderMode::RealTimeMode);
    consts.desiredAudioThreadNumber = m_configuration->desiredAudioThreadNumber();
    consts.minTrackCountForMultithreading = m_configuration->minTrackCountForMultithreading();

    // Setup audio engine
    m_audioEngine->init(outputSpec, consts);

    m_synthResolver->init(m_configuration->defaultAudioInputParams(), outputSpec);

    m_soundFontRepository->init();
    m_workerPlayback->init();
    m_workerChannelController->init(m_workerPlayback);

#ifdef MUSE_MODULE_AUDIO_WORKER
#ifdef Q_OS_WASM
    m_webAudioChannel = std::make_shared<WebAudioChannel>();
    m_webAudioChannel->open([this](float* stream, size_t samples) {
        unsigned samplesPerChannel = samples / 2;
        process(stream, samplesPerChannel);
    });
#endif
#endif
}

void StartWorkerController::deinit()
{
#ifdef MUSE_MODULE_AUDIO_WORKER
#ifdef Q_OS_WASM
    m_webAudioChannel->close();
#endif
#endif

    m_workerPlayback->deinit();
    m_workerChannelController->deinit();
    m_audioEngine->deinit();
}

void StartWorkerController::process(float* stream, unsigned samplesPerChannel)
{
    m_audioEngine->processAudioData();
    m_audioEngine->popAudioData(stream, samplesPerChannel);
}
