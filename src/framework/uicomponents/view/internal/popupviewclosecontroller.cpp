/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "popupviewclosecontroller.h"

#include <QApplication>
#include <QQuickWindow>

using namespace muse::uicomponents;

PopupViewCloseController::PopupViewCloseController(const modularity::ContextPtr& iocCtx, QObject* parent)
    : QObject(parent), muse::Injectable(iocCtx)
{
}

void PopupViewCloseController::init()
{
    connect(qApp, &QApplication::applicationStateChanged, this, &PopupViewCloseController::onApplicationStateChanged);

    interactiveProvider()->currentUriAboutToBeChanged().onNotify(this, [this]() {
        notifyAboutClose();
    });
}

bool PopupViewCloseController::active() const
{
    return m_active;
}

void PopupViewCloseController::setActive(bool active)
{
    if (m_active == active) {
        return;
    }

    m_active = active;

    doUpdateEventFilters();
}

QQuickItem* PopupViewCloseController::parentItem() const
{
    return m_parentItem;
}

void PopupViewCloseController::setParentItem(QQuickItem* parentItem)
{
    if (m_parentItem) {
        m_parentItem->disconnect(this);
    }

    m_parentItem = parentItem;

    connect(m_parentItem, &QQuickItem::visibleChanged, this, [this]() {
        if (!m_parentItem || !m_parentItem->isVisible()) {
            notifyAboutClose();
        }
    });

    connect(m_parentItem, &QQuickItem::destroyed, this, [this]() {
        qApp->removeEventFilter(this);
        notifyAboutClose();
    });
}

void PopupViewCloseController::setWindow(QWindow* window)
{
    m_popupWindow = window;
}

void PopupViewCloseController::setIsCloseOnPressOutsideParent(bool arg)
{
    m_isCloseOnPressOutsideParent = arg;
}

void PopupViewCloseController::setCanClosed(bool arg)
{
    m_canClosed = arg;
}

muse::async::Notification PopupViewCloseController::closeNotification() const
{
    return m_closeNotification;
}

bool PopupViewCloseController::eventFilter(QObject* watched, QEvent* event)
{
    if (QEvent::Close == event->type() && watched == parentWindow()) {
        notifyAboutClose();
    } else if (QEvent::MouseButtonPress == event->type()) {
        doFocusOut(static_cast<QMouseEvent*>(event)->globalPosition());
    } else if (QEvent::FocusOut == event->type() && watched == popupWindow()) {
        doFocusOut(QCursor::pos());
    } else if (QEvent::Close == event->type() && watched == popupWindow()) {
        if (!m_canClosed) {
            event->ignore();
        }
    } else if (QEvent::HoverMove == event->type() || QEvent::HoverEnter == event->type()) {
        // Track mouse position for amazon triangle
        QWindow* window = popupWindow();
        if (window) {
            QPointF globalPos = static_cast<QMouseEvent*>(event)->globalPosition();
            QPointF localPos = window->mapFromGlobal(globalPos);

            // Notify about mouse movement (this updates MenuView's internal state)
            if (m_mouseMoveCallback) {
                m_mouseMoveCallback(localPos);
            }

            // NOTE: We no longer filter hover events when in Amazon triangle.
            // Instead, we let the events through and block submenu opening logic at QML level.
            // This allows proper hover state tracking while still preventing accidental submenu changes.
        }
    }

    return QObject::eventFilter(watched, event);
}

void PopupViewCloseController::onApplicationStateChanged(Qt::ApplicationState state)
{
    if (!m_active || !m_isCloseOnPressOutsideParent) {
        return;
    }

    // Hack for https://github.com/musescore/MuseScore/issues/29656 on Linux
    if (interactiveProvider()->isSelectColorOpened()) {
        return;
    }

    if (state != Qt::ApplicationActive) {
        notifyAboutClose();
    }
}

void PopupViewCloseController::doFocusOut(const QPointF& mousePos)
{
    if (m_isCloseOnPressOutsideParent) {
        if (!isMouseWithinBoundaries(mousePos)) {
            notifyAboutClose();
        }
    }
}

