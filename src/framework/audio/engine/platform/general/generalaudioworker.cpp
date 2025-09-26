/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "generalaudioworker.h"

#include "global/runtime.h"
#include "global/concurrency/threadutils.h"
#include "global/async/processevents.h"

#ifdef Q_OS_WIN
#include "global/platform/win/waitabletimer.h"
#endif

#include "audio/common/audiosanitizer.h"

#include "../../internal/enginecontroller.h"

#include "log.h"

using namespace muse;
using namespace muse::modularity;
using namespace muse::audio;
using namespace muse::audio::engine;

static uint64_t toWinTime(const msecs_t msecs)
{
    return msecs * 10000;
}

GeneralAudioWorker::GeneralAudioWorker(std::shared_ptr<rpc::IRpcChannel> rpcChannel)
    : m_rpcChannel(rpcChannel)
{
    m_engineController = std::make_shared<EngineController>(rpcChannel);
}

GeneralAudioWorker::~GeneralAudioWorker()
{
    if (m_running) {
        stop();
    }
}

void GeneralAudioWorker::registerExports()
{
    m_engineController->registerExports();
    // optimization
    m_engine = audioEngine();
}

void GeneralAudioWorker::run(const OutputSpec& outputSpec, const AudioEngineConfig& conf)
{
    m_running = true;
    m_thread = std::make_unique<std::thread>([this, outputSpec, conf]() {
        th_main(outputSpec, conf);
    });

    if (!muse::setThreadPriority(*m_thread, ThreadPriority::High)) {
        LOGE() << "Unable to change audio thread priority";
    }
}

void GeneralAudioWorker::setInterval(const msecs_t interval)
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_intervalMsecs = interval;
    m_intervalInWinTime = toWinTime(interval);
}

void GeneralAudioWorker::stop()
{
    m_running = false;
    if (m_thread) {
        m_thread->join();
    }
}

bool GeneralAudioWorker::isRunning() const
{
    return m_running;
}

void GeneralAudioWorker::popAudioData(float* dest, size_t sampleCount)
{
    m_engine->popAudioData(dest, sampleCount);
}

static msecs_t audioWorkerInterval(const samples_t samples, const sample_rate_t sampleRate)
{
    msecs_t interval = float(samples) / 4.f / float(sampleRate) * 1000.f;
    interval = std::max(interval, msecs_t(1));

    // Found experementaly on a slow laptop (2 core) running on battery power
    interval = std::min(interval, msecs_t(10));

    return interval;
}

void GeneralAudioWorker::th_main(const OutputSpec& outputSpec, const AudioEngineConfig& conf)
{
    runtime::setThreadName("audio_engine");
    AudioSanitizer::setupEngineThread();
    ONLY_AUDIO_ENGINE_THREAD;

    m_rpcChannel->setupOnEngine();

    m_engineController->preInit();
    m_engineController->init(outputSpec, conf);

    m_engine->outputSpecChanged().onReceive(this, [this](const OutputSpec& spec) {
        msecs_t interval = audioWorkerInterval(spec.samplesPerChannel, spec.sampleRate);
        setInterval(interval);
    });

    m_intervalMsecs = audioWorkerInterval(outputSpec.samplesPerChannel, outputSpec.sampleRate);
    m_intervalInWinTime = toWinTime(m_intervalMsecs);

#ifdef Q_OS_WIN
    WaitableTimer timer;
    bool timerValid = timer.init();
    if (timerValid) {
        LOGI() << "Waitable timer successfully created, interval: " << m_intervalMsecs << " ms";
    }
#endif

    while (m_running) {
        async::processEvents();
        m_rpcChannel->process();
        m_engine->processAudioData();

#ifdef Q_OS_WIN
        if (!timerValid || !timer.setAndWait(m_intervalInWinTime)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_intervalMsecs));
        }
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(m_intervalMsecs));
#endif
    }

    m_engineController->deinit();
}
