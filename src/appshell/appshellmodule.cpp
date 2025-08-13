/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "view/welcomedialogmodel.h"
#include "view/firstlaunchsetup/firstlaunchsetupmodel.h"
#include "view/firstlaunchsetup/themespagemodel.h"
#include "view/firstlaunchsetup/tutorialspagemodel.h"
#include "view/preferences/preferencesmodel.h"
#include "view/preferences/generalpreferencesmodel.h"
#include "view/preferences/updatepreferencesmodel.h"
#include "view/preferences/appearancepreferencesmodel.h"
#include "view/preferences/folderspreferencesmodel.h"
#include "view/preferences/noteinputpreferencesmodel.h"
#include "view/preferences/advancedpreferencesmodel.h"
#include "view/preferences/canvaspreferencesmodel.h"
#include "view/preferences/saveandpublishpreferencesmodel.h"
#include "view/preferences/scorepreferencesmodel.h"
#include "view/preferences/importpreferencesmodel.h"
#include "view/preferences/audiomidipreferencesmodel.h"
#include "view/preferences/percussionpreferencesmodel.h"
#include "view/preferences/commonaudioapiconfigurationmodel.h"
#include "view/preferences/braillepreferencesmodel.h"
#include "view/publish/publishtoolbarmodel.h"
#include "view/internal/maintoolbarmodel.h"

#ifdef Q_OS_MAC
#include "view/appmenumodel.h"
#include "view/internal/platform/macos/macosappmenumodelhook.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
#include "view/internal/platform/macos/macosscrollinghook.h"
#endif
#else
#include "view/navigableappmenumodel.h"
#endif

using namespace mu::appshell;
using namespace muse;
using namespace muse::modularity;
using namespace muse::ui;
using namespace muse::dock;

static void appshell_init_qrc()
{
    Q_INIT_RESOURCE(appshell);
}

std::string AppShellModule::moduleName() const
{
    return "appshell";
}

void AppShellModule::registerExports()
{
    m_applicationActionController = std::make_shared<ApplicationActionController>(iocContext());
    m_applicationUiActions = std::make_shared<ApplicationUiActions>(m_applicationActionController, iocContext());
    m_appShellConfiguration = std::make_shared<AppShellConfiguration>(iocContext());
    m_sessionsManager = std::make_shared<SessionsManager>(iocContext());

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0) && defined(Q_OS_MAC)
    m_scrollingHook = std::make_shared<MacOSScrollingHook>();
#endif

    ioc()->registerExport<IAppShellConfiguration>(moduleName(), m_appShellConfiguration);
    ioc()->registerExport<IStartupScenario>(moduleName(), new StartupScenario(iocContext()));
    ioc()->registerExport<ISessionsManager>(moduleName(), m_sessionsManager);

#ifdef Q_OS_MAC
    ioc()->registerExport<IAppMenuModelHook>(moduleName(), std::make_shared<MacOSAppMenuModelHook>());
#else
    ioc()->registerExport<IAppMenuModelHook>(moduleName(), std::make_shared<AppMenuModelHookStub>());
#endif
}

void AppShellModule::resolveImports()
{
    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(m_applicationUiActions);
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
        ir->registerUri(Uri("musescore://welcomedialog"), ContainerMeta(ContainerType::QmlDialog, "WelcomeDialog.qml"));
        ir->registerUri(Uri("musescore://firstLaunchSetup"),
                        ContainerMeta(ContainerType::QmlDialog, "FirstLaunchSetup/FirstLaunchSetupDialog.qml"));
        ir->registerUri(Uri("muse://preferences"), ContainerMeta(ContainerType::QmlDialog, "Preferences/PreferencesDialog.qml"));
    }
}

void AppShellModule::registerResources()
{
    appshell_init_qrc();
}

