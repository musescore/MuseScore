//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA and others
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

#ifndef IMODULESETUP_H
#define IMODULESETUP_H

#include <QString>
#include <QtQml/qqml.h>

//---------------------------------------------------------
//   AbstractModuleSetup
//---------------------------------------------------------

class AbstractModuleSetup {
   public:
      void setup() {
            registerExports();
            registerResources();
            registerQmlTypes();
            onStartInit();
            }

   protected:
      virtual QString moduleName() const = 0;

      virtual void registerExports() {}

      virtual void registerResources() {}

      virtual void registerQmlTypes() {}

      virtual void onStartInit() {}
      };

#endif // IMODULESETUP_H
