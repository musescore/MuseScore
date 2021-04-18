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
#include "workspacestubmodule.h"

#include "modularity/ioc.h"

#include "framework/ui/iinteractiveuriregister.h"
#include "framework/ui/iuiengine.h"

#include "workspaceconfigurationstub.h"
#include "workspacemanagerstub.h"
#include "workspacedatastreamregisterstub.h"
#include "internal/workspacecreatorstub.h"

using namespace mu::workspace;
using namespace mu::framework;
using namespace mu::ui;

static void workspace_init_qrc()
{
    Q_INIT_RESOURCE(workspace);
}

std::string WorkspaceStubModule::moduleName() const
{
    return "workspace_stub";
}

void WorkspaceStubModule::registerExports()
{
    ioc()->registerExport<IWorkspaceConfiguration>(moduleName(), new WorkspaceConfigurationStub());
    ioc()->registerExport<IWorkspaceManager>(moduleName(), new WorkspaceManagerStub());
    ioc()->registerExport<IWorkspaceDataStreamRegister>(moduleName(), new WorkspaceDataStreamRegisterStub());
    ioc()->registerExport<IWorkspaceCreator>(moduleName(), new WorkspaceCreatorStub());
}

void WorkspaceStubModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://workspace/select"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/Workspace/WorkspacesDialog.qml"));

        ir->registerUri(Uri("musescore://workspace/create"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/Workspace/NewWorkspaceDialog.qml"));
    }
}

void WorkspaceStubModule::registerResources()
{
    workspace_init_qrc();
}

void WorkspaceStubModule::registerUiTypes()
{
    ioc()->resolve<IUiEngine>(moduleName())->addSourceImportPath(workspace_QML_IMPORT);
}
