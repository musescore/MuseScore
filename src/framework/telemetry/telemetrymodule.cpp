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

#include "telemetrymodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "internal/telemetryconfiguration.h"

#include "global/iglobalconfiguration.h"
#include "internal/dump/crashhandler.h"

#include "devtools/telemetrydevtools.h"

#include "ui/iinteractiveuriregister.h"

#include "log.h"
#include "config.h"

using namespace mu::telemetry;
using namespace mu::modularity;
using namespace mu::ui;

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
}

void TelemetryModule::resolveImports()
{
}

void TelemetryModule::registerUiTypes()
{
    qmlRegisterType<TelemetryDevTools>("MuseScore.Telemetry", 1, 0, "TelemetryDevTools");
}

void TelemetryModule::onInit(const framework::IApplication::RunMode&)
{
    s_configuration->init();

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
#endif

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
#endif
}
