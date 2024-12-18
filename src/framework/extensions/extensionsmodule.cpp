/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include "extensionsmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "ui/iinteractiveuriregister.h"

#include "internal/extensionsprovider.h"
#include "internal/extensionsconfiguration.h"
#include "internal/extensionsactioncontroller.h"
#include "internal/extensioninstaller.h"
#include "internal/extensionsexecpointsregister.h"

#include "view/extensionbuilder.h"
#include "view/extensionsuiengine.h"
#include "view/extensionslistmodel.h"
#include "view/extensionstoolbarmodel.h"

#include "api/v1/extapiv1.h"

#include "devtools/devextensionslistmodel.h"
#include "devtools/apidumpmodel.h"

#include "muse_framework_config.h"
#ifdef MUSE_MODULE_DIAGNOSTICS
#include "diagnostics/idiagnosticspathsregister.h"
#endif

#include "log.h"

using namespace muse::extensions;
using namespace muse::modularity;

static void extensions_init_qrc()
{
    Q_INIT_RESOURCE(extensions);
}

std::string ExtensionsModule::moduleName() const
{
    return "extensions";
}

void ExtensionsModule::registerExports()
{
    m_configuration = std::make_shared<ExtensionsConfiguration>(iocContext());
    m_provider = std::make_shared<ExtensionsProvider>(iocContext());
    m_actionController = std::make_shared<ExtensionsActionController>(iocContext());
    m_execPointsRegister = std::make_shared<ExtensionsExecPointsRegister>();

    ioc()->registerExport<IExtensionsProvider>(moduleName(), m_provider);
    ioc()->registerExport<IExtensionsConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IExtensionsUiEngine>(moduleName(), new ExtensionsUiEngine(iocContext()));
    ioc()->registerExport<IExtensionInstaller>(moduleName(), new ExtensionInstaller());
    ioc()->registerExport<IExtensionsExecPointsRegister>(moduleName(), m_execPointsRegister);
}

void ExtensionsModule::registerResources()
{
    extensions_init_qrc();
}

void ExtensionsModule::registerUiTypes()
{
    qmlRegisterType<ExtensionsListModel>("Muse.Extensions", 1, 0, "ExtensionsListModel");
    qmlRegisterType<ExtensionBuilder>("Muse.Extensions", 1, 0, "ExtensionBuilder");
    qmlRegisterType<DevExtensionsListModel>("Muse.Extensions", 1, 0, "DevExtensionsListModel");
    qmlRegisterType<ApiDumpModel>("Muse.Extensions", 1, 0, "ApiDumpModel");
    qmlRegisterType<ExtensionsToolBarModel>("Muse.Extensions", 1, 0, "ExtensionsToolBarModel");
}

void ExtensionsModule::resolveImports()
{
    auto ir = ioc()->resolve<ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("muse://extensions/viewer"), "Muse/Extensions/ExtensionViewerDialog.qml");
        ir->registerQmlUri(Uri("muse://extensions/apidump"), "Muse/Extensions/ExtensionsApiDumpDialog.qml");
    }

    m_execPointsRegister->reg(moduleName(), { EXEC_DISABLED, TranslatableString("extensions", "Disabled") });
    m_execPointsRegister->reg(moduleName(), { EXEC_MANUALLY, TranslatableString("extensions", "Manually") });
}

void ExtensionsModule::registerApi()
{
    apiv1::ExtApiV1::registerQmlTypes();
}

void ExtensionsModule::onInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_configuration->init();
    m_actionController->init();

    if (mode == IApplication::RunMode::ConsoleApp) {
        m_provider->reloadExtensions();
        m_extensionsLoaded = true;
    }

#ifdef MUSE_MODULE_DIAGNOSTICS
    auto pr = ioc()->resolve<muse::diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        pr->reg("extensions: defaultPath", m_configuration->defaultPath());
        pr->reg("extensions: userPath", m_configuration->userPath());
        pr->reg("plugins (legacy): defaultPath", m_configuration->pluginsDefaultPath());
        pr->reg("plugins (legacy): userPath", m_configuration->pluginsUserPath());
    }
#endif
}

void ExtensionsModule::onDelayedInit()
{
    if (!m_extensionsLoaded) {
        m_provider->reloadExtensions();
        m_extensionsLoaded = true;
    }
}
