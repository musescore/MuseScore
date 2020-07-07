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
#include "globalconfiguration.h"

#include <QString>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

#include "config.h"

using namespace mu;
using namespace mu::framework;

io::path GlobalConfiguration::sharePath() const
{
    if (m_sharePath.empty()) {
        m_sharePath = io::pathFromQString(getSharePath());
    }

    return m_sharePath;
}

io::path GlobalConfiguration::dataPath() const
{
    if (m_dataPath.empty()) {
        m_dataPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation).toStdString();
    }

    return m_dataPath;
}

QString GlobalConfiguration::getSharePath() const
{
#ifdef Q_OS_WIN
    QDir dir(QCoreApplication::applicationDirPath() + QString("/../" INSTALL_NAME));
    return dir.absolutePath() + "/";
#else
#ifdef Q_OS_MAC
    QDir dir(QCoreApplication::applicationDirPath() + QString("/../Resources"));
    return dir.absolutePath() + "/";
#else
    // Try relative path (needed for portable AppImage and non-standard installations)
    QDir dir(QCoreApplication::applicationDirPath() + QString("/../share/" INSTALL_NAME));
    if (dir.exists()) {
        return dir.absolutePath() + "/";
    }
    // Otherwise fall back to default location (e.g. if binary has moved relative to share)
    return QString(INSTPREFIX "/share/" INSTALL_NAME);
#endif
#endif
}
