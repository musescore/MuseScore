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

#include "telemetrysetup.h"

#include "servicesresolver.h"

#include "interfaces/itelemetryservice.h"
#include "services/telemetryservice.h"

#include "models/telemetrypermissionmodel.h"

//---------------------------------------------------------
//   TelemetrySetup
//---------------------------------------------------------

TelemetrySetup::TelemetrySetup()
      {
      }

//---------------------------------------------------------
//   registerExports
//---------------------------------------------------------

void TelemetrySetup::registerExports()
      {
      ServicesResolver::registerService<ITelemetryService, TelemetryService>([]() -> TelemetryService* {
            return new TelemetryService();
            });
      }

//---------------------------------------------------------
//   moduleName
//---------------------------------------------------------

QString TelemetrySetup::moduleName() const
      {
      return QStringLiteral("telemetry");
      }

//---------------------------------------------------------
//   registerResources
//---------------------------------------------------------

void TelemetrySetup::registerResources()
      {
      Q_INIT_RESOURCE(telemetry_resources);
      }

//---------------------------------------------------------
//   registerQmlTypes
//---------------------------------------------------------

void TelemetrySetup::registerQmlTypes()
      {
      qmlRegisterType<TelemetryPermissionModel>("MuseScore.Telemetry", 3, 3, "TelemetryPermissionModel");
      }
