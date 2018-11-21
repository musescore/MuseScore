//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================
#ifndef __AUTO_UPDATER_H__
#define __AUTO_UPDATER_H__

namespace Ms
{
      class GeneralAutoUpdater
      {
      public:
            virtual void checkUpdates() {}
            virtual void checkForUpdatesNow() {}
            virtual void cleanup() {}
      };

} //Ms

#endif AUTO_UPDATER_H__