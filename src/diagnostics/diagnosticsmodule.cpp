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
#include "diagnosticsmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "global/iglobalconfiguration.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

#include "internal/diagnosticsconfiguration.h"
#include "internal/diagnosticsactions.h"
#include "internal/diagnosticsactionscontroller.h"
#include "internal/diagnosticspathsregister.h"
#include "internal/engravingelementsprovider.h"

#include "internal/crashhandler/crashhandler.h"

#include "view/diagnosticspathsmodel.h"
#include "view/system/profilerviewmodel.h"

#include "view/keynav/diagnosticnavigationmodel.h"
#include "view/keynav/abstractkeynavdevitem.h"
#include "view/keynav/keynavdevsection.h"
#include "view/keynav/keynavdevsubsection.h"
#include "view/keynav/keynavdevcontrol.h"

#include "view/diagnosticaccessiblemodel.h"

#include "view/engraving/engravingelementsmodel.h"

#include "devtools/crashhandlerdevtoolsmodel.h"

#include "log.h"
#include "config.h"

using namespace mu::diagnostics;
using namespace mu::modularity;

static std::shared_ptr<DiagnosticsConfiguration> s_configuration = std::make_shared<DiagnosticsConfiguration>();
static std::shared_ptr<DiagnosticsActionsController> s_actionsController = std::make_shared<DiagnosticsActionsController>();

std::string DiagnosticsModule::moduleName() const
{
    return "diagnostics";
}

void DiagnosticsModule::registerExports()
{
    ioc()->registerExport<IDiagnosticsPathsRegister>(moduleName(), new DiagnosticsPathsRegister());
    ioc()->registerExport<EngravingElementsProvider>(moduleName(), new EngravingElementsProvider());
}

void DiagnosticsModule::resolveImports()
{
    auto ir = ioc()->resolve<ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://diagnostics/system/paths"), "MuseScore/Diagnostics/DiagnosticPathsDialog.qml");
        ir->registerQmlUri(Uri("musescore://diagnostics/system/profiler"), "MuseScore/Diagnostics/DiagnosticProfilerDialog.qml");
        ir->registerQmlUri(Uri("musescore://diagnostics/navigation/tree"), "MuseScore/Diagnostics/DiagnosticNavigationDialog.qml");
        ir->registerQmlUri(Uri("musescore://diagnostics/accessible/tree"), "MuseScore/Diagnostics/DiagnosticAccessibleDialog.qml");
        ir->registerQmlUri(Uri("musescore://diagnostics/engraving/elements"), "MuseScore/Diagnostics/EngravingElementsDialog.qml");
    }

    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<DiagnosticsActions>());
    }
}

void DiagnosticsModule::registerUiTypes()
{
    qmlRegisterType<DiagnosticsPathsModel>("MuseScore.Diagnostics", 1, 0, "DiagnosticsPathsModel");
    qmlRegisterType<ProfilerViewModel>("MuseScore.Diagnostics", 1, 0, "ProfilerViewModel");

    qmlRegisterType<DiagnosticNavigationModel>("MuseScore.Diagnostics", 1, 0, "DiagnosticNavigationModel");
    qmlRegisterUncreatableType<AbstractKeyNavDevItem>("MuseScore.Diagnostics", 1, 0, "AbstractKeyNavDevItem", "Cannot create a Abstract");
    qmlRegisterUncreatableType<KeyNavDevSubSection>("MuseScore.Diagnostics", 1, 0, "KeyNavDevSubSection", "Cannot create");
    qmlRegisterUncreatableType<KeyNavDevSection>("MuseScore.Diagnostics", 1, 0, "KeyNavDevSection", "Cannot create a KeyNavDevSection");
    qmlRegisterUncreatableType<KeyNavDevControl>("MuseScore.Diagnostics", 1, 0, "KeyNavDevControl", "Cannot create a KeyNavDevControl");

    qmlRegisterType<DiagnosticAccessibleModel>("MuseScore.Diagnostics", 1, 0, "DiagnosticAccessibleModel");

    qmlRegisterType<EngravingElementsModel>("MuseScore.Diagnostics", 1, 0, "EngravingElementsModel");

    qmlRegisterType<CrashHandlerDevToolsModel>("MuseScore.Diagnostics", 1, 0, "CrashHandlerDevToolsModel");
}

void DiagnosticsModule::onInit(const framework::IApplication::RunMode&)
{
    s_configuration->init();
    s_actionsController->init();

    auto globalConf = modularity::ioc()->resolve<framework::IGlobalConfiguration>(moduleName());
    IF_ASSERT_FAILED(globalConf) {
        return;
    }

#ifdef BUILD_CRASHPAD_CLIENT

    static CrashHandler s_crashHandler;

#ifdef _MSC_VER
    io::path handlerFile("crashpad_handler.exe");
#else
    io::path handlerFile("crashpad_handler");
#endif // _MSC_VER

    io::path handlerPath = globalConf->appBinPath() + "/" + handlerFile;
    io::path dumpsDir = globalConf->userAppDataPath() + "/logs/dumps";
    fileSystem()->makePath(dumpsDir);
    std::string serverUrl(CRASH_REPORT_URL);

    if (!s_configuration->isDumpUploadAllowed()) {
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
#endif // BUILD_CRASHPAD_CLIENT
}
