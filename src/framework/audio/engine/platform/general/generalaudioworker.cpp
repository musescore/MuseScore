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

#include "global/concurrency/threadutils.h"

#ifdef Q_OS_WIN
#include "global/platform/win/waitabletimer.h"
#endif

#include "audio/common/audiosanitizer.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::engine;

static uint64_t toWinTime(const msecs_t msecs)
{
    return msecs * 10000;
}

GeneralAudioWorker::GeneralAudioWorker()
{
}

GeneralAudioWorker::~GeneralAudioWorker()
{
    if (m_running) {
        stop();
    }
}

void GeneralAudioWorker::run(Callback callback)
{
    m_thread = std::make_unique<std::thread>([this, callback]() {
        th_main(callback);
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

static msecs_t audioWorkerInterval(const samples_t samples, const sample_rate_t sampleRate)
{
    msecs_t interval = float(samples) / 4.f / float(sampleRate) * 1000.f;
    interval = std::max(interval, msecs_t(1));

    // Found experementaly on a slow laptop (2 core) running on battery power
    interval = std::min(interval, msecs_t(10));

    return interval;
}

void GeneralAudioWorker::setInterval(const samples_t samples, const sample_rate_t sampleRate)
{
    setInterval(audioWorkerInterval(samples, sampleRate));
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

void GeneralAudioWorker::th_main(Callback callback)
{
    m_intervalMsecs = 1;
    m_intervalInWinTime = toWinTime(m_intervalMsecs);

    m_running = true;

#ifdef Q_OS_WIN
    WaitableTimer timer;
    bool timerValid = timer.init();
    if (timerValid) {
        LOGI() << "Waitable timer successfully created, interval: " << m_intervalMsecs << " ms";
    }
#endif

    while (m_running) {
        callback();

#ifdef Q_OS_WIN
        if (!timerValid || !timer.setAndWait(m_intervalInWinTime)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_intervalMsecs));
        }
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(m_intervalMsecs));
#endif
    }
}
