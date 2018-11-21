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

#ifdef WIN_SPARKLE_ENABLED
#include "winsparkle/winsparkle.h"
#endif

namespace Ms
{
      class GeneralAutoUpdater
      {
      public:
            virtual void checkUpdates() = 0;
            virtual void checkForUpdatesNow() = 0;
            virtual void cleanup() = 0;
      };

      class SparkleAutoUpdater : public GeneralAutoUpdater
      {
      public:
            virtual void checkUpdates();
            virtual void checkForUpdatesNow();
            virtual void cleanup() {}
      };

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
