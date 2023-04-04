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
#include "uimodule.h"

#include <QtQml>
#include <QFontDatabase>

#include "modularity/ioc.h"

#include "internal/uiengine.h"
#include "internal/mainwindow.h"
#include "internal/uiconfiguration.h"
#include "internal/interactiveuriregister.h"
#include "internal/uiactionsregister.h"
#include "internal/navigationcontroller.h"
#include "internal/navigationuiactions.h"

#ifdef Q_OS_MAC
#include "internal/platform/macos/macosplatformtheme.h"
#include "view/platform/macos/macosmainwindowbridge.h"
#elif defined(Q_OS_WIN)
#include "internal/platform/windows/windowsplatformtheme.h"
#include "view/mainwindowbridge.h"
#elif defined(Q_OS_LINUX)
#include "internal/platform/linux/linuxplatformtheme.h"
#include "view/mainwindowbridge.h"
#else
#include "internal/platform/stub/stubplatformtheme.h"
#include "view/mainwindowbridge.h"
#endif

#include "view/qmltooltip.h"
#include "view/iconcodes.h"
#include "view/musicalsymbolcodes.h"
#include "view/navigationsection.h"
#include "view/navigationpanel.h"
#include "view/navigationpopuppanel.h"
#include "view/navigationcontrol.h"
#include "view/navigationevent.h"
#include "view/qmlaccessible.h"
#include "view/focuslistener.h"

#include "view/internal/errordetailsmodel.h"
#include "view/internal/progressdialogmodel.h"

#include "dev/interactivetestsmodel.h"
#include "dev/testdialog.h"

using namespace mu::ui;
using namespace mu::modularity;

static std::shared_ptr<UiConfiguration> s_configuration = std::make_shared<UiConfiguration>();
static std::shared_ptr<UiActionsRegister> s_uiactionsRegister = std::make_shared<UiActionsRegister>();
static std::shared_ptr<NavigationController> s_keyNavigationController = std::make_shared<NavigationController>();
static std::shared_ptr<NavigationUiActions> s_keyNavigationUiActions = std::make_shared<NavigationUiActions>();

#ifdef Q_OS_MAC
static std::shared_ptr<MacOSPlatformTheme> s_platformTheme = std::make_shared<MacOSPlatformTheme>();
#elif defined(Q_OS_WIN)
static std::shared_ptr<WindowsPlatformTheme> s_platformTheme = std::make_shared<WindowsPlatformTheme>();
#elif defined(Q_OS_LINUX)
static std::shared_ptr<LinuxPlatformTheme> s_platformTheme = std::make_shared<LinuxPlatformTheme>();
#else
static std::shared_ptr<StubPlatformTheme> s_platformTheme = std::make_shared<StubPlatformTheme>();
#endif

static void ui_init_qrc()
{
    Q_INIT_RESOURCE(ui);
}

std::string UiModule::moduleName() const
{
    return "ui";
}

void UiModule::registerExports()
{
    ioc()->registerExport<IUiConfiguration>(moduleName(), s_configuration);
    ioc()->registerExportNoDelete<IUiEngine>(moduleName(), UiEngine::instance());
    ioc()->registerExport<IMainWindow>(moduleName(), new MainWindow());
    ioc()->registerExport<IInteractiveProvider>(moduleName(), UiEngine::instance()->interactiveProvider());
    ioc()->registerExport<IInteractiveUriRegister>(moduleName(), new InteractiveUriRegister());
    ioc()->registerExport<IPlatformTheme>(moduleName(), s_platformTheme);
    ioc()->registerExport<IUiActionsRegister>(moduleName(), s_uiactionsRegister);
    ioc()->registerExport<INavigationController>(moduleName(), s_keyNavigationController);
}

void UiModule::resolveImports()
{
    auto ar = ioc()->resolve<IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(s_keyNavigationUiActions);
    }

    auto ir = modularity::ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerWidgetUri(Uri("musescore://devtools/interactive/testdialog"), TestDialog::static_metaTypeId());
        ir->registerQmlUri(Uri("musescore://devtools/interactive/sample"), "DevTools/Interactive/SampleDialog.qml");
    }
}

