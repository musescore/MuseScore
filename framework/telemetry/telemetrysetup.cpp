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

#include <QtQml/qqml.h>

#include "modularity/ioc.h"

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
    mu::framework::ioc()->registerExport<ITelemetryService>("telemetry", new TelemetryService());
}

//---------------------------------------------------------
//   moduleName
//---------------------------------------------------------

std::string TelemetrySetup::moduleName() const
{
    return "telemetry";
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

void TelemetrySetup::registerUiTypes()
{
    qmlRegisterType<TelemetryPermissionModel>("MuseScore.Telemetry", 3, 3, "TelemetryPermissionModel");
}
