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

#include "audio/common/audiosanitizer.h"

#ifdef Q_OS_WIN
#include "global/platform/win/waitabletimer.h"
#endif

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::worker;

static uint64_t toWinTime(const msecs_t msecs)
{
    return msecs * 10000;
}

std::thread::id AudioThread::ID;

AudioThread::~AudioThread()
{
    if (m_running) {
        stop();
    }
}

void AudioThread::run(const Runnable& onStart, const Runnable& loopBody, const msecs_t interval)
{
    m_onStart = onStart;
    m_mainLoopBody = loopBody;
    m_intervalMsecs = interval;
    m_intervalInWinTime = toWinTime(interval);

    m_running = true;
    m_thread = std::make_unique<std::thread>([this]() {
        main();
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

void AudioThread::main()
{
    runtime::setThreadName("audio_worker");

    AudioThread::ID = std::this_thread::get_id();

    if (m_onStart) {
        m_onStart();
    }

#ifdef Q_OS_WIN
    WaitableTimer timer;
    bool timerValid = timer.init();
    if (timerValid) {
        LOGI() << "Waitable timer successfully created, interval: " << m_intervalMsecs << " ms";
    }
#endif

    while (m_running) {
        if (m_mainLoopBody) {
            m_mainLoopBody();
        }

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
