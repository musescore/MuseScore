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

#include "muse_framework_config.h"

#include "log.h"

using namespace muse::ui;
using namespace muse::modularity;

static const std::string module_name = "ui";

std::string UiModule::moduleName() const
{
    return module_name;
}

void UiModule::registerExports()
{
    m_uiengine = std::make_shared<UiEngine>(globalCtx());
    m_configuration = std::make_shared<UiConfiguration>(globalCtx());

    #ifdef Q_OS_MAC
    m_platformTheme = std::make_shared<MacOSPlatformTheme>();
    #elif defined(Q_OS_WIN)
    m_platformTheme = std::make_shared<WindowsPlatformTheme>();
    #elif defined(Q_OS_LINUX)
    m_platformTheme = std::make_shared<LinuxPlatformTheme>();
    #else
    m_platformTheme = std::make_shared<StubPlatformTheme>();
    #endif

    globalIoc()->registerExport<IUiConfiguration>(moduleName(), m_configuration);
    globalIoc()->registerExport<IUiEngine>(moduleName(), m_uiengine);
    globalIoc()->registerExport<IPlatformTheme>(moduleName(), m_platformTheme);

#ifdef MUSE_MULTICONTEXT_WIP
    globalIoc()->registerExport<INavigationController>(moduleName(), new NavigationController(globalCtx()));
    globalIoc()->registerExport<IDragController>(moduleName(), new DragController());
    globalIoc()->registerExport<IWindowsController>(moduleName(), new WindowsController());
    globalIoc()->registerExport<IUiActionsRegister>(module_name, new UiActionsRegister(nullptr));
    globalIoc()->registerExport<IMainWindow>(module_name, new MainWindow());
#endif
}

void UiModule::resolveImports()
{
#ifdef MUSE_MULTICONTEXT_WIP
    auto ar = globalIoc()->resolve<IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<NavigationUiActions>());
    }
#endif
}

void UiModule::registerApi()
{
    using namespace muse::api;

    auto api = globalIoc()->resolve<IApiRegister>(moduleName());
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

    m_uiengine->init();
}

void UiModule::onDeinit()
{
    m_configuration->deinit();
}

// Context

IContextSetup* UiModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new UiModuleContext(ctx);
}

void UiModuleContext::registerExports()
{
    m_uiactionsRegister = std::make_shared<UiActionsRegister>(iocContext());
    m_keyNavigationController = std::make_shared<NavigationController>(iocContext());

    #ifdef Q_OS_MAC
    m_windowsController = std::make_shared<WindowsController>();
    #elif defined(Q_OS_WIN)
    m_windowsController = std::make_shared<WinWindowsController>(iocContext());
    #elif defined(Q_OS_LINUX)
    m_windowsController = std::make_shared<WindowsController>();
    #else
    m_windowsController = std::make_shared<WindowsController>();
    #endif

    ioc()->registerExport<IUiActionsRegister>(module_name, m_uiactionsRegister);
    ioc()->registerExport<INavigationController>(module_name, m_keyNavigationController);
    ioc()->registerExport<IDragController>(module_name, new DragController());
    ioc()->registerExport<IWindowsController>(module_name, m_windowsController);
    ioc()->registerExport<IMainWindow>(module_name, new MainWindow());
}

void UiModuleContext::resolveImports()
{
    auto ar = ioc()->resolve<IUiActionsRegister>(module_name);
    if (ar) {
        ar->reg(std::make_shared<NavigationUiActions>());
    }
}

void UiModuleContext::onInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    m_keyNavigationController->init();
}

void UiModuleContext::onAllInited(const IApplication::RunMode&)
{
    //! NOTE UIActions are collected from many modules, and these modules determine the state of their UIActions.
    //! All modules need to be initialized in order to get the correct state of UIActions.
    //! So, we do init on onStartApp
    m_uiactionsRegister->init();
}
