/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "enginecontroller.h"

#include "global/modularity/ioc.h"

#ifdef Q_OS_WIN
#include "global/platform/win/waitabletimer.h"
#endif

#include "audio/common/audioutils.h"
#include "audio/common/rpc/rpcpacker.h"

#include "audioengineconfiguration.h"
#include "audioengine.h"
#include "engineplayback.h"
#include "enginerpccontroller.h"

#include "fx/fxresolver.h"
#include "fx/musefxresolver.h"

#include "synthesizers/fluidsynth/fluidresolver.h"
#include "synthesizers/synthresolver.h"
#include "synthesizers/soundfontrepository.h"

#include "log.h"

using namespace muse;
using namespace muse::modularity;
using namespace muse::audio;
using namespace muse::audio::engine;
using namespace muse::audio::fx;
using namespace muse::audio::synth;

static std::string moduleName()
{
    return "audio_engine";
}

static muse::modularity::ModulesIoC* ioc()
{
    return muse::modularity::globalIoc();
}

EngineController::EngineController(std::shared_ptr<rpc::IRpcChannel> rpcChannel)
    : m_rpcChannel(rpcChannel)
{
    m_rpcChannel->onMethod(rpc::Method::EngineInit, [this](const rpc::Msg& msg) {
        OutputSpec spec;
        AudioEngineConfig conf;

        IF_ASSERT_FAILED(rpc::RpcPacker::unpack(msg.data, spec, conf)) {
            return;
        }

        init(spec, conf);

        m_rpcChannel->send(rpc::make_response(msg));
    });
}

void EngineController::registerExports()
{
    //! NOTE Services registration
    //! At the moment, it is better to create services
    //! and register them in the IOC in the main thread,
    //! because the IOC thread is not safe.
    //! Therefore, we will need to make own IOC instance for the engine.

    m_configuration = std::make_shared<AudioEngineConfiguration>();
    m_audioEngine = std::make_shared<AudioEngine>();
    m_playback = std::make_shared<EnginePlayback>();
    m_rpcController  = std::make_shared<EngineRpcController>();
    m_fxResolver = std::make_shared<FxResolver>();
    m_synthResolver = std::make_shared<SynthResolver>();
    m_soundFontRepository = std::make_shared<SoundFontRepository>();

    ioc()->registerExport<IAudioEngineConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IAudioEngine>(moduleName(), m_audioEngine);
    ioc()->registerExport<IEnginePlayback>(moduleName(), m_playback);
    ioc()->registerExport<IFxResolver>(moduleName(), m_fxResolver);
    ioc()->registerExport<ISynthResolver>(moduleName(), m_synthResolver);
    ioc()->registerExport<ISoundFontRepository>(moduleName(), m_soundFontRepository);
}

void EngineController::onStartRunning()
{
    //! NOTE After sending a EngineRunning,
    //! we may receive RPC messages, such as for example load a soundfont.
    //! Therefore, we need to subscribe to RPC messages.
    m_rpcController->init();

    //! NOTE It is also possible to do internal registrations of some services
    //! that do not depend on anything.
    m_fxResolver->registerResolver(AudioFxType::MuseFx, std::make_shared<MuseFxResolver>());
    m_synthResolver->registerResolver(AudioSourceType::Fluid, std::make_shared<FluidResolver>());

    //! NOTE We inform that the engine is running and can receive messages
    //! (it has not yet been initialized)
    m_rpcChannel->send(rpc::make_notification(rpc::Method::EngineRunning));
}

void EngineController::init(const OutputSpec& outputSpec, const AudioEngineConfig& conf)
{
    m_configuration->setConfig(conf);

    engine::AudioEngine::RenderConstraints consts;
    consts.minSamplesToReserveWhenIdle = minSamplesToReserve(RenderMode::IdleMode);
    consts.minSamplesToReserveInRealtime = minSamplesToReserve(RenderMode::RealTimeMode);
    consts.desiredAudioThreadNumber = m_configuration->desiredAudioThreadNumber();
    consts.minTrackCountForMultithreading = m_configuration->minTrackCountForMultithreading();

    // Setup audio engine
    m_audioEngine->init(outputSpec, consts);

    m_synthResolver->init(m_configuration->defaultAudioInputParams(), outputSpec);

    m_playback->init();
}

void EngineController::deinit()
{
    m_playback->deinit();
    m_rpcController->deinit();
    m_audioEngine->deinit();
}

OutputSpec EngineController::outputSpec() const
{
    return m_audioEngine->outputSpec();
}

async::Channel<OutputSpec> EngineController::outputSpecChanged() const
{
    return m_audioEngine->outputSpecChanged();
}

void EngineController::process(float* stream, unsigned samplesPerChannel)
{
    m_audioEngine->process(stream, samplesPerChannel);
}

void EngineController::process()
{
    m_audioEngine->processAudioData();
}

void EngineController::popAudioData(float* stream, unsigned samplesPerChannel)
{
    m_audioEngine->popAudioData(stream, samplesPerChannel);
}
