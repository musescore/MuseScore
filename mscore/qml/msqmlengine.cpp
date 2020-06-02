//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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

#include "msqmlengine.h"

#include <QQmlEngine>

namespace Ms {
extern QString mscoreGlobalShare;

//---------------------------------------------------------
//   MsQmlEngine
//---------------------------------------------------------

MsQmlEngine::MsQmlEngine(QObject* parent)
    : QQmlEngine(parent)
{
#ifdef Q_OS_WIN
    QStringList importPaths;
    QDir dir(QCoreApplication::applicationDirPath() + QString("/../qml"));
    importPaths.append(dir.absolutePath());
    setImportPathList(importPaths);
#endif
#ifdef Q_OS_MAC
    QStringList importPaths;
    QDir dir(mscoreGlobalShare + QString("/qml"));
    importPaths.append(dir.absolutePath());
    setImportPathList(importPaths);
#endif

    addImportPath(":/qml");
}
}
