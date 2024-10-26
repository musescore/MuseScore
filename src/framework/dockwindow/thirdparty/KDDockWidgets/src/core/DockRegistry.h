/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_DOCKREGISTRY_P_H
#define KD_DOCKREGISTRY_P_H

#include "kddockwidgets/KDDockWidgets.h"
#include "kddockwidgets/core/View.h"
#include "kddockwidgets/QtCompat_p.h"
#include "kddockwidgets/core/EventFilterInterface.h"

#include <map>
#include <memory>

/**
 * DockRegistry is a singleton that knows about all DockWidgets.
 * It's used so we can restore layouts.
 * It's a private implementation detail.
 */
namespace KDDockWidgets {


namespace Core {
class FloatingWindow;
class Layout;
class SideBar;
class SideBarGroupings;
class MainWindow;
class DockWidget;
class Group;
class MainWindowMDIViewInterface;
class MainWindowViewInterface;
class FocusScope;
struct WindowBeingDragged;
}

class MainWindowMDI;

class DOCKS_EXPORT DockRegistry : public Core::Object, public Core::EventFilterInterface
{
    Q_OBJECT

public:
    enum class DockByNameFlag {
        None = 0,
        ConsultRemapping = 1,
        CreateIfNotFound =
            2, ///< Creates the dock widget via the user's widget factory in case it doesn't exist
        SilentIfNotFound = 4 ///< don't print errors if not found, it will be created later
    };
    Q_DECLARE_FLAGS(DockByNameFlags, DockByNameFlag)

    static DockRegistry *self();
    ~DockRegistry();
    void registerDockWidget(Core::DockWidget *);
    void unregisterDockWidget(Core::DockWidget *);

    void registerMainWindow(Core::MainWindow *);
    void unregisterMainWindow(Core::MainWindow *);

    void registerFloatingWindow(Core::FloatingWindow *);
    void unregisterFloatingWindow(Core::FloatingWindow *);

    void registerGroup(Core::Group *);
    void unregisterGroup(Core::Group *);

    void registerLayoutSaver();
    void unregisterLayoutSaver();

    /// Returns the dock widget that contains the widget with active focus
    /// Doesn't necessarily mean that this DockWidget has QWidget::focus, but that it contains
    /// the QApplication::focusObject() widget.
    Q_INVOKABLE KDDockWidgets::Core::DockWidget *focusedDockWidget() const;

    Q_INVOKABLE bool containsDockWidget(const QString &uniqueName) const;
    Q_INVOKABLE bool containsMainWindow(const QString &uniqueName) const;

    Q_INVOKABLE KDDockWidgets::Core::DockWidget *
    dockByName(const QString &, KDDockWidgets::DockRegistry::DockByNameFlags = {}) const;
    Q_INVOKABLE KDDockWidgets::Core::MainWindow *mainWindowByName(const QString &) const;

    bool isSane() const;

    ///@brief returns all DockWidget instances
    Vector<Core::DockWidget *> dockwidgets() const;

    ///@brief overload returning only the ones with the specified names
    Vector<Core::DockWidget *> dockWidgets(const Vector<QString> &names);

    ///@brief returns all closed DockWidget instances
    /// @param honourSkipped If true, won't include dock widgets with LayoutSaverOption::Skip
    Vector<Core::DockWidget *> closedDockwidgets(bool honourSkipped) const;

    ///@brief returns all MainWindow instances
    Vector<Core::MainWindow *> mainwindows() const;

    /// @brief returns all MainWindow instances
    /// Like mainwindows(), but with better suited for QtQuick and better terminology
    /// as we're phasing out the "MainWindow" name there
    Vector<Core::MainWindowViewInterface *> mainDockingAreas() const;

    ///@brief overload returning only the ones with the specified names
    Vector<Core::MainWindow *> mainWindows(const Vector<QString> &names);

    ///@brief returns a list of all Frame instances
    Vector<Core::Group *> groups() const;

    ///@brief returns all FloatingWindow instances. Not necessarily all floating dock widgets,
    /// As there might be DockWidgets which weren't morphed yet.
    Vector<Core::FloatingWindow *>
    floatingWindows(bool includeBeingDeleted = false, bool honourSkipped = false) const;

    ///@brief overload that returns list of QWindow. This is more friendly for supporting both
    /// QtWidgets and QtQuick
    Vector<std::shared_ptr<Core::Window>> floatingQWindows() const;

    ///@brief returns whether if there's at least one floating window
    Q_INVOKABLE bool hasFloatingWindows() const;

    ///@brief returns the FloatingWindow with handle @p windowHandle
    Core::FloatingWindow *
    floatingWindowForHandle(std::shared_ptr<Core::Window> windowHandle) const;

    ///@brief returns the FloatingWindow with handle @p hwnd
    Core::FloatingWindow *floatingWindowForHandle(Core::WId hwnd) const;

    ///@brief returns the MainWindow with handle @p windowHandle
    Core::MainWindow *mainWindowForHandle(std::shared_ptr<Core::Window> windowHandle) const;

    ///@brief Returns the list with all visiblye top-level parents of our FloatingWindow and
    /// MainWindow instances.
    ///
    /// Typically these are the FloatingWindows and MainWindows themselves. However, since a
    /// MainWindow can be embedded into another widget (for whatever reason, like a QWinWidget),
    /// it means that a top-level can be something else.
    ///
    /// Every returned widget is either a FloatingWindow, MainWindow, or something that contains a
    /// MainWindow.
    ///
    /// If @p excludeFloatingDocks is true then FloatingWindow won't be returned
    Vector<std::shared_ptr<Core::Window>> topLevels(bool excludeFloatingDocks = false) const;

