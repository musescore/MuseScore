//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA
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

#ifndef __TELEMETRYMANAGER_H__
#define __TELEMETRYMANAGER_H__

#ifdef BUILD_TELEMETRY_MODULE

#include "modularity/ioc.h"
#include "framework/telemetry/interfaces/itelemetryservice.h"

namespace Ms {
//---------------------------------------------------------
//   TelemetryManager
//---------------------------------------------------------

class TelemetryManager
{
    INJECT(telemetry, ITelemetryService, _telemetryService)
    static std::unique_ptr<TelemetryManager> mgr;

    static TelemetryManager* instance()
    {
        if (!mgr) {
            mgr.reset(new TelemetryManager());
        }
        return mgr.get();
    }

public:
    static ITelemetryService* telemetryService()
    {
        return instance()->_telemetryService().get();
    }
};
} // namespace Ms

#endif // BUILD_TELEMETRY_MODULE
#endif // __TELEMETRYMANAGER_H__
