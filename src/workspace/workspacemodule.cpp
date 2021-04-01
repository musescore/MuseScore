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

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "framework/ui/iinteractiveuriregister.h"
#include "framework/ui/iuiengine.h"
#include "framework/ui/iuiactionsregister.h"

#include "internal/workspaceconfiguration.h"
#include "internal/workspacemanager.h"
#include "internal/workspacedatastreamregister.h"
#include "internal/workspacecreator.h"
#include "internal/workspaceactioncontroller.h"
#include "internal/workspaceuiactions.h"

#include "internal/workspacesettingsstream.h"
#include "internal/workspacetoolbarstream.h"

#include "internal/workspacesettings.h"

#include "view/workspacelistmodel.h"
#include "view/currentworkspacemodel.h"
#include "view/newworkspacemodel.h"

using namespace mu::workspace;
using namespace mu::framework;
using namespace mu::ui;

static std::shared_ptr<WorkspaceManager> s_manager = std::make_shared<WorkspaceManager>();
static std::shared_ptr<WorkspaceDataStreamRegister> s_streamRegister = std::make_shared<WorkspaceDataStreamRegister>();
static std::shared_ptr<WorkspaceConfiguration> s_configuration = std::make_shared<WorkspaceConfiguration>();
static std::shared_ptr<WorkspaceSettings> s_settings = std::make_shared<WorkspaceSettings>();
static std::shared_ptr<WorkspaceActionController> s_actionController = std::make_shared<WorkspaceActionController>();

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
    ioc()->registerExport<IWorkspaceConfiguration>(moduleName(), s_configuration);
    ioc()->registerExport<IWorkspaceManager>(moduleName(), s_manager);
    ioc()->registerExport<WorkspaceDataStreamRegister>(moduleName(), s_streamRegister);
    ioc()->registerExport<IWorkspaceCreator>(moduleName(), std::make_shared<WorkspaceCreator>());
    ioc()->registerExport<IWorkspaceSettings>(moduleName(), s_settings);
}

void WorkspaceModule::resolveImports()
{
    s_streamRegister->regStream(std::make_shared<WorkspaceSettingsStream>(WorkspaceTag::Settings));
    s_streamRegister->regStream(std::make_shared<WorkspaceSettingsStream>(WorkspaceTag::UiArrangement));
    s_streamRegister->regStream(std::make_shared<WorkspaceToolbarStream>());

    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<WorkspaceUiActions>(s_actionController));
    }

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
    qmlRegisterType<WorkspaceListModel>("MuseScore.Workspace", 1, 0, "WorkspaceListModel");
    qmlRegisterType<CurrentWorkspaceModel>("MuseScore.Workspace", 1, 0, "CurrentWorkspaceModel");
    qmlRegisterType<NewWorkspaceModel>("MuseScore.Workspace", 1, 0, "NewWorkspaceModel");

    ioc()->resolve<IUiEngine>(moduleName())->addSourceImportPath(workspace_QML_IMPORT);
}

void WorkspaceModule::onInit(const IApplication::RunMode& mode)
{
    if (IApplication::RunMode::Converter == mode) {
        return;
    }

    s_configuration->init();
    s_manager->init();
    s_settings->init();
    s_actionController->init();
}

void WorkspaceModule::onDeinit()
{
    s_manager->deinit();
}
