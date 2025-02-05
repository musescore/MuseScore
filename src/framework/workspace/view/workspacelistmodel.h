/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "global/async/asyncable.h"

#include "modularity/ioc.h"
#include "global/iinteractive.h"
#include "iworkspacemanager.h"

namespace muse::workspace {
class WorkspaceListModel : public QAbstractListModel, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QVariant selectedWorkspace READ selectedWorkspace NOTIFY selectedWorkspaceChanged)

    Inject<IWorkspaceManager> workspacesManager = { this };
    Inject<IInteractive> interactive = { this };

public:
    explicit WorkspaceListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    QVariant selectedWorkspace() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE bool apply();
    Q_INVOKABLE void createNewWorkspace();
    Q_INVOKABLE void selectWorkspace(int workspaceIndex);
    Q_INVOKABLE void removeWorkspace(int workspaceIndex);
    Q_INVOKABLE void resetWorkspace(int workspaceIndex);

signals:
    void selectedWorkspaceChanged(QVariant selectedWorkspace);

private:
    void setSelectedWorkspace(IWorkspacePtr workspace);

    QVariantMap workspaceToObject(IWorkspacePtr workspace) const;
    bool isIndexValid(int index) const;

    enum Roles {
        RoleName = Qt::UserRole + 1,
        RoleIsSelected,
        RoleIsRemovable,
        RoleIsEdited
    };

    QList<IWorkspacePtr> m_workspaces;
    IWorkspacePtr m_selectedWorkspace;
};
}
