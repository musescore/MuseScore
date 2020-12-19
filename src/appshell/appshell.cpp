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
#include <QCommandLineParser>

#include "log.h"
#include "modularity/ioc.h"
#include "ui/internal/uiengine.h"
#include "version.h"
#include "config.h"

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
    globalModule.onInit();
    //! NOTE Now we can use logger and profiler

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
    QCommandLineParser parser;
    parseCommandLineArguments(parser);
    applyCommandLineArguments(parser);

    // ====================================================
    // Setup modules: onInit
    // ====================================================
    for (mu::framework::IModuleSetup* m : m_modules) {
        m->onInit();
    }

    // ====================================================
    // Setup Qml Engine
    // ====================================================
    QQmlApplicationEngine* engine = new QQmlApplicationEngine();
    //! NOTE Move ownership to UiEngine
    framework::UiEngine::instance()->moveQQmlEngine(engine);

#ifdef QML_LOAD_FROM_SOURCE
    const QUrl url(QString(appshell_QML_IMPORT) + "/Main.qml");
#else
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
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
    engine->load(url);

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
    // Run main loop
    // ====================================================
    int code = app.exec();

    // ====================================================
    // Quit and deinit
    // ====================================================
    framework::UiEngine::instance()->quit();

    for (mu::framework::IModuleSetup* m : m_modules) {
        m->onDeinit();
    }

    PROFILER_PRINT;

    globalModule.onDeinit();

    return code;
}

void AppShell::parseCommandLineArguments(QCommandLineParser& parser) const
{
    parser.addHelpOption(); // -?, -h, --help
    parser.addVersionOption(); // -v, --version

    parser.addOption(QCommandLineOption({ "j", "job" }, "Process a conversion job", "file"));
    //! NOTE Here will be added others options

    parser.process(QCoreApplication::arguments());
}

void AppShell::applyCommandLineArguments(QCommandLineParser& parser)
{
    QStringList options = parser.optionNames();
    qDebug() << "options: " << options;

    for (const QString& opt : options) {
        ICommandLineHandlerPtr h = clregister()->handler(opt.toStdString());
        if (h) {
            ICommandLineHandler::Values hvals;
            QStringList values = parser.values(opt);
            for (const QString& v : values) {
                hvals.push_back(v.toStdString());
            }

            h->exec(hvals);
        } else {
            LOGW() << "Not found command line handler for option: " << opt;
        }
    }
}
