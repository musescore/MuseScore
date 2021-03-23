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

#include "appshellmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "actions/iactionsregister.h"
#include "ui/iinteractiveuriregister.h"
#include "uicomponents/imenucontrollersregister.h"

#include "internal/applicationactions.h"
#include "internal/applicationactioncontroller.h"
#include "internal/appshellconfiguration.h"
#include "internal/notationpagestate.h"

#include "view/devtools/settingslistmodel.h"
#include "view/dockwindow/docksetup.h"
#include "view/appmenumodel.h"
#include "view/notationpagemodel.h"
#include "view/aboutmodel.h"
#include "view/filemenucontroller.h"
#include "view/editmenucontroller.h"
#include "view/viewmenucontroller.h"
#include "view/formatmenucontroller.h"
#include "view/toolsmenucontroller.h"
#include "view/helpmenucontroller.h"
#include "view/preferencesmodel.h"
#include "view/generalpreferencesmodel.h"
#include "view/updatepreferencesmodel.h"
#include "view/appearancepreferencesmodel.h"
#include "view/programmestartpreferencesmodel.h"
#include "view/folderspreferencesmodel.h"
#include "view/noteinputpreferencesmodel.h"
#include "view/startupmodel.h"

using namespace mu::appshell;
using namespace mu::framework;
using namespace mu::ui;
using namespace mu::uicomponents;

static std::shared_ptr<ApplicationActionController> s_applicationActionController = std::make_shared<ApplicationActionController>();
static std::shared_ptr<AppShellConfiguration> s_appShellConfiguration = std::make_shared<AppShellConfiguration>();
static std::shared_ptr<NotationPageState> s_notationPageState = std::make_shared<NotationPageState>();

static std::shared_ptr<FileMenuController> s_fileMenuController = std::make_shared<FileMenuController>();
static std::shared_ptr<EditMenuController> s_editMenuController = std::make_shared<EditMenuController>();
static std::shared_ptr<ViewMenuController> s_viewMenuController = std::make_shared<ViewMenuController>();
static std::shared_ptr<FormatMenuController> s_formatMenuController = std::make_shared<FormatMenuController>();
static std::shared_ptr<ToolsMenuController> s_toolsMenuController = std::make_shared<ToolsMenuController>();
static std::shared_ptr<HelpMenuController> s_helpMenuController = std::make_shared<HelpMenuController>();

static void appshell_init_qrc()
{
    Q_INIT_RESOURCE(appshell);
}

AppShellModule::AppShellModule()
{
}

std::string AppShellModule::moduleName() const
{
    return "appshell";
}

void AppShellModule::registerExports()
{
    ioc()->registerExport<IAppShellConfiguration>(moduleName(), s_appShellConfiguration);
    ioc()->registerExport<IApplicationActionController>(moduleName(), s_applicationActionController);
    ioc()->registerExport<INotationPageState>(moduleName(), s_notationPageState);
}

void AppShellModule::resolveImports()
{
    auto ar = ioc()->resolve<actions::IActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<ApplicationActions>());
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://home"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("musescore://notation"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("musescore://sequencer"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("musescore://publish"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("musescore://devtools"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("musescore://about/musescore"), ContainerMeta(ContainerType::QmlDialog, "AboutDialog.qml"));
        ir->registerUri(Uri("musescore://about/musicxml"), ContainerMeta(ContainerType::QmlDialog, "AboutMusicXMLDialog.qml"));
        ir->registerUri(Uri("musescore://preferences"), ContainerMeta(ContainerType::QmlDialog, "Preferences/PreferencesDialog.qml"));
    }

    auto mcr = ioc()->resolve<IMenuControllersRegister>(moduleName());
    if (mcr) {
        mcr->registerController(MenuType::File, s_fileMenuController);
        mcr->registerController(MenuType::Edit, s_editMenuController);
        mcr->registerController(MenuType::View, s_viewMenuController);
        mcr->registerController(MenuType::Format, s_formatMenuController);
        mcr->registerController(MenuType::Tools, s_toolsMenuController);
        mcr->registerController(MenuType::Help, s_helpMenuController);
    }
}

void AppShellModule::registerResources()
{
    appshell_init_qrc();
}

void AppShellModule::registerUiTypes()
{
    dock::DockSetup::registerQmlTypes();

    qmlRegisterType<SettingListModel>("MuseScore.Preferences", 1, 0, "SettingListModel");
    qmlRegisterType<PreferencesModel>("MuseScore.Preferences", 1, 0, "PreferencesModel");
    qmlRegisterType<GeneralPreferencesModel>("MuseScore.Preferences", 1, 0, "GeneralPreferencesModel");
    qmlRegisterType<UpdatePreferencesModel>("MuseScore.Preferences", 1, 0, "UpdatePreferencesModel");
    qmlRegisterType<AppearancePreferencesModel>("MuseScore.Preferences", 1, 0, "AppearancePreferencesModel");
    qmlRegisterType<ProgrammeStartPreferencesModel>("MuseScore.Preferences", 1, 0, "ProgrammeStartPreferencesModel");
    qmlRegisterType<FoldersPreferencesModel>("MuseScore.Preferences", 1, 0, "FoldersPreferencesModel");
    qmlRegisterType<NoteInputPreferencesModel>("MuseScore.Preferences", 1, 0, "NoteInputPreferencesModel");
    qmlRegisterType<AppMenuModel>("MuseScore.AppShell", 1, 0, "AppMenuModel");
    qmlRegisterType<NotationPageModel>("MuseScore.AppShell", 1, 0, "NotationPageModel");
    qmlRegisterType<AboutModel>("MuseScore.AppShell", 1, 0, "AboutModel");
    qmlRegisterType<StartupModel>("MuseScore.AppShell", 1, 0, "StartupModel");
}

void AppShellModule::onInit(const IApplication::RunMode&)
{
    s_appShellConfiguration->init();
    s_applicationActionController->init();
    s_notationPageState->init();

    s_fileMenuController->init();
    s_editMenuController->init();
    s_viewMenuController->init();
    s_formatMenuController->init();
    s_toolsMenuController->init();
    s_helpMenuController->init();
}
