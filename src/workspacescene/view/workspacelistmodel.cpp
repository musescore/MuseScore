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

#include "workspacelistmodel.h"

#include "ui/uitypes.h"
#include "palette/palettetypes.h"

#include "log.h"

using namespace mu::workspacescene;
using namespace muse;
using namespace muse::workspace;

static const QString NAME_KEY("name");
static const QString IS_SELECTED_KEY("isSelected");
static const QString IS_REMOVABLE_KEY("isRemovable");
static const QString INDEX_KEY("index");

WorkspaceListModel::WorkspaceListModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void WorkspaceListModel::load()
{
    beginResetModel();
    m_workspaces.clear();

    IWorkspacePtrList workspaces = workspacesManager()->workspaces();

    IWorkspacePtr currentWorkspace = workspacesManager()->currentWorkspace();
    IWorkspacePtr selectedWorkspace;

    for (const IWorkspacePtr& workspace : workspaces) {
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
    QStringList workspaceNames;
    for (const IWorkspacePtr& workspace: m_workspaces) {
        workspaceNames << QString::fromStdString(workspace->name());
    }

    UriQuery uri("muse://workspace/create");
    uri.addParam("sync", Val(true));
    uri.addParam("workspaceNames", Val(workspaceNames.join(',')));

    RetVal<Val> obj = interactive()->open(uri);
    if (!obj.ret) {
        return;
    }

    QVariantMap meta = obj.val.toQVariant().toMap();
    QString name = meta.value("name").toString();
    IF_ASSERT_FAILED(!name.isEmpty()) {
        return;
    }

    IWorkspacePtr newWorkspace = workspacesManager()->newWorkspace(name.toStdString());
    newWorkspace->setIsManaged(muse::ui::WS_UiSettings, meta.value(muse::ui::WS_UiSettings).toBool());
    newWorkspace->setIsManaged(muse::ui::WS_UiStates, meta.value(muse::ui::WS_UiStates).toBool());
    newWorkspace->setIsManaged(muse::ui::WS_UiToolConfigs, meta.value(muse::ui::WS_UiToolConfigs).toBool());
    newWorkspace->setIsManaged(palette::WS_Palettes, meta.value(palette::WS_Palettes).toBool());

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
        setSelectedWorkspace(workspacesManager()->defaultWorkspace());
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
