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

#include "muse_framework_config.h"

#include "modularity/ioc.h"
#include "internal/globalconfiguration.h"

#include "logger.h"
#include "logremover.h"
#include "profiler.h"

#include "internal/baseapplication.h"
#include "internal/invoker.h"
#include "internal/cryptographichash.h"
#include "internal/process.h"
#include "internal/systeminfo.h"

#ifdef MUSE_MODULE_UI
#include "internal/interactive.h"
#endif

#include "runtime.h"
#include "async/processevents.h"

#include "settings.h"

#include "io/internal/filesystem.h"

#include "api/internal/apiregister.h"
#include "api/iapiregister.h"
#include "api/logapi.h"
#include "api/interactiveapi.h"
#include "api/filesystemapi.h"
#include "api/processapi.h"

#ifdef MUSE_MODULE_DIAGNOSTICS
#include "diagnostics/idiagnosticspathsregister.h"
#endif

#ifdef Q_OS_WIN
#include "platform/win/waitabletimer.h"
#endif

#include "log.h"

using namespace muse;
using namespace muse::modularity;
using namespace muse::io;

std::shared_ptr<Invoker> GlobalModule::s_asyncInvoker = {};

class ApplicationStub : public BaseApplication
{
public:

    ApplicationStub()
        : BaseApplication(std::make_shared<modularity::Context>()) {}

    void perform() override {}
    void finish() override {}
};

GlobalModule::GlobalModule()
{
}

std::string GlobalModule::moduleName() const
{
    return "global";
}

void GlobalModule::registerExports()
{
    if (!m_application) {
        m_application = std::make_shared<ApplicationStub>();
    }

    m_configuration = std::make_shared<GlobalConfiguration>(iocContext());
    s_asyncInvoker = std::make_shared<Invoker>();
    m_systemInfo = std::make_shared<SystemInfo>();

    ioc()->registerExport<IApplication>(moduleName(), m_application);
    ioc()->registerExport<IGlobalConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<ISystemInfo>(moduleName(), m_systemInfo);
    ioc()->registerExport<IFileSystem>(moduleName(), new FileSystem());
    ioc()->registerExport<ICryptographicHash>(moduleName(), new CryptographicHash());
    ioc()->registerExport<IProcess>(moduleName(), new Process());
    ioc()->registerExport<api::IApiRegister>(moduleName(), new api::ApiRegister());

#ifdef MUSE_MODULE_UI
    ioc()->registerExport<IInteractive>(moduleName(), new Interactive(iocContext()));
#endif
}

void GlobalModule::registerApi()
{
    using namespace muse::api;

    auto api = ioc()->resolve<IApiRegister>(moduleName());
    if (api) {
        api->regApiCreator(moduleName(), "api.log", new ApiCreator<LogApi>());
        api->regApiCreator(moduleName(), "api.interactive", new api::ApiCreator<InteractiveApi>());
        api->regApiCreator(moduleName(), "api.process", new ApiCreator<ProcessApi>());
        api->regApiCreator(moduleName(), "api.filesystem", new ApiCreator<FileSystemApi>());
    }
}

void GlobalModule::onPreInit(const IApplication::RunMode& mode)
{
    muse::runtime::mainThreadId(); //! NOTE Needs only call
    muse::runtime::setThreadName("main");

    //! NOTE: settings must be inited before initialization of any module
    //! because modules can use settings at the moment of their initialization
    settings()->load();

    //! --- Setup logger ---
    using namespace muse::logger;
    Logger* logger = Logger::instance();
    logger->clearDests();

    //! Console
    if (mode == IApplication::RunMode::GuiApp || muse::runtime::isDebug()) {
        logger->addDest(new ConsoleLogDest(LogLayout("${time} | ${type|5} | ${thread|15} | ${tag|15} | ${message}")));
    }

    //! Log file
    io::path_t logFilePath = "none";
#ifndef Q_OS_WASM
    io::path_t logPath = m_configuration->userAppDataPath() + "/logs";
    fileSystem()->makePath(logPath);

    logFilePath = logPath;
    String logFileNamePattern;

    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        logFileNamePattern = u"audiopluginregistration_yyMMdd.log";

        //! This creates a file named "data/logs/audiopluginregistration_yyMMdd.log"
        logFilePath += "/audiopluginregistration_"
                       + QDateTime::currentDateTime().toString("yyMMdd")
                       + ".log";
    } else {
        logFileNamePattern = BaseApplication::appName() + u"_yyMMdd_HHmmss.log";

        //! This creates a file named "data/logs/AppName_yyMMdd_HHmmss.log"
        logFilePath += u"/" + BaseApplication::appName() + u"_"
                       + String::fromQString(QDateTime::currentDateTime().toString("yyMMdd_HHmmss"))
                       + u".log";
    }

    //! Remove old logs
    LogRemover::removeLogs(logPath, 7, logFileNamePattern);

    FileLogDest* logFile = new FileLogDest(logFilePath.toStdString(),
                                           LogLayout("${datetime} | ${type|5} | ${thread|15} | ${tag|15} | ${message}"));

    logger->addDest(logFile);
    LOGI() << "log path: " << logFilePath;
