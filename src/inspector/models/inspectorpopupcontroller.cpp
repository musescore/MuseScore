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

#include "inspectorpopupcontroller.h"

#include <QApplication>
#include <QWindow>

#include "uicomponents/view/popupview.h"

#include "log.h"

using namespace mu::inspector;
using namespace mu::uicomponents;

InspectorPopupController::InspectorPopupController(QObject* parent)
    : QObject(parent)
{
}

InspectorPopupController::~InspectorPopupController()
{
    closePopup();
}

void InspectorPopupController::load()
{
    IF_ASSERT_FAILED(m_popup && m_visualControl) {
        return;
    }

    connect(m_popup, &PopupView::isOpenedChanged, this, [this]() {
        if (m_popup->isOpened()) {
            qApp->installEventFilter(this);
        } else {
            qApp->removeEventFilter(this);
        }
    });

    connect(m_visualControl, &QQuickItem::visibleChanged, this, [this]() {
        if (!m_visualControl->isVisible()) {
            closePopup();
        }
    });

    connect(m_visualControl, &QQuickItem::enabledChanged, this, [this]() {
        if (!m_visualControl->isEnabled()) {
            closePopup();
        }
    });

    connect(qApp, &QApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
        if (state != Qt::ApplicationActive) {
            closePopup();
        }
    });
}

QQuickItem* InspectorPopupController::visualControl() const
{
    return m_visualControl;
}

PopupView* InspectorPopupController::popup() const
{
    return m_popup;
}

void InspectorPopupController::setVisualControl(QQuickItem* control)
{
    if (m_visualControl == control) {
        return;
    }

    m_visualControl = control;
    emit visualControlChanged();
}

void InspectorPopupController::setPopup(PopupView* popup)
{
    if (m_popup == popup) {
        return;
    }

    m_popup = popup;
    emit popupChanged();
}

bool InspectorPopupController::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::FocusOut
        || event->type() == QEvent::MouseButtonPress) {
        closePopupIfNeed(QCursor::pos());
    } else if (event->type() == QEvent::Move && watched == mainWindow()->qWindow()) {
        if (m_popup->isOpened()) {
            closePopup();
        }
    }

    return QObject::eventFilter(watched, event);
}

void InspectorPopupController::closePopupIfNeed(const QPoint& mouseGlobalPos)
{
    if (!m_popup || !m_visualControl) {
        return;
    }

    const QQuickItem* anchorItem = m_popup->anchorItem();
    const QQuickItem* popupContent = m_popup->contentItem();
    if (!anchorItem || !popupContent) {
        return;
    }

    auto globalRect = [](const QQuickItem* item) -> QRect {
        QPointF globalPos = item->mapToGlobal(QPoint(0, 0));
        return QRect(globalPos.x(), globalPos.y(), item->width(), item->height());
    };

    QRect globalVisualControlRect = globalRect(m_visualControl);
    QRect globalPopupContentRect = globalRect(popupContent);

    if (globalVisualControlRect.contains(mouseGlobalPos) || globalPopupContentRect.contains(mouseGlobalPos)) {
        return;
    }

    QRect windowGeometry = mainWindow()->qWindow()->geometry();
    if (windowGeometry.contains(mouseGlobalPos)) {
        return;
    }

    closePopup();
}

void InspectorPopupController::closePopup()
{
    QMetaObject::invokeMethod(this, "doClosePopup");
}

void InspectorPopupController::doClosePopup()
{
    if (m_popup) {
        m_popup->close();
    }
}
