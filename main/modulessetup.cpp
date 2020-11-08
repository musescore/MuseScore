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

#include "modulessetup.h"
#include "config.h"

#ifdef BUILD_TELEMETRY_MODULE
#include "telemetry/telemetrysetup.h"
#endif

#ifdef AVSOMR
#include "avsomr/avsomrsetup.h"
#endif

//---------------------------------------------------------
//   ModulesSetup
//---------------------------------------------------------

ModulesSetup::ModulesSetup()
      {

      m_modulesSetupList
#ifdef BUILD_TELEMETRY_MODULE
              << new TelemetrySetup()
#endif
#ifdef AVSOMR
              << new Ms::Avs::AvsOmrSetup()
#endif
              ;
      }

//---------------------------------------------------------
//   setup
//---------------------------------------------------------

void ModulesSetup::setup()
      {
      for (AbstractModuleSetup* moduleSetup : qAsConst(m_modulesSetupList))
            moduleSetup->setup();
      }