#endif

    if (m_loggerLevel) {
        logger->setLevel(m_loggerLevel.value());
    } else {
#ifdef MUSE_MODULE_GLOBAL_LOGGER_DEBUGLEVEL
        logger->setLevel(muse::logger::Level::Debug);
#else
        logger->setLevel(muse::logger::Level::Normal);
#endif
    }

    LOGI() << "=== Started " << m_application->title()
           << " " << m_application->fullVersion().toString()
           << ", build: " << m_application->build() << " ===";

    //! --- Setup profiler ---
    using namespace muse::profiler;
    struct MyPrinter : public Profiler::Printer
    {
        void printDebug(const std::string& str) override { LOG_STREAM(Logger::DEBG, "Profiler", Color::Magenta)() << str; }
        void printInfo(const std::string& str) override { LOG_STREAM(Logger::INFO, "Profiler", Color::Magenta)() << str; }
    };

    Profiler::Options profOpt;
    profOpt.stepTimeEnabled = true;
    profOpt.timersEnabled = true;
    profOpt.funcsTimeEnabled = true;
    profOpt.funcsTraceEnabled = false;
    profOpt.funcsMaxThreadCount = 100;
    profOpt.statTopCount = 150;

    Profiler* profiler = Profiler::instance();
    profiler->setup(profOpt, new MyPrinter());

    //! --- Setup Invoker ---

    Invoker::setup();

    async::onMainThreadInvoke([](const std::function<void()>& f, bool isAlwaysQueued) {
        s_asyncInvoker->invoke(f, isAlwaysQueued);
    });

    //! --- Diagnostics ---
#ifdef MUSE_MODULE_DIAGNOSTICS
    auto pr = ioc()->resolve<muse::diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        pr->reg("appBinPath", m_configuration->appBinPath());
        pr->reg("appBinDirPath", m_configuration->appBinDirPath());
        pr->reg("appDataPath", m_configuration->appDataPath());
        pr->reg("appConfigPath", m_configuration->appConfigPath());
        pr->reg("userAppDataPath", m_configuration->userAppDataPath());
        pr->reg("userBackupPath", m_configuration->userBackupPath());
        pr->reg("userDataPath", m_configuration->userDataPath());
        pr->reg("log file", logFilePath);
        pr->reg("settings file", settings()->filePath());
    }
#endif
}

void GlobalModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
    m_systemInfo->init();

    m_endTimePeriod = false;

#ifdef Q_OS_WIN
    WaitableTimer timer;
    if (m_configuration->highResolutionTimers() || !timer.init()) {
        // Improves the accuracy of Sleep() on Windows
        // Without it, errors up to 15 ms are possible, which is critical for the audio thread
        // and can significantly degrade sound quality
        // However, this approach may result in higher CPU usage as a trade-off
        LOGI() << "Use timeBeginPeriod(1)";
        timeBeginPeriod(1);
        m_endTimePeriod = true;
    }
#endif
}

void GlobalModule::onDeinit()
{
    invokeQueuedCalls();

#ifdef Q_OS_WIN
    if (m_endTimePeriod) {
        timeEndPeriod(1);
    }
#endif
}

void GlobalModule::invokeQueuedCalls()
{
    s_asyncInvoker->invokeQueuedCalls();
}

void GlobalModule::setLoggerLevel(const muse::logger::Level& level)
{
    m_loggerLevel = level;
}
