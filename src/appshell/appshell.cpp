//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#include "appshell.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#ifndef Q_OS_WASM
#include <QThreadPool>
#endif
#ifdef KDAB_DOCKWIDGETS
#include "kddockwidgets/Config.h"
#endif
#include "log.h"
#include "modularity/ioc.h"
#include "ui/internal/uiengine.h"
#include "version.h"
#include "config.h"

#include "commandlinecontroller.h"

#include "framework/global/globalmodule.h"

using namespace mu::appshell;

//! NOTE Separately to initialize logger and profiler as early as possible
static mu::framework::GlobalModule globalModule;

AppShell::AppShell()
{
}

void AppShell::addModule(mu::framework::IModuleSetup* module)
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

    const char* appName;
    if (framework::Version::unstable()) {
        appName  = "MuseScore4Development";
    } else {
        appName  = "MuseScore4";
    }

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(appName);
    QCoreApplication::setOrganizationName("MuseScore");
    QCoreApplication::setOrganizationDomain("musescore.org");
    QCoreApplication::setApplicationVersion(QString::fromStdString(framework::Version::fullVersion()));

    // ====================================================
    // Setup modules: Resources, Exports, Imports, UiTypes
    // ====================================================
    globalModule.registerResources();
    globalModule.registerExports();
    globalModule.registerUiTypes();

    for (mu::framework::IModuleSetup* m : m_modules) {
        m->registerResources();
    }

    for (mu::framework::IModuleSetup* m : m_modules) {
        m->registerExports();
    }

    globalModule.resolveImports();
    for (mu::framework::IModuleSetup* m : m_modules) {
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
    // Setup modules: onInit
    // ====================================================
    globalModule.onInit(runMode);
    for (mu::framework::IModuleSetup* m : m_modules) {
        m->onInit(runMode);
    }

    // ====================================================
    // Setup modules: onStartApp (on next event loop)
    // ====================================================
    QMetaObject::invokeMethod(qApp, [this]() {
        globalModule.onStartApp();
        for (mu::framework::IModuleSetup* m : m_modules) {
            m->onStartApp();
        }
    }, Qt::QueuedConnection);

    // ====================================================
    // Run
    // ====================================================

    switch (runMode) {
    case framework::IApplication::RunMode::Converter: {
        // ====================================================
        // Process Converter
        // ====================================================
        auto task = commandLine.converterTask();
        QMetaObject::invokeMethod(qApp, [this, task]() {
                int code = processConverter(task);
                qApp->exit(code);
            }, Qt::QueuedConnection);
    } break;
    case framework::IApplication::RunMode::Editor: {
        // ====================================================
        // Setup Qml Engine
        // ====================================================
        QQmlApplicationEngine* engine = new QQmlApplicationEngine();

#ifdef KDAB_DOCKWIDGETS
        KDDockWidgets::Config::self().setQmlEngine(engine);
        const QString mainQmlFile = "/Main.KDAB.qml";
#elif Q_OS_WASM
        const QString mainQmlFile = "/Main.wasm.qml";
#else
        const QString mainQmlFile = "/Main.qml";
#endif
        //! NOTE Move ownership to UiEngine
        ui::UiEngine::instance()->moveQQmlEngine(engine);

#ifdef QML_LOAD_FROM_SOURCE
        const QUrl url(QString(appshell_QML_IMPORT) + mainQmlFile);
#else
        const QUrl url(QStringLiteral("qrc:/qml") + mainQmlFile);
#endif

        QObject::connect(engine, &QQmlApplicationEngine::objectCreated,
                         &app, [url](QObject* obj, const QUrl& objUrl) {
                if (!obj && url == objUrl) {
                    LOGE() << "failed Qml load\n";
                    QCoreApplication::exit(-1);
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
    }
    }

    // ====================================================
    // Run main loop
    // ====================================================
    int retCode = app.exec();

    // ====================================================
    // Quit
    // ====================================================

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
    for (mu::framework::IModuleSetup* m : m_modules) {
        m->onDeinit();
    }

    PROFILER_PRINT;

    globalModule.onDeinit();

    return retCode;
}

int AppShell::processConverter(const CommandLineController::ConverterTask& task)
{
    Ret ret;
    if (task.isBatchMode) {
        ret = converter()->batchConvert(task.inputFile);
        if (!ret) {
            LOGE() << "failed batch convert, error: " << ret.toString();
        }
    } else {
        ret = converter()->fileConvert(task.inputFile, task.outputFile);
        if (!ret) {
            LOGE() << "failed file convert, error: " << ret.toString();
        }
    }

    return ret.code();
}
