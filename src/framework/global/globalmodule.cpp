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
#include "globalmodule.h"

#include <QTimer>
#include <QDateTime>

#include "modularity/ioc.h"
#include "internal/globalconfiguration.h"

#include "log.h"
#include "logremover.h"
#include "thirdparty/haw_logger/logger/logdefdest.h"
#include "version.h"
#include "config.h"

#include "internal/application.h"
#include "internal/interactive.h"
#include "invoker.h"

#include "runtime.h"
#include "async/processevents.h"

#include "settings.h"

#include "io/internal/filesystem.h"

#include "diagnostics/idiagnosticspathsregister.h"

#include "config.h"

using namespace mu::framework;
using namespace mu::modularity;
using namespace mu::io;

static std::shared_ptr<GlobalConfiguration> s_globalConf = std::make_shared<GlobalConfiguration>();

static Invoker s_asyncInvoker;

std::string GlobalModule::moduleName() const
{
    return "global";
}

void GlobalModule::registerExports()
{
    ioc()->registerExport<IApplication>(moduleName(), new Application());
    ioc()->registerExport<IGlobalConfiguration>(moduleName(), s_globalConf);
    ioc()->registerExport<IInteractive>(moduleName(), new Interactive());
    ioc()->registerExport<IFileSystem>(moduleName(), new FileSystem());
}

void GlobalModule::onInit(const IApplication::RunMode& mode)
{
    mu::runtime::mainThreadId(); //! NOTE Needs only call
    mu::runtime::setThreadName("main");

    //! NOTE: settings must be inited before initialization of any module
    //! because modules can use settings at the moment of their initialization
    settings()->load();

    //! --- Setup logger ---
    using namespace haw::logger;
    Logger* logger = Logger::instance();
    logger->clearDests();

    //! Console
    if (mode == IApplication::RunMode::Editor || mu::runtime::isDebug()) {
        logger->addDest(new ConsoleLogDest(LogLayout("${time} | ${type|5} | ${thread} | ${tag|10} | ${message}")));
    }

    io::path_t logPath = s_globalConf->userAppDataPath() + u"/logs";
    fileSystem()->makePath(logPath);

    //! Remove old logs
    LogRemover::removeLogs(logPath, 7, "MuseScore_yyMMdd_HHmmss.log");

    //! File, this creates a file named "data/logs/MuseScore_yyMMdd_HHmmss.log"
    io::path_t logFilePath = logPath + u"/MuseScore_"
                             + QDateTime::currentDateTime().toString("yyMMdd_HHmmss")
                             + u".log";

    FileLogDest* logFile = new FileLogDest(logFilePath.toStdString(),
                                           LogLayout("${datetime} | ${type|5} | ${thread} | ${tag|10} | ${message}"));

    logger->addDest(logFile);

#ifdef LOGGER_DEBUGLEVEL_ENABLED
    logger->setLevel(haw::logger::Debug);
#else
    logger->setLevel(haw::logger::Normal);
#endif

    LOGI() << "log path: " << logFile->filePath();
    LOGI() << "=== Started MuseScore " << framework::Version::fullVersion() << ", build number " << BUILD_NUMBER << " ===";

    //! --- Setup profiler ---
    using namespace haw::profiler;
    struct MyPrinter : public Profiler::Printer
    {
        void printDebug(const std::string& str) override { LOG_STREAM(Logger::DEBG, "Profiler", "")() << str; }
        void printInfo(const std::string& str) override { LOG_STREAM(Logger::INFO, "Profiler", "")() << str; }
    };

    Profiler::Options profOpt;
    profOpt.stepTimeEnabled = true;
    profOpt.funcsTimeEnabled = true;
    profOpt.funcsTraceEnabled = false;
    profOpt.funcsMaxThreadCount = 100;
    profOpt.dataTopCount = 150;

    Profiler* profiler = Profiler::instance();
    profiler->setup(profOpt, new MyPrinter());

    //! --- Setup Invoker ---

    Invoker::setup();

    mu::async::onMainThreadInvoke([](const std::function<void()>& f, bool isAlwaysQueued) {
        s_asyncInvoker.invoke(f, isAlwaysQueued);
    });

    //! --- Diagnostics ---
    auto pr = ioc()->resolve<diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        pr->reg("appBinPath", s_globalConf->appBinPath());
        pr->reg("appDataPath", s_globalConf->appDataPath());
        pr->reg("appConfigPath", s_globalConf->appConfigPath());
        pr->reg("userAppDataPath", s_globalConf->userAppDataPath());
        pr->reg("userBackupPath", s_globalConf->userBackupPath());
        pr->reg("userDataPath", s_globalConf->userDataPath());
        pr->reg("log file", logFile->filePath());
        pr->reg("settings file", settings()->filePath());
    }
}