void AppShellModule::registerUiTypes()
{
    qmlRegisterType<SettingListModel>("MuseScore.Preferences", 1, 0, "SettingListModel");
    qmlRegisterType<PreferencesModel>("MuseScore.Preferences", 1, 0, "PreferencesModel");
    qmlRegisterType<GeneralPreferencesModel>("MuseScore.Preferences", 1, 0, "GeneralPreferencesModel");
    qmlRegisterType<UpdatePreferencesModel>("MuseScore.Preferences", 1, 0, "UpdatePreferencesModel");
    qmlRegisterType<AppearancePreferencesModel>("MuseScore.Preferences", 1, 0, "AppearancePreferencesModel");
    qmlRegisterType<FoldersPreferencesModel>("MuseScore.Preferences", 1, 0, "FoldersPreferencesModel");
    qmlRegisterType<NoteInputPreferencesModel>("MuseScore.Preferences", 1, 0, "NoteInputPreferencesModel");
    qmlRegisterType<AdvancedPreferencesModel>("MuseScore.Preferences", 1, 0, "AdvancedPreferencesModel");
    qmlRegisterType<CanvasPreferencesModel>("MuseScore.Preferences", 1, 0, "CanvasPreferencesModel");
    qmlRegisterType<SaveAndPublishPreferencesModel>("MuseScore.Preferences", 1, 0, "SaveAndPublishPreferencesModel");
    qmlRegisterType<ScorePreferencesModel>("MuseScore.Preferences", 1, 0, "ScorePreferencesModel");
    qmlRegisterType<ImportPreferencesModel>("MuseScore.Preferences", 1, 0, "ImportPreferencesModel");
    qmlRegisterType<AudioMidiPreferencesModel>("MuseScore.Preferences", 1, 0, "AudioMidiPreferencesModel");
    qmlRegisterType<PercussionPreferencesModel>("MuseScore.Preferences", 1, 0, "PercussionPreferencesModel");
    qmlRegisterType<CommonAudioApiConfigurationModel>("MuseScore.Preferences", 1, 0, "CommonAudioApiConfigurationModel");
    qmlRegisterType<BraillePreferencesModel>("MuseScore.Preferences", 1, 0, "BraillePreferencesModel");

#if defined(Q_OS_MACOS)
    qmlRegisterType<AppMenuModel>("MuseScore.AppShell", 1, 0, "PlatformAppMenuModel");
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    qmlRegisterType<AppMenuModel>("MuseScore.AppShell", 1, 0, "PlatformAppMenuModel");
    qmlRegisterType<NavigableAppMenuModel>("MuseScore.AppShell", 1, 0, "AppMenuModel");
#else
    qmlRegisterType<NavigableAppMenuModel>("MuseScore.AppShell", 1, 0, "AppMenuModel");
#endif

    qmlRegisterType<MainWindowTitleProvider>("MuseScore.AppShell", 1, 0, "MainWindowTitleProvider");
    qmlRegisterType<NotationPageModel>("MuseScore.AppShell", 1, 0, "NotationPageModel");
    qmlRegisterType<NotationStatusBarModel>("MuseScore.AppShell", 1, 0, "NotationStatusBarModel");
    qmlRegisterType<AboutModel>("MuseScore.AppShell", 1, 0, "AboutModel");
    qmlRegisterType<WelcomeDialogModel>("MuseScore.AppShell", 1, 0, "WelcomeDialogModel");
    qmlRegisterType<FirstLaunchSetupModel>("MuseScore.AppShell", 1, 0, "FirstLaunchSetupModel");
    qmlRegisterType<ThemesPageModel>("MuseScore.AppShell", 1, 0, "ThemesPageModel");
    qmlRegisterType<TutorialsPageModel>("MuseScore.AppShell", 1, 0, "TutorialsPageModel");
    qmlRegisterType<PublishToolBarModel>("MuseScore.AppShell", 1, 0, "PublishToolBarModel");
    qmlRegisterType<MainToolBarModel>("MuseScore.AppShell", 1, 0, "MainToolBarModel");
}

void AppShellModule::onPreInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_applicationActionController->preInit();
}

void AppShellModule::onInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_appShellConfiguration->init();
    m_applicationActionController->init();
    m_applicationUiActions->init();
    m_sessionsManager->init();

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0) && defined(Q_OS_MAC)
    m_scrollingHook->init();
#endif
}

void AppShellModule::onAllInited(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    //! NOTE: process QEvent::FileOpen as early as possible if it was postponed
#ifdef Q_OS_MACOS
    qApp->processEvents();
#endif
}

void AppShellModule::onDeinit()
{
    m_sessionsManager->deinit();
}
