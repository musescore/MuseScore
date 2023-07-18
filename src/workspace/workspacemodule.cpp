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
#include "workspacemodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "framework/ui/iinteractiveuriregister.h"
#include "framework/ui/iuiengine.h"
#include "framework/ui/iuiactionsregister.h"

#include "internal/workspaceconfiguration.h"
#include "internal/workspacemanager.h"
#include "internal/workspaceactioncontroller.h"
#include "internal/workspaceuiactions.h"
#include "internal/workspacesdataprovider.h"

#include "view/workspacelistmodel.h"
#include "view/newworkspacemodel.h"

#include "diagnostics/idiagnosticspathsregister.h"

using namespace mu::workspace;
using namespace mu::modularity;
using namespace mu::ui;

static void workspace_init_qrc()
{
    Q_INIT_RESOURCE(workspace);
}

std::string WorkspaceModule::moduleName() const
{
    return "workspace";
}

void WorkspaceModule::registerExports()
{
    m_manager = std::make_shared<WorkspaceManager>();
    m_configuration = std::make_shared<WorkspaceConfiguration>();
    m_actionController = std::make_shared<WorkspaceActionController>();
    m_provider= std::make_shared<WorkspacesDataProvider>();

    ioc()->registerExport<IWorkspaceConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IWorkspaceManager>(moduleName(), m_manager);
    ioc()->registerExport<IWorkspacesDataProvider>(moduleName(), m_provider);
}

void WorkspaceModule::resolveImports()
{
    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<WorkspaceUiActions>(m_actionController));
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://workspace/select"), "MuseScore/Workspace/WorkspacesDialog.qml");
        ir->registerQmlUri(Uri("musescore://workspace/create"), "MuseScore/Workspace/NewWorkspaceDialog.qml");
    }
}

void WorkspaceModule::registerResources()
{
    workspace_init_qrc();
}

void WorkspaceModule::registerUiTypes()
{
    qmlRegisterType<WorkspaceListModel>("MuseScore.Workspace", 1, 0, "WorkspaceListModel");
    qmlRegisterType<NewWorkspaceModel>("MuseScore.Workspace", 1, 0, "NewWorkspaceModel");

    ioc()->resolve<IUiEngine>(moduleName())->addSourceImportPath(workspace_QML_IMPORT);
}

void WorkspaceModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (mode != framework::IApplication::RunMode::GuiApp) {
        return;
    }

    m_configuration->init();
    m_manager->init();
    m_provider->init();
    m_actionController->init();

    auto pr = ioc()->resolve<diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        io::paths_t paths = m_configuration->workspacePaths();
        for (const io::path_t& p : paths) {
            pr->reg("workspace", p);
        }
    }
}

void WorkspaceModule::onDeinit()
{
    m_manager->deinit();
}
