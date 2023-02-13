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

#include "appshell.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QStyleHints>
#ifndef Q_OS_WASM
#include <QThreadPool>
#endif

#include "view/internal/splashscreen.h"

#include "view/dockwindow/docksetup.h"

#include "modularity/ioc.h"
#include "ui/internal/uiengine.h"
#include "muversion.h"

#include "commandlinecontroller.h"

#include "framework/global/globalmodule.h"

#include "log.h"

using namespace mu::appshell;

//! NOTE Separately to initialize logger and profiler as early as possible
static mu::framework::GlobalModule globalModule;

AppShell::AppShell()
{
}

void AppShell::addModule(modularity::IModuleSetup* module)
{
    m_modules.push_back(module);
}

int AppShell::run(int argc, char** argv)
{
    // ====================================================
    // Setup global Qt application variables
    // ====================================================
    qputenv("QT_STYLE_OVERRIDE", "Fusion");
    qputenv("QML_DISABLE_DISK_CACHE", "true");

#ifdef Q_OS_LINUX
    if (qEnvironmentVariable("QT_QPA_PLATFORM") != "offscreen") {
        qputenv("QT_QPA_PLATFORMTHEME", "gtk3");
    }
#endif

    const char* appName;
    if (framework::MUVersion::unstable()) {
        appName  = "MuseScore4Development";
    } else {
        appName  = "MuseScore4";
    }

#ifdef Q_OS_WIN
    // NOTE: There are some problems with rendering the application window on some integrated graphics processors
    //       see https://github.com/musescore/MuseScore/issues/8270
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

    if (!qEnvironmentVariableIsSet("QT_OPENGL_BUGLIST")) {
        qputenv("QT_OPENGL_BUGLIST", ":/resources/win_opengl_buglist.json");
    }
#endif

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    //! NOTE: For unknown reasons, Linux scaling for 1 is defined as 1.003 in fractional scaling.
    //!       Because of this, some elements are drawn with a shift on the score.
    //!       Let's make a Linux hack and round values above 0.75(see RoundPreferFloor)
#ifdef Q_OS_LINUX
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
#elif defined(Q_OS_WIN)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QGuiApplication::styleHints()->setMousePressAndHoldInterval(250);

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(appName);
    QCoreApplication::setOrganizationName("MuseScore");
    QCoreApplication::setOrganizationDomain("musescore.org");
    QCoreApplication::setApplicationVersion(QString::fromStdString(framework::MUVersion::fullVersion().toStdString()));

#if !defined(Q_OS_WIN) && !defined(Q_OS_DARWIN) && !defined(Q_OS_WASM)
    // Any OS that uses Freedesktop.org Desktop Entry Specification (e.g. Linux, BSD)
    QGuiApplication::setDesktopFileName("org.musescore.MuseScore" MUSESCORE_INSTALL_SUFFIX ".desktop");
#endif

    // ====================================================
    // Setup modules: Resources, Exports, Imports, UiTypes
    // ====================================================
    globalModule.registerResources();
    globalModule.registerExports();
    globalModule.registerUiTypes();

    for (mu::modularity::IModuleSetup* m : m_modules) {
        m->registerResources();
    }

    for (mu::modularity::IModuleSetup* m : m_modules) {
        m->registerExports();
    }

    globalModule.resolveImports();
    for (mu::modularity::IModuleSetup* m : m_modules) {
        m->registerUiTypes();
        m->resolveImports();
    }

    // ====================================================
    // Parse and apply command line options
    // ====================================================
    CommandLineController commandLine;
    commandLine.parse(QCoreApplication::arguments());
    commandLine.apply();
    framework::IApplication::RunMode runMode = muapplication()->runMode();

    // ====================================================
    // Setup modules: onPreInit
    // ====================================================
    globalModule.onPreInit(runMode);
    for (mu::modularity::IModuleSetup* m : m_modules) {
        m->onPreInit(runMode);
    }

    SplashScreen* splashScreen = nullptr;
    if (runMode == framework::IApplication::RunMode::Editor) {
        splashScreen = new SplashScreen();
        splashScreen->show();
    }

    // ====================================================
    // Setup modules: onInit
    // ====================================================
    globalModule.onInit(runMode);
    for (mu::modularity::IModuleSetup* m : m_modules) {
        m->onInit(runMode);
    }

    // ====================================================
    // Setup modules: onAllInited
    // ====================================================
    globalModule.onAllInited(runMode);
    for (mu::modularity::IModuleSetup* m : m_modules) {
        m->onAllInited(runMode);
    }

    // ====================================================
    // Setup modules: onStartApp (on next event loop)
    // ====================================================
    QMetaObject::invokeMethod(qApp, [this]() {
        globalModule.onStartApp();
        for (mu::modularity::IModuleSetup* m : m_modules) {
            m->onStartApp();
        }
    }, Qt::QueuedConnection);

    // ====================================================
    // Run
    // ====================================================

    switch (runMode) {
    case framework::IApplication::RunMode::Converter: {
        // ====================================================
        // Process Autobot
        // ====================================================
        CommandLineController::Autobot autobot = commandLine.autobot();
        if (!autobot.testCaseNameOrFile.isEmpty()) {
            QMetaObject::invokeMethod(qApp, [this, autobot]() {
                    processAutobot(autobot);
                }, Qt::QueuedConnection);
        } else {
            // ====================================================
            // Process Diagnostic
            // ====================================================
            CommandLineController::Diagnostic diagnostic = commandLine.diagnostic();
            if (diagnostic.type != CommandLineController::DiagnosticType::Undefined) {
                QMetaObject::invokeMethod(qApp, [this, diagnostic]() {
                        int code = processDiagnostic(diagnostic);
                        qApp->exit(code);
                    }, Qt::QueuedConnection);
            } else {
                // ====================================================
                // Process Converter
                // ====================================================
                CommandLineController::ConverterTask task = commandLine.converterTask();
                QMetaObject::invokeMethod(qApp, [this, task]() {
                        int code = processConverter(task);
                        qApp->exit(code);
                    }, Qt::QueuedConnection);
            }
        }
    } break;
    case framework::IApplication::RunMode::Editor: {
        // ====================================================
        // Setup Qml Engine
        // ====================================================
        QQmlApplicationEngine* engine = new QQmlApplicationEngine();

        dock::DockSetup::setup(engine);

#if defined(Q_OS_WIN)
        const QString mainQmlFile = "/platform/win/Main.qml";
#elif defined(Q_OS_MACOS)
        const QString mainQmlFile = "/platform/mac/Main.qml";
#elif defined(Q_OS_LINUX)
        const QString mainQmlFile = "/platform/linux/Main.qml";
#elif defined(Q_OS_WASM)
        const QString mainQmlFile = "/Main.wasm.qml";
#endif
        //! NOTE Move ownership to UiEngine
        ui::UiEngine::instance()->moveQQmlEngine(engine);

#ifdef MUE_ENABLE_LOAD_QML_FROM_SOURCE
        const QUrl url(QString(appshell_QML_IMPORT) + mainQmlFile);
#else
        const QUrl url(QStringLiteral("qrc:/qml") + mainQmlFile);
#endif

        QObject::connect(engine, &QQmlApplicationEngine::objectCreated,
                         &app, [this, url](QObject* obj, const QUrl& objUrl) {
                if (!obj && url == objUrl) {
                    LOGE() << "failed Qml load\n";
                    QCoreApplication::exit(-1);
                }

                if (url == objUrl) {
                    // ====================================================
                    // Setup modules: onDelayedInit
                    // ====================================================

                    globalModule.onDelayedInit();
                    for (mu::modularity::IModuleSetup* m : m_modules) {
                        m->onDelayedInit();
                    }
                }
            }, Qt::QueuedConnection);

        QObject::connect(engine, &QQmlEngine::warnings, [](const QList<QQmlError>& warnings) {
                for (const QQmlError& e : warnings) {
                    LOGE() << "error: " << e.toString().toStdString() << "\n";
                }
            });

        // ====================================================
        // Load Main qml
        // ====================================================

        //! Needs to be set because we use transparent windows for PopupView.
        //! Needs to be called before any QQuickWindows are shown.
        QQuickWindow::setDefaultAlphaBuffer(true);

        engine->load(url);

        if (splashScreen) {
            splashScreen->close();
            delete splashScreen;
        }
    }
    }

    // ====================================================
    // Run main loop
    // ====================================================
    int retCode = app.exec();

    // ====================================================
    // Quit
    // ====================================================

    PROFILER_PRINT;

    // Wait Thread Poll
#ifndef Q_OS_WASM
    QThreadPool* globalThreadPool = QThreadPool::globalInstance();
    if (globalThreadPool) {
        LOGI() << "activeThreadCount: " << globalThreadPool->activeThreadCount();
        globalThreadPool->waitForDone();
    }
#endif
    // Engine quit
    ui::UiEngine::instance()->quit();

    // Deinit

    for (mu::modularity::IModuleSetup* m : m_modules) {
        m->onDeinit();
    }

    globalModule.onDeinit();

    for (mu::modularity::IModuleSetup* m : m_modules) {
        m->onDestroy();
    }

    globalModule.onDestroy();

    // Delete modules
    qDeleteAll(m_modules);
    m_modules.clear();
    mu::modularity::ioc()->reset();

    return retCode;
}

