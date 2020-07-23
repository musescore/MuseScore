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
#include "workspacemodule.h"

#include "modularity/ioc.h"

#include "internal/workspaceconfiguration.h"
#include "internal/workspacemanager.h"
#include "internal/workspacedatastreamregister.h"

#include "internal/workspacesettingsstream.h"
#include "internal/workspacetoolbarstream.h"

using namespace mu::workspace;

static std::shared_ptr<WorkspaceManager> m_manager = std::make_shared<WorkspaceManager>();
static std::shared_ptr<WorkspaceDataStreamRegister> m_sregister = std::make_shared<WorkspaceDataStreamRegister>();

std::string WorkspaceModule::moduleName() const
{
    return "workspace";
}

void WorkspaceModule::registerExports()
{
    framework::ioc()->registerExport<IWorkspaceConfiguration>(moduleName(), new WorkspaceConfiguration());
    framework::ioc()->registerExport<IWorkspaceManager>(moduleName(), m_manager);
    framework::ioc()->registerExport<WorkspaceDataStreamRegister>(moduleName(), m_sregister);
}

void WorkspaceModule::resolveImports()
{
    m_sregister->regStream("Preferences", std::make_shared<WorkspaceSettingsStream>());
    m_sregister->regStream("Toolbar", std::make_shared<WorkspaceSettingsStream>());
}

void WorkspaceModule::onInit()
{
    m_manager->init();
}
