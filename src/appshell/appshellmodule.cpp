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

#include "appshellmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "ui/iuiactionsregister.h"
#include "ui/iinteractiveuriregister.h"

#include "internal/applicationuiactions.h"
#include "internal/applicationactioncontroller.h"
#include "internal/appshellconfiguration.h"
#include "internal/startupscenario.h"
#include "internal/applicationactioncontroller.h"
#include "internal/sessionsmanager.h"

#include "view/devtools/settingslistmodel.h"
#include "view/mainwindowtitleprovider.h"
#include "view/notationpagemodel.h"
#include "view/notationstatusbarmodel.h"
#include "view/aboutmodel.h"
#include "view/firstlaunchsetup/firstlaunchsetupmodel.h"
#include "view/firstlaunchsetup/themespagemodel.h"
#include "view/preferences/preferencesmodel.h"
#include "view/preferences/generalpreferencesmodel.h"
#include "view/preferences/updatepreferencesmodel.h"
#include "view/preferences/appearancepreferencesmodel.h"
#include "view/preferences/programmestartpreferencesmodel.h"
#include "view/preferences/folderspreferencesmodel.h"
#include "view/preferences/noteinputpreferencesmodel.h"
#include "view/preferences/advancedpreferencesmodel.h"
#include "view/preferences/canvaspreferencesmodel.h"
#include "view/preferences/scorepreferencesmodel.h"
#include "view/preferences/importpreferencesmodel.h"
#include "view/preferences/iopreferencesmodel.h"
#include "view/preferences/commonaudioapiconfigurationmodel.h"
#include "view/framelesswindow/framelesswindowmodel.h"
#include "view/publish/publishtoolbarmodel.h"
#include "view/windowdroparea.h"
#include "view/internal/maintoolbarmodel.h"

#include "view/dockwindow/docksetup.h"

#ifdef Q_OS_MAC
#include "view/appmenumodel.h"
#include "view/internal/platform/macos/macosappmenumodelhook.h"
#include "view/internal/platform/macos/macosscrollinghook.h"
#else
#include "view/navigableappmenumodel.h"
#endif

using namespace mu::appshell;
using namespace mu::framework;
using namespace mu::modularity;
using namespace mu::ui;
using namespace mu::dock;

static std::shared_ptr<ApplicationActionController> s_applicationActionController = std::make_shared<ApplicationActionController>();
static std::shared_ptr<ApplicationUiActions> s_applicationUiActions = std::make_shared<ApplicationUiActions>(s_applicationActionController);
static std::shared_ptr<AppShellConfiguration> s_appShellConfiguration = std::make_shared<AppShellConfiguration>();
static std::shared_ptr<SessionsManager> s_sessionsManager = std::make_shared<SessionsManager>();

#ifdef Q_OS_MAC
static std::shared_ptr<MacOSScrollingHook> s_scrollingHook = std::make_shared<MacOSScrollingHook>();
#endif

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
    DockSetup::registerExports();

    ioc()->registerExport<IAppShellConfiguration>(moduleName(), s_appShellConfiguration);
    ioc()->registerExport<IApplicationActionController>(moduleName(), s_applicationActionController);
    ioc()->registerExport<IStartupScenario>(moduleName(), new StartupScenario());
    ioc()->registerExport<ISessionsManager>(moduleName(), s_sessionsManager);

#ifdef Q_OS_MAC
    ioc()->registerExport<IAppMenuModelHook>(moduleName(), std::make_shared<MacOSAppMenuModelHook>());
#else
    ioc()->registerExport<IAppMenuModelHook>(moduleName(), std::make_shared<AppMenuModelHookStub>());
#endif
}

void AppShellModule::resolveImports()
{
    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(s_applicationUiActions);
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
        ir->registerUri(Uri("musescore://firstLaunchSetup"),
                        ContainerMeta(ContainerType::QmlDialog, "FirstLaunchSetup/FirstLaunchSetupDialog.qml"));
        ir->registerUri(Uri("musescore://preferences"), ContainerMeta(ContainerType::QmlDialog, "Preferences/PreferencesDialog.qml"));
    }
}

void AppShellModule::registerResources()
{
    appshell_init_qrc();
}

