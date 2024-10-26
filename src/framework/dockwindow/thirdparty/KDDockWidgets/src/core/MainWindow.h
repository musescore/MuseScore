/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/


/**
 * @file
 * @brief The MainWindow base-class that's shared between QtWidgets and QtQuick stack.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#ifndef KD_MAINWINDOW_BASE_H
#define KD_MAINWINDOW_BASE_H

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/KDDockWidgets.h"
#include "kddockwidgets/LayoutSaver.h"
#include "kddockwidgets/core/Controller.h"

class TestDocks;

namespace KDDockWidgets {

namespace QtWidgets {
class MainWindow;
}

namespace QtQuick {
class MainWindow;
}

namespace flutter {
class MainWindow;
}

namespace Core {

class MainWindowViewInterface;
class MDILayout;
class DropArea;
class Group;
class Layout;
class SideBar;
class DockWidget;

/**
 * @brief The MainWindow base-class. MainWindow and MainWindowBase are only
 * split in two so we can share some code with the QtQuick implementation,
 * which also derives from MainWindowBase.
 *
 * Do not use instantiate directly in user code. Use MainWindow instead.
 */
class DOCKS_EXPORT MainWindow : public Controller
{
    Q_OBJECT
public:
    typedef Vector<MainWindow *> List;

    explicit MainWindow(View *view, const QString &uniqueName, MainWindowOptions options);

    ~MainWindow() override;

    /**
     * @brief Docks a DockWidget into the central group, tabbed.
     * @warning Requires that the MainWindow was constructed with MainWindowOption_HasCentralFrame
     * option.
     * @param dockwidget The dockwidget to dock.
     *
     * @sa Core::DockWidget::addDockWidgetAsTab()
     */
    void addDockWidgetAsTab(KDDockWidgets::Core::DockWidget *dockwidget);

    /**
     * @brief Docks a DockWidget into this main window.
     * @param dockWidget The dock widget to add into this MainWindow
     * @param location the location where to dock
     * @param relativeTo In case we're docking in relation to another dock widget
     * @param initialOption Allows to specify an InitialOption. Which is useful to add the dock
     * widget as hidden, recording only a placeholder in the tab. So it's restored to tabbed when
     * eventually shown.
     */
    void addDockWidget(KDDockWidgets::Core::DockWidget *dockWidget,
                       KDDockWidgets::Location location,
                       KDDockWidgets::Core::DockWidget *relativeTo = nullptr,
                       const KDDockWidgets::InitialOption &initialOption = {});

    // dev mode only for now, as it still has bugs.
    // We need to be able to dock to relativeTo=hidden dock
    /**
     * Docks a DockWidget into this main window at the specified side.
     * To be used only with MainWindowOption_HasCentralWidget or MainWindowOption_HasCentralFrame.
     * This is to mimic old QDockWidget system where there were 4 sides around the central widget (not infinite nesting).
     *
     * Example:
     * Adding dock widget D1 to Right results in:
     *   +---------+--+
     *   | central |D1|
     *   +---------+--+
     * but adding D2 to Right as well will result in
     *   +---------+--+
     *   |         |D1|
     *   | central +--+
     *   |         |D2|
     *   +---------+--+
     *   which is actually equivalent to addDockWidget(D2, Location_onBottom, /relativeTo=/D1);
     */
    void addDockWidgetToSide(KDDockWidgets::Core::DockWidget *dockWidget,
                             KDDockWidgets::Location location, const KDDockWidgets::InitialOption &initialOption = {});
    /**
     * @brief Sets a persistent central widget. It can't be detached.
     *
     * Requires passing MainWindowOption_HasCentralWidget in the CTOR.
     * This is similar to the central group concept of MainWindowOption_HasCentralFrame,
     * with the difference that it won't show a tabs.
     *
     * @param widget The QWidget (or QQuickItem if built with QtQuick support) that you
     * want to set.
     *
     * Example: examples/qtwidgets_dockwidgets --central-widget
     */
    void setPersistentCentralView(std::shared_ptr<View> widget);
    std::shared_ptr<View> persistentCentralView() const;

    /// @brief Returns the main window options that were passed via constructor.
    MainWindowOptions options() const;

    /**
     * @brief Sets the affinities names. Dock widgets can only dock into main windows of the same
     * affinity.
     *
     * By default the affinity is empty and a dock widget can dock into any main window. Usually you
     * won't ever need to call this function, unless you have requirements where certain dock
     * widgets can only dock into certain main windows. @sa
     * Core::DockWidget::setAffinities().
     *
     * Note: Call this function right after creating your main window, before docking any dock
     * widgets into a main window and before restoring any layout.
     *
     * Note: Currently you can only call this function once, to keep the code simple and avoid
     * edge cases. This will only be changed if a good use case comes up that requires changing
     * affinities multiple times.
     *
     * @p name The affinity names.
     */
    void setAffinities(const Vector<QString> &names);

