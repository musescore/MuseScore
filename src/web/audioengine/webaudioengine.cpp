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
#include "webaudioengine.h"

#include "global/modularity/ioc.h"

#include "global/runtime.h"

#include "audio/common/rpc/platform/web/webrpcchannel.h"

#include "audio/engine/internal/enginecontroller.h"

#include "log.h"

using namespace muse;
using namespace muse::io;
using namespace muse::audio;
using namespace muse::audio::engine;
using namespace muse::audio::rpc;

WebAudioEngine* WebAudioEngine::instance()
{
    static WebAudioEngine w;
    return &w;
}

static std::string moduleName()
{
    return "audio_engine";
}

static modularity::ModulesIoC* ioc()
{
    return modularity::globalIoc();
}

void WebAudioEngine::init()
{
    muse::runtime::mainThreadId(); //! NOTE Needs only call
    muse::runtime::setThreadName("worker");

    m_rpcChannel = std::make_shared<WebRpcChannel>();
    m_controller = std::make_shared<EngineController>(m_rpcChannel);

    ioc()->registerExport<IRpcChannel>(moduleName(), m_rpcChannel);
    m_controller->registerExports();

    m_rpcChannel->setupOnEngine();
    m_rpcChannel->send(rpc::make_notification(Method::EngineStarted));

    LOGI() << "Web audio engine inited";
}

void WebAudioEngine::process(float* stream, unsigned samplesPerChannel)
{
    m_controller->process(stream, samplesPerChannel);
}
