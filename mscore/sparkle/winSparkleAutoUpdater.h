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

#ifndef __WIN_SPARKLE_AUTO_UPDATER_H__
#define __WIN_SPARKLE_AUTO_UPDATER_H__

#include "winsparkle/winsparkle.h"
#include "sparkle/autoUpdater.h"

namespace Ms
{
      class WinSparkleAutoUpdater : public GeneralAutoUpdater
      {
      public:
            virtual void checkUpdates()
            {
                  // Initialize WinSparkle as soon as the app itself is initialized, right
                  // before entering the event loop:
                  win_sparkle_set_appcast_url(WIN_SPARKLE_APPCAST_URL);
                  win_sparkle_set_app_details(L"musescore.org", L"MuseScore", L"3.0");
                  win_sparkle_init();
            }

            virtual void checkForUpdatesNow()
            {
                  win_sparkle_check_update_with_ui();
            }

            virtual void cleanup()
            {
                  win_sparkle_cleanup();
            }
      };

} //Ms

#endif
