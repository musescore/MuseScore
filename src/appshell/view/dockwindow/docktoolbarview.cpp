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

#include "docktoolbarview.h"

#include <QTimer>

#include "thirdparty/KDDockWidgets/src/DockWidgetQuick.h"
#include "thirdparty/KDDockWidgets/src/private/TitleBar_p.h"
#include "thirdparty/KDDockWidgets/src/private/DragController_p.h"

#include "log.h"

using namespace mu::dock;

static constexpr qreal TOOLBAR_GRIP_MARGIN = 4;
static constexpr qreal TOOLBAR_GRIP_WIDTH = 28;
static constexpr qreal TOOLBAR_GRIP_HEIGHT = 36;

const int DockToolBarView::MIN_SIDE_SIZE = 48;
const int DockToolBarView::MAX_SIDE_SIZE = std::numeric_limits<int>::max();

class DockToolBarView::DraggableArea : public KDDockWidgets::QWidgetAdapter, public KDDockWidgets::Draggable
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

DockToolBarView::DockToolBarView(QQuickItem* parent)
    : DockBase(parent),
    //! NOTE: parent (MouseArea) will be set later
    m_draggableArea(new DraggableArea())
{
    setAllowedAreas(Qt::TopDockWidgetArea);
    setLocation(DockLocation::Top);

    setMinimumWidth(MIN_SIDE_SIZE);
    setMaximumWidth(MAX_SIDE_SIZE);
    setMinimumHeight(MIN_SIDE_SIZE);
    setMaximumHeight(MIN_SIDE_SIZE);

    setWidth(MAX_SIDE_SIZE);
    setHeight(MIN_SIDE_SIZE);
}

bool DockToolBarView::movable() const
{
    return m_movable;
}

Qt::Orientation DockToolBarView::orientation() const
{
    return m_orientation;
}

void DockToolBarView::setDraggableMouseArea(QQuickItem* mouseArea)
{
    IF_ASSERT_FAILED(m_draggableArea) {
        return;
    }

    m_draggableArea->setParent(mouseArea);
    m_draggableArea->setMouseArea(mouseArea);
}

QSize DockToolBarView::horizontalPreferredSize() const
{
    return m_horizontalPreferredSize;
}

QSize DockToolBarView::verticalPreferredSize() const
{
    return m_verticalPreferredSize;
}

void DockToolBarView::setMinimumWidth(int width)
{
    if (movable() && orientation() == Qt::Horizontal) {
        width += TOOLBAR_GRIP_WIDTH + TOOLBAR_GRIP_MARGIN;
    }

    DockBase::setMinimumWidth(width);
}

void DockToolBarView::setMinimumHeight(int height)
{
    if (movable() && orientation() == Qt::Vertical) {
        height += TOOLBAR_GRIP_HEIGHT + TOOLBAR_GRIP_MARGIN;
    }

    DockBase::setMinimumHeight(height);
}

void DockToolBarView::setMaximumWidth(int width)
{
    int preferredWidth = this->width();

    if (movable() && orientation() == Qt::Horizontal) {
        preferredWidth = TOOLBAR_GRIP_WIDTH + TOOLBAR_GRIP_MARGIN;
    }

    width = std::max(width, preferredWidth);
    DockBase::setMaximumWidth(width);
}

void DockToolBarView::setMaximumHeight(int height)
{
    int preferredHeight = this->height();

    if (movable() && orientation() == Qt::Horizontal) {
        preferredHeight = TOOLBAR_GRIP_HEIGHT + TOOLBAR_GRIP_MARGIN;
    }

    height = std::max(height, preferredHeight);
    DockBase::setMaximumHeight(height);
}

void DockToolBarView::setMovable(bool movable)
{
    if (m_movable == movable) {
        return;
    }

    m_movable = movable;
    emit movableChanged(m_movable);
}

void DockToolBarView::setOrientation(Qt::Orientation orientation)
{
    bool isChangingAllowed = isOrientationChangingAllowed();

    if (orientation == m_orientation || !isChangingAllowed) {
        return;
    }

    m_orientation = orientation;
    emit orientationChanged(orientation);

    updateSizeConstraints();
}

void DockToolBarView::setHorizontalPreferredSize(QSize horizontalPreferredSize)
{
    if (m_horizontalPreferredSize == horizontalPreferredSize) {
        return;
    }

    m_horizontalPreferredSize = horizontalPreferredSize;
    emit horizontalPreferredSizeChanged(m_horizontalPreferredSize);
}

void DockToolBarView::setVerticalPreferredSize(QSize verticalPreferredSize)
{
    if (m_verticalPreferredSize == verticalPreferredSize) {
        return;
    }

    m_verticalPreferredSize = verticalPreferredSize;
    emit verticalPreferredSizeChanged(m_verticalPreferredSize);
}

void DockToolBarView::componentComplete()
{
    DockBase::componentComplete();

    updateSizeConstraints();

    m_draggableArea->setDockWidget(dockWidget());
}

DockType DockToolBarView::type() const
{
    return DockType::ToolBar;
}

void DockToolBarView::updateSizeConstraints()
{
    bool isHorizontal = m_orientation == Qt::Horizontal;
    QSize preferredSize = isHorizontal ? horizontalPreferredSize() : verticalPreferredSize();

    if (preferredSize.isEmpty()) {
        return;
    }

    if (isHorizontal) {
        setHeight(MIN_SIDE_SIZE);
        setWidth(preferredSize.width());

        setMinimumWidth(preferredSize.width());
        setMinimumHeight(MIN_SIDE_SIZE);
        setMaximumWidth(MAX_SIDE_SIZE);
        setMaximumHeight(MIN_SIDE_SIZE);
    } else {
        setWidth(MIN_SIDE_SIZE);
        setHeight(preferredSize.height());

        setMinimumWidth(MIN_SIDE_SIZE);
        setMinimumHeight(preferredSize.height());
        setMaximumWidth(MIN_SIDE_SIZE);
        setMaximumHeight(MAX_SIDE_SIZE);
    }
}

bool DockToolBarView::isOrientationChangingAllowed() const
{
    return allowedAreas().testFlag(Qt::LeftDockWidgetArea)
           || allowedAreas().testFlag(Qt::RightDockWidgetArea);
}
