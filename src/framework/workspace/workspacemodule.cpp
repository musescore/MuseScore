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
#include "workspacemodule.h"

#include "modularity/ioc.h"

#include "framework/ui/iuiactionsregister.h"
#include "framework/interactive/iinteractiveuriregister.h"

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

static const std::string mname("workspace");

std::string WorkspaceModule::moduleName() const
{
    return mname;
}

void WorkspaceModule::registerExports()
{
    m_configuration = std::make_shared<WorkspaceConfiguration>(globalCtx());
    globalIoc()->registerExport<IWorkspaceConfiguration>(mname, m_configuration);
}

void WorkspaceModule::resolveImports()
{
    auto ir = globalIoc()->resolve<muse::interactive::IInteractiveUriRegister>(mname);
    if (ir) {
        ir->registerQmlUri(Uri("muse://workspace/select"), "Muse.Workspace", "WorkspacesDialog");
        ir->registerQmlUri(Uri("muse://workspace/create"), "Muse.Workspace", "NewWorkspaceDialog");
    }
}

void WorkspaceModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();

#ifdef MUSE_MODULE_DIAGNOSTICS
    auto pr = globalIoc()->resolve<muse::diagnostics::IDiagnosticsPathsRegister>(mname);
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
}

IContextSetup* WorkspaceModule::newContext(const kors::modularity::ContextPtr& ctx) const
{
    return new WorkspaceContext(ctx);
}

void WorkspaceContext::registerExports()
{
    m_actionController = std::make_shared<WorkspaceActionController>(iocContext());
    m_manager = std::make_shared<WorkspaceManager>(iocContext());
    m_provider = std::make_shared<WorkspacesDataProvider>(iocContext());
    ioc()->registerExport<IWorkspaceManager>(mname, m_manager);
    ioc()->registerExport<IWorkspacesDataProvider>(mname, m_provider);
}

void WorkspaceContext::onInit(const IApplication::RunMode&)
{
    m_manager->init();
    m_provider->init();
    m_actionController->init();
}

void WorkspaceContext::resolveImports()
{
    auto ar = ioc()->resolve<ui::IUiActionsRegister>(mname);
    if (ar) {
        ar->reg(std::make_shared<WorkspaceUiActions>(m_actionController, iocContext()));
    }
}

void WorkspaceContext::onDeinit()
{
    m_manager->deinit();
}
