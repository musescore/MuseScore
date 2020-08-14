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

#ifndef ITELEMETRYSERVICE_H
#define ITELEMETRYSERVICE_H

#include <QString>
#include <QVariant>
#include <QVariantMap>

#include "modularity/imoduleexport.h"

//---------------------------------------------------------
//   ITelemetryService
//---------------------------------------------------------

class ITelemetryService : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ITelemetryService)

public:

    virtual ~ITelemetryService() = default;

    virtual void sendEvent(const QString& category,const QString& action,const QString& label = QString(),
                           const QVariant& value = QVariant(),const QVariantMap& customValues = QVariantMap()) = 0;

    virtual void sendException(const QString& exceptionDescription,bool exceptionFatal = true,
                               const QVariantMap& customValues = QVariantMap()) = 0;

    virtual void startSession() = 0;
    virtual void endSession() = 0;
};

#endif // ITELEMETRYSERVICE_H
