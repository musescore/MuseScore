/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>

#include "linuxplatformtheme.h"

using namespace mu::ui;
using namespace mu::async;

LinuxPlatformTheme::LinuxPlatformTheme()
{
}

void LinuxPlatformTheme::startListening()
{
}

void LinuxPlatformTheme::stopListening()
{
}

bool LinuxPlatformTheme::isFollowSystemThemeAvailable() const
{
    return false;
}

bool LinuxPlatformTheme::isSystemThemeDark() const
{
    return false;
}

bool LinuxPlatformTheme::isGlobalMenuAvailable() const
{
    const QDBusConnection connection = QDBusConnection::sessionBus();
    static const QString registrarService = QStringLiteral("com.canonical.AppMenu.Registrar");
    if (const auto iface = connection.interface()) {
        return iface->isServiceRegistered(registrarService);
    }
    return false;
}

Notification LinuxPlatformTheme::platformThemeChanged() const
{
    return m_platformThemeChanged;
}

void LinuxPlatformTheme::applyPlatformStyleOnAppForTheme(const ThemeCode&)
{
}

void LinuxPlatformTheme::applyPlatformStyleOnWindowForTheme(QWindow*, const ThemeCode&)
{
}
