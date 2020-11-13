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

#include "workspacelistmodel.h"

#include "log.h"

using namespace mu::workspace;

static const QString NAME_KEY("name");
static const QString IS_SELECTED_KEY("isSelected");
static const QString CAN_REMOVE_KEY("canRemove");
static const QString INDEX_KEY("index");

WorkspaceListModel::WorkspaceListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void WorkspaceListModel::load()
{
    m_workspaces.clear();

    beginResetModel();

    RetValCh<IWorkspacePtrList> workspaces = workspacesManager()->allWorkspaces();
    if (!workspaces.ret) {
        LOGE() << workspaces.ret.toString();
    }

    IWorkspacePtr currentWorkspace = workspacesManager()->currentWorkspace().val;

    for (const IWorkspacePtr& workspace : workspaces.val) {
        if (workspace == currentWorkspace) {
            m_selectedWorkspace = workspace;
        }

        m_workspaces << workspace;
    }

    workspaces.ch.onReceive(this, [this, &workspaces](const IWorkspacePtrList&) {
        workspaces.ch.close();
        load();
    });

    std::sort(m_workspaces.begin(), m_workspaces.end(), [this](const IWorkspacePtr& workspace1, const IWorkspacePtr& workspace2) {
        return workspace1->name() < workspace2->name();
    });

    endResetModel();

    emit selectedWorkspaceChanged();
}

QVariant WorkspaceListModel::selectedWorkspace() const
{
    return workspaceToObject(m_selectedWorkspace);
}

QVariant WorkspaceListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !isIndexValid(index.row())) {
        return QVariant();
    }

    QVariantMap workspace = workspaceToObject(m_workspaces[index.row()]);

    switch (role) {
    case RoleName:
        return workspace[NAME_KEY];
    case RoleCanRemove:
        return workspace[CAN_REMOVE_KEY];
    case RoleIsSelected:
        return workspace[IS_SELECTED_KEY];
    }

    return QVariant();
}

QVariantMap WorkspaceListModel::workspaceToObject(IWorkspacePtr workspace) const
{
    std::string workspaceName = workspace ? workspace->name() : "";
    std::string selectedWorkspaceName = m_selectedWorkspace ? m_selectedWorkspace->name() : "";

    QVariantMap result;
    result[NAME_KEY] = QString::fromStdString(workspaceName);
    result[CAN_REMOVE_KEY] = workspaceName != DEFAULT_WORKSPACE_NAME;
    result[IS_SELECTED_KEY] = workspaceName == selectedWorkspaceName;
    result[INDEX_KEY] = m_workspaces.indexOf(workspace);

    return result;
}

int WorkspaceListModel::rowCount(const QModelIndex&) const
{
    return m_workspaces.size();
}

QHash<int, QByteArray> WorkspaceListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleName, NAME_KEY.toUtf8() },
        { RoleIsSelected, IS_SELECTED_KEY.toUtf8() },
        { RoleCanRemove, CAN_REMOVE_KEY.toUtf8() }
    };

    return roles;
}

void WorkspaceListModel::createNewWorkspace()
{
    RetVal<Val> obj = interactive()->open("musescore://workspace/create?sync=true");
    if (!obj.ret) {
        return;
    }

    IWorkspacePtr newWorkspace = obj.val.toQVariant().value<IWorkspacePtr>();
    if (!newWorkspace) {
        return;
    }

    beginInsertRows(QModelIndex(), m_workspaces.size(), m_workspaces.size());
    m_workspaces << newWorkspace;
    endInsertRows();
}

void WorkspaceListModel::selectWorkspace(int workspaceIndex)
{
    if (!isIndexValid(workspaceIndex)) {
        return;
    }

    IWorkspacePtr workspace = m_workspaces[workspaceIndex];

    if (m_selectedWorkspace == workspace) {
        return;
    }

    QModelIndex previousModelIndex = index(m_workspaces.indexOf(m_selectedWorkspace));
    QModelIndex currentModelIndex = index(workspaceIndex);

    m_selectedWorkspace = workspace;

    emit dataChanged(previousModelIndex, previousModelIndex);
    emit dataChanged(currentModelIndex, currentModelIndex);
    emit selectedWorkspaceChanged();
}

void WorkspaceListModel::removeWorkspace(int workspaceIndex)
{
    if (!isIndexValid(workspaceIndex)) {
        return;
    }

    beginRemoveRows(QModelIndex(), workspaceIndex, workspaceIndex);
    m_workspaces.removeAt(workspaceIndex);
    endRemoveRows();
}

bool WorkspaceListModel::isIndexValid(int index) const
{
    return index >= 0 && index < static_cast<int>(m_workspaces.size());
}
