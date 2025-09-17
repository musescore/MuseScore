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
#include "webaudioworker.h"

#include "audio/common/rpc/irpcchannel.h"
#include "audio/common/rpc/rpcpacker.h"

#include "log.h"

using namespace muse::audio::worker;
using namespace muse::audio::rpc;

WebAudioWorker::WebAudioWorker(std::shared_ptr<rpc::IRpcChannel> rpcChannel)
    : m_rpcChannel(rpcChannel)
{
    m_rpcChannel->onMethod(Method::WorkerStarted, [this](const Msg&) {
        LOGI() << "recieved message from worker about WorkerStarted";
        m_running = true;

        if (m_initPending.pending) {
            init(m_initPending.outputSpec, m_initPending.conf);
            m_initPending = {};
        }
    });
}

WebAudioWorker::~WebAudioWorker()
{
    if (m_running) {
        stop();
    }
}

void WebAudioWorker::registerExports()
{
    // noop
}

void WebAudioWorker::init(const OutputSpec& outputSpec, const AudioWorkerConfig& conf)
{
    m_rpcChannel->send(rpc::make_request(Method::WorkerInit, RpcPacker::pack(outputSpec, conf)));
}

void WebAudioWorker::run(const OutputSpec& outputSpec, const AudioWorkerConfig& conf)
{
    if (m_running) {
        init(outputSpec, conf);
    } else {
        m_initPending.outputSpec = outputSpec;
        m_initPending.conf = conf;
        m_initPending.pending = true;
    }
}

void WebAudioWorker::stop()
{
    m_running = false;
}

bool WebAudioWorker::isRunning() const
{
    return m_running;
}

void WebAudioWorker::popAudioData(float* dest, size_t sampleCount)
{
    // not implemented yet
}
