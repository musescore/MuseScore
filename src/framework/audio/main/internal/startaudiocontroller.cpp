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
#include "startaudiocontroller.h"

#include "global/realfn.h"
#include "global/runtime.h"
#include "global/async/processevents.h"

#include "audio/common/audiosanitizer.h"
#include "audio/common/rpc/rpcpacker.h"

#include "audio/engine/platform/general/generalaudioworker.h"

#ifndef Q_OS_WASM
#include "audio/engine/internal/enginecontroller.h"
#endif

#include "audio/devtools/inputlag.h"

#include "muse_framework_config.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::rpc;

static void measureInputLag(const float* buf, const size_t size)
{
    if (INPUT_LAG_TIMER_STARTED) {
        for (size_t i = 0; i < size; ++i) {
            if (!muse::RealIsNull(buf[i])) {
                STOP_INPUT_LAG_TIMER;
                return;
            }
        }
    }
}

StartAudioController::StartAudioController(std::shared_ptr<rpc::IRpcChannel> rpcChannel)
    : m_rpcChannel(rpcChannel)
{
#ifndef Q_OS_WASM
    m_engineController = std::make_shared<engine::EngineController>(rpcChannel);
#endif
}

void StartAudioController::registerExports()
{
#ifndef Q_OS_WASM
    m_engineController->registerExports();
#endif
}

#ifndef Q_OS_WASM
void StartAudioController::th_setupEngine()
{
    LOGI() << "begin engine run";
    runtime::setThreadName("audio_engine");
    AudioSanitizer::setupEngineThread();
    ONLY_AUDIO_ENGINE_THREAD;

    m_rpcChannel->setupOnEngine();
    m_engineController->onStartRunning();

    LOGI() << "audio engine running";
}

#endif

void StartAudioController::init()
{
    m_rpcChannel->onMethod(rpc::Method::EngineRunning, [this](const rpc::Msg&) {
        soundFontController()->loadSoundFonts();

        m_isEngineRunning.set(true);
    });

#ifndef Q_OS_WASM
#ifdef MUSE_MODULE_AUDIO_WORKER_ENABLED
    m_worker = std::make_shared<engine::GeneralAudioWorker>();
    m_worker->run([this]() {
        static bool once = false;
        if (!once) {
            th_setupEngine();

            OutputSpec spec = m_engineController->outputSpec();
            if (spec.isValid()) {
                m_worker->setInterval(spec.samplesPerChannel, spec.sampleRate);
            }

            m_engineController->outputSpecChanged().onReceive(nullptr, [this](const OutputSpec& spec) {
                if (spec.isValid()) {
                    m_worker->setInterval(spec.samplesPerChannel, spec.sampleRate);
                }
            });

            once = true;
        }

        m_rpcChannel->process();
        m_engineController->process();
    });
#endif
#endif
}

bool StartAudioController::isAudioStarted() const
{
    return m_isAudioStarted.val;
}

async::Channel<bool> StartAudioController::isAudioStartedChanged() const
{
    return m_isAudioStarted.ch;
}

void StartAudioController::startAudioProcessing(const IApplication::RunMode& mode)
{
    IAudioDriver::Spec requiredSpec;
    requiredSpec.format = IAudioDriver::Format::AudioF32;
    requiredSpec.output.sampleRate = configuration()->sampleRate();
    requiredSpec.output.audioChannelCount = configuration()->audioChannelsCount();
    requiredSpec.output.samplesPerChannel = configuration()->driverBufferSize();

    //! NOTE In the web, callback works via messages and is configured in the worker
#ifndef Q_OS_WASM

    bool shouldMeasureInputLag = configuration()->shouldMeasureInputLag();
    requiredSpec.callback = [this, shouldMeasureInputLag](void* /*userdata*/, uint8_t* stream, int byteCount) {
        std::memset(stream, 0, byteCount);
        auto samplesPerChannel = byteCount / (2 * sizeof(float));
        float* dest = reinterpret_cast<float*>(stream);

#ifdef MUSE_MODULE_AUDIO_WORKER_ENABLED
        m_engineController->popAudioData(dest, samplesPerChannel);
#else
        static bool once = false;
        if (!once) {
            th_setupEngine();
            once = true;
        }

        m_rpcChannel->process();
        m_engineController->process(dest, samplesPerChannel);
#endif

        if (shouldMeasureInputLag) {
            measureInputLag(dest, samplesPerChannel * 2);
        }
    };

#endif // Q_OS_WASM

    IAudioDriver::Spec activeSpec;
    if (mode == IApplication::RunMode::GuiApp) {
        audioDriver()->init();

        if (!audioDriver()->open(requiredSpec, &activeSpec)) {
            return;
        }
    } else {
        activeSpec = requiredSpec;
    }

    AudioEngineConfig conf = configuration()->engineConfig();
    auto sendEngineInit = [this, activeSpec, conf]() {
        m_rpcChannel->send(rpc::make_request(Method::EngineInit, RpcPacker::pack(activeSpec.output, conf)), [this](const Msg&) {
            m_isAudioStarted.set(true);
        });
    };

    if (m_isEngineRunning.val) {
        sendEngineInit();
    } else {
        m_isEngineRunning.ch.onReceive(this, [sendEngineInit](bool arg) {
            if (arg) {
                sendEngineInit();
            }
        });
    }
}

void StartAudioController::stopAudioProcessing()
{
    m_isAudioStarted.set(false);

    if (audioDriver()->isOpened()) {
        audioDriver()->close();
    }
#ifndef Q_OS_WASM
    if (m_worker->isRunning()) {
        m_worker->stop();
    }
#endif
}

IAudioDriverPtr StartAudioController::audioDriver() const
{
    return audioDriverController()->audioDriver();
}
