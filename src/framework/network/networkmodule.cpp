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
#include "networkmodule.h"

#include "modularity/ioc.h"
#include "internal/networkmanagercreator.h"
#include "internal/networkconfiguration.h"

#include "global/api/iapiregister.h"

#ifdef MUSE_MODULE_NETWORK_WEBSOCKET
#include "api/websocketapi.h"
#include "api/websocketserverapi.h"
#endif

using namespace muse::network;

std::string NetworkModule::moduleName() const
{
    return "network";
}

void NetworkModule::registerExports()
{
    m_configuration = std::make_shared<NetworkConfiguration>(iocContext());

    ioc()->registerExport<INetworkManagerCreator>(moduleName(), new NetworkManagerCreator());
    ioc()->registerExport<INetworkConfiguration>(moduleName(), m_configuration);
}

void NetworkModule::registerApi()
{
    using namespace muse::api;

    auto api = ioc()->resolve<IApiRegister>(moduleName());
    if (api) {
#ifdef MUSE_MODULE_NETWORK_WEBSOCKET
        api->regApiCreator(moduleName(), "api.websocket", new ApiCreator<api::WebSocketApi>());
        api->regApiCreator(moduleName(), "api.websocketserver", new ApiCreator<api::WebSocketServerApi>());
#endif
    }
}

void NetworkModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
}
