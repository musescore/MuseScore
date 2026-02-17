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
#include "diagnosticsmodule.h"

#include "modularity/ioc.h"
#include "global/iglobalconfiguration.h"
#include "interactive/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

#include "internal/diagnosticsconfiguration.h"
#include "internal/diagnosticsactions.h"
#include "internal/diagnosticsactionscontroller.h"
#include "internal/diagnosticspathsregister.h"
#include "internal/savediagnosticfilesscenario.h"

#include "muse_framework_config.h"

#ifdef MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT
#include "internal/crashhandler/crashhandler.h"
#endif

#include "log.h"

using namespace muse::diagnostics;
using namespace muse;
using namespace muse::modularity;

static const std::string mname("diagnostics");

std::string DiagnosticsModule::moduleName() const
{
    return mname;
}

void DiagnosticsModule::registerExports()
{
    m_configuration = std::make_shared<DiagnosticsConfiguration>(globalCtx());
    globalIoc()->registerExport<IDiagnosticsConfiguration>(mname, m_configuration);
}

void DiagnosticsModule::resolveImports()
{
    auto ir = globalIoc()->resolve<muse::interactive::IInteractiveUriRegister>(mname);
    if (ir) {
        ir->registerQmlUri(Uri("muse://diagnostics/system/paths"), "Muse.Diagnostics", "DiagnosticPathsDialog");
        ir->registerQmlUri(Uri("muse://diagnostics/system/graphicsinfo"), "Muse.Diagnostics", "DiagnosticGraphicsInfoDialog");
        ir->registerQmlUri(Uri("muse://diagnostics/system/profiler"), "Muse.Diagnostics", "DiagnosticProfilerDialog");
        ir->registerQmlUri(Uri("muse://diagnostics/navigation/tree"), "Muse.Diagnostics", "DiagnosticNavigationDialog");
        ir->registerQmlUri(Uri("muse://diagnostics/accessible/tree"), "Muse.Diagnostics", "DiagnosticAccessibleDialog");
        ir->registerQmlUri(Uri("muse://diagnostics/actions/list"), "Muse.Diagnostics", "DiagnosticActionsDialog");
    }
}

void DiagnosticsModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();

    auto globalConf = globalIoc()->resolve<IGlobalConfiguration>(mname);
    IF_ASSERT_FAILED(globalConf) {
        return;
    }

#ifdef MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT

    static CrashHandler s_crashHandler;

#ifdef Q_OS_WIN
    const muse::io::path_t handlerFile("crashpad_handler.exe");
#else
    const muse::io::path_t handlerFile("crashpad_handler");
#endif

    const muse::io::path_t handlerPath = globalConf->appBinDirPath() + "/" + handlerFile;
    const muse::io::path_t dumpsDir = globalConf->userAppDataPath() + "/logs/dumps";
    fileSystem()->makePath(dumpsDir);

    std::string serverUrl { MUSE_MODULE_DIAGNOSTICS_CRASHREPORT_URL };
    if (!m_configuration->isDumpUploadAllowed()) {
        serverUrl.clear();
        LOGD() << "not allowed dump upload";
    } else {
        LOGD() << "crash server url: " << serverUrl;
    }

    bool ok = s_crashHandler.start(handlerPath, dumpsDir, serverUrl);
    if (!ok) {
        LOGE() << "failed start crash handler";
    } else {
        LOGI() << "success start crash handler";
    }

#else
    LOGW() << "crash handling disabled";
#endif // MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT
}

IContextSetup* DiagnosticsModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new DiagnosticsContext(ctx);
}

void DiagnosticsContext::registerExports()
{
    m_actionsController = std::make_shared<DiagnosticsActionsController>(iocContext());

    ioc()->registerExport<IDiagnosticsPathsRegister>(mname, new DiagnosticsPathsRegister());
    ioc()->registerExport<ISaveDiagnosticFilesScenario>(mname, new SaveDiagnosticFilesScenario(iocContext()));
}

void DiagnosticsContext::resolveImports()
{
    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(mname);
    if (ar) {
        ar->reg(std::make_shared<DiagnosticsActions>());
    }
}

void DiagnosticsContext::onInit(const IApplication::RunMode&)
{
    m_actionsController->init();
}
