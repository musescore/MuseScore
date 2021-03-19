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

static const Settings::Key REQUEST_TELEMETRY_PERMISSION("telemetry", "telemetry/telemetry_access_requested");
static const Settings::Key IS_TELEMETRY_ALLOWED("telemetry", "telemetry/allowed");
static const Settings::Key IS_DUMP_UPLOAD_ALLOWED("telemetry", "telemetry/is_dump_upload_allowed");

void TelemetryConfiguration::init()
{
    settings()->setDefaultValue(REQUEST_TELEMETRY_PERMISSION, Val(true));
    settings()->setDefaultValue(IS_TELEMETRY_ALLOWED, Val(false));
    settings()->setDefaultValue(IS_DUMP_UPLOAD_ALLOWED, Val(true));

    settings()->valueChanged(IS_TELEMETRY_ALLOWED).onReceive(this, [this](const Val& allowed) {
        m_isTelemetryAllowedChannel.send(allowed.toBool());
    });
}

bool TelemetryConfiguration::needRequestTelemetryPermission() const
{
    return settings()->value(REQUEST_TELEMETRY_PERMISSION).toBool();
}

mu::ValCh<bool> TelemetryConfiguration::isTelemetryAllowed() const
{
    mu::ValCh<bool> allowed;
    allowed.ch = m_isTelemetryAllowedChannel;
    static QString id(TELEMETRY_TRACK_ID);
    allowed.val = !id.isEmpty() && settings()->value(IS_TELEMETRY_ALLOWED).toBool();

    return allowed;
}

void TelemetryConfiguration::setIsTelemetryAllowed(bool val)
{
    return settings()->setValue(IS_TELEMETRY_ALLOWED, Val(val));
}

bool TelemetryConfiguration::isDumpUploadAllowed() const
{
    return settings()->value(IS_DUMP_UPLOAD_ALLOWED).toBool();
}

void TelemetryConfiguration::setIsDumpUploadAllowed(bool val)
{
    settings()->setValue(IS_DUMP_UPLOAD_ALLOWED, Val(val));
}
