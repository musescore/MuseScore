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

#include "dropindicators.h"
#include "dropindicatorswindow.h"

#include "thirdparty/KDDockWidgets/src/Config.h"
#include "thirdparty/KDDockWidgets/src/private/DragController_p.h"
#include "thirdparty/KDDockWidgets/src/private/Utils_p.h"
#include "thirdparty/KDDockWidgets/src/FrameworkWidgetFactory.h"

#include "log.h"

namespace mu::dock {
KDDockWidgets::Location locationToMultisplitterLocation(DropIndicators::DropLocation location)
{
    switch (location) {
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_Left:
        return KDDockWidgets::Location_OnLeft;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_Top:
        return KDDockWidgets::Location_OnTop;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_Right:
        return KDDockWidgets::Location_OnRight;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_Bottom:
        return KDDockWidgets::Location_OnBottom;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_OutterLeft:
        return KDDockWidgets::Location_OnLeft;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_OutterTop:
        return KDDockWidgets::Location_OnTop;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_OutterRight:
        return KDDockWidgets::Location_OnRight;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_OutterBottom:
        return KDDockWidgets::Location_OnBottom;
    default:
        return KDDockWidgets::Location_None;
    }
}

Qt::DockWidgetArea locationToDockArea(KDDockWidgets::DropIndicatorOverlayInterface::DropLocation location)
{
    using DropLocation = KDDockWidgets::DropIndicatorOverlayInterface;

    switch (location) {
    case DropLocation::DropLocation_Left: return Qt::LeftDockWidgetArea;
    case DropLocation::DropLocation_Right: return Qt::RightDockWidgetArea;
    case DropLocation::DropLocation_Top: return Qt::TopDockWidgetArea;
    case DropLocation::DropLocation_Bottom: return Qt::BottomDockWidgetArea;
    case DropLocation::DropLocation_None:
    default: return Qt::NoDockWidgetArea;
    }
}

static DropIndicatorsWindow* createIndicatorWindow(DropIndicators* dropIndicators)
{
    auto window = new DropIndicatorsWindow(dropIndicators);
    window->setObjectName(QStringLiteral("_docks_IndicatorWindow_Overlay"));
    return window;
}
}

using namespace mu::dock;

DropIndicators::DropIndicators(KDDockWidgets::DropArea* dropArea)
    : KDDockWidgets::DropIndicatorOverlayInterface(dropArea),
    m_rubberBand(KDDockWidgets::Config::self().frameworkWidgetFactory()->createRubberBand(dropArea)),
    m_indicatorsWindow(createIndicatorWindow(this))
{
}

DropIndicators::~DropIndicators()
{
    delete m_indicatorsWindow;
}

KDDockWidgets::DropIndicatorOverlayInterface::DropLocation DropIndicators::hover_impl(QPoint globalPos)
{
    DropLocation dropLocation = DropLocation_None;
    QRect dropRect = QRect();

    if (isToolBar()) {
        dropLocation = dropLocationForToolBar(globalPos);
        dropRect = dropAreaRectForToolBar(dropLocation);
    } else {
        dropLocation = m_indicatorsWindow->dropLocationForPosition(globalPos);
        dropRect = dropAreaRectForPanel(dropLocation);
    }

    showDropAreaIfNeed(dropRect, dropLocation);

    return dropLocation;
}

QPoint DropIndicators::posForIndicator(DropLocation loc) const
{
    return m_indicatorsWindow->positionForIndicator(loc);
}

bool DropIndicators::outterLeftIndicatorVisible() const
{
    return false;
}

bool DropIndicators::outterRightIndicatorVisible() const
{
    return false;
}

bool DropIndicators::outterTopIndicatorVisible() const
{
    return false;
}

bool DropIndicators::outterBottomIndicatorVisible() const
{
    return false;
}

bool DropIndicators::centralIndicatorVisible() const
{
    return isIndicatorVisible(DropLocation::DropLocation_Center);
}

bool DropIndicators::innerLeftIndicatorVisible() const
{
    return isIndicatorVisible(DropLocation::DropLocation_Left);
}

bool DropIndicators::innerRightIndicatorVisible() const
{
    return isIndicatorVisible(DropLocation::DropLocation_Right);
}

bool DropIndicators::innerTopIndicatorVisible() const
{
    return isIndicatorVisible(DropLocation::DropLocation_Top);
}

bool DropIndicators::innerBottomIndicatorVisible() const
{
    return isIndicatorVisible(DropLocation::DropLocation_Bottom);
}

bool DropIndicators::isIndicatorVisible(DropLocation location) const
{
    if (isToolBar()) {
        return false;
    }

    return isDropAllowed(location);
}

bool DropIndicators::onResize(QSize)
{
    m_indicatorsWindow->resize(window()->size());
    return false;
}

void DropIndicators::updateVisibility()
{
    if (isHovered()) {
        m_indicatorsWindow->setVisible(true);
        updateWindowPosition();
        m_indicatorsWindow->raise();
    } else {
        m_rubberBand->setVisible(false);
        m_indicatorsWindow->setVisible(false);
        mainWindow()->setDockingHelperVisible(false);
    }

    m_draggedDockProperties = readPropertiesFromObject(draggedDock());

    emit indicatorsVisibilityChanged();
}

