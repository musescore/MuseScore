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
#include "rpcmodule.h"
#include "internal/server.h"
#include "internal/client.h"
#include "modularity/ioc.h"

using namespace mu::rpc;

static std::shared_ptr<Server> s_server = std::make_shared<Server>();
static std::shared_ptr<Client> s_client = std::make_shared<Client>();

std::string RpcModule::moduleName() const
{
    return "rpc_module";
}

void RpcModule::registerExports()
{
    framework::ioc()->registerExport<IRPCServer>(moduleName(), s_server);
    framework::ioc()->registerExport<IRPCClient>(moduleName(), s_client);
}

void RpcModule::registerUiTypes()
{
}

void RpcModule::onInit(const framework::IApplication::RunMode& mode)
{
}

void RpcModule::onDeinit()
{
}
