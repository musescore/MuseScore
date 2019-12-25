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

#include "telemetrypermissiondialog.h"

#include <QQuickItem>

//---------------------------------------------------------
//   TelemetryPermissionDialog
//---------------------------------------------------------

TelemetryPermissionDialog::TelemetryPermissionDialog() : QQuickView()
      {
      setMinimumWidth(500);
      setMinimumHeight(460);

      setFlags(Qt::CustomizeWindowHint); ///@note Hidding a native frame with 'X' close button


      QUrl url = QUrl(QStringLiteral("qrc:/qml/TelemetryPermissionDialog.qml"));

      setSource(url);

      setModality(Qt::ApplicationModal);
      setResizeMode(QQuickView::SizeViewToRootObject);
      setTitle("");

      QObject* rootItem = qobject_cast<QObject*>(rootObject());
      rootObject()->setWidth(minimumWidth());

      connect(rootItem, SIGNAL(closeRequested()), this, SLOT(close()));
      }
