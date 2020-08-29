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
#include "audiothreadstreamworker.h"

#include "log.h"
#include "runtime.h"
#include "async/processevents.h"

#include "rpcstreamcontroller.h"

using namespace mu::audio::worker;

AudioThreadStreamWorker::AudioThreadStreamWorker(const std::shared_ptr<QueuedRpcStreamChannel>& chan)
    : m_channel(chan)
{
}

AudioThreadStreamWorker::~AudioThreadStreamWorker()
{
    if (m_running.load()) {
        stop();
    }
}

void AudioThreadStreamWorker::run()
{
    m_running.store(true);
    m_thread = std::make_shared<std::thread>(AudioStreamProcess, this);
}

void AudioThreadStreamWorker::stop()
{
    m_running.store(false);
    m_thread->join();
    m_controller = nullptr;
}

void AudioThreadStreamWorker::AudioStreamProcess(AudioThreadStreamWorker* self)
{
    self->doAudioStreamProcess();
}

void AudioThreadStreamWorker::doAudioStreamProcess()
{
    mu::runtime::setThreadName("audio_worker");

    IF_ASSERT_FAILED(m_channel) {
        return;
    }

    m_channel->setupWorkerThread();

    m_controller = std::make_shared<RpcStreamController>();
    m_controller->setup();

    while (m_running.load()) {
        mu::async::processEvents();
        m_channel->process();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
