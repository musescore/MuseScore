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
#include "indicatorswindow.h"

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

static IndicatorsWindow* createIndicatorWindow(DropIndicators* dropIndicators)
{
    auto window = new IndicatorsWindow(dropIndicators);
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
    return m_indicatorsWindow->hover(globalPos);
}

QPoint DropIndicators::posForIndicator(DropLocation loc) const
{
    return m_indicatorsWindow->posForIndicator(loc);
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
    return isIndicatorVisible(IndicatorType::Outter, Qt::TopDockWidgetArea);
}

bool DropIndicators::outterBottomIndicatorVisible() const
{
    return isIndicatorVisible(IndicatorType::Outter, Qt::BottomDockWidgetArea);
}

bool DropIndicators::centralIndicatorVisible() const
{
    return isIndicatorVisible(IndicatorType::Central);
}

bool DropIndicators::innerLeftIndicatorVisible() const
{
    return isIndicatorVisible(IndicatorType::Inner, Qt::LeftDockWidgetArea);
}

bool DropIndicators::innerRightIndicatorVisible() const
{
    return isIndicatorVisible(IndicatorType::Inner, Qt::RightDockWidgetArea);
}

bool DropIndicators::innerTopIndicatorVisible() const
{
    return isIndicatorVisible(IndicatorType::Inner, Qt::TopDockWidgetArea);
}

bool DropIndicators::innerBottomIndicatorVisible() const
{
    return isIndicatorVisible(IndicatorType::Inner, Qt::BottomDockWidgetArea);
}

bool DropIndicators::isIndicatorVisible(IndicatorType type, Qt::DockWidgetArea area) const
{
    if (isToolBar()) {
        return false;
    }

    switch (type) {
    case IndicatorType::Outter:
        return isAreaAllowed(area);
    case IndicatorType::Central:
        return !hoveringOverDock(DockType::Central);
    case IndicatorType::Inner: {
        if (hoveringOverDock(DockType::Central)) {
            return isAreaAllowed(area);
        }

        return true;
    }
    }

    return false;
}

bool DropIndicators::hoveringOverDock(DockType type) const
{
    const KDDockWidgets::DockWidgetBase* dock = hoveredDock();
    DockProperties properties = readPropertiesFromObject(dock);
    return properties.type == type;
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

bool DropIndicators::isAreaAllowed(Qt::DockWidgetArea area) const
{
    return m_draggedDockProperties.allowedAreas.testFlag(area);
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

bool DropIndicators::onResize(QSize)
{
    m_indicatorsWindow->resize(window()->size());
    return false;
}

void DropIndicators::updateVisibility()
{
    if (isHovered() && m_hoveredFrame) {
        m_indicatorsWindow->setVisible(true);
        updateWindowPosition();
        m_indicatorsWindow->raise();
    } else {
        m_rubberBand->setVisible(false);
        m_indicatorsWindow->setVisible(false);
    }

    m_draggedDockProperties = DockProperties();

    auto windowBeingDragged = KDDockWidgets::DragController::instance()->windowBeingDragged();
    if (!windowBeingDragged || windowBeingDragged->dockWidgets().isEmpty()) {
        return;
    }

    auto dock = windowBeingDragged->dockWidgets().first();
    m_draggedDockProperties = readPropertiesFromObject(dock); 

    if (hoveredDock()) {
        LOGI() << "$$$ hovering over: " << hoveredDock()->uniqueName();
    }

    emit indicatorsVisibilityChanged();
}

void DropIndicators::setDropLocation(DropLocation location)
{
    setCurrentDropLocation(location);

    if (location == DropLocation_None) {
        m_rubberBand->setVisible(false);
        return;
    }

    if (location == DropLocation_Center) {
        m_rubberBand->setGeometry(m_hoveredFrame ? m_hoveredFrame->QWidgetAdapter::geometry() : rect());
        m_rubberBand->setVisible(true);
        return;
    }

    KDDockWidgets::Location multisplitterLocation = locationToMultisplitterLocation(location);
    KDDockWidgets::Frame* relativeToFrame = nullptr;

    switch (location) {
    case DropLocation_Left:
    case DropLocation_Top:
    case DropLocation_Right:
    case DropLocation_Bottom:
        if (!m_hoveredFrame) {
            qWarning() << "DropIndicators::setCurrentDropLocation: frame is null. location=" << location
                       << "; isHovered=" << isHovered()
                       << "; dropArea->widgets=" << m_dropArea->items();
            Q_ASSERT(false);
            return;
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

    QRect rect = m_dropArea->rectForDrop(windowBeingDragged, multisplitterLocation,
                                         m_dropArea->itemForFrame(relativeToFrame));

    auto draggedDock = this->draggedDock();

    if (m_hoveredFrame && isToolBar() && draggedDock) {
        QString draggedDockName = draggedDock->uniqueName();

        if (hoveringOverDock(DockType::ToolBar)) {
            mainWindow()->requestChangeToolBarOrientation(draggedDockName, framework::Orientation::Horizontal);
        } else {
            mainWindow()->requestChangeToolBarOrientation(draggedDockName, framework::Orientation::Vertical);
        }
    }

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
