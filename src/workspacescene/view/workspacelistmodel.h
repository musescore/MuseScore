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

#ifndef MU_WORKSPACES_WORKSPACELISTMODEL_H
#define MU_WORKSPACES_WORKSPACELISTMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "workspace/iworkspacemanager.h"
#include "global/iinteractive.h"

#include "global/async/asyncable.h"

namespace mu::workspacescene {
class WorkspaceListModel : public QAbstractListModel, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QVariant selectedWorkspace READ selectedWorkspace NOTIFY selectedWorkspaceChanged)

    Inject<muse::workspace::IWorkspaceManager> workspacesManager = { this };
    Inject<muse::IInteractive> interactive = { this };

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
    void setSelectedWorkspace(muse::workspace::IWorkspacePtr workspace);

    QVariantMap workspaceToObject(muse::workspace::IWorkspacePtr workspace) const;
    bool isIndexValid(int index) const;

    enum Roles {
        RoleName = Qt::UserRole + 1,
        RoleIsSelected,
        RoleIsRemovable,
        RoleIsEdited
    };

    QList<muse::workspace::IWorkspacePtr> m_workspaces;
    muse::workspace::IWorkspacePtr m_selectedWorkspace;
};
}

#endif // MU_WORKSPACES_WORKSPACELISTMODEL_H
