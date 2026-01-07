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
#include "uimodule.h"

#include <QFontDatabase>
#include <qqml.h>

#include "modularity/ioc.h"

#include "internal/uiengine.h"
#include "internal/mainwindow.h"
#include "internal/uiconfiguration.h"
#include "internal/interactiveuriregister.h"
#include "internal/uiactionsregister.h"
#include "internal/navigationcontroller.h"
#include "internal/navigationuiactions.h"
#include "internal/dragcontroller.h"
#include "view/iconcodes.h"

#ifdef Q_OS_MAC
#include "internal/platform/macos/macosplatformtheme.h"
#include "internal/windowscontroller.h"
#elif defined(Q_OS_WIN)
#include "internal/platform/windows/windowsplatformtheme.h"
#include "internal/platform/windows/winwindowscontroller.h"
#elif defined(Q_OS_LINUX)
#include "internal/platform/linux/linuxplatformtheme.h"
#include "internal/windowscontroller.h"
#else
#include "internal/platform/stub/stubplatformtheme.h"
#include "internal/windowscontroller.h"
#endif

#include "global/api/iapiregister.h"
#include "api/navigationapi.h"
#include "api/keyboardapi.h"

#include "dev/testdialog.h"

#include "log.h"

using namespace muse::ui;
using namespace muse::modularity;

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
    m_windowsController = std::make_shared<WinWindowsController>(iocContext());
    #elif defined(Q_OS_LINUX)
    m_platformTheme = std::make_shared<LinuxPlatformTheme>();
    m_windowsController = std::make_shared<WindowsController>();
    #else
    m_windowsController = std::make_shared<WindowsController>();
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
        ir->registerQmlUri(Uri("muse://interactive/standard"), "Muse.Ui.Dialogs", "StandardDialog");
        ir->registerQmlUri(Uri("muse://interactive/progress"), "Muse.Ui.Dialogs", "ProgressDialog");
        ir->registerQmlUri(Uri("muse://interactive/selectfile"), "Muse.Ui.Dialogs", "FileDialog");
        ir->registerQmlUri(Uri("muse://interactive/selectdir"), "Muse.Ui.Dialogs", "FolderDialog");

        ir->registerWidgetUri<TestDialog>(Uri("muse://devtools/interactive/testdialog"));
        ir->registerQmlUri(Uri("muse://devtools/interactive/sample"), "Muse.Ui.Dialogs", "SampleDialog");
    }
}

void UiModule::registerApi()
{
    using namespace muse::api;

    auto api = ioc()->resolve<IApiRegister>(moduleName());
    if (api) {
        api->regApiCreator(moduleName(), "MuseInternal.Navigation", new ApiCreator<muse::api::NavigationApi>());
        api->regApiCreator(moduleName(), "MuseInternal.Keyboard", new ApiCreator<muse::api::KeyboardApi>());
        api->regApiSingltone(moduleName(), "MuseApi.Theme", m_uiengine->theme());

        qmlRegisterUncreatableMetaObject(IconCode::staticMetaObject, "MuseApi.Controls", 1, 0, "IconCode",
                                         "Not creatable as it is an enum type");
    }
}

void UiModule::onPreInit(const IApplication::RunMode&)
{
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
