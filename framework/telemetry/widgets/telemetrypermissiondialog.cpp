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
#include <QApplication>
#include <QDesktopWidget>

//---------------------------------------------------------
//   TelemetryPermissionDialog
//---------------------------------------------------------

TelemetryPermissionDialog::TelemetryPermissionDialog(QQmlEngine* engine)
    : QQuickView(engine, nullptr)
{
    setMinimumWidth(500);
    setMinimumHeight(460);

    setFlags(Qt::Dialog | Qt::CustomizeWindowHint);   ///@note Hidding a native frame with 'X' close button

    // ToDo for Qt 5.15: QDesktopWidget::availableGeometry() vs. QGuiApplication::screens() ??
    QRect desktopRect = QApplication::desktop()->availableGeometry();
    QPoint center = desktopRect.center();

    setPosition(center.x() - minimumWidth() * 0.5, center.y() - minimumHeight() * 0.5);

    QUrl url = QUrl(QStringLiteral("qrc:/qml/TelemetryPermissionDialog.qml"));

    setSource(url);

    setModality(Qt::ApplicationModal);
    setResizeMode(QQuickView::SizeViewToRootObject);
    setTitle("");

    rootObject()->setWidth(minimumWidth());

    connect(rootObject(), SIGNAL(closeRequested()), this, SLOT(close()));
    connect(rootObject(), SIGNAL(closeRequested()), this, SIGNAL(closeRequested()));
}

//---------------------------------------------------------
//   focusInEvent
//---------------------------------------------------------

void TelemetryPermissionDialog::focusInEvent(QFocusEvent* evt)
{
    QQuickView::focusInEvent(evt);
    rootObject()->forceActiveFocus();
}
