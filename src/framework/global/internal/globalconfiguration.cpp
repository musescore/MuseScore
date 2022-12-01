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
#include "globalconfiguration.h"

#include <QString>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

#include "config.h"
#include "settings.h"
#include "log.h"

using namespace mu;
using namespace mu::framework;

static const Settings::Key BACKUP_KEY("global", "application/backup/subfolder");

static const std::string MUSESCORE_URL("https://www.musescore.org/");

io::path_t GlobalConfiguration::appBinPath() const
{
    return io::path_t(QCoreApplication::applicationDirPath());
}

io::path_t GlobalConfiguration::appDataPath() const
{
    if (m_appDataPath.empty()) {
        m_appDataPath = resolveAppDataPath();
    }
    return m_appDataPath;
}

QString GlobalConfiguration::resolveAppDataPath() const
{
#ifdef Q_OS_WIN
    QDir dir(QCoreApplication::applicationDirPath() + QString("/../" INSTALL_NAME));
    return dir.absolutePath() + "/";
#elif defined(Q_OS_MAC)
    QDir dir(QCoreApplication::applicationDirPath() + QString("/../Resources"));
    return dir.absolutePath() + "/";
#elif defined(Q_OS_WASM)
    return "/files/share";
#else
    // Try relative path (needed for portable AppImage and non-standard installations)
    QDir dir(QCoreApplication::applicationDirPath() + QString("/../share/" INSTALL_NAME));
    if (dir.exists()) {
        return dir.absolutePath() + "/";
    }
    // Otherwise fall back to default location (e.g. if binary has moved relative to share)
    return QString(INSTPREFIX "/share/" INSTALL_NAME);
#endif
}

io::path_t GlobalConfiguration::appConfigPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
}

io::path_t GlobalConfiguration::userAppDataPath() const
{
    if (m_userAppDataPath.empty()) {
        m_userAppDataPath = resolveUserAppDataPath();
    }
    return m_userAppDataPath;
}

QString GlobalConfiguration::resolveUserAppDataPath() const
{
#if defined(WIN_PORTABLE)
    return QDir::cleanPath(QString("%1/../../../Data/settings").arg(QCoreApplication::applicationDirPath()));
#elif defined(Q_OS_WASM)
    return QString("/files/data");
#else
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
#endif
}

io::path_t GlobalConfiguration::userBackupPath() const
{
    return settings()->value(BACKUP_KEY).toString();
}

io::path_t GlobalConfiguration::userDataPath() const
{
    static io::path_t p = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + QCoreApplication::applicationName();
    return p;
}

io::path_t GlobalConfiguration::homePath() const
{
    static io::path_t p = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    return p;
}

io::path_t GlobalConfiguration::downloadsPath() const
{
    static io::path_t p = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    return p;
}

bool GlobalConfiguration::useFactorySettings() const
{
    return false;
}

bool GlobalConfiguration::enableExperimental() const
{
    return false;
}

std::string GlobalConfiguration::museScoreUrl() const
{
    return MUSESCORE_URL;
}
