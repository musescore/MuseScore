/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "docktabsmodel.h"
#include "log.h"

using namespace muse::dock;

DockTabsModel::DockTabsModel(QObject* parent)
    : QAbstractListModel{parent}
{
}

QVariant DockTabsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const DockTab& tab = m_dockTabs.at(index.row());

    switch (role) {
    case Title: return tab.title;
    case ContextMenu: return tab.contextMenu;
    case ToolBarComponent: return tab.toolBarComponent;
    }

    return QVariant();
}

QHash<int, QByteArray> DockTabsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { Title, "title" },
        { ContextMenu, "contextMenu" },
        { ToolBarComponent, "toolBarComponent" },
    };
    return roles;
}

void DockTabsModel::init(const KDDockWidgets::DockWidgetBase::List& widgets)
{
    beginResetModel();

    m_dockTabs.clear();

    for (const KDDockWidgets::DockWidgetBase* dock : widgets) {
        DockTab tab;
        tab.title = dock->title();
        tab.contextMenu = dock->property(CONTEXT_MENU_MODEL_PROPERTY);
        tab.toolBarComponent = dock->property(TOOLBAR_COMPONENT_PROPERTY);
        m_dockTabs.emplaceBack(tab);
    }

    endResetModel();
    emit numTabsChanged();
}

void DockTabsModel::toolBarComponentChanged(const KDDockWidgets::DockWidgetBase* dock)
{
    const int idx = indexOfDock(dock->title());
    if (idx < 0 || idx >= m_dockTabs.size()) {
        return;
    }

    m_dockTabs[idx].toolBarComponent = dock->property(TOOLBAR_COMPONENT_PROPERTY);

    const QModelIndex qIdx = index(idx, 0);
    emit dataChanged(qIdx, qIdx, { ToolBarComponent });
}

void DockTabsModel::contextMenuChanged(const KDDockWidgets::DockWidgetBase* dock)
{
    const int idx = indexOfDock(dock->title());
    if (idx < 0 || idx >= m_dockTabs.size()) {
        return;
    }

    m_dockTabs[idx].contextMenu = dock->property(CONTEXT_MENU_MODEL_PROPERTY);

    const QModelIndex qIdx = index(idx, 0);
    emit dataChanged(qIdx, qIdx, { ContextMenu });
}

int DockTabsModel::indexOfDock(const QString& title)
{
    for (int i = 0; i < m_dockTabs.size(); ++i) {
        const DockTab& tab = m_dockTabs.at(i);
        if (tab.title == title) {
            return i;
        }
    }
    return -1;
}
