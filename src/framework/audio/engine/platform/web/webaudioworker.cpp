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
#include "webaudioworker.h"

#include "audio/common/rpc/irpcchannel.h"

#include "log.h"

using namespace muse;
using namespace muse::audio::engine;
using namespace muse::audio::rpc;

WebAudioWorker::WebAudioWorker(std::shared_ptr<rpc::IRpcChannel> rpcChannel)
    : m_rpcChannel(rpcChannel)
{
    m_rpcChannel->onMethod(Method::EngineStarted, [this](const Msg&) {
        LOGI() << "recieved message from worker about WorkerStarted";
        m_running.set(true);
    });
}

WebAudioWorker::~WebAudioWorker()
{
    if (m_running.val) {
        stop();
    }
}

void WebAudioWorker::registerExports()
{
    // noop
}

void WebAudioWorker::run()
{
    // noop
}

void WebAudioWorker::stop()
{
    m_running.set(false);
}

bool WebAudioWorker::isRunning() const
{
    return m_running.val;
}

async::Channel<bool> WebAudioWorker::isRunningChanged() const
{
    return m_running.ch;
}

void WebAudioWorker::popAudioData(float*, size_t)
{
    // noop
}
