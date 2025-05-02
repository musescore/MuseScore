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

void PopupViewCloseController::setIsCloseOnPressOutsideParent(bool close)
{
    m_isCloseOnPressOutsideParent = close;
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
    }

    return QObject::eventFilter(watched, event);
}

void PopupViewCloseController::onApplicationStateChanged(Qt::ApplicationState state)
{
    if (!m_active || !m_isCloseOnPressOutsideParent) {
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
