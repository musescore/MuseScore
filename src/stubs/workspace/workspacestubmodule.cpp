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
#include "workspacestubmodule.h"

#include "modularity/ioc.h"

#include "framework/ui/iinteractiveuriregister.h"
#include "framework/ui/iuiengine.h"

#include "workspaceconfigurationstub.h"
#include "workspacemanagerstub.h"
#include "workspacesdataproviderstub.h"

using namespace mu::workspace;
using namespace mu::modularity;
using namespace mu::ui;

static void workspace_init_qrc()
{
    Q_INIT_RESOURCE(workspace);
}

std::string WorkspaceModule::moduleName() const
{
    return "workspace_stub";
}

void WorkspaceModule::registerExports()
{
    ioc()->registerExport<IWorkspaceConfiguration>(moduleName(), new WorkspaceConfigurationStub());
    ioc()->registerExport<IWorkspaceManager>(moduleName(), new WorkspaceManagerStub());
    ioc()->registerExport<IWorkspacesDataProvider>(moduleName(), new WorkspacesDataProviderStub());
}

void WorkspaceModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://workspace/select"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/Workspace/WorkspacesDialog.qml"));

        ir->registerUri(Uri("musescore://workspace/create"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/Workspace/NewWorkspaceDialog.qml"));
    }
}

void WorkspaceModule::registerResources()
{
    workspace_init_qrc();
}

void WorkspaceModule::registerUiTypes()
{
    std::shared_ptr<ui::IUiEngine> ui = ioc()->resolve<ui::IUiEngine>(moduleName());
    if (ui) {
        ui->addSourceImportPath(workspace_QML_IMPORT);
    }
}
