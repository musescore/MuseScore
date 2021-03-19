//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#ifndef MU_TELEMETRY_ITELEMETRYCONFIGURATION_H
#define MU_TELEMETRY_ITELEMETRYCONFIGURATION_H

#include "modularity/imoduleexport.h"
#include "retval.h"

namespace mu::telemetry {
class ITelemetryConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ITelemetryConfiguration)

public:
    virtual ~ITelemetryConfiguration() = default;

    virtual bool needRequestTelemetryPermission() const = 0;

    virtual ValCh<bool> isTelemetryAllowed() const = 0;
    virtual void setIsTelemetryAllowed(bool val) = 0;

    virtual bool isDumpUploadAllowed() const = 0;
    virtual void setIsDumpUploadAllowed(bool val) = 0;
};
}

#endif // MU_TELEMETRY_ITELEMETRYCONFIGURATION_H
