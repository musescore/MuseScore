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

#include "log.h"
#include "runtime.h"
#include "async/processevents.h"

#ifdef Q_OS_WASM
#include <emscripten/html5.h>
#endif

using namespace mu::audio;

std::thread::id AudioThread::ID;

AudioThread::~AudioThread()
{
    if (m_running) {
        stop();
    }
}

void AudioThread::run(const Runnable& onStart, const Runnable& loopBody)
{
    m_onStart = onStart;
    m_mainLoopBody = loopBody;

#ifndef Q_OS_WASM
    m_running = true;
    m_thread = std::make_unique<std::thread>([this]() {
        main();
    });
#else
    emscripten_set_timeout_loop([](double, void* userData) -> EM_BOOL {
        reinterpret_cast<AudioThread*>(userData)->loopBody();
        return EM_TRUE;
    }, 2, this);
#endif
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
    mu::runtime::setThreadName("audio_worker");

    AudioThread::ID = std::this_thread::get_id();

    if (m_onStart) {
        m_onStart();
    }

    while (m_running) {
        mu::async::processEvents();

        if (m_mainLoopBody) {
            m_mainLoopBody();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (m_onFinished) {
        m_onFinished();
    }
}
