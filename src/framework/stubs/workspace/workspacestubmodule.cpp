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
#include "workspacestubmodule.h"

#include "modularity/ioc.h"

#include "workspaceconfigurationstub.h"
#include "workspacemanagerstub.h"
#include "workspacesdataproviderstub.h"

using namespace muse::workspace;
using namespace muse::modularity;

static const std::string mname("workspace_stub");

std::string WorkspaceModule::moduleName() const
{
    return mname;
}

void WorkspaceModule::registerExports()
{
    globalIoc()->registerExport<IWorkspaceConfiguration>(mname, new WorkspaceConfigurationStub());
}

IContextSetup* WorkspaceModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new WorkspaceContext(ctx);
}

void WorkspaceContext::registerExports()
{
    ioc()->registerExport<IWorkspaceManager>(mname, new WorkspaceManagerStub());
    ioc()->registerExport<IWorkspacesDataProvider>(mname, new WorkspacesDataProviderStub());
}
