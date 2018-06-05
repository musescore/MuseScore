//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

namespace Ms {

QStringList Extension::getDirectoriesByType(const char* type)
      {
      QStringList result;
      QDir d(preferences.myExtensionsPath);
      for (auto dd : d.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot| QDir::Readable | QDir::NoSymLinks)) {
            QDir extensionsDir(dd.absoluteFilePath());
            auto extDir = extensionsDir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot| QDir::Readable | QDir::NoSymLinks, QDir::Name);
            // take the most recent version only
            if (!extDir.isEmpty()) {
                  QString sfzDir = QString("%1/%2").arg(extDir.last().absoluteFilePath()).arg(type);
                  if (QFileInfo(sfzDir).exists())
                        result.append(sfzDir);
                  }
            }
      return result;
      }
}

