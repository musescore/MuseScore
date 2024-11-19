/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "SideBar.h"
#include "DockWidget_p.h"
#include "MainWindow.h"
#include "core/ViewFactory.h"
#include "core/Logging_p.h"
#include "views/SideBarViewInterface.h"
#include "Config.h"

#include <unordered_map>
#include <utility>

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

class SideBar::Private
{
public:
    void removeConnection(DockWidget *dw)
    {
        auto it = connections.find(dw);
        if (it == connections.end()) {
            KDDW_ERROR("Could not find DockWidget to remove in side bar connections");
        } else {
            connections.erase(it);
        }
    }
    std::unordered_map<DockWidget *, KDBindings::ScopedConnection> connections;
};

SideBar::SideBar(SideBarLocation location, MainWindow *parent)
    : Controller(ViewType::SideBar, Config::self().viewFactory()->createSideBar(this, parent->view()))
    , d(new Private())
    , m_mainWindow(parent)
    , m_location(location)
    , m_orientation((location == SideBarLocation::North || location == SideBarLocation::South)
                        ? Qt::Horizontal
                        : Qt::Vertical)
{
    updateVisibility();

    if (isVertical()) {
        view()->setFixedWidth(30);
    } else {
        view()->setFixedHeight(30);
    }

    view()->init();
}

SideBar::~SideBar()
{
    delete d;
}

void SideBar::addDockWidget(DockWidget *dw)
{
    if (!dw)
        return;

    if (m_dockWidgets.contains(dw)) {
        KDDW_ERROR("Already contains dock widget with title={}", dw->title());
        return;
    }

    KDBindings::ScopedConnection conn = dw->d->aboutToDelete.connect([this](auto dock) { removeDockWidget(dock); });
    d->connections[dw] = std::move(conn);

    m_dockWidgets.push_back(dw);
    dynamic_cast<Core::SideBarViewInterface *>(view())->addDockWidget_Impl(dw);
    updateVisibility();
}

void SideBar::removeDockWidget(DockWidget *dw)
{
    if (!m_dockWidgets.contains(dw)) {
        KDDW_ERROR("Doesn't contain dock widget with title={}", dw->title());
        return;
    }

    d->removeConnection(dw);
    m_dockWidgets.removeOne(dw);
    dynamic_cast<Core::SideBarViewInterface *>(view())->removeDockWidget_Impl(dw);
    dw->d->removedFromSideBar.emit();
    updateVisibility();
}

bool SideBar::containsDockWidget(DockWidget *dw) const
{
    return m_dockWidgets.contains(dw);
}

void SideBar::onButtonClicked(DockWidget *dw)
{
    toggleOverlay(dw);
}

void SideBar::updateVisibility()
{
    setVisible(!isEmpty());
}

Qt::Orientation SideBar::orientation() const
{
    return m_orientation;
}

bool SideBar::isEmpty() const
{
    return m_dockWidgets.isEmpty();
}

SideBarLocation SideBar::location() const
{
    return m_location;
}

MainWindow *SideBar::mainWindow() const
{
    return m_mainWindow;
}

void SideBar::toggleOverlay(DockWidget *dw)
{
    m_mainWindow->toggleOverlayOnSideBar(dw);
}

Vector<QString> SideBar::serialize() const
{
    Vector<QString> ids;
    ids.reserve(m_dockWidgets.size());
    for (DockWidget *dw : m_dockWidgets)
        ids.push_back(dw->uniqueName());

    return ids;
}

void SideBar::clear()
{
    for (DockWidget *dw : std::as_const(m_dockWidgets))
        removeDockWidget(dw);
}

Vector<DockWidget *> SideBar::dockWidgets() const
{
    return m_dockWidgets;
}
