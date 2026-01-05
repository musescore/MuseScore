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
#include "ui/iinteractiveuriregister.h"
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

std::string DiagnosticsModule::moduleName() const
{
    return "diagnostics";
}

void DiagnosticsModule::registerExports()
{
    m_configuration = std::make_shared<DiagnosticsConfiguration>(iocContext());
    m_actionsController = std::make_shared<DiagnosticsActionsController>(iocContext());

    ioc()->registerExport<IDiagnosticsPathsRegister>(moduleName(), new DiagnosticsPathsRegister());
    ioc()->registerExport<IDiagnosticsConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<ISaveDiagnosticFilesScenario>(moduleName(), new SaveDiagnosticFilesScenario(iocContext()));
}

void DiagnosticsModule::resolveImports()
{
    auto ir = ioc()->resolve<muse::ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("muse://diagnostics/system/paths"), "Muse.Diagnostics", "DiagnosticPathsDialog");
        ir->registerQmlUri(Uri("muse://diagnostics/system/graphicsinfo"), "Muse.Diagnostics", "DiagnosticGraphicsInfoDialog");
        ir->registerQmlUri(Uri("muse://diagnostics/system/profiler"), "Muse.Diagnostics", "DiagnosticProfilerDialog");
        ir->registerQmlUri(Uri("muse://diagnostics/navigation/tree"), "Muse.Diagnostics", "DiagnosticNavigationDialog");
        ir->registerQmlUri(Uri("muse://diagnostics/accessible/tree"), "Muse.Diagnostics", "DiagnosticAccessibleDialog");
        ir->registerQmlUri(Uri("muse://diagnostics/actions/list"), "Muse.Diagnostics", "DiagnosticActionsDialog");
    }

    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<DiagnosticsActions>());
    }
}

void DiagnosticsModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
    m_actionsController->init();

    auto globalConf = ioc()->resolve<IGlobalConfiguration>(moduleName());
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
