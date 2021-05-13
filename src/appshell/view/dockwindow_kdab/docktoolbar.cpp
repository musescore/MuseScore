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

static const qreal TOOLBAR_GRIP_MARGIN = 4;
static const qreal TOOLBAR_GRIP_WIDTH = 28;
static const qreal TOOLBAR_GRIP_HEIGHT = 36;

static constexpr int MIN_SIDE_SIZE = 48;
static constexpr int MAX_SIDE_SIZE = std::numeric_limits<int>::max();

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

    setLocation(DockLocation::Top);
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

QSize DockToolBar::horizontalPreferredSize() const
{
    return m_horizontalPreferredSize;
}

QSize DockToolBar::verticalPreferredSize() const
{
    return m_verticalPreferredSize;
}

DockToolBar* DockToolBar::tabifyToolBar() const
{
    return m_tabifyToolBar;
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
    int preferredWidth = this->width();

    if (movable() && orientation() == Qt::Horizontal) {
        preferredWidth = TOOLBAR_GRIP_WIDTH + TOOLBAR_GRIP_MARGIN;
    }

    width = std::max(width, preferredWidth);
    DockBase::setMaximumWidth(width);
}

void DockToolBar::setMaximumHeight(int height)
{
    int preferredHeight = this->height();

    if (movable() && orientation() == Qt::Horizontal) {
        preferredHeight = TOOLBAR_GRIP_HEIGHT + TOOLBAR_GRIP_MARGIN;
    }

    height = std::max(height, preferredHeight);
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

void DockToolBar::setHorizontalPreferredSize(QSize horizontalPreferredSize)
{
    if (m_horizontalPreferredSize == horizontalPreferredSize) {
        return;
    }

    m_horizontalPreferredSize = horizontalPreferredSize;
    emit horizontalPreferredSizeChanged(m_horizontalPreferredSize);
}

void DockToolBar::setVerticalPreferredSize(QSize verticalPreferredSize)
{
    if (m_verticalPreferredSize == verticalPreferredSize) {
        return;
    }

    m_verticalPreferredSize = verticalPreferredSize;
    emit verticalPreferredSizeChanged(m_verticalPreferredSize);
}

void DockToolBar::setTabifyToolBar(DockToolBar* tabifyToolBar)
{
    if (m_tabifyToolBar == tabifyToolBar) {
        return;
    }

    m_tabifyToolBar = tabifyToolBar;
    emit tabifyToolBarChanged(m_tabifyToolBar);
}

void DockToolBar::componentComplete()
{
    DockBase::componentComplete();

    connect(this, &DockToolBar::orientationChanged, [this](Qt::Orientation orientation) {
        QSize preferredSize = this->preferredSize();

        if (orientation == Qt::Horizontal) {
            setSizeConstraints(QSize(preferredSize.width(), MIN_SIDE_SIZE), QSize(MAX_SIDE_SIZE, MIN_SIDE_SIZE));
        } else {
            setSizeConstraints(QSize(MIN_SIDE_SIZE, preferredSize.height()), QSize(MIN_SIDE_SIZE, MAX_SIDE_SIZE));
        }
    });

    QSize preferredSize = this->preferredSize();
    setSizeConstraints(QSize(preferredSize.width(), MIN_SIDE_SIZE), QSize(MAX_SIDE_SIZE, MIN_SIDE_SIZE));

    m_draggableArea->setDockWidget(dockWidget());
}

QSize DockToolBar::preferredSize() const
{
    return m_orientation == Qt::Horizontal ? horizontalPreferredSize() : verticalPreferredSize();
}

DockType DockToolBar::type() const
{
    return DockType::ToolBar;
}
