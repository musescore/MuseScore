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

#include "log.h"

using namespace muse::audio::worker;

WebAudioWorker::WebAudioWorker(std::shared_ptr<rpc::IRpcChannel> rpcChannel)
    : m_rpcChannel(rpcChannel)
{
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

void WebAudioWorker::run(const OutputSpec& outputSpec, const AudioWorkerConfig& conf)
{
    m_running = true;
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
}
