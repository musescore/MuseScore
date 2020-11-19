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

#include "telemetrysetup.h"

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "itelemetryservice.h"
#include "internal/telemetryservice.h"
#include "view/telemetrypermissionmodel.h"

#include "global/iglobalconfiguration.h"
#include "internal/dump/crashhandler.h"

#include "log.h"

using namespace mu::telemetry;

static CrashHandler s_crashHandler;

static void telemetry_init_qrc()
{
    Q_INIT_RESOURCE(telemetry);
}

std::string TelemetrySetup::moduleName() const
{
    return "telemetry";
}

void TelemetrySetup::registerResources()
{
    telemetry_init_qrc();
}

void TelemetrySetup::registerExports()
{
    mu::framework::ioc()->registerExport<ITelemetryService>("telemetry", new TelemetryService());
}

void TelemetrySetup::registerUiTypes()
{
    qmlRegisterType<TelemetryPermissionModel>("MuseScore.Telemetry", 3, 3, "TelemetryPermissionModel");
}

static void crash() { volatile int* a = (int*)(NULL); *a = 1; }

void TelemetrySetup::onInit()
{
    auto globalConf = framework::ioc()->resolve<framework::IGlobalConfiguration>(moduleName());
    IF_ASSERT_FAILED(globalConf) {
        return;
    }

#ifdef _MSC_VER
    io::path handlerFile("crashpad_handler.com");
#else
    io::path handlerFile("crashpad_handler");
#endif

    io::path handlerPath = globalConf->appDirPath() + "/" + handlerFile;
    io::path dumpsDir = globalConf->logsPath() + "/dumps";
    std::string sandboxUrl = "https://sentry.musescore.org/api/3/minidump/?sentry_key=1260147a791c40349bbf717b94dc29c4";

    bool ok = s_crashHandler.start(handlerPath, dumpsDir, sandboxUrl);
    if (!ok) {
        LOGE() << "failed start crash handler";
    }

    //! NOTE For test creating a dump
    // crash();
}
