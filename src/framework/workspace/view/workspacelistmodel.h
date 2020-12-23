//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_WORKSPACE_WORKSPACELISTMODEL_H
#define MU_WORKSPACE_WORKSPACELISTMODEL_H

#include <QAbstractListModel>

#include "iworkspacemanager.h"
#include "iinteractive.h"
#include "modularity/ioc.h"

#include "async/asyncable.h"

namespace mu::workspace {
class WorkspaceListModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(workspace, IWorkspaceManager, workspacesManager)
    INJECT(workspace, framework::IInteractive, interactive)

    Q_PROPERTY(QVariant selectedWorkspace READ selectedWorkspace NOTIFY selectedWorkspaceChanged)

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

signals:
    void selectedWorkspaceChanged(QVariant selectedWorkspace);

private:
    void setSelectedWorkspace(IWorkspacePtr workspace);

    QVariantMap workspaceToObject(IWorkspacePtr workspace) const;
    bool isIndexValid(int index) const;

    enum Roles {
        RoleName = Qt::UserRole + 1,
        RoleIsSelected,
        RoleIsRemovable
    };

    QList<IWorkspacePtr> m_workspaces;
    IWorkspacePtr m_selectedWorkspace;
};
}

#endif // MU_WORKSPACE_WORKSPACELISTMODEL_H
