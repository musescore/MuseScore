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
#include "baseapplication.h"

#include "types/version.h"

#include "muse_framework_config.h"

#ifndef NO_QT_SUPPORT
#include <QApplication>
#include <QProcess>
#endif

#include "log.h"

using namespace muse;

String BaseApplication::appName()
{
#ifdef MUSE_APP_NAME
    return String::fromAscii(MUSE_APP_NAME);
#else
    return String();
#endif
}

String BaseApplication::appTitle()
{
#ifdef MUSE_APP_TITLE
#ifdef MUSE_APP_UNSTABLE
    return String::fromAscii(MUSE_APP_TITLE) + u" Development";
#else
    return String::fromAscii(MUSE_APP_TITLE);
#endif
#else
    return String();
#endif
}

bool BaseApplication::appUnstable()
{
#ifdef MUSE_APP_UNSTABLE
    return true;
#else
    return false;
#endif
}

Version BaseApplication::appVersion()
{
#ifdef MUSE_APP_VERSION
    static Version v(MUSE_APP_VERSION);
#else
    static Version v("0.0.0");
#endif
    return v;
}

Version BaseApplication::appFullVersion()
{
    static Version fv(appVersion());

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

String BaseApplication::appBuild()
{
#ifdef MUSE_APP_BUILD_NUMBER
    return String::fromAscii(MUSE_APP_BUILD_NUMBER);
#else
    return String();
#endif
}

String BaseApplication::appRevision()
{
#ifdef MUSE_APP_REVISION
    return String::fromAscii(MUSE_APP_REVISION);
#else
    return String();
#endif
}

BaseApplication::BaseApplication(const modularity::ContextPtr& ctx)
    : m_iocContext(ctx)
{
}

void BaseApplication::setRunMode(const RunMode& mode)
{
    m_runMode = mode;
}

IApplication::RunMode BaseApplication::runMode() const
{
    return m_runMode;
}

bool BaseApplication::noGui() const
{
    switch (m_runMode) {
    case RunMode::GuiApp: return false;
    case RunMode::ConsoleApp: return true;
    case RunMode::AudioPluginRegistration: return true;
    }
    return false;
}

#ifndef NO_QT_SUPPORT
QWindow* BaseApplication::focusWindow() const
{
    return qApp->focusWindow();
}

bool BaseApplication::notify(QObject* object, QEvent* event)
{
    return qApp->notify(object, event);
}

Qt::KeyboardModifiers BaseApplication::keyboardModifiers() const
{
    return QApplication::keyboardModifiers();
}

#endif

void BaseApplication::restart()
{
#ifdef QT_QPROCESS_SUPPORTED
    QString program = qApp->arguments()[0];

    // NOTE: remove the first argument - the program name
    QStringList arguments = qApp->arguments().mid(1);

    QCoreApplication::exit();

    QProcess::startDetached(program, arguments);

#else
    NOT_SUPPORTED;
#endif
}

const modularity::ContextPtr BaseApplication::iocContext() const
{
    return m_iocContext;
}

modularity::ModulesIoC* BaseApplication::ioc() const
{
    return modularity::_ioc(m_iocContext);
}

void BaseApplication::removeIoC()
{
    modularity::_ioc(m_iocContext)->reset();
    modularity::removeIoC(m_iocContext);
}