bool DropIndicators::isDropAllowed(DropLocation location) const
{
    if (location == DropLocation_None) {
        return false;
    }

    const KDDockWidgets::DockWidgetBase* hoveredDock = this->hoveredDock();
    if (!hoveredDock || hoveredDock->isFloating()) {
        return false;
    }

    DockProperties hoveredDockProperties = readPropertiesFromObject(hoveredDock);
    DockProperties draggedDockProperties = readPropertiesFromObject(draggedDock());

    DockType hoveredDockType = hoveredDockProperties.type;
    DockType draggedDockType = draggedDockProperties.type;

    auto isDragged = [draggedDockType](DockType type) {
        return draggedDockType == type;
    };

    auto isHovered = [hoveredDockType](DockType type) {
        return hoveredDockType == type;
    };

    static const QSet<DropLocation> sideLocations {
        DropLocation::DropLocation_Left,
        DropLocation::DropLocation_Right
    };

    if (hoveredDockType == draggedDockType) {
        return sideLocations.contains(location) || isDragged(DockType::Panel);
    }

    if (isHovered(DockType::Central)) {
        Qt::DockWidgetArea area = locationToDockArea(location);
        return draggedDockProperties.allowedAreas.testFlag(area);
    }

    return false;
}

bool DropIndicators::isToolBar() const
{
    return m_draggedDockProperties.type == DockType::ToolBar;
}

const KDDockWidgets::DockWidgetBase* DropIndicators::draggedDock() const
{
    auto windowBeingDragged = KDDockWidgets::DragController::instance()->windowBeingDragged();
    if (!windowBeingDragged || windowBeingDragged->dockWidgets().isEmpty()) {
        return nullptr;
    }

    return windowBeingDragged->dockWidgets().first();
}

const KDDockWidgets::DockWidgetBase* DropIndicators::hoveredDock() const
{
    if (!m_hoveredFrame) {
        return nullptr;
    }

    auto docks = m_hoveredFrame->dockWidgets();

    if (docks.isEmpty()) {
        return nullptr;
    }

    return docks.first();
}

DropIndicators::DropLocation DropIndicators::dropLocationForToolBar(const QPoint& hoveredGlobalPos) const
{
    DockType hoveredDockType = readPropertiesFromObject(hoveredDock()).type;
    if (hoveredDockType != DockType::ToolBar) {
        return DropLocation_None;
    }

    QRect dropAreaRect = hoveredFrameRect();

    int distanceToLeftCorner = std::abs(dropAreaRect.x() - hoveredGlobalPos.x());
    int distanceToRightCorner = std::abs(dropAreaRect.x() + dropAreaRect.width() - hoveredGlobalPos.x());

    if (distanceToRightCorner < distanceToLeftCorner) {
        return DropLocation_Right;
    }

    return DropLocation_Left;
}

QRect DropIndicators::dropAreaRectForToolBar(DropLocation location) const
{
    const KDDockWidgets::DockWidgetBase* draggedDock = this->draggedDock();
    const KDDockWidgets::DockWidgetBase* hoveredDock = this->hoveredDock();

    if (!draggedDock || !hoveredDock) {
        return QRect();
    }

    DockType hoveredDockType = readPropertiesFromObject(hoveredDock).type;
    if (hoveredDockType != DockType::ToolBar) {
        return QRect();
    }

    QRect dropAreaRect = hoveredFrameRect();

    if (location == DropLocation_Right) {
        dropAreaRect.setX(dropAreaRect.x() + dropAreaRect.width() - draggedDock->width());
    }

    if (draggedDock->width() <= hoveredDock->width() / 2) {
        dropAreaRect.setWidth(draggedDock->width());
    }

    return dropAreaRect;
}

QRect DropIndicators::dropAreaRectForPanel(DropLocation location) const
{
    if (location == DropLocation_Center) {
        return m_hoveredFrame ? m_hoveredFrame->QWidgetAdapter::geometry() : rect();
    }

    KDDockWidgets::Location multisplitterLocation = locationToMultisplitterLocation(location);
    const KDDockWidgets::Frame* relativeToFrame = nullptr;

    switch (location) {
    case DropLocation_Left:
    case DropLocation_Top:
    case DropLocation_Right:
    case DropLocation_Bottom:
        if (!m_hoveredFrame) {
            return QRect();
        }
        relativeToFrame = m_hoveredFrame;
        break;
    case DropLocation_OutterLeft:
    case DropLocation_OutterTop:
    case DropLocation_OutterRight:
    case DropLocation_OutterBottom:
        break;
    default:
        break;
    }

    auto windowBeingDragged = KDDockWidgets::DragController::instance()->windowBeingDragged();
    auto draggedDock = this->draggedDock();

    if (m_hoveredFrame && isToolBar() && draggedDock) {
        QString draggedDockName = draggedDock->uniqueName();

        if (hoveringOverDock(DockType::ToolBar)) {
            mainWindow()->requestChangeToolBarOrientation(draggedDockName, framework::Orientation::Horizontal);
        } else {
            mainWindow()->requestChangeToolBarOrientation(draggedDockName, framework::Orientation::Vertical);
        }
    }

    return m_dropArea->rectForDrop(windowBeingDragged, multisplitterLocation,
                                   m_dropArea->itemForFrame(relativeToFrame));
}

void DropIndicators::showDropAreaIfNeed(const QRect& rect, DropLocation location)
{
    if (rect.isNull()) {
        return;
    }

    if (!isDropAllowed(location)) {
        return;
    }

    if (isToolBar()) {
        mainWindow()->setDockingHelperVisible(true);
    }

    setCurrentDropLocation(location);

    m_rubberBand->setGeometry(rect);
    m_rubberBand->setVisible(true);
}

void DropIndicators::updateWindowPosition()
{
    QRect rect = this->rect();
    if (KDDockWidgets::isWindow(m_indicatorsWindow)) {
        // On all non-wayland platforms it's a top-level.
        QPoint pos = mapToGlobal(QPoint(0, 0));
        rect.moveTo(pos);
    }
    m_indicatorsWindow->setGeometry(rect);
}
