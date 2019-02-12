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
#ifndef __SPARKLE_AUTO_UPDATER_H__
#define __SPARKLE_AUTO_UPDATER_H__

#include "sparkle/autoUpdater.h"

namespace Ms
{
      class SparkleAutoUpdater : public GeneralAutoUpdater
      {
      public:
            virtual void checkUpdates();
            virtual void checkForUpdatesNow();
      };

} //Ms

#endif //__SPARKLE_AUTO_UPDATER_H__
