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

#include "dockpageview.h"

#include "docktoolbarview.h"
#include "dockingholderview.h"
#include "dockcentralview.h"
#include "dockpanelview.h"
#include "dockstatusbarview.h"

#include "log.h"

using namespace mu::dock;

DockPageView::DockPageView(QQuickItem* parent)
    : QQuickItem(parent),
    m_mainToolBars(this),
    m_toolBars(this),
    m_toolBarsDockingHolders(this),
    m_panels(this),
    m_panelsDockingHolders(this)
{
}

void DockPageView::init()
{
    TRACEFUNC;

    for (DockBase* dock : allDocks()) {
        dock->init();
    }

    emit inited();
}

QString DockPageView::uri() const
{
    return m_uri;
}

void DockPageView::setParams(const QVariantMap& params)
{
    emit setParamsRequested(params);
}

QQmlListProperty<DockToolBarView> DockPageView::mainToolBarsProperty()
{
    return m_mainToolBars.property();
}

QQmlListProperty<DockToolBarView> DockPageView::toolBarsProperty()
{
    return m_toolBars.property();
}

QQmlListProperty<DockPanelView> DockPageView::panelsProperty()
{
    return m_panels.property();
}

QQmlListProperty<DockingHolderView> DockPageView::toolBarsDockingHoldersProperty()
{
    return m_toolBarsDockingHolders.property();
}

QQmlListProperty<DockingHolderView> DockPageView::panelsDockingHoldersProperty()
{
    return m_panelsDockingHolders.property();
}

QList<DockToolBarView*> DockPageView::mainToolBars() const
{
    return m_mainToolBars.list();
}

QList<DockToolBarView*> DockPageView::toolBars() const
{
    return m_toolBars.list();
}

QList<DockingHolderView*> DockPageView::toolBarsHolders() const
{
    return m_toolBarsDockingHolders.list();
}

DockCentralView* DockPageView::centralDock() const
{
    return m_central;
}

DockStatusBarView* DockPageView::statusBar() const
{
    return m_statusBar;
}

QList<DockPanelView*> DockPageView::panels() const
{
    return m_panels.list();
}

QList<DockingHolderView*> DockPageView::panelsHolders() const
{
    return m_panelsDockingHolders.list();
}

DockBase* DockPageView::dockByName(const QString& dockName) const
{
    for (DockBase* dock : allDocks()) {
        if (dock->objectName() == dockName) {
            return dock;
        }
    }

    return nullptr;
}

DockingHolderView* DockPageView::holder(DockType type, Location location) const
{
    QList<DockingHolderView*> holders;

    if (type == DockType::ToolBar) {
        holders = m_toolBarsDockingHolders.list();
    } else if (type == DockType::Panel) {
        holders = m_panelsDockingHolders.list();
    }

    for (DockingHolderView* holder : holders) {
        if (holder->location() == location) {
            return holder;
        }
    }

    return nullptr;
}

QList<DockPanelView*> DockPageView::possibleTabs(const DockPanelView* panel) const
{
    QList<DockPanelView*> tabs;

    auto isPanelAllowedAsTab = [panel](const DockPanelView* p) {
        if (!p || p == panel) {
            return false;
        }

        return p->isOpen() && !p->floating();
    };

    DockPanelView* rootPanel = findRootPanel(panel);
    DockPanelView* nextPanel = rootPanel ? rootPanel->tabifyPanel() : nullptr;

    while (nextPanel) {
        if (isPanelAllowedAsTab(nextPanel)) {
            tabs << nextPanel;
        }

        nextPanel = nextPanel->tabifyPanel();
    }

    if (!tabs.contains(rootPanel) && isPanelAllowedAsTab(rootPanel)) {
        tabs.prepend(rootPanel);
    }

    return tabs;
}

DockPanelView* DockPageView::findRootPanel(const DockPanelView* panel) const
{
    for (DockPanelView* panel_ : panels()) {
        DockPanelView* tabifyPanel = panel_->tabifyPanel();

        while (tabifyPanel) {
            if (tabifyPanel == panel) {
                return panel_;
            }

            tabifyPanel = tabifyPanel->tabifyPanel();
        }
    }

    return const_cast<DockPanelView*>(panel);
}

bool DockPageView::isDockOpen(const QString& dockName) const
{
    const DockBase* dock = dockByName(dockName);
    return dock ? dock->isOpen() : false;
}

void DockPageView::toggleDock(const QString& dockName)
{
    setDockOpen(dockName, !isDockOpen(dockName));
}

void DockPageView::setDockOpen(const QString& dockName, bool open)
{
    DockBase* dock = dockByName(dockName);
    if (!dock) {
        return;
    }

    if (!open) {
        dock->close();
        return;
    }

    DockPanelView* panel = dynamic_cast<DockPanelView*>(dock);
    if (!panel) {
        dock->open();
        return;
    }

    DockPanelView* destinationPanel = findPanelForTab(panel);
    if (destinationPanel) {
        destinationPanel->addPanelAsTab(panel);
    } else {
        panel->open();
    }
}

DockPanelView* DockPageView::findPanelForTab(const DockPanelView* tab) const
{
    for (DockPanelView* panel : panels()) {
        if (panel->tabifyPanel() != tab) {
            continue;
        }

        if (panel->isOpen() && !panel->floating()) {
            return panel;
        }

        return findPanelForTab(panel);
    }

    return nullptr;
}

bool DockPageView::isDockFloating(const QString& dockName) const
{
    const DockBase* dock = dockByName(dockName);
    return dock ? dock->floating() : false;
}

void DockPageView::toggleDockFloating(const QString& dockName)
{
    DockBase* dock = dockByName(dockName);
    if (!dock) {
        return;
    }

    dock->setFloating(!dock->floating());
}

void DockPageView::setUri(const QString& uri)
{
    if (uri == m_uri) {
        return;
    }

    m_uri = uri;
    emit uriChanged(uri);
}

void DockPageView::setCentralDock(DockCentralView* central)
{
    if (central == m_central) {
        return;
    }

    m_central = central;
    emit centralDockChanged(central);
}

void DockPageView::setStatusBar(DockStatusBarView* statusBar)
{
    if (statusBar == m_statusBar) {
        return;
    }

    m_statusBar = statusBar;
    emit statusBarChanged(statusBar);
}

void DockPageView::componentComplete()
{
    QQuickItem::componentComplete();

    Q_ASSERT(!m_uri.isEmpty());
    Q_ASSERT(m_central != nullptr);
}

QList<DockBase*> DockPageView::allDocks() const
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
