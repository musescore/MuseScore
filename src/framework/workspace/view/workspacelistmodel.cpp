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
static const QString IS_REMOVABLE_KEY("isRemovable");
static const QString INDEX_KEY("index");

WorkspaceListModel::WorkspaceListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void WorkspaceListModel::load()
{
    beginResetModel();
    m_workspaces.clear();

    RetVal<IWorkspacePtrList> workspaces = workspacesManager()->workspaces();
    if (!workspaces.ret) {
        LOGE() << workspaces.ret.toString();
    }

    IWorkspacePtr currentWorkspace = workspacesManager()->currentWorkspace().val;
    IWorkspacePtr selectedWorkspace;

    for (const IWorkspacePtr& workspace : workspaces.val) {
        if (workspace == currentWorkspace) {
            selectedWorkspace = workspace;
        }

        m_workspaces << workspace;
    }

    std::sort(m_workspaces.begin(), m_workspaces.end(), [](const IWorkspacePtr& workspace1, const IWorkspacePtr& workspace2) {
        return workspace1->name() < workspace2->name();
    });

    endResetModel();

    setSelectedWorkspace(selectedWorkspace);
}

QVariant WorkspaceListModel::selectedWorkspace() const
{
    QVariantMap obj = workspaceToObject(m_selectedWorkspace);
    if (obj.isEmpty()) {
        return QVariant();
    }

    return obj;
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
    case RoleIsRemovable:
        return workspace[IS_REMOVABLE_KEY];
    case RoleIsSelected:
        return workspace[IS_SELECTED_KEY];
    }

    return QVariant();
}

void WorkspaceListModel::setSelectedWorkspace(IWorkspacePtr workspace)
{
    if (m_selectedWorkspace == workspace) {
        return;
    }

    QModelIndex previousSelectedModelIndex = index(m_workspaces.indexOf(m_selectedWorkspace));
    QModelIndex currentSelectedModelIndex = index(m_workspaces.indexOf(workspace));

    m_selectedWorkspace = workspace;

    emit selectedWorkspaceChanged(selectedWorkspace());
    emit dataChanged(previousSelectedModelIndex, previousSelectedModelIndex);
    emit dataChanged(currentSelectedModelIndex, currentSelectedModelIndex);
}

QVariantMap WorkspaceListModel::workspaceToObject(IWorkspacePtr workspace) const
{
    if (!workspace) {
        return QVariantMap();
    }

    std::string workspaceName = workspace->name();
    std::string selectedWorkspaceName = m_selectedWorkspace ? m_selectedWorkspace->name() : "";

    QVariantMap result;
    result[NAME_KEY] = QString::fromStdString(workspaceName);
    result[IS_REMOVABLE_KEY] = workspaceName != DEFAULT_WORKSPACE_NAME;
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
        { RoleIsRemovable, IS_REMOVABLE_KEY.toUtf8() }
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

    int newWorkspaceIndex = m_workspaces.size();

    beginInsertRows(QModelIndex(), newWorkspaceIndex, newWorkspaceIndex);
    m_workspaces << newWorkspace;
    endInsertRows();

    selectWorkspace(newWorkspaceIndex);
}

void WorkspaceListModel::selectWorkspace(int workspaceIndex)
{
    if (!isIndexValid(workspaceIndex)) {
        return;
    }

    setSelectedWorkspace(m_workspaces[workspaceIndex]);
}

void WorkspaceListModel::removeWorkspace(int workspaceIndex)
{
    if (!isIndexValid(workspaceIndex)) {
        return;
    }

    beginRemoveRows(QModelIndex(), workspaceIndex, workspaceIndex);
    IWorkspacePtr removedWorkspace = m_workspaces.takeAt(workspaceIndex);
    endRemoveRows();

    if (removedWorkspace == m_selectedWorkspace) {
        setSelectedWorkspace(nullptr);
    }
}

bool WorkspaceListModel::apply()
{
    IWorkspacePtrList newWorkspaceList;

    for (const IWorkspacePtr& workspace : m_workspaces) {
        newWorkspaceList.push_back(workspace);
    }

    Ret ret = workspacesManager()->setWorkspaces(newWorkspaceList);
    if (!ret) {
        LOGE() << ret.toString();
    }

    return ret;
}

bool WorkspaceListModel::isIndexValid(int index) const
{
    return index >= 0 && index < static_cast<int>(m_workspaces.size());
}
