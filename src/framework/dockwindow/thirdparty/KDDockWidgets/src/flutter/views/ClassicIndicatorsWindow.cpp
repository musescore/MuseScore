/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "ClassicIndicatorsWindow.h"
#include "kddockwidgets/core/indicators/ClassicDropIndicatorOverlay.h"
#include "kddockwidgets/core/Group.h"
#include "View.h"
#include "core/Utils_p.h"
#include "core/Logging_p.h"
#include "core/View_p.h"
#include "flutter/Platform.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;
using namespace KDDockWidgets::flutter;

IndicatorWindow::IndicatorWindow(ClassicDropIndicatorOverlay *controller, Core::View *parent)
    : flutter::View(controller, ViewType::DropAreaIndicatorOverlay, parent)
    , classicIndicators(controller)
{
    Platform::platformFlutter()->onDropIndicatorOverlayCreated(this);
}

IndicatorWindow::~IndicatorWindow()
{
    Platform::platformFlutter()->onDropIndicatorOverlayDestroyed(this);
}

Point IndicatorWindow::posForIndicator(DropLocation loc) const
{
    return posForIndicator_flutter(loc);
}

DropLocation IndicatorWindow::hover(Point globalPos)
{
    if (!isMounted())
        return DropLocation_None;

    if (m_updatePending)
        updatePositions();

    const Point localPos = mapFromGlobal(globalPos);
    return hover_flutter(localPos);
}

void IndicatorWindow::updatePositions()
{
    auto sz = size();
    const bool updated = updatePositions_flutter(sz.width(), sz.height(),
                                                 classicIndicators->hoveredGroup(), visibleDropIndicatorLocations());
    m_updatePending = !updated;
}

void IndicatorWindow::raise()
{
    // Nothing to do for flutter, it's raised
}

void IndicatorWindow::setGeometry(Rect geo)
{
    flutter::View::setGeometry(geo);
}

void IndicatorWindow::setObjectName(const QString &)
{
    // Not needed for flutter
}

void IndicatorWindow::setVisible(bool is)
{
    if (is == isVisible())
        return;

    flutter::View::setVisible(is);
    Platform::platformFlutter()->rebuildWindowOverlay();
}

void IndicatorWindow::resize(Size size)
{
    flutter::View::resize(size);
}

bool IndicatorWindow::isWindow() const
{
    return true;
}

Core::Group *IndicatorWindow::hoveredGroup() const
{
    return classicIndicators->hoveredGroup();
}

Point IndicatorWindow::posForIndicator_flutter(DropLocation) const
{
    KDDW_WARN("IndicatorWindow::posForIndicator_flutter: Implemented in dart instead");
    return {};
}

DropLocation IndicatorWindow::hover_flutter(Point)
{
    KDDW_WARN("IndicatorWindow::hover_flutter: Implemented in dart instead");
    return {};
}

bool IndicatorWindow::updatePositions_flutter(int, int, Core::Group *, int)
{
    KDDW_WARN("IndicatorWindow::updatePositions_flutter: Implemented in dart instead");
    return false;
}

Core::View *IndicatorWindow::rubberBand() const
{
    return classicIndicators->rubberBand();
}

int IndicatorWindow::visibleDropIndicatorLocations() const
{
    int result = 0;

    for (auto loc : { DropLocation_Left,
                      DropLocation_Top,
                      DropLocation_Right,
                      DropLocation_Bottom,
                      DropLocation_Center,
                      DropLocation_OutterLeft,
                      DropLocation_OutterTop,
                      DropLocation_OutterRight,
                      DropLocation_OutterBottom }) {
        if (classicIndicators->dropIndicatorVisible(loc))
            result |= loc;
    }

    return result;
}