int AppShell::processConverter(const CommandLineController::ConverterTask& task)
{
    Ret ret = make_ret(Ret::Code::Ok);
    io::path_t stylePath = task.params[CommandLineController::ParamKey::StylePath].toString();
    bool forceMode = task.params[CommandLineController::ParamKey::ForceMode].toBool();

    switch (task.type) {
    case CommandLineController::ConvertType::Batch:
        ret = converter()->batchConvert(task.inputFile, stylePath, forceMode);
        break;
    case CommandLineController::ConvertType::ConvertScoreParts:
        ret = converter()->convertScoreParts(task.inputFile, task.outputFile, stylePath);
        break;
    case CommandLineController::ConvertType::File:
        ret = converter()->fileConvert(task.inputFile, task.outputFile, stylePath, forceMode);
        break;
    case CommandLineController::ConvertType::ExportScoreMedia: {
        io::path_t highlightConfigPath = task.params[CommandLineController::ParamKey::HighlightConfigPath].toString();
        ret = converter()->exportScoreMedia(task.inputFile, task.outputFile, highlightConfigPath, stylePath, forceMode);
    } break;
    case CommandLineController::ConvertType::ExportScoreMeta:
        ret = converter()->exportScoreMeta(task.inputFile, task.outputFile, stylePath, forceMode);
        break;
    case CommandLineController::ConvertType::ExportScoreParts:
        ret = converter()->exportScoreParts(task.inputFile, task.outputFile, stylePath, forceMode);
        break;
    case CommandLineController::ConvertType::ExportScorePartsPdf:
        ret = converter()->exportScorePartsPdfs(task.inputFile, task.outputFile, stylePath, forceMode);
        break;
    case CommandLineController::ConvertType::ExportScoreTranspose: {
        std::string scoreTranspose = task.params[CommandLineController::ParamKey::ScoreTransposeOptions].toString().toStdString();
        ret = converter()->exportScoreTranspose(task.inputFile, task.outputFile, scoreTranspose, stylePath, forceMode);
    } break;
    case CommandLineController::ConvertType::ExportScoreVideo: {
        ret = converter()->exportScoreVideo(task.inputFile, task.outputFile);
    } break;
    case CommandLineController::ConvertType::SourceUpdate: {
        std::string scoreSource = task.params[CommandLineController::ParamKey::ScoreSource].toString().toStdString();
        ret = converter()->updateSource(task.inputFile, scoreSource, forceMode);
    } break;
    }

    if (!ret) {
        LOGE() << "failed convert, error: " << ret.toString();
    }

    return ret.code();
}

