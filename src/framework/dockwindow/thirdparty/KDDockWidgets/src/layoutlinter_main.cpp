/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Config.h"
#include "qtwidgets/ViewFactory.h"

#include "core/MainWindow.h"
#include "core/DockWidget.h"
#include "core/Platform.h"

#include <QDebug>
#include <QString>
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QTimer>

#include "nlohmann/json.hpp"

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

static bool s_isVerbose = false;

struct LinterConfig
{
    struct MainWindow
    {
        std::string name;
        MainWindowOptions options = {};
        bool maximize = false;
    };

    RestoreOptions restoreOptions = {};
    std::vector<std::string> filesToLint;
    std::vector<MainWindow> mainWindows;

    bool isEmpty() const
    {
        return filesToLint.empty();
    }
};

inline void from_json(const nlohmann::json &j, LinterConfig::MainWindow &mw)
{
    mw.name = j["name"];
    mw.options = MainWindowOptions(j.value("options", 0));
    mw.maximize = j.value("maximize", false);
}

inline void from_json(const nlohmann::json &j, LinterConfig &ls)
{
    ls.filesToLint = j.value("files", std::vector<std::string>());
    ls.mainWindows = j.value("mainWindows", std::vector<LinterConfig::MainWindow>());
    ls.restoreOptions = RestoreOptions(j.value("restoreOptions", int(RestoreOption::RestoreOption_None)));
}

LinterConfig requestedLinterConfig(const QCommandLineParser &parser, const QString &configFile)
{
    LinterConfig c;

    const auto positionalArguments = parser.positionalArguments();
    if (configFile.isEmpty() && positionalArguments.isEmpty()) {
        qWarning() << "Expected either a config file or positional arguments";
        return {};
    } else if (!configFile.isEmpty() && !positionalArguments.isEmpty()) {
        qWarning() << "Expected either a config file or positional arguments, not both";
        return {};
    } else if (!positionalArguments.isEmpty()) {
        std::transform(positionalArguments.begin(), positionalArguments.end(), std::back_inserter(c.filesToLint),
                       [](const QString &str) {
                           return str.toStdString();
                       });
    } else {
        QFile f(configFile);
        if (!f.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to open" << configFile;
            return {};
        }
        const QByteArray jsonData = f.readAll();
        QDir::setCurrent(QFileInfo(configFile).path());
        nlohmann::json j = nlohmann::json::parse(jsonData, nullptr, /*allow_exceptions=*/false);
        from_json(j, c);
    }

    return c;
}

static bool lint(const QString &filename, LinterConfig config, bool isVerbose)
{
    if (isVerbose) {
        qDebug() << "Linting" << filename << "with options" << config.restoreOptions;
    }

    DockWidgetFactoryFunc dwFunc = [](const QString &dwName) {
        return Config::self().viewFactory()->createDockWidget(dwName)->asDockWidgetController();
    };

    /// MainWindow factory for the easy cases.
    MainWindowFactoryFunc mwFunc = [](const QString &mwName, MainWindowOptions mainWindowOptions) {
        return Platform::instance()->createMainWindow(mwName, {}, mainWindowOptions);
    };

    KDDockWidgets::Config::self().setDockWidgetFactoryFunc(dwFunc);
    KDDockWidgets::Config::self().setMainWindowFactoryFunc(mwFunc);

    // Create the main windows specified from -c <file>
    for (auto mw : config.mainWindows) {
        const QString name = QString::fromStdString(mw.name);
        if (isVerbose)
            qDebug() << "Pre-creating main window" << name << "with options" << mw.options;

        auto mainWindow = Platform::instance()->createMainWindow(name, {}, mw.options);
        if (mw.maximize)
            mainWindow->view()->showMaximized();
        else
            mainWindow->view()->show();
    }

    LayoutSaver restorer(config.restoreOptions);
    return restorer.restoreFromFile(filename);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const auto frontends = Platform::frontendTypes();
    if (frontends.empty()) {
        qWarning() << "Error: Your KDDockWidgets installation doesn't support any frontend!";
        return -1;
    }

    QCommandLineParser parser;
    parser.setApplicationDescription("KDDockWidgets layout linter");
    QCommandLineOption forceQtQuick = { { "q", "force-qtquick" }, "Forces usage of QtQuick" };
    QCommandLineOption configFileOpt = { { "c", "config" }, "Linter config file", "configfile" };
    QCommandLineOption verboseOpt = { { "v", "verbose" }, "Verbose output" };
    QCommandLineOption strictOpt = { { "s", "strict" }, "Strict mode" };
    QCommandLineOption waitAtEndOpt = { { "w", "wait" }, "Waits instead of exiting. For debugging purposes." };

    parser.addOption(configFileOpt);
    parser.addOption(verboseOpt);
    parser.addOption(waitAtEndOpt);
    parser.addOption(strictOpt);
    parser.addPositionalArgument("layout", "layout json file");
    parser.addHelpOption();

    FrontendType frontendType = FrontendType::QtWidgets;

#if defined(KDDW_FRONTEND_QTQUICK)
#if defined(KDDW_FRONTEND_QTWIDGETS)
    // There's both frontends, so add a switch to chose QtQuick
    parser.addOption(forceQtQuick);
#else
    // There's only QtQuick
    frontendType = FrontendType::QtQuick;
#endif
#endif

    parser.process(*qApp);

#if defined(KDDW_FRONTEND_QTQUICK) && defined(KDDW_FRONTEND_QTWIDGETS)
    if (parser.isSet(forceQtQuick))
        frontendType = FrontendType::QtQuick;
#endif

    KDDockWidgets::initFrontend(frontendType);
    KDDockWidgets::Config::self().setLayoutSaverStrictMode(parser.isSet(strictOpt));

    s_isVerbose = parser.isSet(verboseOpt);
    const LinterConfig lc = requestedLinterConfig(parser, parser.isSet(configFileOpt) ? parser.value(configFileOpt) : QString());
    if (lc.isEmpty()) {
        qWarning() << "Bailing out";
        return 3;
    }

    int exitCode = 0;
    for (const std::string &layout : lc.filesToLint) {
        if (!lint(QString::fromStdString(layout), lc, s_isVerbose))
            exitCode = 2;
    }

    if (s_isVerbose) {
        if (exitCode == 0) {
            qDebug() << "Success";
        } else {
            qDebug() << "Error";
        }
    }

    if (parser.isSet(waitAtEndOpt)) {
        // For debugging inspection purposes.

        QTimer::singleShot(1000, [] {
            LayoutSaver saver;
            const QByteArray saved = saver.serializeLayout();
            if (s_isVerbose)
                qDebug() << "Testing if serialize works" << !saved.isEmpty();
        });

        app.exec();
    }

    return exitCode;
}
