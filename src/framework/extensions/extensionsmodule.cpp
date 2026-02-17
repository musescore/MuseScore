/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

#include "modularity/ioc.h"

#include "interactive/iinteractiveuriregister.h"

#include "internal/extensionsprovider.h"
#include "internal/extensionsconfiguration.h"
#include "internal/extensionsactioncontroller.h"
#include "internal/extensioninstaller.h"
#include "internal/extensionsexecpointsregister.h"
#include "internal/extensionsuiengine.h"

#include "api/v1/extapiv1.h"

#include "muse_framework_config.h"
#ifdef MUSE_MODULE_DIAGNOSTICS
#include "diagnostics/idiagnosticspathsregister.h"
#endif

using namespace muse::extensions;
using namespace muse::modularity;

std::string ExtensionsModule::moduleName() const
{
    return "extensions";
}

void ExtensionsModule::registerExports()
{
    m_configuration = std::make_shared<ExtensionsConfiguration>(globalCtx());
    m_provider = std::make_shared<ExtensionsProvider>(globalCtx());
    m_execPointsRegister = std::make_shared<ExtensionsExecPointsRegister>();

    globalIoc()->registerExport<IExtensionsProvider>(moduleName(), m_provider);
    globalIoc()->registerExport<IExtensionsConfiguration>(moduleName(), m_configuration);
    globalIoc()->registerExport<IExtensionsUiEngine>(moduleName(), new ExtensionsUiEngine(globalCtx()));
    globalIoc()->registerExport<IExtensionInstaller>(moduleName(), new ExtensionInstaller(globalCtx()));
    globalIoc()->registerExport<IExtensionsExecPointsRegister>(moduleName(), m_execPointsRegister);
}

void ExtensionsModule::resolveImports()
{
    auto ir = globalIoc()->resolve<interactive::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("muse://extensions/viewer"), "Muse.Extensions", "ExtensionViewerDialog");
        ir->registerQmlUri(Uri("muse://extensions/apidump"), "Muse.Extensions", "ExtensionsApiDumpDialog");
    }

    m_execPointsRegister->reg(moduleName(), { EXEC_DISABLED, TranslatableString("extensions", "Disabled") });
    m_execPointsRegister->reg(moduleName(), { EXEC_MANUALLY, TranslatableString("extensions", "Manually") });
}

void ExtensionsModule::registerApi()
{
    apiv1::ExtApiV1::registerQmlTypes();
}

void ExtensionsModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
    m_provider->reloadExtensions();

#ifdef MUSE_MODULE_DIAGNOSTICS
    auto pr = globalIoc()->resolve<muse::diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        pr->reg("extensions: defaultPath", m_configuration->defaultPath());
        pr->reg("extensions: userPath", m_configuration->userPath());
        pr->reg("plugins (legacy): defaultPath", m_configuration->pluginsDefaultPath());
        pr->reg("plugins (legacy): userPath", m_configuration->pluginsUserPath());
    }
#endif
}

// Context
IContextSetup* ExtensionsModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new ExtensionsContext(ctx);
}

void ExtensionsContext::registerExports()
{
    m_actionController = std::make_shared<ExtensionsActionController>(iocContext());
}

void ExtensionsContext::onInit(const IApplication::RunMode&)
{
    m_actionController->init();
}
