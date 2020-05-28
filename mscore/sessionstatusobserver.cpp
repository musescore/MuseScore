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

#include "sessionstatusobserver.h"
#include "musescore.h"
#include "telemetrymanager.h"

namespace Ms {
void SessionStatusObserver::prevSessionStatus(bool sessionFileFound, const QString& sessionFullVersion, bool clean)
{
#ifndef TELEMETRY_DISABLED
    //if session status data IS the enabled telemetry data
    if (Ms::enabledTelemetryDataTypes & Ms::TelemetryDataCollectionType::COLLECT_CRASH_FREE_DATA) {
        QString status;
        QString label;
        if (mscoreFirstStart) {
            status = QStringLiteral("first-start");
        } else if (!sessionFileFound) {
            status = QStringLiteral("session-file-not-found");
        } else {
            const bool versionChanged = MuseScore::fullVersion() != sessionFullVersion;
            if (versionChanged) {
                status = QStringLiteral("version-changed");
                label = sessionFullVersion;
            } else if (clean) {
                status = QStringLiteral("clean");
            } else {
                status = QStringLiteral("dirty");
            }
        }
        TelemetryManager::telemetryService()->sendEvent("prev-session-status", status, label);
    }
#else
    Q_UNUSED(sessionFileFound);
    Q_UNUSED(sessionFullVersion);
    Q_UNUSED(clean);
#endif
}
} // namespace Ms
