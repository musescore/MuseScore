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

#include <QQmlEngine>
#include <QFontDatabase>

#include "modularity/ioc.h"

#include "internal/uiengine.h"
#include "internal/mainwindow.h"
#include "internal/uiconfiguration.h"
#include "internal/interactiveuriregister.h"
#include "internal/uiactionsregister.h"
#include "internal/navigationcontroller.h"
#include "internal/navigationuiactions.h"
#include "internal/dragcontroller.h"

#ifdef Q_OS_MAC
#include "internal/platform/macos/macosplatformtheme.h"
#include "internal/windowscontroller.h"
#include "view/platform/macos/macosmainwindowbridge.h"
#elif defined(Q_OS_WIN)
#include "internal/platform/windows/windowsplatformtheme.h"
#include "internal/platform/windows/winwindowscontroller.h"
#include "view/mainwindowbridge.h"
#elif defined(Q_OS_LINUX)
#include "internal/platform/linux/linuxplatformtheme.h"
#include "internal/windowscontroller.h"
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
#include "view/qmldrag.h"
#include "view/windowsmodel.h"

#include "view/internal/errordetailsmodel.h"
#include "view/internal/progressdialogmodel.h"

#include "global/api/iapiregister.h"
#include "api/navigationapi.h"
#include "api/keyboardapi.h"

#include "dev/interactivetestsmodel.h"
#include "dev/testdialog.h"

#include "log.h"

using namespace muse::ui;
using namespace muse::modularity;

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
    m_uiengine = std::make_shared<UiEngine>(iocContext());
    m_configuration = std::make_shared<UiConfiguration>(iocContext());
    m_uiactionsRegister = std::make_shared<UiActionsRegister>(iocContext());
    m_keyNavigationController = std::make_shared<NavigationController>(iocContext());
    m_keyNavigationUiActions = std::make_shared<NavigationUiActions>();

    #ifdef Q_OS_MAC
    m_platformTheme = std::make_shared<MacOSPlatformTheme>();
    m_windowsController = std::make_shared<WindowsController>();
    #elif defined(Q_OS_WIN)
    m_platformTheme = std::make_shared<WindowsPlatformTheme>();
    m_windowsController = std::make_shared<WinWindowsController>();
    #elif defined(Q_OS_LINUX)
    m_platformTheme = std::make_shared<LinuxPlatformTheme>();
    m_windowsController = std::make_shared<WindowsController>();
    #else
    m_platformTheme = std::make_shared<StubPlatformTheme>();
    #endif

    ioc()->registerExport<IUiConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IUiEngine>(moduleName(), m_uiengine);
    ioc()->registerExport<IMainWindow>(moduleName(), new MainWindow());
    ioc()->registerExport<IInteractiveProvider>(moduleName(), m_uiengine->interactiveProvider());
    ioc()->registerExport<IInteractiveUriRegister>(moduleName(), new InteractiveUriRegister());
    ioc()->registerExport<IPlatformTheme>(moduleName(), m_platformTheme);
    ioc()->registerExport<IUiActionsRegister>(moduleName(), m_uiactionsRegister);
    ioc()->registerExport<INavigationController>(moduleName(), m_keyNavigationController);
    ioc()->registerExport<IDragController>(moduleName(), new DragController());
    ioc()->registerExport<IWindowsController>(moduleName(), m_windowsController);
}

void UiModule::resolveImports()
{
    auto ar = ioc()->resolve<IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(m_keyNavigationUiActions);
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("muse://interactive/standard"), "Muse/Ui/internal/StandardDialog.qml");
        ir->registerQmlUri(Uri("muse://interactive/progress"), "Muse/Ui/internal/ProgressDialog.qml");
        ir->registerQmlUri(Uri("muse://interactive/selectfile"), "Muse/Ui/internal/FileDialog.qml");
        ir->registerQmlUri(Uri("muse://interactive/selectdir"), "Muse/Ui/internal/FolderDialog.qml");

        ir->registerWidgetUri<TestDialog>(Uri("muse://devtools/interactive/testdialog"));
        ir->registerQmlUri(Uri("muse://devtools/interactive/sample"), "DevTools/Interactive/SampleDialog.qml");
    }
}

