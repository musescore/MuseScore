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
#include "dockcentral.h"
#include "dockpanel.h"
#include "dockstatusbar.h"

#include "log.h"

using namespace mu::dock;

DockPage::DockPage(QQuickItem* parent)
    : QQuickItem(parent),
    m_mainToolBars(this),
    m_toolBars(this),
    m_panels(this),
    m_statusBars(this)
{
}

void DockPage::init()
{
    for (DockBase* dock : allDocks()) {
        dock->init();
    }
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

QQmlListProperty<DockStatusBar> DockPage::statusBarsProperty()
{
    return m_statusBars.property();
}

QList<DockToolBar*> DockPage::mainToolBars() const
{
    return m_mainToolBars.list();
}

QList<DockToolBar*> DockPage::toolBars() const
{
    return m_toolBars.list();
}

DockCentral* DockPage::centralDock() const
{
    return m_central;
}

QList<DockPanel*> DockPage::panels() const
{
    return m_panels.list();
}

QList<DockStatusBar*> DockPage::statusBars() const
{
    return m_statusBars.list();
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

void DockPage::close()
{
    TRACEFUNC;

    for (DockBase* dock : allDocks()) {
        dock->close();
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
    auto statusBars = this->statusBars();

    QList<DockBase*> docks;
    docks << QList<DockBase*>(mainToolBars.begin(), mainToolBars.end());
    docks << QList<DockBase*>(toolbars.begin(), toolbars.end());
    docks << m_central;
    docks << QList<DockBase*>(panels.begin(), panels.end());
    docks << QList<DockBase*>(statusBars.begin(), statusBars.end());

    return docks;
}
