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

static std::shared_ptr<WorkspaceManager> s_manager = std::make_shared<WorkspaceManager>();
static std::shared_ptr<WorkspaceDataStreamRegister> s_streamRegister = std::make_shared<WorkspaceDataStreamRegister>();
static std::shared_ptr<WorkspaceConfiguration> s_configuration = std::make_shared<WorkspaceConfiguration>();

std::string WorkspaceModule::moduleName() const
{
    return "workspace";
}

void WorkspaceModule::registerExports()
{
    framework::ioc()->registerExport<IWorkspaceConfiguration>(moduleName(), s_configuration);
    framework::ioc()->registerExport<IWorkspaceManager>(moduleName(), s_manager);
    framework::ioc()->registerExport<WorkspaceDataStreamRegister>(moduleName(), s_streamRegister);
}

void WorkspaceModule::resolveImports()
{
    s_streamRegister->regStream("Preferences", std::make_shared<WorkspaceSettingsStream>());
    s_streamRegister->regStream("Toolbar", std::make_shared<WorkspaceToolbarStream>());
}

void WorkspaceModule::onInit()
{
    s_configuration->init();
    s_manager->init();
}
