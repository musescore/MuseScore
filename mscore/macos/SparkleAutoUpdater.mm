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

#include "SparkleAutoUpdater.h"

#include "Sparkle/Sparkle.h"
#include <Sparkle/SUUpdater.h>

static SUUpdater*  updater = [[SUUpdater sharedUpdater] retain];

void SparkleAutoUpdater::checkUpdates()
      {
      [updater checkForUpdatesInBackground];
      }

void SparkleAutoUpdater::checkForUpdatesNow()
      {
      [updater checkForUpdates:NULL];
      }
