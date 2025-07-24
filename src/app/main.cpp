/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include <csignal>

#include <QTextCodec>
#include <QApplication>
#include <QStyleHints>
#include <QQuickWindow>

#include "appfactory.h"
#include "internal/commandlineparser.h"
#include "global/iapplication.h"

#include "muse_framework_config.h"
#include "app_config.h"

#include "log.h"

#if (defined (_MSCVER) || defined (_MSC_VER))
#include <vector>
#include <algorithm>
#include <windows.h>
#include <shellapi.h>
#endif

#ifndef MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT
static void crashCallback(int signum)
{
    const char* signame = "UNKNOWN SIGNAME";
    const char* sigdescript = "";
    switch (signum) {
    case SIGILL:
        signame = "SIGILL";
        sigdescript = "Illegal Instruction";
        break;
    case SIGSEGV:
        signame = "SIGSEGV";
        sigdescript =  "Invalid memory reference";
        break;
    }
    LOGE() << "Oops! Application crashed with signal: [" << signum << "] " << signame << "-" << sigdescript;
    exit(EXIT_FAILURE);
}

#endif

static void app_init_qrc()
{
    Q_INIT_RESOURCE(app);

#ifdef Q_OS_WIN
    Q_INIT_RESOURCE(app_win);
#endif
}

int main(int argc, char** argv)
{
#ifndef MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT
    signal(SIGSEGV, crashCallback);
    signal(SIGILL, crashCallback);
    signal(SIGFPE, crashCallback);
#endif

    // ====================================================
    // Setup global Qt application variables
    // ====================================================

    // Force the 8-bit text encoding to UTF-8. This is the default encoding on all supported platforms except for MSVC under Windows, which
    // would otherwise default to the local ANSI code page and cause corruption of any non-ANSI Unicode characters in command-line arguments.
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    app_init_qrc();

    qputenv("QT_STYLE_OVERRIDE", "Fusion");
    qputenv("QML_DISABLE_DISK_CACHE", "true");

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    if (!qEnvironmentVariableIsSet("QT_QUICK_FLICKABLE_WHEEL_DECELERATION")) {
        qputenv("QT_QUICK_FLICKABLE_WHEEL_DECELERATION", "5000");
    }
#endif

#ifdef Q_OS_LINUX
    if (qEnvironmentVariable("QT_QPA_PLATFORM") != "offscreen") {
        qputenv("QT_QPA_PLATFORMTHEME", "gtk3");
    }

    //! NOTE Forced X11, with Wayland there are a number of problems now
    if (qEnvironmentVariable("QT_QPA_PLATFORM") == "") {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }
#endif

#ifdef Q_OS_WIN
    // NOTE: There are some problems with rendering the application window on some integrated graphics processors
    //       see https://github.com/musescore/MuseScore/issues/8270
    if (!qEnvironmentVariableIsSet("QT_OPENGL_BUGLIST")) {
        qputenv("QT_OPENGL_BUGLIST", ":/resources/win_opengl_buglist.json");
    }
#endif

//! NOTE: For unknown reasons, Linux scaling for 1 is defined as 1.003 in fractional scaling.
//!       Because of this, some elements are drawn with a shift on the score.
//!       Let's make a Linux hack and round values above 0.75(see RoundPreferFloor)
#ifdef Q_OS_LINUX
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
#elif defined(Q_OS_WIN)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QGuiApplication::styleHints()->setMousePressAndHoldInterval(250);

// Can't use MUSE_APP_TITLE until next major release, because this "application name" is used to determine
// where user settings are stored. Changing it would result in all user settings being lost.
#ifdef MUSE_APP_UNSTABLE
    QCoreApplication::setApplicationName("MuseScore4Development");
#else
    QCoreApplication::setApplicationName("MuseScore4");
#endif
    QCoreApplication::setOrganizationName("MuseScore");
    QCoreApplication::setOrganizationDomain("musescore.org");
    QCoreApplication::setApplicationVersion(MUSE_APP_VERSION);

#if !defined(Q_OS_WIN) && !defined(Q_OS_DARWIN) && !defined(Q_OS_WASM)
    // Any OS that uses Freedesktop.org Desktop Entry Specification (e.g. Linux, BSD)
#ifndef MUSE_APP_INSTALL_SUFFIX
#define MUSE_APP_INSTALL_SUFFIX ""
#endif
    QGuiApplication::setDesktopFileName("org.musescore.MuseScore" MUSE_APP_INSTALL_SUFFIX);
#endif

#if (defined (_MSCVER) || defined (_MSC_VER))
    // // On MSVC under Windows, we need to manually retrieve the command-line arguments and convert them from UTF-16 to UTF-8.
    // // This prevents data loss if there are any characters that wouldn't fit in the local ANSI code page.

    auto utf8_encode = [](const wchar_t* wstr) -> std::string
    {
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], -1, 0, 0, 0, 0);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], -1, &strTo[0], size_needed, 0, 0);
        return strTo;
    };

    int argc_utf16 = 0;
    wchar_t** argv_utf16 = CommandLineToArgvW(GetCommandLineW(), &argc_utf16);
    std::vector<std::string> argsUtf8; // store data
    for (int i = 0; i < argc_utf16; ++i) {
        argsUtf8.push_back(utf8_encode(argv_utf16[i]));
    }

    std::vector<char*> argsUtf8_с; // convert to char*
    for (std::string& arg : argsUtf8) {
        argsUtf8_с.push_back(arg.data());
    }

    // Don't use the arguments passed to main(), because they're in the local ANSI code page.
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    int argcFinal = argc_utf16;
    char** argvFinal = argsUtf8_с.data();
#else

    int argcFinal = argc;
    char** argvFinal = argv;

#endif

    using namespace muse;
    using namespace mu::app;

    // ====================================================
    // Parse command line options
    // ====================================================
#ifdef MUE_ENABLE_CONSOLEAPP
    CommandLineParser commandLineParser;
    commandLineParser.init();
    commandLineParser.parse(argcFinal, argvFinal);

    IApplication::RunMode runMode = commandLineParser.runMode();
    QCoreApplication* qapp = nullptr;

    if (runMode == IApplication::RunMode::AudioPluginRegistration) {
        qapp = new QCoreApplication(argcFinal, argvFinal);
    } else {
        qapp = new QApplication(argcFinal, argvFinal);
    }

    commandLineParser.processBuiltinArgs(*qapp);
    CmdOptions opt = commandLineParser.options();

#else
    QCoreApplication* qapp = new QApplication(argcFinal, argvFinal);
    CmdOptions opt;
    opt.runMode = IApplication::RunMode::GuiApp;
#endif

    AppFactory f;
    std::shared_ptr<muse::IApplication> app = f.newApp(opt);

    app->perform();

    // ====================================================
    // Run main loop
    // ====================================================
    int code = qapp->exec();

    // ====================================================
    // Quit
    // ====================================================

    app->finish();

    delete qapp;

    LOGI() << "Goodbye!! code: " << code;
    return code;
}
