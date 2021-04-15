/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/

#include "telemetryservice.h"

#include "thirdparty/qt-google-analytics/ganalytics.h"
#include "config.h"

using namespace mu::telemetry;

void TelemetryService::sendEvent(const QString& category, const QString& action, const QString& label,
                                 const QVariant& value, const QVariantMap& customValues)
{
    if (!isTelemetryAllowed() || category.isEmpty() || action.isEmpty()) {
        return;
    }

    GAnalytics::instance(TELEMETRY_TRACK_ID)->sendEvent(category, action, label, value, customValues);
}

void TelemetryService::sendException(const QString& exceptionDescription, bool exceptionFatal,
                                     const QVariantMap& customValues)
{
    if (!isTelemetryAllowed()) {
        return;
    }

    GAnalytics::instance(TELEMETRY_TRACK_ID)->sendException(exceptionDescription, exceptionFatal, customValues);
}

void TelemetryService::startSession()
{
    if (!isTelemetryAllowed()) {
        return;
    }

    GAnalytics::instance(TELEMETRY_TRACK_ID)->startSession();
}

void TelemetryService::endSession()
{
    if (!isTelemetryAllowed()) {
        return;
    }

    GAnalytics::instance(TELEMETRY_TRACK_ID)->endSession();
}

bool TelemetryService::isTelemetryAllowed() const
{
    return configuration()->isTelemetryAllowed().val;
}