void UiModule::registerResources()
{
    ui_init_qrc();
}

void UiModule::registerUiTypes()
{
    qmlRegisterUncreatableType<UiEngine>("MuseScore.Ui", 1, 0, "UiEngine", "Cannot create an UiEngine");
    qmlRegisterUncreatableType<UiTheme>("MuseScore.Ui", 1, 0, "QmlTheme", "Cannot create a QmlTheme");
    qmlRegisterUncreatableType<QmlToolTip>("MuseScore.Ui", 1, 0, "QmlToolTip", "Cannot create a QmlToolTip");
    qmlRegisterUncreatableType<IconCode>("MuseScore.Ui", 1, 0, "IconCode", "Cannot create an IconCode");
    qmlRegisterUncreatableType<MusicalSymbolCodes>("MuseScore.Ui", 1, 0, "MusicalSymbolCodes",
                                                   "Cannot create an MusicalSymbolCodes");
    qmlRegisterUncreatableType<InteractiveProvider>("MuseScore.Ui", 1, 0, "QmlInteractiveProvider", "Cannot create");
    qmlRegisterUncreatableType<ContainerType>("MuseScore.Ui", 1, 0, "ContainerType", "Cannot create a ContainerType");

    qmlRegisterUncreatableType<NavigationEvent>("MuseScore.Ui", 1, 0, "NavigationEvent", "Cannot create a KeyNavigationEvent");
    qmlRegisterType<NavigationSection>("MuseScore.Ui", 1, 0, "NavigationSection");
    qmlRegisterType<NavigationPanel>("MuseScore.Ui", 1, 0, "NavigationPanel");
    qmlRegisterType<NavigationPopupPanel>("MuseScore.Ui", 1, 0, "NavigationPopupPanel");
    qmlRegisterType<NavigationControl>("MuseScore.Ui", 1, 0, "NavigationControl");
    qmlRegisterType<AccessibleItem>("MuseScore.Ui", 1, 0, "AccessibleItem");
    qmlRegisterUncreatableType<MUAccessible>("MuseScore.Ui", 1, 0, "MUAccessible", "Cannot create a enum type");

    qmlRegisterType<FocusListener>("MuseScore.Ui", 1, 0, "FocusListener");

#ifdef Q_OS_MAC
    qmlRegisterType<MacOSMainWindowBridge>("MuseScore.Ui", 1, 0, "MainWindowBridge");
#else
    qmlRegisterType<MainWindowBridge>("MuseScore.Ui", 1, 0, "MainWindowBridge");
#endif

    qmlRegisterType<ErrorDetailsModel>("MuseScore.Ui", 1, 0, "ErrorDetailsModel");
    qmlRegisterType<ProgressDialogModel>("MuseScore.Ui", 1, 0, "ProgressDialogModel");

    qmlRegisterType<InteractiveTestsModel>("MuseScore.Ui", 1, 0, "InteractiveTestsModel");
    qRegisterMetaType<TestDialog>("TestDialog");

    modularity::ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(ui_QML_IMPORT);
}

void UiModule::onPreInit(const framework::IApplication::RunMode& mode)
{
    if (mode == framework::IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    s_configuration->initSettings();
}

void UiModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (mode != framework::IApplication::RunMode::GuiApp) {
        return;
    }

    QFontDatabase::addApplicationFont(":/fonts/mscore/MusescoreIcon.ttf"); // icons

    s_configuration->initThemes();
    s_keyNavigationController->init();
}

void UiModule::onAllInited(const framework::IApplication::RunMode& mode)
{
    if (framework::IApplication::RunMode::GuiApp != mode) {
        return;
    }

    //! NOTE Some of the settings are taken from the workspace,
    //! we need to be sure that the workspaces are initialized.
    //! So, we loads these settings on onStartApp
    s_configuration->load();

    //! NOTE UIActions are collected from many modules, and these modules determine the state of their UIActions.
    //! All modules need to be initialized in order to get the correct state of UIActions.
    //! So, we do init on onStartApp
    s_uiactionsRegister->init();
}

void UiModule::onDeinit()
{
    s_configuration->deinit();
}
