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
#include "audiothread.h"

#include "global/runtime.h"
#include "global/threadutils.h"
#include "global/async/processevents.h"
#include "global/modularity/ioc.h"

#ifdef Q_OS_WIN
#include "global/platform/win/waitabletimer.h"
#endif

#include "audio/common/audiosanitizer.h"
#include "audio/common/rpc/irpcchannel.h"

#include "audioworkerconfiguration.h"
#include "audioengine.h"
#include "audiobuffer.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::worker;
using namespace muse::audio::rpc;

static uint64_t toWinTime(const msecs_t msecs)
{
    return msecs * 10000;
}

std::thread::id AudioThread::ID;

AudioThread::AudioThread(std::shared_ptr<rpc::IRpcChannel> channel)
    : m_channel(channel)
{
    m_intervalInWinTime = toWinTime(m_intervalMsecs);
}

AudioThread::~AudioThread()
{
    if (m_running) {
        stop();
    }
}

static std::string moduleName()
{
    return "audio_worker";
}

static muse::modularity::ModulesIoC* ioc()
{
    return muse::modularity::globalIoc();
}

void AudioThread::run()
{
    // Services registration
    //! NOTE At the moment, it is better to create services
    //! and register them in the IOC in the main thread,
    //! because the IOC thread is not safe.
    //! Therefore, we will need to make own IOC instance for the worker.

    m_configuration = std::make_shared<AudioWorkerConfiguration>();
    m_audioEngine = std::make_shared<AudioEngine>();
    m_audioBuffer = std::make_shared<AudioBuffer>();

    ioc()->registerExport<IAudioWorkerConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IAudioEngine>(moduleName(), m_audioEngine);

    // Run thread
    m_running = true;
    m_thread = std::make_unique<std::thread>([this]() {
        th_main();
    });

    if (!muse::setThreadPriority(*m_thread, ThreadPriority::High)) {
        LOGE() << "Unable to change audio thread priority";
    }
}

void AudioThread::setInterval(const msecs_t interval)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_intervalMsecs = interval;
    m_intervalInWinTime = toWinTime(interval);
}

void AudioThread::stop(const Runnable& onFinished)
{
    m_onFinished = onFinished;
    m_running = false;
    if (m_thread) {
        m_thread->join();
    }
}

bool AudioThread::isRunning() const
{
    return m_running;
}

void AudioThread::th_main()
{
    runtime::setThreadName("audio_worker");

    AudioThread::ID = std::this_thread::get_id();

    m_channel->initOnWorker();

    m_configuration->init();

    m_channel->onMethod(Method::WorkerConfigInited, [this](const Msg&) {
        LOGI() << "WorkerConfigInited";

        m_audioBuffer->init(m_configuration->audioChannelsCount());
    });

    m_channel->send(rpc::make_notification(Method::WorkerStarted));

#ifdef Q_OS_WIN
    WaitableTimer timer;
    bool timerValid = timer.init();
    if (timerValid) {
        LOGI() << "Waitable timer successfully created, interval: " << m_intervalMsecs << " ms";
    }
#endif

    while (m_running) {
        ONLY_AUDIO_WORKER_THREAD;
        async::processEvents();
        m_channel->process();
        m_audioEngine->procces();

#ifdef Q_OS_WIN
        if (!timerValid || !timer.setAndWait(m_intervalInWinTime)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_intervalMsecs));
        }
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(m_intervalMsecs));
#endif
    }

    if (m_onFinished) {
        m_onFinished();
    }
}
