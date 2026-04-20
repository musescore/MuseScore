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

#include "enginerpccontroller.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::engine;
using namespace muse::audio::fx;
using namespace muse::audio::synth;

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

    m_rpcChannel->onMethod(rpc::Method::EngineDeinit, [this](const rpc::Msg& msg) {
        deinit();
        m_rpcChannel->send(rpc::make_response(msg));
    });
}

void EngineController::onStartRunning()
{
    //! NOTE After sending a EngineRunning,
    //! we may receive RPC messages, such as for example load a soundfont.
    //! Therefore, we need to subscribe to RPC messages.
    m_rpcController = std::make_shared<EngineRpcController>();
    m_rpcController->init();

    //! NOTE We inform that the engine is running and can receive messages
    //! (it has not yet been initialized)
    m_rpcChannel->send(rpc::make_notification(rpc::Method::EngineRunning));
}

void EngineController::init(const OutputSpec& outputSpec, const AudioEngineConfig& conf)
{
    //! AUDIO THREAD

    //! NOTE These services are global, but EngineController is contextual...
    //! There are currently design issues, we need to figure out how to fix them.
    configuration()->setConfig(conf);
    synthResolver()->init(configuration()->defaultAudioInputParams(), outputSpec);
    // ------------------------------------------------------------

    IAudioEngine::RenderConstraints consts;
    consts.minSamplesToReserveWhenIdle = minSamplesToReserve(RenderMode::IdleMode);
    consts.minSamplesToReserveInRealtime = minSamplesToReserve(RenderMode::RealTimeMode);
    consts.desiredAudioThreadNumber = configuration()->desiredAudioThreadNumber();
    consts.minTrackCountForMultithreading = configuration()->minTrackCountForMultithreading();

    // Setup audio engine
    audioEngine()->init(outputSpec, consts);

    playback()->init();

    transportEventsDispatcher()->init();
}

void EngineController::deinit()
{
    //! AUDIO THREAD
    playback()->deinit();
    audioEngine()->deinit();
    m_rpcController->deinit();
}

OutputSpec EngineController::outputSpec() const
{
    return audioEngine()->outputSpec();
}

async::Channel<OutputSpec> EngineController::outputSpecChanged() const
{
    return audioEngine()->outputSpecChanged();
}

void EngineController::process(float* stream, unsigned samplesPerChannel)
{
    audioEngine()->process(stream, samplesPerChannel);
}

void EngineController::process()
{
    audioEngine()->processAudioData();
}

void EngineController::popAudioData(float* stream, unsigned samplesPerChannel)
{
    audioEngine()->popAudioData(stream, samplesPerChannel);
}
