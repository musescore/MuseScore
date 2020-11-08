//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2018 Werner Schweer and others
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

#include "extension.h"
#include "preferences.h"
#include "libmscore/utils.h"

namespace Ms {

//---------------------------------------------------------
//   getDirectoriesByType
//---------------------------------------------------------

QStringList Extension::getDirectoriesByType(const char* type)
      {
      QStringList result;
      QDir d(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS));
      for (const auto &dd : d.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot| QDir::Readable | QDir::NoSymLinks)) {
            QDir extensionsDir(dd.absoluteFilePath());
            auto extDir = extensionsDir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot| QDir::Readable | QDir::NoSymLinks, QDir::Name);
            // take the most recent version only
            if (!extDir.isEmpty()) {
                  QString typeDir = QString("%1/%2").arg(extDir.last().absoluteFilePath()).arg(type);
                  if (QFileInfo(typeDir).exists())
                        result.append(typeDir);
                  }
            }
      return result;
      }

//---------------------------------------------------------
//   isInstalled
//---------------------------------------------------------

bool Extension::isInstalled(QString extensionId)
      {
      QDir extensionDir(QString("%1/%2").arg(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS)).arg(extensionId));
      return extensionDir.exists();
      }

//---------------------------------------------------------
//   getLatestVersion
//---------------------------------------------------------

QString Extension::getLatestVersion(QString extensionId)
      {
      QString result = "0.0";
      QDir extensionDir(QString("%1/%2").arg(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS)).arg(extensionId));
      auto extDir = extensionDir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot| QDir::Readable | QDir::NoSymLinks, QDir::Name);
      if (!extDir.isEmpty())
            result = extDir.last().fileName();
      return result;
      }
}
