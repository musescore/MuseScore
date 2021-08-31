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

    if (isDraggedDockToolBar()) {
        dropLocation = dropLocationForToolBar(globalPos);
        dropRect = dropAreaRectForToolBar(dropLocation);
    } else {
        dropLocation = m_indicatorsWindow->dropLocationForPosition(globalPos);
        dropRect = dropAreaRectForPanel(dropLocation);
    }

    if (needShowToolBarHolders()) {
        dockWindow()->showDockingHolder(globalPos, IDockWindow::ToolBar);
    }

    if (needShowPanelHolders()) {
        dockWindow()->showDockingHolder(globalPos, IDockWindow::Panel);
    }

    if (isDropAllowed(dropLocation)) {
        setCurrentDropLocation(dropLocation);
        showDropAreaIfNeed(dropRect);
    } else {
        hideDropArea();
    }

    updateToolBarOrientation();

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
    if (isDraggedDockToolBar()) {
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
    m_indicatorsWindow->setVisible(isHovered());

    if (isHovered()) {
        updateWindowPosition();
        m_indicatorsWindow->raise();
    } else {
        dockWindow()->hideAllDockingHolders();
        hideDropArea();
    }

    emit indicatorsVisibilityChanged();
}

bool DropIndicators::isDropAllowed(DropLocation location) const
{
    if (location == DropLocation_None) {
        return false;
    }

    const KDDockWidgets::DockWidgetBase* hoveredDock = this->hoveredDock();
    const KDDockWidgets::DockWidgetBase* draggedDock = this->draggedDock();

    if (!hoveredDock || !draggedDock || hoveredDock->isFloating()) {
        return false;
    }

    DockProperties hoveredDockProperties = readPropertiesFromObject(hoveredDock);
    DockProperties draggedDockProperties = readPropertiesFromObject(draggedDock);

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
    bool isSideLocation = sideLocations.contains(location);

    switch (hoveredDockType) {
    case DockType::Central: {
        if (isDragged(DockType::Panel)) {
            Qt::DockWidgetArea area = locationToDockArea(location);
            bool isAreaAllowed = draggedDockProperties.allowedAreas.testFlag(area);

            // For top/bottom location, we need to use the Panel-/Toolbar docking holders
            // Because a panel or toolbar at the top needs to go also outside the left/right
            // panels/toolbars. Therefore, only side locations are allowed.
            return isSideLocation && isAreaAllowed;
        }
    } break;

    case DockType::Panel:
    case DockType::PanelDockingHolder: {
        if (!isDragged(DockType::Panel)) {
            return false;
        }

        // TODO: Determine location of hovered panel or panel docking holder and check if
        // that is one of the allowed areas of the dragged panel

        if (isHovered(DockType::PanelDockingHolder)) {
            // Avoid tabbing with docking holder, because it breaks the holders system
            return location != DropLocation_Center;
        }

        return true;
    } break;

    case DockType::ToolBar:
    case DockType::ToolBarDockingHolder: {
        if (!isDragged(DockType::ToolBar)) {
            return false;
        }

        // TODO: what is our policy with vertical toolbars?
        // Currently not important because there's at most one toolbar that may become vertical.
        bool equalOrientations = dockOrientation(*hoveredDock) == dockOrientation(*draggedDock);
        return isSideLocation && equalOrientations;
    } break;

    default:
        break;
    }

    return false;
}

bool DropIndicators::isDropOnHoveredDockAllowed() const
{
    DockType hoveredDockType = dockType(hoveredDock());

    if (isDraggedDockToolBar()) {
        return hoveredDockType == DockType::ToolBar || hoveredDockType == DockType::ToolBarDockingHolder;
    }

    if (isDraggedDockPanel()) {
        return hoveredDockType == DockType::Panel || hoveredDockType == DockType::PanelDockingHolder;
    }

    return true;
}

bool DropIndicators::isDraggedDockToolBar() const
{
    return dockType(draggedDock()) == DockType::ToolBar;
}

bool DropIndicators::isDraggedDockPanel() const
{
    return dockType(draggedDock()) == DockType::Panel;
}

bool DropIndicators::needShowToolBarHolders() const
{
    if (!isDraggedDockToolBar()) {
        return false;
    }

    Qt::DockWidgetAreas areas = readPropertiesFromObject(draggedDock()).allowedAreas;
    return areas.testFlag(Qt::LeftDockWidgetArea) || areas.testFlag(Qt::RightDockWidgetArea);
}

bool DropIndicators::needShowPanelHolders() const
{
    if (!isDraggedDockPanel()) {
        return false;
    }

    Qt::DockWidgetAreas areas = readPropertiesFromObject(draggedDock()).allowedAreas;
    return areas.testFlag(Qt::TopDockWidgetArea) || areas.testFlag(Qt::BottomDockWidgetArea);
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

mu::framework::Orientation DropIndicators::dockOrientation(const KDDockWidgets::DockWidgetBase& dock) const
{
    return dock.width() < dock.height() ? framework::Orientation::Vertical : framework::Orientation::Horizontal;
}

DockType DropIndicators::dockType(const KDDockWidgets::DockWidgetBase* dock) const
{
    return readPropertiesFromObject(dock).type;
}

DropIndicators::DropLocation DropIndicators::dropLocationForToolBar(const QPoint& hoveredGlobalPos) const
{
    if (!isDropOnHoveredDockAllowed()) {
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

    return m_dropArea->rectForDrop(windowBeingDragged, multisplitterLocation,
                                   m_dropArea->itemForFrame(relativeToFrame));
}

void DropIndicators::showDropAreaIfNeed(const QRect& dropRect)
{
    if (dropRect.isValid() && isDropOnHoveredDockAllowed()) {
        m_rubberBand->setGeometry(dropRect);
        m_rubberBand->setVisible(true);
    }
}

void DropIndicators::hideDropArea()
{
    setCurrentDropLocation(DropLocation_None);
    m_rubberBand->setVisible(false);
}

void DropIndicators::updateToolBarOrientation()
{
    auto draggedDock = this->draggedDock();
    auto hoveredDock = this->hoveredDock();

    if (!isDraggedDockToolBar() || !draggedDock || !hoveredDock) {
        return;
    }

    framework::Orientation newOrientation = framework::Orientation::Horizontal;
    bool isHoveredDockVertical = dockOrientation(*hoveredDock) == framework::Orientation::Vertical;

    if (isDropOnHoveredDockAllowed() && isHoveredDockVertical) {
        newOrientation = framework::Orientation::Vertical;
    }

    dockWindow()->setToolBarOrientation(draggedDock->uniqueName(), newOrientation);
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
