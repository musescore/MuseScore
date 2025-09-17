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
#include "webworkerapi.h"

#include "global/modularity/ioc.h"
#include "global/globalmodule.h"

#include "audio/common/rpc/platform/web/webrpcchannel.h"

#include "audio/worker/internal/startworkercontroller.h"

#include "log.h"

using namespace muse;
using namespace muse::web::worker;
using namespace muse::audio;
using namespace muse::audio::worker;
using namespace muse::audio::rpc;

WebWorkerApi* WebWorkerApi::instance()
{
    static WebWorkerApi w;
    return &w;
}

static std::string moduleName()
{
    return "audio_worker";
}

static modularity::ModulesIoC* ioc()
{
    return modularity::globalIoc();
}

void WebWorkerApi::init()
{
    m_globalModule = std::make_shared<GlobalModule>();
    m_rpcChannel = std::make_shared<WebRpcChannel>();
    m_startWorkerController = std::make_shared<StartWorkerController>(m_rpcChannel);

    m_globalModule->registerExports();
    ioc()->registerExport<IRpcChannel>(moduleName(), m_rpcChannel);
    m_startWorkerController->registerExports();

    m_rpcChannel->setupOnWorker();

    m_globalModule->onPreInit(IApplication::RunMode::ConsoleApp);
    m_globalModule->onInit(IApplication::RunMode::ConsoleApp);

    m_rpcChannel->send(rpc::make_notification(Method::WorkerStarted));

    LOGI() << "Inited";
}