void PopupViewCloseController::doUpdateEventFilters()
{
    if (active()) {
        qApp->installEventFilter(this);
    } else {
        qApp->removeEventFilter(this);
    }
}

bool PopupViewCloseController::isMouseWithinBoundaries(const QPointF& mousePos) const
{
    QWindow* window = popupWindow();
    if (!window) {
        return false;
    }

    QRectF viewRect = window->geometry();
    if (viewRect.contains(mousePos)) {
        return true;
    }

    //! NOTE We also check the parent because often clicking on the parent should toggle the popup,
    //! but if we don't check a parent here, the popup will be closed and reopened.
    QQuickItem* parent = parentItem();
    QPointF localPos = parent->mapFromGlobal(mousePos);
    QRectF parentRect = QRectF(0, 0, parent->width(), parent->height());
    if (parentRect.contains(localPos)) {
        return true;
    }

    //! NOTE We also check child windows
    for (QWindow* child : QGuiApplication::allWindows()) {
        if (!child->isVisible()) {
            continue;
        }

        if (!window->isAncestorOf(child, QWindow::IncludeTransients)) {
            continue;
        }

        QRectF childRect = child->geometry();
        if (childRect.contains(mousePos)) {
            return true;
        }
    }

    //! NOTE Check amazon triangle - if mouse is in the triangle, it's considered within boundaries
    //! This prevents the menu from closing when the mouse is moving toward a submenu
    // if (m_amazonTriangleActive) {
    //     // Convert global mousePos to local coordinates relative to the parent item
    //         QPointF localPos = window->mapFromGlobal(mousePos);
    //         if (isPointInTriangle(localPos)) {
    //             return true;
    //         }
    // }

    // Hack for https://github.com/musescore/MuseScore/issues/29656
    if (interactiveProvider()->isSelectColorOpened()) {
        return true;
    }

    return false;
}

void PopupViewCloseController::notifyAboutClose()
{
    m_closeNotification.notify();
}

QWindow* PopupViewCloseController::parentWindow() const
{
    if (m_parentItem && m_parentItem->window()) {
        return m_parentItem->window();
    }

    return mainWindow()->qWindow();
}

QWindow* PopupViewCloseController::popupWindow() const
{
    return m_popupWindow;
}

void PopupViewCloseController::setAmazonTriangle(const QPointF& p1, const QPointF& p2, const QPointF& p3, bool active)
{
    m_triangleP1 = p1;
    m_triangleP2 = p2;
    m_triangleP3 = p3;
    if (p1.x() > p2.x()) {
        m_amazonTriangleActive = false;
    } else {
        m_amazonTriangleActive = active;
    }
}

void PopupViewCloseController::setMouseMoveCallback(std::function<void(const QPointF&)> callback)
{
    m_mouseMoveCallback = callback;
}

bool PopupViewCloseController::isPointInTriangle(const QPointF& point) const
{
    if (!m_amazonTriangleActive) {
        return false;
    }

    // Helper lambda to calculate cross product sign
    auto sign = [](const QPointF& p1, const QPointF& p2, const QPointF& p3) -> qreal {
        return (p1.x() - p3.x()) * (p2.y() - p3.y()) - (p2.x() - p3.x()) * (p1.y() - p3.y());
    };

    qreal d1 = sign(point, m_triangleP1, m_triangleP2);
    qreal d2 = sign(point, m_triangleP2, m_triangleP3);
    qreal d3 = sign(point, m_triangleP3, m_triangleP1);

    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    // LOGD("Checking (%f, %f). Triangle [(%f, %f) - (%f, %f) - (%f, %f)] -> %s",
    //         point.x(), point.y(),
    //         m_triangleP1.x(), m_triangleP1.y(),
    //         m_triangleP2.x(), m_triangleP2.y(),
    //         m_triangleP3.x(), m_triangleP3.y(),
    //         !(hasNeg && hasPos)  ? "true" : "false"
    //     );

    // Point is inside if all cross products have the same sign
    return !(hasNeg && hasPos);
}
