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

#include "dropcontroller.h"

#include "../idockwindow.h"

#include "qml/Muse/Dock/dockcentralview.h"
#include "qml/Muse/Dock/dockingholderview.h"
#include "qml/Muse/Dock/dockpageview.h"
#include "qml/Muse/Dock/dockpanelview.h"
#include "qml/Muse/Dock/docktoolbarview.h"

#include "globaltypes.h"

// This include pulls in kdbindings/signal.h which is incompatible with
// Qt's `emit` macro being defined. Temporarily undefine it, then restore it.
#ifdef emit
#undef emit
#include "kddockwidgets/src/core/DragController_p.h"
#define emit
#else
#include "kddockwidgets/src/core/DragController_p.h"
#endif

#include "kddockwidgets/src/qtquick/views/DockWidget.h"
#include "kddockwidgets/src/qtquick/views/View.h"

#include "log.h"

using KDDropLocation = KDDockWidgets::DropLocation;

namespace muse::dock {
static constexpr double MAX_DISTANCE_TO_HOLDER = 50;

static KDDropLocation dropLocationToKDDockLocation(Location location)
{
    switch (location) {
    case Location::Undefined: return KDDropLocation::DropLocation_None;
    case Location::Left: return KDDropLocation::DropLocation_Left;
    case Location::Right: return KDDropLocation::DropLocation_Right;
    case Location::Center: return KDDropLocation::DropLocation_Center;
    case Location::Top: return KDDropLocation::DropLocation_Top;
    case Location::Bottom: return KDDropLocation::DropLocation_Bottom;
    }

    return KDDropLocation::DropLocation_None;
}

static bool isPointAllowedForDrop(const QPoint& point, const DropDestination& dropDestination)
{
    QRect dropRect = dropDestination.dock->frameGeometry();

    if (!dropRect.contains(point)) {
        return false;
    }

    if (dropDestination.dropDistance == 0) {
        return true;
    }

    if (dropDestination.dropLocation == Location::Left) {
        if (std::abs(dropRect.left() - point.x()) <= dropDestination.dropDistance) {
            return true;
        }
    }

    if (dropDestination.dropLocation == Location::Right) {
        if (std::abs(dropRect.right() - point.x()) <= dropDestination.dropDistance) {
            return true;
        }
    }

    return false;
}
}

using namespace muse::dock;

DropController::DropController(KDDockWidgets::Core::ClassicDropIndicatorOverlay* classicIndicators,
                               KDDockWidgets::Core::View* parent,
                               const modularity::ContextPtr& iocCtx)
    : Contextable(iocCtx)
    , m_classicIndicators(classicIndicators)
{
    Q_UNUSED(parent)
}

KDDropLocation DropController::hover(KDDockWidgets::Point globalPos)
{
    DockBase* draggedDock = this->draggedDock();
    if (!draggedDock) {
        return KDDockWidgets::DropLocation_None;
    }

    QPoint hoveredLocalPos = dockWindow()->asItem().mapFromGlobal(globalPos).toPoint();
    DropDestination dropDestination = resolveDropDestination(draggedDock, hoveredLocalPos);

    if (auto toolBar = dynamic_cast<DockToolBarView*>(draggedDock)) {
        updateToolBarOrientation(toolBar, dropDestination);
    }

    setCurrentDropDestination(draggedDock, dropDestination);

    if (m_currentDropDestination.isValid()) {
        auto* dw = m_currentDropDestination.dock->dockWidget();
        auto* dwView = qobject_cast<KDDockWidgets::QtQuick::DockWidget*>(
            KDDockWidgets::QtQuick::asQQuickItem(dw));
        if (dwView) {
            m_classicIndicators->setHoveredGroup(dwView->group());
        }
    }

    return dropLocationToKDDockLocation(m_currentDropDestination.dropLocation);
}

void DropController::setVisible(bool visible)
{
    if (visible) {
        return;
    }

    DockBase* draggedDock = this->draggedDock();
    if (auto toolBar = dynamic_cast<DockToolBarView*>(draggedDock)) {
        updateToolBarOrientation(toolBar);
    }

    endHover();
}

