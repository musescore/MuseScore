//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "telemetrymodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "internal/telemetryconfiguration.h"
#include "internal/telemetryservice.h"
#include "view/telemetrypermissionmodel.h"

#include "global/iglobalconfiguration.h"
#include "internal/dump/crashhandler.h"

#include "devtools/telemetrydevtools.h"

#include "ui/iinteractiveuriregister.h"

#include "log.h"
#include "config.h"

using namespace mu::telemetry;
using namespace mu::framework;

static std::shared_ptr<TelemetryConfiguration> s_configuration = std::make_shared<TelemetryConfiguration>();

static void telemetry_init_qrc()
{
    Q_INIT_RESOURCE(telemetry);
}

std::string TelemetryModule::moduleName() const
{
    return "telemetry";
}

void TelemetryModule::registerResources()
{
    telemetry_init_qrc();
}

void TelemetryModule::registerExports()
{
    ioc()->registerExport<ITelemetryConfiguration>(moduleName(), s_configuration);
    ioc()->registerExport<ITelemetryService>(moduleName(), new TelemetryService());
}

void TelemetryModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://telemetry/permission"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/Telemetry/TelemetryPermissionDialog.qml"));
    }
}

void TelemetryModule::registerUiTypes()
{
    qmlRegisterType<TelemetryPermissionModel>("MuseScore.Telemetry", 1, 0, "TelemetryPermissionModel");
    qmlRegisterType<TelemetryDevTools>("MuseScore.Telemetry", 1, 0, "TelemetryDevTools");
}

void TelemetryModule::onInit(const framework::IApplication::RunMode&)
{
    s_configuration->init();

    auto globalConf = framework::ioc()->resolve<framework::IGlobalConfiguration>(moduleName());
    IF_ASSERT_FAILED(globalConf) {
        return;
    }

#ifdef BUILD_CRASHPAD_CLIENT

    static CrashHandler s_crashHandler;

#ifdef _MSC_VER
    io::path handlerFile("crashpad_handler.exe");
#else
    io::path handlerFile("crashpad_handler");
#endif

    io::path handlerPath = globalConf->appDirPath() + "/" + handlerFile;
    io::path dumpsDir = globalConf->logsPath() + "/dumps";
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
#endif
}
