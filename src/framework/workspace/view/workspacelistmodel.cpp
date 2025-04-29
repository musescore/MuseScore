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

#include "internal/workspaceutils.h"

#include "translation.h"
#include "log.h"

using namespace muse::workspace;
using namespace muse;

static const QString NAME_KEY("name");
static const QString IS_SELECTED_KEY("isSelected");
static const QString IS_BUILTIN_KEY("isBuiltin");
static const QString IS_EDITED_KEY("isEdited");
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

    std::sort(m_workspaces.begin(), m_workspaces.end(), WorkspaceUtils::workspaceLessThan);

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

QString WorkspaceListModel::appTitle() const
{
    return application()->title();
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
    case RoleIsBuiltin:
        return workspace[IS_BUILTIN_KEY];
    case RoleIsEdited:
        return workspace[IS_EDITED_KEY];
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
    result[IS_BUILTIN_KEY] = workspace->isBuiltin();
    result[IS_EDITED_KEY] = workspace->isEdited();
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
        { RoleName,       NAME_KEY.toUtf8() },
        { RoleIsSelected, IS_SELECTED_KEY.toUtf8() },
        { RoleIsBuiltin,  IS_BUILTIN_KEY.toUtf8() },
        { RoleIsEdited,   IS_EDITED_KEY.toUtf8() }
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

    IWorkspacePtr newWorkspace = workspacesManager()->cloneWorkspace(m_selectedWorkspace, name.toStdString());
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
        setSelectedWorkspace(workspacesManager()->defaultWorkspace());
    }
}

void WorkspaceListModel::resetWorkspace(int workspaceIndex)
{
    if (!isIndexValid(workspaceIndex)) {
        return;
    }

    int resetButton = static_cast<int>(IInteractive::Button::CustomButton) + 1;
    std::string question = muse::trc("workspace",
                                     "This action will reset your workspace to its factory default layout and cannot be undone. Do you want to continue?");

    IInteractive::Button btn = interactive()->warning(muse::trc("workspace", "Resetting workspaces"), question, {
        IInteractive::ButtonData(resetButton, muse::trc("workspace", "Reset workspace"), true, false, IInteractive::ButtonRole::AcceptRole),
        interactive()->buttonData(IInteractive::Button::Cancel)
    }).standardButton();

    if (static_cast<int>(btn) != resetButton) {
        return;
    }

    IWorkspacePtr workspace = m_workspaces.at(workspaceIndex);
    workspace->reset();

    QModelIndex modelIndex = index(workspaceIndex);
    emit dataChanged(modelIndex, modelIndex);
}

bool WorkspaceListModel::renameWorkspace(int workspaceIndex, const QString& newName)
{
    if (!isIndexValid(workspaceIndex)) {
        return false;
    }

    IWorkspacePtr workspace = m_workspaces.at(workspaceIndex);
    workspace->assignNewName(newName.toStdString());

    QModelIndex modelIndex = index(workspaceIndex);
    emit dataChanged(modelIndex, modelIndex, { RoleName });

    return true;
}

QString WorkspaceListModel::validateWorkspaceName(int workspaceIndex, const QString& name) const
{
    if (name.isEmpty()) {
        return muse::qtrc("workspace", "Name cannot be empty");
    }

    QString nameLower = name.toLower();

    for (int i = 0; i < m_workspaces.size(); ++i) {
        if (i == workspaceIndex) {
            continue;
        }

        if (QString::fromStdString(m_workspaces[i]->name()).toLower() == nameLower) {
            return muse::qtrc("workspace", "Name already exists");
        }
    }

    return {};
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
