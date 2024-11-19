/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "core/DropIndicatorOverlay.h"
#include "core/DropIndicatorOverlay_p.h"
#include "Config.h"
#include "Platform.h"
#include "ViewFactory.h"

#include "core/DropArea.h"
#include "core/Group.h"
#include "core/Logging_p.h"
#include "core/Controller_p.h"
#include "core/WindowBeingDragged_p.h"

#include "core/DragController_p.h"
#include "core/DockRegistry_p.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

DropIndicatorOverlay::DropIndicatorOverlay(DropArea *dropArea, View *view)
    : Controller(ViewType::DropAreaIndicatorOverlay, view)
    , d(new Private())
    , m_dropArea(dropArea)
{
    setVisible(false);
    view->setViewName(QStringLiteral("DropIndicatorOverlay"));

    // Set transparent for mouse events so that topLevel->childAt() never returns the drop indicator
    // overlay
    view->enableAttribute(Qt::WA_TransparentForMouseEvents);

    d->dropIndicatorsInhibitedConnection = DockRegistry::self()->dptr()->dropIndicatorsInhibitedChanged.connect([this](bool inhibited) {
        if (inhibited) {
            removeHover();
        } else {
            // Re-add hover. Fastest way is simply faking a mouse move
            if (auto state = object_cast<StateDragging *>(DragController::instance()->activeState())) {
                state->handleMouseMove(Platform::instance()->cursorPos());
            }
        }
    });
}


DropIndicatorOverlay::DropIndicatorOverlay(Core::DropArea *dropArea)
    : DropIndicatorOverlay(dropArea, Platform::instance()->createView(this, dropArea->view()))
{
}

DropIndicatorOverlay::~DropIndicatorOverlay()
{
    delete d;
}

void DropIndicatorOverlay::setWindowBeingDragged(bool is)
{
    if (is == m_draggedWindowIsHovering)
        return;

    m_draggedWindowIsHovering = is;
    if (is) {
        view()->setGeometry(m_dropArea->rect());
        view()->raise();
    } else {
        setHoveredGroup(nullptr);
    }

    setVisible(is);
    updateVisibility();
}

Rect DropIndicatorOverlay::hoveredGroupRect() const
{
    return m_hoveredGroupRect;
}

void DropIndicatorOverlay::setHoveredGroup(Core::Group *group)
{
    if (group == m_hoveredGroup)
        return;

    if (WindowBeingDragged *wbd = DragController::instance()->windowBeingDragged()) {
        if (wbd->isInWaylandDrag(group)) {
            // With wayland, we don't detach the group before the mouse release.
            // Instead, we start a QDrag, with this group as cursor QPixmap.
            // Here we catch the case where we're dragging the pixmap onto the group
            // where're already dragging. That's a no-op.
            return;
        }
    }

    if (m_hoveredGroup)
        d->groupConnection = KDBindings::ScopedConnection();

    m_hoveredGroup = group;
    if (m_hoveredGroup) {
        d->groupConnection = group->Controller::dptr()->aboutToBeDeleted.connect([this] { onGroupDestroyed(); });
        setHoveredGroupRect(m_hoveredGroup->view()->geometry());
    } else {
        setHoveredGroupRect(Rect());
    }

    updateVisibility();
    d->hoveredGroupChanged.emit(m_hoveredGroup);
    onHoveredGroupChanged(m_hoveredGroup);
}

bool DropIndicatorOverlay::isHovered() const
{
    return m_draggedWindowIsHovering;
}

DropLocation DropIndicatorOverlay::currentDropLocation() const
{
    return m_currentDropLocation;
}

