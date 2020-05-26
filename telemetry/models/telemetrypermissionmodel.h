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

#ifndef TELEMETRYPERMISSIONMODEL_H
#define TELEMETRYPERMISSIONMODEL_H

#include <QObject>
#include <QSettings>
#include <QString>

//---------------------------------------------------------
//   TelemetryPermissionModel
//---------------------------------------------------------

class TelemetryPermissionModel : public QObject
{
    Q_OBJECT

public:
    explicit TelemetryPermissionModel(QObject* parent = nullptr);

    Q_INVOKABLE void accept();
    Q_INVOKABLE void reject();
    Q_INVOKABLE void openLink(const QString& link);

private:
    QSettings m_settings;
};

#endif // TELEMETRYPERMISSIONMODEL_H