int AppShell::processDiagnostic(const CommandLineController::Diagnostic& task)
{
    if (!diagnosticDrawProvider()) {
        return make_ret(Ret::Code::NotSupported);
    }

    Ret ret = make_ret(Ret::Code::Ok);

    if (task.input.isEmpty()) {
        return make_ret(Ret::Code::UnknownError);
    }

    io::paths_t input;
    for (const QString& p : task.input) {
        input.push_back(p);
    }

    io::path_t output = task.output;

    if (output.empty()) {
        output = "./";
    }

    switch (task.type) {
    case CommandLineController::DiagnosticType::GenDrawData:
        ret = diagnosticDrawProvider()->generateDrawData(input.front(), output);
        break;
    case CommandLineController::DiagnosticType::ComDrawData:
        IF_ASSERT_FAILED(input.size() == 2) {
            return make_ret(Ret::Code::UnknownError);
        }
        ret = diagnosticDrawProvider()->compareDrawData(input.at(0), input.at(1), output);
        break;
    case CommandLineController::DiagnosticType::DrawDataToPng:
        ret = diagnosticDrawProvider()->drawDataToPng(input.front(), output);
        break;
    case CommandLineController::DiagnosticType::DrawDiffToPng: {
        io::path_t diffPath = input.at(0);
        io::path_t refPath;
        if (input.size() > 1) {
            refPath = input.at(1);
        }
        ret = diagnosticDrawProvider()->drawDiffToPng(diffPath, refPath, output);
    } break;
    default:
        break;
    }

    if (!ret) {
        LOGE() << "diagnostic ret: " << ret.toString();
    }

    return ret.code();
}

void AppShell::processAutobot(const CommandLineController::Autobot& task)
{
    using namespace mu::autobot;
    async::Channel<StepInfo, Ret> stepCh = autobot()->stepStatusChanged();
    stepCh.onReceive(nullptr, [](const StepInfo& step, const Ret& ret){
        if (!ret) {
            LOGE() << "failed step: " << step.name << ", ret: " << ret.toString();
            qApp->exit(ret.code());
        } else {
            LOGI() << "success step: " << step.name << ", ret: " << ret.toString();
        }
    });

    async::Channel<io::path_t, IAutobot::Status> statusCh = autobot()->statusChanged();
    statusCh.onReceive(nullptr, [](const io::path_t& path, IAutobot::Status st){
        if (st == IAutobot::Status::Finished) {
            LOGI() << "success finished, path: " << path;
            qApp->exit(0);
        }
    });

    IAutobot::Options opt;
    opt.context = task.testCaseContextNameOrFile;
    opt.contextVal = task.testCaseContextValue.toStdString();
    opt.func = task.testCaseFunc.toStdString();
    opt.funcArgs = task.testCaseFuncArgs.toStdString();

    autobot()->execScript(task.testCaseNameOrFile, opt);
}
