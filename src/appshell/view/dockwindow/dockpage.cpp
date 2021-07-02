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

#include "dockpage.h"

#include "docktoolbar.h"
#include "docktoolbarholder.h"
#include "dockcentral.h"
#include "dockpanel.h"
#include "dockpanelholder.h"
#include "dockstatusbar.h"

#include "log.h"

using namespace mu::dock;

DockPage::DockPage(QQuickItem* parent)
    : QQuickItem(parent),
    m_mainToolBars(this),
    m_toolBars(this),
    m_toolBarsDockingHolders(this),
    m_panels(this),
    m_panelsDockingHolders(this)
{
}

void DockPage::init()
{
    for (DockBase* dock : allDocks()) {
        dock->init();
    }

    emit inited();
}

QString DockPage::uri() const
{
    return m_uri;
}

QQmlListProperty<DockToolBar> DockPage::mainToolBarsProperty()
{
    return m_mainToolBars.property();
}

QQmlListProperty<DockToolBar> DockPage::toolBarsProperty()
{
    return m_toolBars.property();
}

QQmlListProperty<DockPanel> DockPage::panelsProperty()
{
    return m_panels.property();
}

QQmlListProperty<DockToolBarHolder> DockPage::toolBarsDockingHoldersProperty()
{
    return m_toolBarsDockingHolders.property();
}

QQmlListProperty<DockPanelHolder> DockPage::panelsDockingHoldersProperty()
{
    return m_panelsDockingHolders.property();
}

QList<DockToolBar*> DockPage::mainToolBars() const
{
    return m_mainToolBars.list();
}

QList<DockToolBar*> DockPage::toolBars() const
{
    //! NOTE: Order is important for correct drawing
    auto list = m_toolBars.list();

    DockToolBarHolder* leftHolder = toolBarHolderByLocation(DockBase::DockLocation::Left);
    if (leftHolder) {
        list.prepend(leftHolder);
    }

    DockToolBarHolder* rightHolder = toolBarHolderByLocation(DockBase::DockLocation::Right);
    if (rightHolder) {
        list.append(rightHolder);
    }

    DockToolBarHolder* bottomHolder = toolBarHolderByLocation(DockBase::DockLocation::Bottom);
    if (bottomHolder) {
        list.prepend(bottomHolder);
    }

    DockToolBarHolder* topHolder = toolBarHolderByLocation(DockBase::DockLocation::Top);
    if (topHolder) {
        list.append(topHolder);
    }

    return list;
}

QList<DockToolBarHolder*> DockPage::toolBarsHolders() const
{
    return m_toolBarsDockingHolders.list();
}

DockCentral* DockPage::centralDock() const
{
    return m_central;
}

DockStatusBar* DockPage::statusBar() const
{
    return m_statusBar;
}

QList<DockPanel*> DockPage::panels() const
{
    //! NOTE: Order is important for correct drawing
    auto list = m_panels.list();

    DockPanelHolder* leftHolder = panelHolderByLocation(DockBase::DockLocation::Left);
    if (leftHolder) {
        list.prepend(leftHolder);
    }

    DockPanelHolder* rightHolder = panelHolderByLocation(DockBase::DockLocation::Right);
    if (rightHolder) {
        list.append(rightHolder);
    }

    DockPanelHolder* bottomHolder = panelHolderByLocation(DockBase::DockLocation::Bottom);
    if (bottomHolder) {
        list.prepend(bottomHolder);
    }

    DockPanelHolder* topHolder = panelHolderByLocation(DockBase::DockLocation::Top);
    if (topHolder) {
        list.append(topHolder);
    }

    return list;
}

QList<DockPanelHolder*> DockPage::panelsHolders() const
{
    return m_panelsDockingHolders.list();
}

DockBase* DockPage::dockByName(const QString& dockName) const
{
    for (DockBase* dock : allDocks()) {
        if (dock->objectName() == dockName) {
            return dock;
        }
    }

    return nullptr;
}

DockToolBarHolder* DockPage::toolBarHolderByLocation(DockBase::DockLocation location) const
{
    for (DockToolBarHolder* holder : m_toolBarsDockingHolders.list()) {
        if (holder->location() == location) {
            return holder;
        }
    }

    return nullptr;
}

DockPanelHolder* DockPage::panelHolderByLocation(DockBase::DockLocation location) const
{
    for (DockPanelHolder* holder : m_panelsDockingHolders.list()) {
        if (holder->location() == location) {
            return holder;
        }
    }

    return nullptr;
}

bool DockPage::isDockShown(const QString& dockName) const
{
    const DockBase* dock = dockByName(dockName);
    return dock ? dock->isShown() : false;
}

void DockPage::toggleDockVisibility(const QString& dockName)
{
    DockBase* dock = dockByName(dockName);
    if (dock) {
        dock->toggle();
    }
}

void DockPage::setUri(const QString& uri)
{
    if (uri == m_uri) {
        return;
    }

    m_uri = uri;
    emit uriChanged(uri);
}

void DockPage::setCentralDock(DockCentral* central)
{
    if (central == m_central) {
        return;
    }

    m_central = central;
    emit centralDockChanged(central);
}

void DockPage::setStatusBar(DockStatusBar* statusBar)
{
    if (statusBar == m_statusBar) {
        return;
    }

    m_statusBar = statusBar;
    emit statusBarChanged(statusBar);
}

void DockPage::close()
{
    TRACEFUNC;

    for (DockBase* dock : allDocks()) {
        dock->hide();
    }
}

void DockPage::componentComplete()
{
    QQuickItem::componentComplete();

    Q_ASSERT(!m_uri.isEmpty());
}

QList<DockBase*> DockPage::allDocks() const
{
    auto mainToolBars = this->mainToolBars();
    auto toolbars = this->toolBars();
    auto panels = this->panels();

    QList<DockBase*> docks;
    docks << QList<DockBase*>(mainToolBars.begin(), mainToolBars.end());
    docks << QList<DockBase*>(toolbars.begin(), toolbars.end());
    docks << QList<DockBase*>(panels.begin(), panels.end());

    docks << m_central;

    if (m_statusBar) {
        docks << m_statusBar;
    }

    return docks;
}
