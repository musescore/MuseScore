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

#include "dragcontroller.h"

#include "../idockwindow.h"
#include "../docktypes.h"

using namespace mu::dock;

using DockWidget = KDDockWidgets::DockWidgetBase;
using KDDropLocation = KDDockWidgets::DropIndicatorOverlayInterface::DropLocation;

namespace mu::dock {
static const DockWidget* draggedDock()
{
    auto windowBeingDragged = KDDockWidgets::DragController::instance()->windowBeingDragged();
    if (!windowBeingDragged || windowBeingDragged->dockWidgets().isEmpty()) {
        return nullptr;
    }

    return windowBeingDragged->dockWidgets().first();
}

static KDDropLocation dropLocationToKDDockLocation(DropLocation::Location location)
{
    switch (location) {
    case DropLocation::None: return KDDropLocation::DropLocation_None;
    case DropLocation::Left: return KDDropLocation::DropLocation_Left;
    case DropLocation::Right: return KDDropLocation::DropLocation_Right;
    case DropLocation::Center: return KDDropLocation::DropLocation_Center;
    case DropLocation::Top: return KDDropLocation::DropLocation_Top;
    case DropLocation::Bottom: return KDDropLocation::DropLocation_Bottom;
    }

    return KDDropLocation::DropLocation_None;
}
}

DragController::DragController(KDDockWidgets::DropArea* dropArea)
    : KDDockWidgets::DropIndicatorOverlayInterface(dropArea)
{

}

KDDropLocation DragController::hover_impl(QPoint globalPos)
{
    IDockWindow* window = dockWindow();
    const DockWidget* dock = draggedDock();

    if (!window || !dock) {
        return DropLocation_None;
    }

    dock::DropLocation::Location location = window->hover(dock->uniqueName(), globalPos);
    setCurrentDropLocation(dropLocationToKDDockLocation(location));

    return currentDropLocation();
}

void DragController::updateVisibility()
{
    if (draggedDock()) {
        return;
    }

    setCurrentDropLocation(DropLocation_None);

    if (auto window = dockWindow()) {
        window->endHover();
    }
}

QPoint DragController::posForIndicator(KDDropLocation) const
{
    return QPoint();
}

IDockWindow* DragController::dockWindow() const
{
    return dockWindowProvider()->window();
}