void UiModule::registerApi()
{
    using namespace muse::api;

    auto api = ioc()->resolve<IApiRegister>(moduleName());
    if (api) {
        api->regApiCreator(moduleName(), "api.navigation", new ApiCreator<muse::api::NavigationApi>());
        api->regApiCreator(moduleName(), "api.keyboard", new ApiCreator<muse::api::KeyboardApi>());
        api->regApiSingltone(moduleName(), "api.theme", m_uiengine->theme());
    }
}

void UiModule::registerResources()
{
    ui_init_qrc();
}

void UiModule::registerUiTypes()
{
    qmlRegisterUncreatableType<UiEngine>("Muse.Ui", 1, 0, "UiEngine", "Cannot create an UiEngine");
    qmlRegisterUncreatableType<api::ThemeApi>("Muse.Ui", 1, 0, "QmlTheme", "Cannot create a QmlTheme");
    qmlRegisterUncreatableType<QmlToolTip>("Muse.Ui", 1, 0, "QmlToolTip", "Cannot create a QmlToolTip");
    qmlRegisterUncreatableType<IconCode>("Muse.Ui", 1, 0, "IconCode", "Cannot create an IconCode");
    qmlRegisterUncreatableType<MusicalSymbolCodes>("Muse.Ui", 1, 0, "MusicalSymbolCodes",
                                                   "Cannot create an MusicalSymbolCodes");
    qmlRegisterUncreatableType<InteractiveProvider>("Muse.Ui", 1, 0, "QmlInteractiveProvider", "Cannot create");
    qmlRegisterUncreatableType<ContainerType>("Muse.Ui", 1, 0, "ContainerType", "Cannot create a ContainerType");

    qmlRegisterUncreatableType<NavigationEvent>("Muse.Ui", 1, 0, "NavigationEvent", "Cannot create a KeyNavigationEvent");
    qmlRegisterType<QmlDataFormatter>("Muse.Ui", 1, 0, "DataFormatter");
    qmlRegisterType<NavigationSection>("Muse.Ui", 1, 0, "NavigationSection");
    qmlRegisterType<NavigationPanel>("Muse.Ui", 1, 0, "NavigationPanel");
    qmlRegisterType<NavigationPopupPanel>("Muse.Ui", 1, 0, "NavigationPopupPanel");
    qmlRegisterType<NavigationControl>("Muse.Ui", 1, 0, "NavigationControl");
    qmlRegisterType<AccessibleItem>("Muse.Ui", 1, 0, "AccessibleItem");
    qmlRegisterUncreatableType<MUAccessible>("Muse.Ui", 1, 0, "MUAccessible", "Cannot create a enum type");
    qmlRegisterType<QmlDrag>("Muse.Ui", 1, 0, "CppDrag");

    qmlRegisterType<FocusListener>("Muse.Ui", 1, 0, "FocusListener");

    qmlRegisterType<WindowsModel>("Muse.Ui", 1, 0, "WindowsModel");

#ifdef Q_OS_MAC
    qmlRegisterType<MacOSMainWindowBridge>("Muse.Ui", 1, 0, "MainWindowBridge");
#else
    qmlRegisterType<MainWindowBridge>("Muse.Ui", 1, 0, "MainWindowBridge");
#endif

    qmlRegisterType<ErrorDetailsModel>("Muse.Ui", 1, 0, "ErrorDetailsModel");
    qmlRegisterType<ProgressDialogModel>("Muse.Ui", 1, 0, "ProgressDialogModel");

    qmlRegisterType<InteractiveTestsModel>("Muse.Ui", 1, 0, "InteractiveTestsModel");

    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(muse_ui_QML_IMPORT);
}

void UiModule::onPreInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_configuration->init();
}

void UiModule::onInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    if (QFontDatabase::addApplicationFont(":/ui/data/MusescoreIcon.ttf") == -1) {
        LOGE() << "Unable load icon font: `:/ui/data/MusescoreIcon.ttf`";
    }

    m_keyNavigationController->init();
}

void UiModule::onAllInited(const IApplication::RunMode& mode)
{
    if (IApplication::RunMode::GuiApp != mode) {
        return;
    }

    //! NOTE Some of the settings are taken from the workspace,
    //! we need to be sure that the workspaces are initialized.
    //! So, we loads these settings on onStartApp
    m_configuration->load();

    //! NOTE UIActions are collected from many modules, and these modules determine the state of their UIActions.
    //! All modules need to be initialized in order to get the correct state of UIActions.
    //! So, we do init on onStartApp
    m_uiactionsRegister->init();

    m_uiengine->init();
}

void UiModule::onDeinit()
{
    m_configuration->deinit();
}