void AppShellModule::registerUiTypes()
{
    DockSetup::registerQmlTypes();

    qmlRegisterType<SettingListModel>("MuseScore.Preferences", 1, 0, "SettingListModel");
    qmlRegisterType<PreferencesModel>("MuseScore.Preferences", 1, 0, "PreferencesModel");
    qmlRegisterType<GeneralPreferencesModel>("MuseScore.Preferences", 1, 0, "GeneralPreferencesModel");
    qmlRegisterType<UpdatePreferencesModel>("MuseScore.Preferences", 1, 0, "UpdatePreferencesModel");
    qmlRegisterType<AppearancePreferencesModel>("MuseScore.Preferences", 1, 0, "AppearancePreferencesModel");
    qmlRegisterType<ProgrammeStartPreferencesModel>("MuseScore.Preferences", 1, 0, "ProgrammeStartPreferencesModel");
    qmlRegisterType<FoldersPreferencesModel>("MuseScore.Preferences", 1, 0, "FoldersPreferencesModel");
    qmlRegisterType<NoteInputPreferencesModel>("MuseScore.Preferences", 1, 0, "NoteInputPreferencesModel");
    qmlRegisterType<AdvancedPreferencesModel>("MuseScore.Preferences", 1, 0, "AdvancedPreferencesModel");
    qmlRegisterType<CanvasPreferencesModel>("MuseScore.Preferences", 1, 0, "CanvasPreferencesModel");
    qmlRegisterType<ScorePreferencesModel>("MuseScore.Preferences", 1, 0, "ScorePreferencesModel");
    qmlRegisterType<ImportPreferencesModel>("MuseScore.Preferences", 1, 0, "ImportPreferencesModel");
    qmlRegisterType<IOPreferencesModel>("MuseScore.Preferences", 1, 0, "IOPreferencesModel");
    qmlRegisterType<CommonAudioApiConfigurationModel>("MuseScore.Preferences", 1, 0, "CommonAudioApiConfigurationModel");

#if defined(Q_OS_MACOS)
    qmlRegisterType<AppMenuModel>("MuseScore.AppShell", 1, 0, "PlatformAppMenuModel");
#elif defined(Q_OS_LINUX)
    qmlRegisterType<AppMenuModel>("MuseScore.AppShell", 1, 0, "PlatformAppMenuModel");
    qmlRegisterType<NavigableAppMenuModel>("MuseScore.AppShell", 1, 0, "AppMenuModel");
#else
    qmlRegisterType<NavigableAppMenuModel>("MuseScore.AppShell", 1, 0, "AppMenuModel");
#endif

    qmlRegisterType<MainWindowTitleProvider>("MuseScore.AppShell", 1, 0, "MainWindowTitleProvider");
    qmlRegisterType<NotationPageModel>("MuseScore.AppShell", 1, 0, "NotationPageModel");
    qmlRegisterType<NotationStatusBarModel>("MuseScore.AppShell", 1, 0, "NotationStatusBarModel");
    qmlRegisterType<AboutModel>("MuseScore.AppShell", 1, 0, "AboutModel");
    qmlRegisterType<FirstLaunchSetupModel>("MuseScore.AppShell", 1, 0, "FirstLaunchSetupModel");
    qmlRegisterType<ThemesPageModel>("MuseScore.AppShell", 1, 0, "ThemesPageModel");
    qmlRegisterType<FramelessWindowModel>("MuseScore.AppShell", 1, 0, "FramelessWindowModel");
    qmlRegisterType<PublishToolBarModel>("MuseScore.AppShell", 1, 0, "PublishToolBarModel");
    qmlRegisterType<MainToolBarModel>("MuseScore.AppShell", 1, 0, "MainToolBarModel");

    qmlRegisterType<WindowDropArea>("MuseScore.Ui", 1, 0, "WindowDropArea");
}

void AppShellModule::onPreInit(const framework::IApplication::RunMode& mode)
{
    if (mode == framework::IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    s_applicationActionController->preInit();
}

void AppShellModule::onInit(const IApplication::RunMode& mode)
{
    if (mode == framework::IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    DockSetup::onInit();

    s_appShellConfiguration->init();
    s_applicationActionController->init();
    s_applicationUiActions->init();
    s_sessionsManager->init();

#ifdef Q_OS_MAC
    s_scrollingHook->init();
#endif
}

void AppShellModule::onAllInited(const framework::IApplication::RunMode& mode)
{
    if (mode == framework::IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    //! NOTE: process QEvent::FileOpen as early as possible if it was postponed
#ifdef Q_OS_MACOS
    qApp->processEvents();
#endif
}

void AppShellModule::onDeinit()
{
    s_sessionsManager->deinit();
}
