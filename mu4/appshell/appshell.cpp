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
#include <iostream>

#include "log.h"
#include "modularity/ioc.h"
#include "ui/internal/uiengine.h"
#include "settings.h"
#include "version.h"
#include "config.h"

using namespace mu::appshell;

AppShell::AppShell()
{
}

int AppShell::run(int argc, char** argv, std::function<void()> moduleSetup)
{
    std::cout << "begin AppShell::run\n";
    qputenv("QT_STYLE_OVERRIDE", "Fusion");
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    const char* appName;
    if (framework::Version::unstable()) {
        appName  = "MuseScore4Development";
    } else {
        appName  = "MuseScore4";
    }

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(appName);
    QCoreApplication::setOrganizationName("MuseScore");
    QCoreApplication::setOrganizationDomain("musescore.org");
    QCoreApplication::setApplicationVersion(QString::fromStdString(framework::Version::fullVersion()));

    qSetMessagePattern("%{function}: %{message}");

    moduleSetup();

    framework::settings()->load();

    QQmlApplicationEngine* engine = new QQmlApplicationEngine();
    //! NOTE Move ownership to UiEngine
    framework::UiEngine::instance()->moveQQmlEngine(engine);

#ifdef QML_LOAD_FROM_SOURCE
    QUrl url(QString(appshell_QML_IMPORT) + "/Main.qml");
#else
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
#endif

    QObject::connect(engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject* obj, const QUrl& objUrl) {
        if (!obj && url == objUrl) {
            std::cout << "failed Qml load\n";
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);

    QObject::connect(engine, &QQmlEngine::warnings, [](const QList<QQmlError>& warnings) {
        for (const QQmlError& e : warnings) {
            std::cout << "error: " << e.toString().toStdString() << "\n";
        }
    });

    engine->load(url);

    return app.exec();
}
