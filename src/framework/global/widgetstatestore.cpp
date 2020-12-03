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
#include "widgetstatestore.h"

#include <QWidget>
#include <QSettings>

void WidgetStateStore::saveGeometry(const QWidget* qw)
{
    QSettings settings;
    QString objectName = qw->objectName();
    Q_ASSERT(!objectName.isEmpty());
    settings.beginGroup("Geometries");
    settings.setValue(objectName, qw->saveGeometry());
    settings.endGroup();
}

void WidgetStateStore::restoreGeometry(QWidget* qw)
{
    //if (!useFactorySettings) { //! TODO
    QSettings settings;
    QString objectName = qw->objectName();
    Q_ASSERT(!objectName.isEmpty());
    settings.beginGroup("Geometries");
    qw->restoreGeometry(settings.value(objectName).toByteArray());
    settings.endGroup();
    // }
}
