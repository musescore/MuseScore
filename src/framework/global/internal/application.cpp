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
#include "application.h"

#ifndef NO_QT_SUPPORT
#include <QApplication>
#include <QProcess>
#endif

#include "types/version.h"

#include "muse_framework_config.h"

#include "log.h"

using namespace muse;

String Application::name() const
{
#ifdef MUSE_APP_TITLE
    return String::fromAscii(MUSE_APP_TITLE);
#else
    return String();
#endif
}

bool Application::unstable() const
{
#ifdef MUSE_APP_UNSTABLE
    return true;
#else
    return false;
#endif
}

Version Application::version() const
{
#ifdef MUSE_APP_VERSION
    static Version v(MUSE_APP_VERSION);
#else
    static Version v("0.0.0");
#endif
    return v;
}

Version Application::fullVersion() const
{
    static Version fv(version());

#ifdef MUSE_APP_VERSION_LABEL
    static bool once = false;
    if (!once) {
        String versionLabel = String::fromAscii(MUSE_APP_VERSION_LABEL);
        if (!versionLabel.isEmpty()) {
            fv.setSuffix(versionLabel);
        }
        once = true;
    }
#endif

    return fv;
}

String Application::build() const
{
#ifdef MUSE_APP_BUILD_NUMBER
    return String::fromAscii(MUSE_APP_BUILD_NUMBER);
#else
    return String();
#endif
}

String Application::revision() const
{
#ifdef MUSE_APP_REVISION
    return String::fromAscii(MUSE_APP_REVISION);
#else
    return String();
#endif
}

void Application::setRunMode(const RunMode& mode)
{
    m_runMode = mode;
}

IApplication::RunMode Application::runMode() const
{
    return m_runMode;
}

bool Application::noGui() const
{
    switch (m_runMode) {
    case RunMode::GuiApp: return false;
    case RunMode::ConsoleApp: return true;
    case RunMode::AudioPluginRegistration: return true;
    }
    return false;
}

#ifndef NO_QT_SUPPORT
QWindow* Application::focusWindow() const
{
    return qApp->focusWindow();
}

bool Application::notify(QObject* object, QEvent* event)
{
    return qApp->notify(object, event);
}

#endif

void Application::restart()
{
#ifndef NO_QT_SUPPORT
    QString program = qApp->arguments()[0];

    // NOTE: remove the first argument - the program name
    QStringList arguments = qApp->arguments().mid(1);

    QCoreApplication::exit();

    QProcess::startDetached(program, arguments);

#else
    NOT_SUPPORTED;
#endif
}
