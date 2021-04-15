/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#include "audiothread.h"

#include "log.h"
#include "runtime.h"
#include "async/processevents.h"

#ifdef Q_OS_WASM
#include <emscripten/html5.h>
#endif

using namespace mu::audio;

AudioThread::AudioThread()
{
    m_channel = std::make_shared<rpc::QueuedRpcChannel>();
}

AudioThread::~AudioThread()
{
    if (m_running) {
        stop();
    }
}

void AudioThread::run(const OnStart& onStart)
{
    m_onStart = onStart;

#ifndef Q_OS_WASM
    m_running = true;
    m_thread = std::make_shared<std::thread>([this]() {
        main();
    });
#else
    emscripten_set_timeout_loop([](double, void* userData) -> EM_BOOL {
        reinterpret_cast<AudioThread*>(userData)->loopBody();
        return EM_TRUE;
    }, 2, this);
#endif
}

void AudioThread::stop(const OnFinished& onFinished)
{
    m_onFinished = onFinished;
    m_running = false;
    if (m_thread) {
        m_thread->join();
    }
}

void AudioThread::setAudioBuffer(std::shared_ptr<IAudioBuffer> buffer)
{
    m_buffer = buffer;
}

void AudioThread::loopBody()
{
    mu::async::processEvents();
    m_channel->process();
    if (m_buffer) {
        m_buffer->forward();
    }
}

void AudioThread::main()
{
    mu::runtime::setThreadName("audio_worker");
    m_channel->setupWorkerThread();

    if (m_onStart) {
        m_onStart();
    }

    while (m_running) {
        loopBody();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    if (m_onFinished) {
        m_onFinished();
    }
}

rpc::QueuedRpcChannelPtr AudioThread::channel() const
{
    return m_channel;
}
