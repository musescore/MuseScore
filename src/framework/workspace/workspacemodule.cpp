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

#include "framework/ui/iuiactionsregister.h"

#include "internal/workspaceconfiguration.h"
#include "internal/workspacemanager.h"
#include "internal/workspaceactioncontroller.h"
#include "internal/workspaceuiactions.h"
#include "internal/workspacesdataprovider.h"

#include "muse_framework_config.h"

#ifdef MUSE_MODULE_DIAGNOSTICS
#include "diagnostics/idiagnosticspathsregister.h"
#endif

using namespace muse::workspace;
using namespace muse::modularity;

std::string WorkspaceModule::moduleName() const
{
    return "workspace";
}

void WorkspaceModule::registerExports()
{
    m_manager = std::make_shared<WorkspaceManager>(iocContext());
    m_configuration = std::make_shared<WorkspaceConfiguration>(iocContext());
    m_actionController = std::make_shared<WorkspaceActionController>(iocContext());
    m_provider= std::make_shared<WorkspacesDataProvider>(iocContext());

    ioc()->registerExport<IWorkspaceConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IWorkspaceManager>(moduleName(), m_manager);
    ioc()->registerExport<IWorkspacesDataProvider>(moduleName(), m_provider);
}

void WorkspaceModule::resolveImports()
{
    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<WorkspaceUiActions>(m_actionController, iocContext()));
    }
}

void WorkspaceModule::onInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    m_configuration->init();
    m_manager->init();
    m_provider->init();
    m_actionController->init();

#ifdef MUSE_MODULE_DIAGNOSTICS
    auto pr = ioc()->resolve<muse::diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        io::paths_t paths = m_configuration->workspacePaths();
        for (const io::path_t& p : paths) {
            pr->reg("workspace", p);
        }
    }
#endif
}

void WorkspaceModule::onDeinit()
{
    m_manager->deinit();
}
