/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief A MultiSplitter with support for drop indicators when hovering over.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#ifndef KD_DROP_AREA_P_H
#define KD_DROP_AREA_P_H
#pragma once

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/KDDockWidgets.h"
#include "kddockwidgets/core/Layout.h"

class TestQtWidgets;
class TestDocks;

namespace KDDockWidgets {

namespace Core {

class Group;
class Draggable;
class DockWidget;
class Separator;
class DropIndicatorOverlay;
class LayoutingSeparator;
struct WindowBeingDragged;

/**
 * MultiSplitter is simply a wrapper around Core::Item in which the hosted widgets are
 * of class KDDockWidgets::Frame. The stuff in Core:: being agnostic and generic, not specific
 * to KDDW.
 *
 * A MultiSplitter is like a QSplitter but supports mixing vertical and horizontal splitters in
 * any combination.
 *
 * It supports adding a widget to the left/top/bottom/right of the whole MultiSplitter or adding
 * relative to a single widget.
 */

class DOCKS_EXPORT DropArea : public Layout
{
    Q_OBJECT
public:
    explicit DropArea(View *parent, MainWindowOptions options, bool isMDIWrapper = false);
    ~DropArea();

    void removeHover();
    DropLocation hover(WindowBeingDragged *draggedWindow, Point globalPos);
    ///@brief Called when a user drops a widget via DND
    bool drop(WindowBeingDragged *droppedWindow, Point globalPos);
    Vector<Core::Group *> groups() const;

    Core::Item *centralFrame() const;
    DropIndicatorOverlay *dropIndicatorOverlay() const;
    void addDockWidget(DockWidget *dw, KDDockWidgets::Location location, DockWidget *relativeTo,
                       const InitialOption &initialOption = {});
    void _addDockWidget(DockWidget *dw, KDDockWidgets::Location location, Item *relativeTo,
                        const InitialOption &initialOption);

    bool containsDockWidget(DockWidget *) const;

    /// Returns whether this layout has a single dock widget which is floating
    /// Implies it's in a FloatingWindow and that it has only one dock widget
    bool hasSingleFloatingGroup() const;

    /// Returns whether this drop area has only 1 group.
    /// See further explanation in FloatingWindow::hasSingleGroup()
    bool hasSingleGroup() const;

    Vector<QString> affinities() const;
    void layoutParentContainerEqually(DockWidget *);

    /// When DockWidgetOption_MDINestable is used, docked MDI dock widgets will be wrapped inside
    /// a DropArea, so they accept drops This DropArea is created implicitly while docking, and this
    /// function will return true
    bool isMDIWrapper() const;

    /// Returns the helper dock widget for implementing DockWidgetOption_MDINestable.
    Core::DockWidget *mdiDockWidgetWrapper() const;

    static Core::Group *createCentralGroup(MainWindowOptions options);

    /**
     * @brief Adds a widget to this MultiSplitter.
     */
    void addWidget(View *widget, KDDockWidgets::Location location,
                   Core::Item *relativeToItem = nullptr,
                   const InitialOption &option = DefaultSizeMode::Fair);

    /**
     * Adds an entire MultiSplitter into this layout. The donor MultiSplitter will be deleted
     * after all its Frames are stolen. All added Frames will preserve their original layout, so,
     * if widgetFoo was at the left of widgetBar when in the donor splitter, then it will still be
     * at left of widgetBar when the whole splitter is dropped into this one.
     */
    void addMultiSplitter(Core::DropArea *splitter, KDDockWidgets::Location location,
                          Core::Group *relativeToGroup = nullptr,
                          const InitialOption &option = DefaultSizeMode::Fair);

    /**
     * Called by the indicators, so they draw the drop rubber band at the correct place.
     * The rect for the rubberband when dropping a widget at the specified location.
     * Excludes the Separator thickness, result is actually smaller than what needed. In other
     * words, the result will be exactly the same as the geometry the widget will get.
     */
    Rect rectForDrop(const WindowBeingDragged *wbd, KDDockWidgets::Location location,
                     const Core::Item *relativeTo) const;

    bool deserialize(const LayoutSaver::MultiSplitter &) override;

    ///@brief returns the list of separators
    Vector<Core::LayoutingSeparator *> separators() const;

    /// @brief See docs for MainWindowBase::layoutEqually()
    void layoutEqually();

    /// @brief overload that just resizes widgets within a sub-tree
    void layoutEqually(Core::ItemBoxContainer *);

    /// @brief Returns the number of items layed-out horizontally or vertically
    /// But honours nesting
    int numSideBySide_recursive(Qt::Orientation) const;

    Core::ItemBoxContainer *rootItem() const;

    /// Returns the current drop location
    /// The user needs to be dragging a window and be over a drop indicator, otherwise DropLocation_None is returned
    DropLocation currentDropLocation() const;
#if defined(DOCKS_DEVELOPER_MODE) || defined(KDDW_FRONTEND_FLUTTER)
public:
#else
private:
#endif
    KDDW_DELETE_COPY_CTOR(DropArea)
    friend class Core::MainWindow;
    friend class Core::Group;
    friend class Core::FloatingWindow;
    friend class DropIndicatorOverlay;
    friend class AnimatedIndicators;

    // For debug/hardening
    bool validateInputs(View *widget, KDDockWidgets::Location location,
                        const Core::Item *relativeToItem, const InitialOption &option) const;


    void setRootItem(Core::ItemBoxContainer *);

    /**
     * @brief Like @ref availableLengthForDrop but just returns the total available width or height
     * (depending on @p orientation) So no need to receive any location.
     * @param orientation If Qt::Vertical then returns the available height. Width otherwise.
     */
    int availableLengthForOrientation(Qt::Orientation orientation) const;

    /**
     * @brief Equivalent to @ref availableLengthForOrientation but returns for both orientations.
     * width is for Qt::Vertical.
     */
    Size availableSize() const;

    template<typename T>
    bool validateAffinity(T *, Core::Group *acceptingGroup = nullptr) const;
    bool drop(WindowBeingDragged *draggedWindow, Core::Group *acceptingGroup, DropLocation);
    bool drop(View *droppedwindow, KDDockWidgets::Location location,
              Core::Group *relativeTo);
    Core::Group *groupContainingPos(Point globalPos) const;
    Core::Group *centralGroup() const;
    void updateFloatingActions();

    class Private;
    Private *const d;
};
}
}

#endif
