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

#ifndef MU_TELEMETRY_TELEMETRYPERMISSIONMODEL_H
#define MU_TELEMETRY_TELEMETRYPERMISSIONMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "itelemetryconfiguration.h"

namespace mu::telemetry {
class TelemetryPermissionModel : public QObject
{
    Q_OBJECT

    INJECT(telemetry, ITelemetryConfiguration, configuration)

public:
    explicit TelemetryPermissionModel(QObject* parent = nullptr);

    Q_INVOKABLE void allowUseTelemetry();
    Q_INVOKABLE void forbidUseTelemetry();
};
}

#endif // MU_TELEMETRY_TELEMETRYPERMISSIONMODEL_H