KDDockWidgets::Point DropController::posForIndicator(KDDockWidgets::DropLocation) const
{
    return KDDockWidgets::Point();
}

void DropController::endHover()
{
    if (!m_currentDropDestination.isValid()) {
        return;
    }

    m_currentDropDestination.dock->hideHighlighting();

    if (m_currentDropDestination.dock->type() == DockType::DockingHolder) {
        m_currentDropDestination.dock->close();
    }

    m_currentDropDestination.clear();
}

bool DropController::isMouseOverDock(const QPoint& mouseLocalPos, const DockBase* dock) const
{
    QRect geometry = dock ? dock->frameGeometry() : QRect();
    return geometry.contains(mouseLocalPos);
}

void DropController::updateToolBarOrientation(DockToolBarView* draggedToolBar, const DropDestination& dropDestination)
{
    IF_ASSERT_FAILED(draggedToolBar) {
        return;
    }

    muse::Orientation orientation = muse::Orientation::Horizontal;

    if (!dropDestination.isValid()) {
        draggedToolBar->setOrientation(static_cast<Qt::Orientation>(orientation));
        return;
    }

    switch (dropDestination.dock->location()) {
    case Location::Left:
    case Location::Right:
        orientation = muse::Orientation::Vertical;
        break;
    case Location::Top:
    case Location::Bottom:
        orientation = muse::Orientation::Horizontal;
        break;
    case Location::Center:
    case Location::Undefined:
        break;
    }

    draggedToolBar->setOrientation(static_cast<Qt::Orientation>(orientation));
}

void DropController::setCurrentDropDestination(const DockBase* draggedDock, const DropDestination& dropDestination)
{
    if (m_currentDropDestination == dropDestination) {
        return;
    }

    endHover();

    m_currentDropDestination = dropDestination;

    if (!m_currentDropDestination.isValid()) {
        return;
    }

    auto showHighlighting = [this, draggedDock]() {
        QRect highlightingRect = resolveHighlightingRect(draggedDock, m_currentDropDestination);
        m_currentDropDestination.dock->showHighlighting(highlightingRect);
    };

    if (m_currentDropDestination.dock->type() != DockType::DockingHolder) {
        showHighlighting();
        return;
    }

    switch (m_currentDropDestination.dock->location()) {
    case Location::Left:
    case Location::Right:
        m_currentDropDestination.dock->setMinimumWidth(draggedDock->minimumWidth());
        break;
    case Location::Top:
    case Location::Bottom:
        m_currentDropDestination.dock->setMinimumHeight(draggedDock->minimumHeight());
        break;
    case Location::Center:
    case Location::Undefined:
        break;
    }

    m_currentDropDestination.dock->open();
    showHighlighting();
    m_currentDropDestination.dock->init();
}

DropDestination DropController::resolveDropDestination(const DockBase* draggedDock, const QPoint& localPos) const
{
    if (draggedDock->type() == DockType::Panel) {
        DropDestination destination;

        destination.dock = resolvePanelForDrop(dynamic_cast<const DockPanelView*>(draggedDock), localPos);
        destination.dropLocation = resolveDropLocation(destination.dock, localPos);

        if (destination.isValid()) {
            return destination;
        }
    }

    const DockingHolderView* holder = resolveDockingHolder(draggedDock->type(), localPos);
    QList<DropDestination> destinations = draggedDock->dropDestinations();

    for (const DropDestination& destination : destinations) {
        if (holder == destination.dock) {
            return destination;
        }

        if (isPointAllowedForDrop(localPos, destination)) {
            return destination;
        }
    }

    return DropDestination();
}

DockingHolderView* DropController::resolveDockingHolder(DockType draggedDockType, const QPoint& localPos) const
{
    if (!dockWindow()->asItem().contains(localPos)) {
        return nullptr;
    }

    QRect centralGeometry = currentPage()->centralDock()->frameGeometry();

    // TODO: Need to take any panels docked at top into account
    if (localPos.y() <= centralGeometry.top() + MAX_DISTANCE_TO_HOLDER) {
        return currentPage()->holder(draggedDockType, Location::Top);
    }

    if (localPos.y() >= centralGeometry.bottom() - MAX_DISTANCE_TO_HOLDER) {
        return currentPage()->holder(draggedDockType, Location::Bottom);
    }

    if (localPos.x() <= MAX_DISTANCE_TO_HOLDER) {
        return currentPage()->holder(draggedDockType, Location::Left);
    }

    if (localPos.x() >= dockWindow()->asItem().boundingRect().right() - MAX_DISTANCE_TO_HOLDER) {
        return currentPage()->holder(draggedDockType, Location::Right);
    }

    return nullptr;
}

