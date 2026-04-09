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

#include "docktoolbarview.h"

#include "kddockwidgets/src/core/DockWidget.h"
#include "kddockwidgets/src/core/FloatingWindow.h"
#include "kddockwidgets/src/core/Draggable_p.h"
#include "kddockwidgets/src/qtquick/views/DockWidget.h"
#include "kddockwidgets/src/qtquick/views/View.h"

// This include pulls in kdbindings/signal.h which is incompatible with
// Qt's `emit` macro being defined. Temporarily undefine it, then restore it.
#ifdef emit
#undef emit
#include "kddockwidgets/src/core/WindowBeingDragged_p.h"
#define emit
#else
#include "kddockwidgets/src/core/WindowBeingDragged_p.h"
#endif

#include "log.h"
#include "docktypes.h"

using namespace muse::dock;

class DockToolBarView::DockToolBarDraggable : public KDDockWidgets::Core::Draggable
{
public:
    explicit DockToolBarDraggable(KDDockWidgets::Core::DockWidget* dw)
        : KDDockWidgets::Core::Draggable(dw->view())
        , m_dockWidget(dw)
    {
    }

    void setMouseArea(QQuickItem* area)
    {
        m_mouseArea = area;
    }

    std::unique_ptr<KDDockWidgets::Core::WindowBeingDragged> makeWindow() override
    {
        if (!m_dockWidget) {
            return {};
        }

        KDDockWidgets::Core::FloatingWindow* floatingWindow = m_dockWidget->floatingWindow();
        if (floatingWindow) {
            return std::make_unique<KDDockWidgets::Core::WindowBeingDragged>(floatingWindow, this);
        }

        m_dockWidget->setFloating(true);
        floatingWindow = m_dockWidget->floatingWindow();
        return std::make_unique<KDDockWidgets::Core::WindowBeingDragged>(floatingWindow, this);
    }

    KDDockWidgets::Core::DockWidget* singleDockWidget() const override
    {
        return m_dockWidget;
    }

    bool isMDI() const override { return false; }
    bool isWindow() const override { return false; }

    KDDockWidgets::Point mapToWindow(KDDockWidgets::Point pos) const override
    {
        if (!m_mouseArea || !m_dockWidget) {
            return pos;
        }

        auto* dockWidgetItem = KDDockWidgets::QtQuick::asQQuickItem(m_dockWidget);
        if (!dockWidgetItem) {
            return pos;
        }

        QPointF result = m_mouseArea->mapToItem(dockWidgetItem, QPointF(pos.x(), pos.y()));
        result.setX(result.x() + DOCK_WINDOW_SHADOW);
        result.setY(result.y() + DOCK_WINDOW_SHADOW);
        return KDDockWidgets::Point(result.x(), result.y());
    }

private:
    KDDockWidgets::Core::DockWidget* m_dockWidget = nullptr;
    QQuickItem* m_mouseArea = nullptr;
};

//! NOTE: parent (MouseArea) will be set later
DockToolBarView::DockToolBarView(QQuickItem* parent)
    : DockBase(DockType::ToolBar, parent)
{
    setLocation(Location::Top);
}

DockToolBarView::~DockToolBarView()
{
    delete m_draggable;
}

Qt::Orientation DockToolBarView::orientation() const
{
    return m_orientation;
}

int DockToolBarView::alignment() const
{
    return m_alignment;
}

void DockToolBarView::setOrientation(Qt::Orientation orientation)
{
    if (orientation == m_orientation) {
        return;
    }

    m_orientation = orientation;
    emit orientationChanged(orientation);
}

void DockToolBarView::setAlignment(int alignment)
{
    if (alignment == m_alignment) {
        return;
    }

    m_alignment = alignment;
    emit alignmentChanged(alignment);
}

void DockToolBarView::setDraggableMouseArea(QQuickItem* mouseArea)
{
    IF_ASSERT_FAILED(m_draggable) {
        return;
    }

    m_draggable->setMouseArea(mouseArea);

    // Redirect mouse events from the grip/mouseArea to the dock widget view
    // so the DragController can initiate dragging when the user drags the grip
    auto* dwView = qobject_cast<KDDockWidgets::QtQuick::DockWidget*>(
        KDDockWidgets::QtQuick::asQQuickItem(dockWidget()));
    if (dwView) {
        dwView->redirectMouseEvents(mouseArea);
    }
}

void DockToolBarView::componentComplete()
{
    DockBase::componentComplete();

    m_draggable = new DockToolBarDraggable(dockWidget());
}

void DockToolBarView::init()
{
    if (canChangeOrientation()) {
        if (height() > width()) {
            setOrientation(Qt::Vertical);
        } else {
            setOrientation(Qt::Horizontal);
        }
    }

    DockBase::init();
}

bool DockToolBarView::canChangeOrientation() const
{
    if (!floatable()) {
        return false;
    }

    for (const DropDestination& dest : dropDestinations()) {
        if (dest.dropLocation == Location::Left || dest.dropLocation == Location::Right) {
            return true;
        }
    }

    return false;
}

void DockToolBarView::resetToDefault()
{
    DockBase::resetToDefault();

    setOrientation(Qt::Horizontal);
}

void DockToolBarView::onGripDoubleClicked()
{
    toggleFloating();
}

void DockToolBarView::toggleFloating()
{
    if (auto* dw = dockWidget()) {
        bool wasFloating = dw->isFloating();

        // If the toolbar is floating, first make it think it is docked so it can update itself
        // for the docked state. Then we can actually dock it. This is a hack for toolbars that
        // change their size between the docked and undocked state, e.g. the Playback toolbar.
        // Otheriwse docking a toolbar that shrinks itself after docking leaves gaps.
        if (wasFloating) {
            doSetFloating(false);
        }

        dw->setFloating(!wasFloating);

        if (wasFloating) {
            if (dw->isFloating()) { // If it didn't dock for some reason, undo above hack
                doSetFloating(true);
            }

            emit floatingChanged();
        }
    }
}
