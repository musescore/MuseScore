//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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
}

AudioThread::~AudioThread()
{
    if (m_running) {
        stop();
    }
}

void AudioThread::run()
{
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

void AudioThread::stop()
{
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
    if (m_buffer) {
        m_buffer->forward();
    }
    rpcServer()->invoke();
}

void AudioThread::main()
{
    mu::runtime::setThreadName("audio_worker");

    while (m_running) {
        mu::async::processEvents();
        loopBody();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
