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

#include "docktoolbar.h"

#include <QTimer>

#include "thirdparty/KDDockWidgets/src/DockWidgetQuick.h"
#include "thirdparty/KDDockWidgets/src/private/TitleBar_p.h"
#include "thirdparty/KDDockWidgets/src/private/DragController_p.h"

#include "log.h"

using namespace mu::dock;

static const qreal TOOLBAR_GRIP_MARGIN(4);
static const qreal TOOLBAR_GRIP_WIDTH(28);
static const qreal TOOLBAR_GRIP_HEIGHT(36);

class DockToolBar::DraggableArea : public KDDockWidgets::QWidgetAdapter, public KDDockWidgets::Draggable
{
public:
    DraggableArea()
        : KDDockWidgets::QWidgetAdapter(),
        KDDockWidgets::Draggable(this)
    {
    }

    std::unique_ptr<KDDockWidgets::WindowBeingDragged> makeWindow() override
    {
        if (!m_dockWidget) {
            return {};
        }

        KDDockWidgets::FloatingWindow* floatingWindow = m_dockWidget->floatingWindow();
        if (floatingWindow) {
            return std::unique_ptr<KDDockWidgets::WindowBeingDragged>(new KDDockWidgets::WindowBeingDragged(floatingWindow, this));
        }

        m_dockWidget->setFloating(true);
        floatingWindow = m_dockWidget->floatingWindow();

        auto draggable = static_cast<KDDockWidgets::Draggable*>(this);
        return std::unique_ptr<KDDockWidgets::WindowBeingDragged>(new KDDockWidgets::WindowBeingDragged(floatingWindow, draggable));
    }

    KDDockWidgets::DockWidgetBase* singleDockWidget() const override
    {
        return m_dockWidget;
    }

    bool isMDI() const override
    {
        return false;
    }

    bool isWindow() const override
    {
        return false;
    }

    QPoint mapToWindow(QPoint pos) const override
    {
        if (!m_mouseArea) {
            return pos;
        }

        QPointF result = m_mouseArea->mapToItem(m_dockWidget, QPointF(pos));
        result.setY(result.y() + m_dockWidget->titleBar()->height());
        return QPoint(result.x(), result.y());
    }

    void setDockWidget(KDDockWidgets::DockWidgetBase* dockWidget)
    {
        m_dockWidget = dockWidget;
    }

    void setMouseArea(QQuickItem* mouseArea)
    {
        m_mouseArea = mouseArea;
        redirectMouseEvents(mouseArea);
    }

private:
    KDDockWidgets::DockWidgetBase* m_dockWidget = nullptr;
    QQuickItem* m_mouseArea = nullptr;
};

DockToolBar::DockToolBar(QQuickItem* parent)
    : DockBase(parent),
    m_draggableArea(new DraggableArea())
{
    setAllowedAreas(Qt::TopDockWidgetArea);
}

bool DockToolBar::movable() const
{
    return m_movable;
}

Qt::Orientation DockToolBar::orientation() const
{
    return m_orientation;
}

void DockToolBar::setDraggableMouseArea(QQuickItem* mouseArea)
{
    IF_ASSERT_FAILED(m_draggableArea) {
        return;
    }

    m_draggableArea->setParent(mouseArea);
    m_draggableArea->setMouseArea(mouseArea);
}

void DockToolBar::setMinimumWidth(int width)
{
    if (movable() && orientation() == Qt::Horizontal) {
        width += TOOLBAR_GRIP_WIDTH + TOOLBAR_GRIP_MARGIN;
    }

    DockBase::setMinimumWidth(width);
}

void DockToolBar::setMinimumHeight(int height)
{
    if (movable() && orientation() == Qt::Vertical) {
        height += TOOLBAR_GRIP_HEIGHT + TOOLBAR_GRIP_MARGIN;
    }

    DockBase::setMinimumHeight(height);
}

void DockToolBar::setMaximumWidth(int width)
{
    int preferedWidth = this->width();

    if (movable() && orientation() == Qt::Horizontal) {
        preferedWidth = TOOLBAR_GRIP_WIDTH + TOOLBAR_GRIP_MARGIN;
    }

    if (preferedWidth > width) {
        width = preferedWidth;
    }

    DockBase::setMaximumWidth(width);
}

void DockToolBar::setMaximumHeight(int height)
{
    int preferedHeight = this->height();

    if (movable() && orientation() == Qt::Horizontal) {
        preferedHeight = TOOLBAR_GRIP_HEIGHT + TOOLBAR_GRIP_MARGIN;
    }

    if (preferedHeight > height) {
        height = preferedHeight;
    }

    DockBase::setMaximumHeight(height);
}

void DockToolBar::setMovable(bool movable)
{
    if (m_movable == movable) {
        return;
    }

    m_movable = movable;
    emit movableChanged(m_movable);
}

void DockToolBar::setOrientation(Qt::Orientation orientation)
{
    if (orientation == m_orientation) {
        return;
    }

    m_orientation = orientation;
    emit orientationChanged(orientation);
}

void DockToolBar::updateOrientation()
{
    if (dockWidget()->isFloating()) {
        return;
    }

    Layouting::ItemBoxContainer* container = dockWidget()->frame()->layoutItem()->parentBoxContainer();
    if (!container) {
        return;
    }

    if (container->isVertical()) {
        setOrientation(Qt::Horizontal);
        return;
    }

    Qt::Orientation newOrientation = Qt::Horizontal;

    for (const Layouting::Item* containerItem: container->childItems()) {
        auto frame = static_cast<KDDockWidgets::Frame*>(containerItem->guestAsQObject());
        if (!frame || frame->dockWidgets().empty()) {
            continue;
        }

        DockType type = readPropertiesFromObject(frame->dockWidgets().first()).type;

        if (type == DockType::Central) {
            newOrientation = Qt::Vertical;
            break;
        }
    }

    setOrientation(newOrientation);
}

void DockToolBar::componentComplete()
{
    DockBase::componentComplete();

    connect(dockWidget(), &KDDockWidgets::DockWidgetQuick::parentChanged, [this]() {
        if (!dockWidget()) {
            return;
        }

        KDDockWidgets::Frame* frame = dockWidget()->frame();
        if (!frame) {
            return;
        }

        connect(frame, &KDDockWidgets::Frame::isInMainWindowChanged, this, [this]() {
            QTimer::singleShot(0, this, &DockToolBar::updateOrientation);
        }, Qt::UniqueConnection);
    });

    m_draggableArea->setDockWidget(dockWidget());
}

DockType DockToolBar::type() const
{
    return DockType::ToolBar;
}
