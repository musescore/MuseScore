/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_DROPINDICATOROVERLAY_H
#define KD_DROPINDICATOROVERLAY_H

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/KDDockWidgets.h"
#include "Controller.h"

namespace KDDockWidgets {

namespace Core {

class DropArea;
class Group;

/// The DropIndicatorOverlay controller has drop indicator state
///
/// Such as:
///   - Current hovered tab group
///   - Current window being dragged
///   - Current visible drop location
/// Each DropArea has an associated DropIndicatorOverlay. Hence, each main window, each floating window
/// has one DropIndicatorOverlay.

class DOCKS_EXPORT DropIndicatorOverlay : public Controller
{
    Q_OBJECT
public:
    /// Constructor
    /// This overload takes the associated view (i.e QWidget where indicators are drawn).
    /// This overload is used by the Segmented Indicators type
    explicit DropIndicatorOverlay(DropArea *dropArea, View *view);

    /// Constructor
    /// Used by the Classic Indicators, creates a dummy/unused view internally.
    /// Classic Indicators is a special case, as it paints the indicators on a separate top-level window
    /// not on the main window itself
    explicit DropIndicatorOverlay(DropArea *dropArea);

    ~DropIndicatorOverlay() override;

    void setHoveredGroup(Group *);
    void setWindowBeingDragged(bool);
    Rect hoveredGroupRect() const;
    bool isHovered() const;
    DropLocation currentDropLocation() const;
    Group *hoveredGroup() const;

    virtual void setCurrentDropLocation(DropLocation);

    KDDockWidgets::DropLocation hover(Point globalPos);

    /// Clears and hides drop indicators
    void removeHover();

    /// @brief returns the position of the specified drop location
    /// The return is in global coordinates
    virtual Point posForIndicator(DropLocation) const = 0;

    /// @brief Returns whether the specified drop indicator should be visible
    virtual bool dropIndicatorVisible(DropLocation) const;

    static KDDockWidgets::Location multisplitterLocationFor(DropLocation);

    class Private;
    Private *dptr() const;

private:
    void onGroupDestroyed();
    void setHoveredGroupRect(Rect);
    Rect m_hoveredGroupRect;
    DropLocation m_currentDropLocation = DropLocation_None;
    Private *const d;

protected:
    virtual DropLocation hover_impl(Point globalPos) = 0;
    virtual void onHoveredGroupChanged(Group *);
    virtual void updateVisibility();

    Group *m_hoveredGroup = nullptr;
    DropArea *const m_dropArea;
    bool m_draggedWindowIsHovering = false;
};

}

}

#endif