    /**
     * @brief Closes all dock widgets, and destroys all FloatingWindows
     * This is called before restoring a layout.
     * @param affinities if specified only closes dock widgets and main windows with the specified
     * affinities
     */
    Q_INVOKABLE void clear(const Vector<QString> &affinities = {});

    /**
     * @brief clear Overload that only clears the specified dockWidgets and main windows.
     */
    void clear(const Vector<Core::DockWidget *> &dockWidgets,
               const Vector<Core::MainWindow *> &mainWindows,
               const Vector<QString> &affinities);

    /**
     * @brief Ensures that all floating DockWidgets have a FloatingWindow as a window.
     *
     * This is to simplify things before saving a layout. So we don't have to care about the case
     * where the window is a DockWidget.
     */
    void ensureAllFloatingWidgetsAreMorphed();

    /**
     * @brief returns true if there's 0 dockwidgets, 0 main windows
     *
     * @param excludeBeingDeleted if true, any window currently being deleted won't count
     */
    bool isEmpty(bool excludeBeingDeleted = false) const;

    /**
     * @brief Returns all main windows which match at least one of the @p affinities
     */
    Vector<Core::MainWindow *> mainWindowsWithAffinity(const Vector<QString> &affinities) const;

    /// @brief Returns the Layout where the specified item is in
    Core::Layout *layoutForItem(const Core::Item *) const;

    /// @brief Returns whether the item is in a main window.
    /// Nesting is honoured. (MDIArea inside DropArea inside MainWindow, for example)
    bool itemIsInMainWindow(const Core::Item *) const;

    bool affinitiesMatch(const Vector<QString> &affinities1, const Vector<QString> &affinities2) const;

    /// @brief Returns a list of all known main window unique names
    Vector<QString> mainWindowsNames() const;

    /// @brief Returns a list of all known dock widget unique names
    Vector<QString> dockWidgetNames() const;

    /// @brief returns if the specified window has some other window on top (with higher Z)
    /// This is an approximation, as we don't have ways to compare Z, so we mostly intersect
    /// geometries.
    /// @param target The window which we want to know if it's probably obscured
    /// @param exclude This window should not be counted as an obscurer. (It's being dragged).
    bool isProbablyObscured(std::shared_ptr<Core::Window> target,
                            Core::FloatingWindow *exclude) const;

    /// @overload
    bool isProbablyObscured(std::shared_ptr<Core::Window> target, Core::WindowBeingDragged *exclude) const;

    ///@brief Returns whether the specified dock widget is in a side bar, and which.
    /// SideBarLocation::None is returned if it's not in a sidebar.
    /// This is only relevant when using the auto-hide and side-bar feature.
    SideBarLocation sideBarLocationForDockWidget(const Core::DockWidget *) const;

    ///@brief Overload that returns the SideBar itself
    Core::SideBar *sideBarForDockWidget(const Core::DockWidget *) const;

    ///@brief Returns the Group which is being resized in a MDI layout. nullptr if none
    Core::Group *groupInMDIResize() const;

    void setCurrentCloseReason(CloseReason);
    CloseReason currentCloseReason();

    class Private;
    Private *dptr() const;

private:
    friend class Core::FocusScope;
    friend class Core::TitleBar;

    explicit DockRegistry(Core::Object *parent = nullptr);
    bool onDockWidgetPressed(Core::DockWidget *dw, MouseEvent *);
    void onFocusedViewChanged(std::shared_ptr<Core::View> view);
    void maybeDelete();
    void setFocusedDockWidget(Core::DockWidget *);

    // EventFilterInterface:
    bool onExposeEvent(std::shared_ptr<Core::Window>) override;
    bool onMouseButtonPress(Core::View *, MouseEvent *) override;

    // To honour Config::Flag_AutoHideAsTabGroups:
    void addSideBarGrouping(const Vector<Core::DockWidget *> &);
    void removeSideBarGrouping(const Vector<Core::DockWidget *> &);
    Vector<Core::DockWidget *> sideBarGroupingFor(Core::DockWidget *) const;

    Private *const d;

    Vector<Core::DockWidget *> m_dockWidgets;
    Vector<Core::MainWindow *> m_mainWindows;
    Vector<Core::Group *> m_groups;
    Vector<Core::FloatingWindow *> m_floatingWindows;

    ///@brief Dock widget id remapping, used by LayoutSaver
    ///
    /// When LayoutSaver is trying to restore dock widget "foo", but it doesn't exist, it will
    /// attempt to call a user provided factory function. That function can however return a dock
    /// widget with another ID, such as "bar". When that happens this map gets a "foo" : "bar"
    /// entry
    mutable std::map<QString, QString> m_dockWidgetIdRemapping;

    // To honour Config::Flag_AutoHideAsTabGroups
    Core::SideBarGroupings *const m_sideBarGroupings;
};

struct CloseReasonSetter
{
    CloseReasonSetter(CloseReason);
    ~CloseReasonSetter();

private:
    CloseReasonSetter(CloseReasonSetter &) = delete;
    CloseReasonSetter &operator=(CloseReason &) = delete;
};

}

#endif
