/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Helpers_p.h"
#include "core/DockRegistry_p.h"
#include "core/DragController_p.h"
#include "core/Group.h"
#include "qtcommon/View.h"

#include <QQuickItem>
#include <QUuid>

using namespace KDDockWidgets;

QtQuickHelpers::QtQuickHelpers()
{
    KDBindings::ScopedConnection conn = DockRegistry::self()->dptr()->groupInMDIResizeChanged.connect([this] {
        Q_EMIT groupInMDIResizeChanged();
    });
    m_connections.push_back(std::move(conn));

    KDBindings::ScopedConnection conn2 = Core::DragController::instance()->isDraggingChanged.connect([this] {
        Q_EMIT isDraggingChanged();
    });
    m_connections.push_back(std::move(conn2));
}

qreal QtQuickHelpers::logicalDpiFactor(const QQuickItem *) const
{
    return 1; // TODO: Support the actual dpi factor.
    // return KDDockWidgets::logicalDpiFactor(item);
}

QObject *QtQuickHelpers::groupViewInMDIResize() const
{
    if (auto group = DockRegistry::self()->groupInMDIResize())
        return QtCommon::View_qt::asQObject(group->view());

    return nullptr;
}

bool QtQuickHelpers::isDragging() const
{
    return Core::DragController::instance()->isDragging();
}

QString QtQuickHelpers::generateUuid() const
{
    return QUuid::createUuid().toString();
}
