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

#pragma once

#include <QAbstractListModel>
#include <QObject>

#include "thirdparty/KDDockWidgets/src/DockWidgetBase.h"

namespace muse::dock {
class DockTabsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int numTabs READ numTabs NOTIFY numTabsChanged)
public:
    enum Roles {
        Title = Qt::UserRole + 1,
        ContextMenu,
        ToolBarComponent,
    };
    Q_ENUM(Roles)

    explicit DockTabsModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& = QModelIndex()) const override { return m_dockTabs.size(); }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void init(const KDDockWidgets::DockWidgetBase::List& widgets);
    Q_INVOKABLE void clear() { init({}); }

    int numTabs() { return m_dockTabs.size(); }

    void toolBarComponentChanged(const KDDockWidgets::DockWidgetBase* dock);
    void contextMenuChanged(const KDDockWidgets::DockWidgetBase* dock);

signals:
    void numTabsChanged();

private:
    struct DockTab {
        QString title;
        QVariant contextMenu;
        QVariant toolBarComponent;
    };

    int indexOfDock(const QString& title);

    QList<DockTab> m_dockTabs;
};
}
