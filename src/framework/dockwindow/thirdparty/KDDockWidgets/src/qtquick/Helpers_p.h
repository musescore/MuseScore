/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_QTQUICK_HELPERS_P_H
#define KD_QTQUICK_HELPERS_P_H

#include <QObject>

#include <kdbindings/signal.h>

#include <vector>

QT_BEGIN_NAMESPACE
class QQuickItem;
QT_END_NAMESPACE

namespace KDDockWidgets {
class QtQuickHelpers : public QObject
{
    Q_OBJECT
    Q_PROPERTY(
        QObject *groupViewInMDIResize READ groupViewInMDIResize NOTIFY groupInMDIResizeChanged)

    Q_PROPERTY(bool isDragging READ isDragging NOTIFY isDraggingChanged)

Q_SIGNALS:
    /// @brief emitted when the MDI group that's being resized changed
    void groupInMDIResizeChanged();

    /// emitted when users starts or stops dragging
    void isDraggingChanged();

public:
    QtQuickHelpers();

    Q_INVOKABLE qreal logicalDpiFactor(const QQuickItem *item) const;

    /// Used by our customtabbar example.
    Q_INVOKABLE QString generateUuid() const;

    QObject *groupViewInMDIResize() const;
    bool isDragging() const;

    std::vector<KDBindings::ScopedConnection> m_connections;
};

}

#endif
