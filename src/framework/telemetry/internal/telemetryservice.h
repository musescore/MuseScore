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

#ifndef MU_TELEMETRY_TELEMETRYSERVICE_H
#define MU_TELEMETRY_TELEMETRYSERVICE_H

#include "itelemetryservice.h"

#include "modularity/ioc.h"
#include "itelemetryconfiguration.h"

namespace mu::telemetry {
class TelemetryService : public ITelemetryService
{
    INJECT(telemetry, ITelemetryConfiguration, configuration)
public:
    TelemetryService() = default;

    void sendEvent(const QString& category, const QString& action, const QString& label, const QVariant& value,
                   const QVariantMap& customValues) override;
    void sendException(const QString& exceptionDescription, bool exceptionFatal,const QVariantMap& customValues) override;
    void startSession() override;
    void endSession() override;

private:
    bool isTelemetryAllowed() const;
};
}

#endif // MU_TELEMETRY_TELEMETRYSERVICE_H
