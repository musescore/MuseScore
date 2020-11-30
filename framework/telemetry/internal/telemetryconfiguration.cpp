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
#include "telemetryconfiguration.h"

#include "global/settings.h"
#include "config.h"

using namespace mu::telemetry;
using namespace mu::framework;

static const std::string module_name("telemetry");

static const Settings::Key IS_TELEMETRY_ALLOWED(module_name, "telemetry/is_telemetry_allowed");
static const Settings::Key IS_DUMP_UPLOAD_ALLOWED(module_name, "telemetry/is_dump_upload_allowed");

void TelemetryConfiguration::init()
{
    settings()->setDefaultValue(IS_TELEMETRY_ALLOWED, Val(false));
    settings()->setDefaultValue(IS_DUMP_UPLOAD_ALLOWED, Val(true));
}

bool TelemetryConfiguration::isTelemetryAllowed() const
{
#ifndef TELEMETRY_DISABLED
    return settings()->value(IS_TELEMETRY_ALLOWED).toBool();
#else
    return false;
#endif
}

bool TelemetryConfiguration::isDumpUploadAllowed() const
{
    return settings()->value(IS_DUMP_UPLOAD_ALLOWED).toBool();
}

void TelemetryConfiguration::setIsDumpUploadAllowed(bool val)
{
    settings()->setValue(IS_DUMP_UPLOAD_ALLOWED, Val(val));
}