    /**
     * @brief Returns the list of affinity names. Empty by default.
     */
    Vector<QString> affinities() const;

    /// @brief layouts all the widgets so they have an equal size within their parent container
    ///
    /// Note that the layout is a tree of nested horizontal and vertical container layouts. The
    /// nodes closer to the root will have more space.
    ///
    /// min/max constraints will still be honoured.
    void layoutEqually();

    /// @brief like layoutEqually() but starts with the container that has @p dockWidget.
    /// While layoutEqually() starts from the root of the layout tree this function starts on a
    /// sub-tree.
    void layoutParentContainerEqually(KDDockWidgets::Core::DockWidget *dockWidget);

    ///@brief Moves the dock widget into one of the MainWindow's sidebar.
    /// Means the dock widget is removed from the layout, and the sidebar shows a button that if
    /// pressed will toggle the dock widget's visibility as an overlay over the layout. This is the
    /// auto-hide functionality.
    ///
    /// The chosen side bar will depend on some heuristics, mostly proximity.
    void moveToSideBar(KDDockWidgets::Core::DockWidget *dw);

    /// @brief overload that allows to specify which sidebar to use, instead of using heuristics.
    void moveToSideBar(KDDockWidgets::Core::DockWidget *dw,
                       KDDockWidgets::SideBarLocation location);

    /// @brief Removes the dock widget from the sidebar and docks it into the main window again
    void restoreFromSideBar(KDDockWidgets::Core::DockWidget *dw);

    ///@brief Shows the dock widget overlayed on top of the main window, placed next to the sidebar
    void overlayOnSideBar(KDDockWidgets::Core::DockWidget *dw);

    ///@brief Shows or hides an overlay. It's assumed the dock widget is already in a side-bar.
    void toggleOverlayOnSideBar(KDDockWidgets::Core::DockWidget *dw);

    /// @brief closes any overlayed dock widget. The sidebar still displays them as button.
    void clearSideBarOverlay(bool deleteGroup = true);

    /// @brief Returns the sidebar this dockwidget is in. nullptr if not in any.
    KDDockWidgets::Core::SideBar *
    sideBarForDockWidget(const KDDockWidgets::Core::DockWidget *dw) const;

    /// @brief Returns whether the specified sidebar is visible
    bool sideBarIsVisible(KDDockWidgets::SideBarLocation location) const;

    /// @brief returns the dock widget which is currently overlayed. nullptr if none.
    /// This is only relevant when using the auto-hide and side-bar feature.
    Core::DockWidget *overlayedDockWidget() const;

    /// @brief Returns whether any side bar is visible
    bool anySideBarIsVisible() const;

    /// @brief Returns whether this main window is using an MDI layout.
    /// In other words, returns true if MainWindowOption_MDI was passed in the ctor.
    bool isMDI() const;

    /// @brief Closes all dock widgets which are docked into this main window
    /// This is convenience to calling DockWidget::close() individually
    /// If force is true, then the individual dock widgets can't stop the process
    /// Returns false if there's at least one dock widget which rejected closing. Returns true
    /// if all dock widgets were closed (0 or more)
    bool closeDockWidgets(bool force = false);

    /// @brief Returns the margin used by overlay docks. Default: 1
    int overlayMargin() const;

    /// @brief Sets the margin used by overlay docks.
    /// Does not modify currently overlayed docks
    void setOverlayMargin(int margin);

    /// @brief Sets the content's margins
    void setContentsMargins(int l, int t, int r, int b);

    /// @brief Returns the side bar at the specified location
    Core::SideBar *sideBar(SideBarLocation location) const;

    // Internal public API:

    ///@internal
    ///@brief returns the drop area.
    DropArea *dropArea() const;

    ///@internal
    ///@brief returns the MultiSplitter.
    DropArea *multiSplitter() const;

    ///@internal
    ///@brief returns the layout, which is usually MultiSplitter, but can be a MDILayout if using MDI
    Layout *layout() const;

    ///@internal
    ///@brief Returns the MDI layout. Or nullptr if this isn't a MDI main window
    MDILayout *mdiLayout() const;

    /// @brief Returns the unique name that was passed via constructor.
    ///        Used internally by the save/restore mechanism.
    /// @internal
    QString uniqueName() const;


    /// @internal
    Margins centerWidgetMargins() const;

    /// @internal
    void init(const QString &name);

protected:
    void setUniqueName(const QString &uniqueName);
    Rect centralAreaGeometry() const;

private:
    class Private;
    Private *const d;

    friend class KDDockWidgets::QtWidgets::MainWindow;
    friend class KDDockWidgets::QtQuick::MainWindow;
    friend class KDDockWidgets::flutter::MainWindow;

    friend class KDDockWidgets::Core::MainWindowViewInterface;
    friend class ::TestDocks;
    friend class KDDockWidgets::LayoutSaver;
    bool deserialize(const LayoutSaver::MainWindow &);
    LayoutSaver::MainWindow serialize() const;
};
}
}

#endif
