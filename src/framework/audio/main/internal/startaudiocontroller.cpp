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
#include "audio/common/workmode.h"
#include "audio/common/alignmentbuffer.h"

#include "audio/engine/platform/general/generalaudioworker.h"

#ifndef Q_OS_WASM
#include "audio/engine/internal/enginecontroller.h"
#endif

#include "audio/devtools/inputlag.h"

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

    workmode::load();

#ifndef Q_OS_WASM
    if (workmode::mode() == workmode::WorkerMode) {
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

            static const std::thread::id thisThId = std::this_thread::get_id();

            //! NOTE MixerChannels can process in the thread pool
            //! and send messages about the audio signal,
            //! to receive them, we need to call async::processMessages here
            async::processMessages(thisThId);

            m_rpcChannel->process();
            m_engineController->process();
        });
    }

    if (workmode::mode() == workmode::WorkerRpcMode) {
        m_worker = std::make_shared<engine::GeneralAudioWorker>();
        m_worker->run([this]() {
            static bool once = false;
            if (!once) {
                th_setupEngine();
                m_worker->setInterval(16 /*msec*/);
                once = true;
            }

            static const std::thread::id thisThId = std::this_thread::get_id();

            async::processMessages(thisThId);
            m_rpcChannel->process();
        });
    }

#endif // not Q_OS_WASM
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

#ifndef Q_OS_WASM

    m_requiredSamplesTotal = requiredSpec.output.samplesPerChannel * requiredSpec.output.audioChannelCount;
    audioDriver()->activeSpecChanged().onReceive(this, [this](const IAudioDriver::Spec& spec) {
        m_requiredSamplesTotal = spec.output.samplesPerChannel * spec.output.audioChannelCount;
    });

    bool shouldMeasureInputLag = configuration()->shouldMeasureInputLag();
    requiredSpec.callback = [this, shouldMeasureInputLag]
                            (void* /*userdata*/, uint8_t* stream, int byteCount) {
        std::memset(stream, 0, byteCount);
        // driver metrics
        const size_t driverSamplesTotal = byteCount / sizeof(float);
        const size_t driverSamplesPerChannel = driverSamplesTotal / 2;
        float* driverDest = reinterpret_cast<float*>(stream);

        //! NOTE In this mode, an alignment buffer is not needed.
        if (workmode::mode() == workmode::WorkerMode) {
            m_engineController->popAudioData(driverDest, (unsigned)driverSamplesPerChannel);
            return;
        }

        const size_t requiredSamplesTotal = m_requiredSamplesTotal;
        const bool useAlignBuffer = driverSamplesTotal < requiredSamplesTotal;

        // process metrics
        size_t procSamplesTotal = 0;
        size_t procSamplesPerChannel = 0;
        float* procDest = 0;

        AlignmentBuffer* alignbuf = nullptr;
        if (useAlignBuffer) {
            // setup align buffer if need
            const size_t blocks = 2;
            const size_t capacity = requiredSamplesTotal * blocks;
            if (!m_alignmentBuffer || m_alignmentBuffer->capacity() != capacity) {
                m_alignmentBuffer = std::make_shared<AlignmentBuffer>(capacity);
            }
            alignbuf = m_alignmentBuffer.get(); // minor optimization and easier debugging
            static thread_local std::vector<float> proc_buf; // temp buffer
            if (proc_buf.size() != requiredSamplesTotal) {
                proc_buf.resize(requiredSamplesTotal);
            }

            // set proc metrics
            procSamplesTotal = requiredSamplesTotal;
            procSamplesPerChannel = procSamplesTotal / 2;
            procDest = &proc_buf[0];
        } else {
            procSamplesTotal = driverSamplesTotal;
            procSamplesPerChannel = procSamplesTotal / 2;
            procDest = driverDest;
        }

        // try fill data
        if (useAlignBuffer) {
            assert(alignbuf);
            if (alignbuf->availableRead() >= driverSamplesTotal) {
                alignbuf->read(driverDest, driverSamplesTotal);
                return;
            }
        }

        // process
        if (workmode::mode() == workmode::WorkerRpcMode) {
            m_engineController->process(procDest, (unsigned)procSamplesPerChannel);
        } else if (workmode::mode() == workmode::DriverMode) {
            static bool once = false;
            if (!once) {
                th_setupEngine();
                once = true;
            }

            m_rpcChannel->process();
            m_engineController->process(procDest, (unsigned)procSamplesPerChannel);
        }

        // write temp and fill driver dest
        if (useAlignBuffer) {
            assert(alignbuf);
            alignbuf->write(procDest, procSamplesTotal);
            alignbuf->read(driverDest, driverSamplesTotal);
        }

        if (shouldMeasureInputLag) {
            measureInputLag(procDest, procSamplesTotal);
        }
    };

#endif // Q_OS_WASM

    IAudioDriver::Spec activeSpec;
    if (mode == IApplication::RunMode::GuiApp) {
        audioDriver()->init();

        audioDriver()->selectOutputDevice(configuration()->audioOutputDeviceId());

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
    // Must call deinit() before stopping worker, so disconnect messages can
    // still be processed on the worker thread
    if (m_engineController) {
        m_engineController->deinit();
        m_engineController.reset();
    }

    if (m_worker && m_worker->isRunning()) {
        m_worker->stop();
    }
#endif
}

IAudioDriverPtr StartAudioController::audioDriver() const
{
    return audioDriverController()->audioDriver();
}
