/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "docktabbar.h"

#include "log.h"

using namespace muse::dock;

DockTabBar::DockTabBar(KDDockWidgets::TabWidget* parent)
    : KDDockWidgets::TabBarQuick(parent)
{
}

bool DockTabBar::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::MouseButtonDblClick: {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        doubleClicked(mouseEvent->pos());
        return true;
    }
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease: {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        onMousePressRelease(mouseEvent);
        return true;
    }
    default:
        break;
    }

    return KDDockWidgets::TabBarQuick::event(event);
}

void DockTabBar::onMousePressRelease(const QMouseEvent* mouseEvent)
{
    QQuickItem* tabBar = tabBarQmlItem();
    if (!mouseEvent || !tabBar) {
        return;
    }

    const QPoint localPos = mouseEvent->pos();

    int tabIndex = tabAt(localPos);
    if (tabIndex < 0) {
        return;
    }

    switch (mouseEvent->type()) {
    case QEvent::MouseButtonPress: {
        m_indexOfPressedTab = tabIndex;
        m_tabChangedOnClick = false;
        TabBar::onMousePress(localPos);
        break;
    }
    case QEvent::MouseButtonRelease: {
        const int currentTabIndex = tabBar->property("currentIndex").toInt();
        if (tabIndex != currentTabIndex && tabIndex == m_indexOfPressedTab) {
            tabBar->setProperty("currentIndex", tabIndex);
            m_tabChangedOnClick = true;
        }
        m_indexOfPressedTab = -1;
        break;
    }
    default: UNREACHABLE;
    }
}

void DockTabBar::doubleClicked(const QPoint& pos) const
{
    if (KDDockWidgets::DockWidgetBase* dw = dockWidgetAt(pos)) {
        dw->setFloating(!dw->isFloating());
    }
}

bool DockTabBar::isPositionDraggable(QPoint localPos) const
{
    if (!m_draggableMouseArea) {
        return false;
    }

    return m_draggableMouseArea->contains(localPos);
}

void DockTabBar::setDraggableMouseArea(QQuickItem* mouseArea)
{
    if (m_draggableMouseArea == mouseArea) {
        return;
    }

    m_draggableMouseArea = mouseArea;
    redirectMouseEvents(mouseArea);
}