DockPanelView* DropController::resolvePanelForDrop(const DockPanelView* panel, const QPoint& localPos) const
{
    QList<DockPanelView*> panels = currentPage()->findPanelsForDropping(panel);

    for (DockPanelView* p : panels) {
        if (isMouseOverDock(localPos, p)) {
            return p;
        }
    }

    return nullptr;
}

muse::dock::Location DropController::resolveDropLocation(const DockBase* hoveredDock, const QPoint& localPos) const
{
    if (!hoveredDock) {
        return Location::Undefined;
    }

    QRect geometry = hoveredDock->frameGeometry();
    Location dockLocation = hoveredDock->location();

    qreal frameEnd = geometry.bottom();
    qreal mousePos = localPos.y();
    Location beginDropLocation = Location::Top;
    Location endDropLocation = Location::Bottom;

    if (dockLocation == Location::Top || dockLocation == Location::Bottom) {
        mousePos = localPos.x();
        frameEnd = geometry.right();
        beginDropLocation = Location::Left;
        endDropLocation = Location::Right;
    }

    if (mousePos <= frameEnd / 3) {
        return beginDropLocation;
    }

    if (mousePos <= frameEnd / 1.5) {
        return Location::Center;
    }

    if (mousePos <= frameEnd) {
        return endDropLocation;
    }

    return Location::Undefined;
}

QRect DropController::resolveHighlightingRect(const DockBase* draggedDock, const DropDestination& destination) const
{
    if (!destination.isValid()) {
        return QRect();
    }

    QRect frameGeometry = destination.dock->frameGeometry();
    int frameWidth = frameGeometry.width();
    int frameHeight = frameGeometry.height();
    QRect fullFrameHighlightingRect = QRect(0, 0, frameWidth, frameHeight);

    if (destination.dock->type() == DockType::DockingHolder) {
        return fullFrameHighlightingRect;
    }

    if (destination.dock->type() == DockType::Central) {
        int draggedDockWidth = draggedDock->frameGeometry().width();

        if (destination.dropLocation == Location::Left) {
            return QRect(0, 0, draggedDockWidth, frameHeight);
        }

        if (destination.dropLocation == Location::Right) {
            return QRect(frameWidth - draggedDockWidth, 0, draggedDockWidth, frameHeight);
        }
    }

    switch (destination.dropLocation) {
    case Location::Top:
        return QRect(0, 0, frameWidth, frameHeight / 2);
    case Location::Bottom:
        return QRect(0, frameHeight / 2, frameWidth, frameHeight / 2);
    case Location::Left:
        return QRect(0, 0, frameWidth / 2, frameHeight);
    case Location::Right:
        return QRect(frameWidth / 2, 0, frameWidth / 2, frameHeight);
    case Location::Center:
        return fullFrameHighlightingRect;
    case Location::Undefined:
        break;
    }

    return QRect();
}

IDockWindow* DropController::dockWindow() const
{
    return dockWindowProvider()->window();
}

DockPageView* DropController::currentPage() const
{
    return dockWindow() ? dockWindow()->currentPage() : nullptr;
}

DockBase* DropController::draggedDock() const
{
    auto windowBeingDragged = KDDockWidgets::Core::DragController::instance()->windowBeingDragged();
    if (!windowBeingDragged || windowBeingDragged->dockWidgets().isEmpty()) {
        return nullptr;
    }

    const DockPageView* page = currentPage();
    if (!page) {
        return nullptr;
    }

    KDDockWidgets::Core::DockWidget* draggedDockWidget = windowBeingDragged->dockWidgets().first();
    for (DockBase* dock : page->allDocks()) {
        if (dock->dockWidget() == draggedDockWidget) {
            return dock;
        }
    }

    return nullptr;
}
