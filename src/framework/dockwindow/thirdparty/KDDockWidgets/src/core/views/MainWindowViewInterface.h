/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/KDDockWidgets.h"

namespace KDDockWidgets {

namespace Core {

class View;
class SideBar;
class MainWindow;
class DockWidgetViewInterface;

/// @brief The interface that MainWindow views should implement
class DOCKS_EXPORT MainWindowViewInterface
{
public:
    explicit MainWindowViewInterface(MainWindow *);
    virtual ~MainWindowViewInterface();
    virtual Margins centerWidgetMargins() const = 0;
    virtual Rect centralAreaGeometry() const = 0;
    virtual void setContentsMargins(int left, int top, int right, int bottom) = 0;

    /// @brief Returns the main window controller
    MainWindow *mainWindow() const;


    // controllers/MainWindow.h public interface:
    QString uniqueName() const;
    Vector<QString> affinities() const;
    void setAffinities(const Vector<QString> &names);
    MainWindowOptions options() const;
    bool isMDI() const;
    bool closeDockWidgets(bool force = false);
    bool sideBarIsVisible(KDDockWidgets::SideBarLocation) const;
    void clearSideBarOverlay(bool deleteFrame = true);
    void layoutEqually();
    bool anySideBarIsVisible() const;

    void addDockWidgetAsTab(DockWidgetViewInterface *dockwidget);
    void addDockWidget(DockWidgetViewInterface *dockWidget, KDDockWidgets::Location location,
                       DockWidgetViewInterface *relativeTo = nullptr,
                       const KDDockWidgets::InitialOption &initialOption = {});

    void moveToSideBar(DockWidgetViewInterface *);
    void moveToSideBar(DockWidgetViewInterface *, KDDockWidgets::SideBarLocation);
    void restoreFromSideBar(DockWidgetViewInterface *);
    void overlayOnSideBar(DockWidgetViewInterface *);
    void toggleOverlayOnSideBar(DockWidgetViewInterface *);
    void layoutParentContainerEqually(DockWidgetViewInterface *);

    void moveToSideBar(const QString &dockId);
    void moveToSideBar(const QString &dockId, KDDockWidgets::SideBarLocation);
    void restoreFromSideBar(const QString &dockId);
    void overlayOnSideBar(const QString &dockId);
    void toggleOverlayOnSideBar(const QString &dockId);
    void layoutParentContainerEqually(const QString &dockId);
    void addDockWidgetAsTab(const QString &dockId);
    void addDockWidget(const QString &dockId, KDDockWidgets::Location,
                       const QString &relativeToDockId = {}, const KDDockWidgets::InitialOption & = {});

    void setPersistentCentralView(std::shared_ptr<Core::View>);

protected:
    MainWindow *const m_mainWindow;
    MainWindowViewInterface(const MainWindowViewInterface &) = delete;
    MainWindowViewInterface &operator=(const MainWindowViewInterface &) = delete;
};

}

}
