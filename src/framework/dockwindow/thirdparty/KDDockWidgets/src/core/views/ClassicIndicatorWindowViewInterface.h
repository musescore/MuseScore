/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/KDDockWidgets.h"

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

namespace KDDockWidgets::Core {

/// @brief The window that will hold the classic indicators
/// This is a window so it can be over the window that is being dragged
class DOCKS_EXPORT ClassicIndicatorWindowViewInterface
{
public:
    ClassicIndicatorWindowViewInterface() = default;
    virtual ~ClassicIndicatorWindowViewInterface();

    virtual void setObjectName(const QString &) = 0;

    /// Called when the mouse is dragging a window over a drop area
    /// The implementer should:
    /// - Iterate over all visible indicators, if any is under the mouse then it should show
    ///   the "active" icon variant.
    virtual DropLocation hover(Point) = 0;

    /// Returns the position for the specified drop indicator
    /// This is used by tests only, so we know where to drop a window
    /// The position is the center of the indicator and is in global coordinates
    virtual Point posForIndicator(DropLocation) const = 0;

    /// Called by the framework to updates the position of the individual drop indicators
    /// This function is called when the drag first starts and also if the window resizes itself (unlikely during drag though)
    /// The implementer should:
    /// - call rect() to know where to put the outer indicators
    /// - call classicIndicators->hoveredGroup(), if not null, will give you the rect for the inner indicators
    virtual void updatePositions() = 0;

    virtual void raise() = 0;
    virtual void setVisible(bool) = 0;
    virtual void resize(Size) = 0;
    virtual void setGeometry(Rect) = 0;
    virtual bool isWindow() const = 0;
    virtual void updateIndicatorVisibility() = 0;

    ClassicIndicatorWindowViewInterface(const ClassicIndicatorWindowViewInterface &) = delete;
    ClassicIndicatorWindowViewInterface &operator=(const ClassicIndicatorWindowViewInterface &) = delete;
};

}