KDDockWidgets::Location DropIndicatorOverlay::multisplitterLocationFor(DropLocation dropLoc)
{
    switch (dropLoc) {
    case DropLocation_None:
        return KDDockWidgets::Location_None;
    case DropLocation_Left:
    case DropLocation_OutterLeft:
        return KDDockWidgets::Location_OnLeft;
    case DropLocation_OutterTop:
    case DropLocation_Top:
        return KDDockWidgets::Location_OnTop;
    case DropLocation_OutterRight:
    case DropLocation_Right:
        return KDDockWidgets::Location_OnRight;
    case DropLocation_OutterBottom:
    case DropLocation_Bottom:
        return KDDockWidgets::Location_OnBottom;
    case DropLocation_Center:
        return KDDockWidgets::Location_None;
    case DropLocation_Inner:
    case DropLocation_Outter:
    case DropLocation_Horizontal:
    case DropLocation_Vertical:
        KDDW_ERROR("Unexpected drop location={}", dropLoc);
        break;
    }

    return KDDockWidgets::Location_None;
}

bool DropIndicatorOverlay::dropIndicatorVisible(DropLocation dropLoc) const
{
    if (dropLoc == DropLocation_None)
        return false;

    WindowBeingDragged *windowBeingDragged = DragController::instance()->windowBeingDragged();
    if (!windowBeingDragged)
        return false;

    const Core::DockWidget::List source = windowBeingDragged->dockWidgets();
    const Core::DockWidget::List target =
        m_hoveredGroup ? m_hoveredGroup->dockWidgets() : Core::DockWidget::List();

    const bool isInner = dropLoc & DropLocation_Inner;
    const bool isOutter = dropLoc & DropLocation_Outter;
    if (isInner) {
        if (!m_hoveredGroup)
            return false;
    } else if (isOutter) {
        // If there's only 1 group in the layout, the outer indicators are redundant, as they do the
        // same thing as the internal ones. But there might be another window obscuring our target,
        // so it's useful to show the outer indicators in this case
        const bool isTheOnlyGroup = m_hoveredGroup && m_hoveredGroup->isTheOnlyGroup();
        if (isTheOnlyGroup
            && !DockRegistry::self()->isProbablyObscured(m_hoveredGroup->view()->window(),
                                                         windowBeingDragged))
            return false;
    } else if (dropLoc == DropLocation_Center) {
        if (!m_hoveredGroup || !m_hoveredGroup->isDockable())
            return false;

        // Only allow to dock to center if the affinities match
        if (!DockRegistry::self()->affinitiesMatch(m_hoveredGroup->affinities(),
                                                   windowBeingDragged->affinities()))
            return false;
    } else {
        KDDW_ERROR("Unknown drop indicator location={}", dropLoc);
        return false;
    }

    if (auto dropIndicatorAllowedFunc = Config::self().dropIndicatorAllowedFunc()) {
        DropArea *dropArea = DragController::instance()->dropAreaUnderCursor();
        if (!dropIndicatorAllowedFunc(dropLoc, source, target, dropArea))
            return false;
    }

    return true;
}

void DropIndicatorOverlay::onGroupDestroyed()
{
    setHoveredGroup(nullptr);
}

DropIndicatorOverlay::Private *DropIndicatorOverlay::dptr() const
{
    return d;
}

void DropIndicatorOverlay::onHoveredGroupChanged(Core::Group *)
{
}

void DropIndicatorOverlay::setCurrentDropLocation(DropLocation location)
{
    if (m_currentDropLocation != location) {
        m_currentDropLocation = location;
        d->currentDropLocationChanged.emit();
    }
}

DropLocation DropIndicatorOverlay::hover(Point globalPos)
{
    const DropLocation loc = hover_impl(globalPos);
    setCurrentDropLocation(loc);
    return loc;
}

void DropIndicatorOverlay::setHoveredGroupRect(Rect rect)
{
    if (m_hoveredGroupRect != rect) {
        m_hoveredGroupRect = rect;
        d->hoveredGroupRectChanged.emit();
    }
}

void DropIndicatorOverlay::removeHover()
{
    setWindowBeingDragged(false);
    setCurrentDropLocation(DropLocation_None);
}

Group *DropIndicatorOverlay::hoveredGroup() const
{
    return m_hoveredGroup;
}

void DropIndicatorOverlay::updateVisibility()
{
}
