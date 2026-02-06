/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include <QGuiApplication>
#include <QWindow>

#include "uicomponents/qml/Muse/UiComponents/popupview.h"
#include "uicomponents/qml/Muse/UiComponents/dropdownview.h"
#include "uicomponents/qml/Muse/UiComponents/menuview.h"

using namespace mu::inspector;
using namespace muse::uicomponents;
using namespace muse::async;

InspectorPopupController::InspectorPopupController(const kors::modularity::ContextPtr& iocCtx)
    :  QObject(nullptr), muse::Contextable(iocCtx)
{
}

InspectorPopupController::~InspectorPopupController()
{
    closePopup();
}

void InspectorPopupController::init()
{
    connect(qApp, &QGuiApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
        if (state != Qt::ApplicationActive) {
            // //! NOTE If the application became inactive
            // //! due to opening a color selection dialog,
            // //! then we do not need to close a popup
            // if (interactive()->isSelectColorOpened()) {
            //     return;
            // }

            // closePopup();
        }
    });
}

void InspectorPopupController::setNotationView(const QQuickItem* view)
{
    m_notationView = view;
}

PopupView* InspectorPopupController::popup() const
{
    return m_popup;
}

void InspectorPopupController::setPopup(PopupView* popup, QQuickItem* control)
{
    if (m_popup == popup && m_visualControl == control) {
        return;
    }

    const bool popupChanged = m_popup != popup;
    if (m_popup && popupChanged) {
        m_popup->disconnect(this);
    }

    m_popup = popup;

    if (m_popup) {
        qApp->installEventFilter(this);

        connect(m_popup, &PopupView::isOpenedChanged, this, [this]() {
            if (m_popup && !m_popup->isOpened()) {
                setPopup(nullptr);
            }
        });

        connect(m_popup, &PopupView::destroyed, this, [this]() {
            setPopup(nullptr);
        });
    } else {
        qApp->removeEventFilter(this);
    }

    if (m_visualControl) {
        m_visualControl->disconnect(this);
    }

    m_visualControl = control;

    if (m_visualControl) {
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

        connect(m_visualControl, &QQuickItem::destroyed, this, [this]() {
            m_visualControl = nullptr;
        });
    }

    if (popupChanged) {
        m_popupChanged.notify();
    }
}

Notification InspectorPopupController::popupChanged() const
{
    return m_popupChanged;
}

DropdownView* InspectorPopupController::dropdown() const
{
    return m_dropdown;
}

void InspectorPopupController::setDropdown(DropdownView* dropdown)
{
    if (m_dropdown == dropdown) {
        return;
    }

    m_dropdown = dropdown;

    if (m_dropdown) {
        connect(m_dropdown, &DropdownView::isOpenedChanged, this, [this]() {
            if (m_dropdown && !m_dropdown->isOpened()) {
                setDropdown(nullptr);
            }
        });

        connect(m_dropdown, &DropdownView::destroyed, this, [this]() {
            setDropdown(nullptr);
        });
    }

    m_dropdownChanged.notify();
}

Notification InspectorPopupController::dropdownChanged() const
{
    return m_dropdownChanged;
}

MenuView* InspectorPopupController::menu() const
{
    return m_menu;
}

void InspectorPopupController::setMenu(MenuView* menu)
{
    if (m_menu == menu) {
        return;
    }

    m_menu = menu;

    if (m_menu) {
        connect(m_menu, &MenuView::isOpenedChanged, this, [this]() {
            if (m_menu && !m_menu->isOpened()) {
                setMenu(nullptr);
            }
        });

        connect(m_menu, &MenuView::destroyed, this, [this]() {
            setMenu(nullptr);
        });
    }

    m_menuChanged.notify();
}

Notification InspectorPopupController::menuChanged() const
{
    return m_menuChanged;
}

bool InspectorPopupController::eventFilter(QObject* watched, QEvent* event)
{
    if (m_popup) {
        if ((event->type() == QEvent::FocusOut || event->type() == QEvent::MouseButtonPress)
            && watched == m_popup->window()) {
            closePopupIfNeed(QCursor::pos());
        } else if (event->type() == QEvent::Move && watched == mainWindow()->qWindow()) {
            if (m_popup->isOpened()) {
                closePopup();
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

void InspectorPopupController::closePopupIfNeed(const QPointF& mouseGlobalPos)
{
    if (!m_popup || !m_visualControl) {
        return;
    }

    const QQuickItem* anchorItem = m_popup->anchorItem();
    const QQuickItem* popupContent = m_popup->contentItem();
    if (!anchorItem || !popupContent) {
        return;
    }

    auto globalRect = [](const QQuickItem* item) -> QRectF {
        if (!item) {
            return QRectF();
        }

        return QRectF(item->mapToGlobal(QPoint(0, 0)), item->size());
    };

    QRectF globalVisualControlRect = globalRect(m_visualControl);
    QRectF globalPopupContentRect = globalRect(popupContent);

    if (globalVisualControlRect.contains(mouseGlobalPos) || globalPopupContentRect.contains(mouseGlobalPos)) {
        return;
    }

    QRectF globalNotationViewRect = globalRect(m_notationView);
    QWindow* windowUnderCursor = qGuiApp->topLevelAt(mouseGlobalPos.toPoint());
    if (windowUnderCursor == mainWindow()->qWindow() && globalNotationViewRect.contains(mouseGlobalPos)) {
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
