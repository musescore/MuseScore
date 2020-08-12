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

#ifndef TELEMETRYSERVICE_H
#define TELEMETRYSERVICE_H

#include "interfaces/itelemetryservice.h"

#include <QSettings>

//---------------------------------------------------------
//   TelemetryService
//---------------------------------------------------------

class TelemetryService : public ITelemetryService
{
public:
    TelemetryService();

    void sendEvent(const QString& category, const QString& action, const QString& label, const QVariant& value,
                   const QVariantMap& customValues) override;
    void sendException(const QString& exceptionDescription, bool exceptionFatal,const QVariantMap& customValues) override;
    void startSession() override;
    void endSession() override;

private:
    bool isTelemetryAllowed() const;

    QSettings m_settings;
};

#endif // TELEMETRYSERVICE_H
