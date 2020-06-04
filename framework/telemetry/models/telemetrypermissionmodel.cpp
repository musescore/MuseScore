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

#include "telemetrypermissionmodel.h"
#include "framework/preferencekeys.h"
#include <QDesktopServices>
#include <QUrl>

//---------------------------------------------------------
//   TelemetryPermissionModel
//---------------------------------------------------------

TelemetryPermissionModel::TelemetryPermissionModel(QObject* parent)
    : QObject(parent)
{
    m_settings.setValue(PREF_APP_STARTUP_TELEMETRY_ACCESS_REQUESTED, true);
}

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void TelemetryPermissionModel::accept()
{
    m_settings.setValue(PREF_APP_TELEMETRY_ALLOWED, true);
}

//---------------------------------------------------------
//   reject
//---------------------------------------------------------

void TelemetryPermissionModel::reject()
{
    m_settings.setValue(PREF_APP_TELEMETRY_ALLOWED, false);
}

//---------------------------------------------------------
//   openLink
//---------------------------------------------------------

void TelemetryPermissionModel::openLink(const QString& link)
{
    QDesktopServices::openUrl(QUrl(link));
}
