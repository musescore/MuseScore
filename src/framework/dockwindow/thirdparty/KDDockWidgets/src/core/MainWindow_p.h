/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "MainWindow.h"
#include "Logging_p.h"
#include "SideBar.h"
#include "Config.h"
#include "ViewFactory.h"
#include "DropArea.h"
#include "DockWidget_p.h"
#include "Group.h"

#include <kdbindings/signal.h>

namespace KDDockWidgets {

namespace Core {

class MainWindow::Private
{
public:
    explicit Private(MainWindow *mainWindow, const QString &, MainWindowOptions options)
        : m_options(options)
        , q(mainWindow)
        , m_supportsAutoHide(Config::self().flags() & Config::Flag_AutoHideSupport)
    {
    }

    void init()
    {
        if (m_supportsAutoHide) {
            for (auto location : { SideBarLocation::North, SideBarLocation::East,
                                   SideBarLocation::West, SideBarLocation::South }) {
                m_sideBars[location] = new Core::SideBar(location, q);
            }
        }
    }

    bool supportsCentralFrame() const
    {
        return m_options & MainWindowOption_HasCentralFrame;
    }

    bool supportsPersistentCentralWidget() const
    {
        if (!dropArea()) {
            // This is the MDI case
            return false;
        }

        return (m_options & MainWindowOption_HasCentralWidget) == MainWindowOption_HasCentralWidget;
    }

    Core::DockWidget *createPersistentCentralDockWidget(const QString &uniqueName) const
    {
        if (!supportsPersistentCentralWidget())
            return nullptr;

        auto dockView = Config::self().viewFactory()->createDockWidget(
            uniqueName + QStringLiteral("-persistentCentralDockWidget"));
        auto dw = dockView->asDockWidgetController();
        dw->dptr()->m_isPersistentCentralDockWidget = true;
        Core::Group *group = dropArea()->centralGroup();
        if (!group) {
            KDDW_ERROR("Expected central group");
            return nullptr;
        }

        group->addTab(dw);
        return dw;
    }

    DropArea *dropArea() const
    {
        return m_layout->asDropArea();
    }

    void onResized(Size)
    {
        if (m_overlayedDockWidget)
            updateOverlayGeometry(m_overlayedDockWidget->d->group()->geometry().size());
    }

    KDBindings::Signal<> uniqueNameChanged;

    /// @brief emitted when the number of docked groups changes
    /// Note that we're using the "Frame" nomenculature instead of "DockWidget" here, as DockWidgets
    /// can be tabbed together, in which case this signal isn't emitted.
    KDBindings::Signal<int> groupCountChanged;

    KDBindings::Signal<int> overlayMarginChanged;

    CursorPositions allowedResizeSides(SideBarLocation loc) const;

    Rect rectForOverlay(Core::Group *, SideBarLocation) const;
    SideBarLocation preferredSideBar(Core::DockWidget *) const;
    void updateOverlayGeometry(Size suggestedSize);
    void clearSideBars();
    Rect windowGeometry() const;

    QString name;
    Vector<QString> affinities;
    const MainWindowOptions m_options;
    MainWindow *const q;
    ObjectGuard<Core::DockWidget> m_overlayedDockWidget;
    std::unordered_map<SideBarLocation, Core::SideBar *> m_sideBars;
    Layout *m_layout = nullptr;
    Core::DockWidget *m_persistentCentralDockWidget = nullptr;
    KDBindings::ScopedConnection m_visibleWidgetCountConnection;
    KDBindings::ScopedConnection m_resizeConnection;
    const bool m_supportsAutoHide;
    int m_overlayMargin = 1;
};

}

}
