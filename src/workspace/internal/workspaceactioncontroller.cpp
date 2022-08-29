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
#include "workspaceactioncontroller.h"

#include "types/val.h"

using namespace mu::workspace;
using namespace mu::actions;

void WorkspaceActionController::init()
{
    dispatcher()->reg(this, "select-workspace", this, &WorkspaceActionController::selectWorkspace);
    dispatcher()->reg(this, "edit-workspace", this, &WorkspaceActionController::editCurrentWorkspace);
    dispatcher()->reg(this, "configure-workspaces", this, &WorkspaceActionController::openConfigureWorkspacesDialog);
    dispatcher()->reg(this, "reset-workspace", [this]() { setCurrentWorkspaceName(DEFAULT_WORKSPACE_NAME); });
}

void WorkspaceActionController::selectWorkspace(const ActionData& args)
{
    std::string selectedWorkspace = !args.empty() ? args.arg<std::string>(0) : "";
    setCurrentWorkspaceName(selectedWorkspace);
}

void WorkspaceActionController::editCurrentWorkspace()
{
    IWorkspacePtrList workspaces = workspacesManager()->workspaces();
    std::string workspaceNames;

    for (const IWorkspacePtr& workspace: workspaces) {
        workspaceNames = workspaceNames + workspace->name() + ",";
    }
    workspaceNames.pop_back();

    UriQuery uri("musescore://workspace/create");
    uri.addParam("sync", Val(true));
    uri.addParam("workspaceNames", Val(workspaceNames));
    uri.addParam("editWorkspace", Val(true));

    RetVal<Val> obj = interactive()->open(uri);
    if (!obj.ret) {
        return;
    }

    IWorkspacePtr currentWorkspace = workspacesManager()->currentWorkspace();
    if (!currentWorkspace) {
        return;
    }

    QVariantMap meta = obj.val.toQVariant().toMap();
    std::string name = meta.value("name").toString().toStdString();
    if (name != currentWorkspace->name()) {
        IWorkspacePtrList updatedWorkspaces;
        for (IWorkspacePtr workspace : workspaces) {
            if (workspace != currentWorkspace) {
                updatedWorkspaces.emplace_back(workspace);
            }
        }

        IWorkspacePtr newWorkspace = workspacesManager()->newWorkspace(name);
        newWorkspace->setIsManaged(DataKey::UiSettings, meta.value("ui_settings").toBool());
        newWorkspace->setIsManaged(DataKey::UiStates, meta.value("ui_states").toBool());
        newWorkspace->setIsManaged(DataKey::UiToolConfigs, meta.value("ui_toolconfigs").toBool());
        newWorkspace->setIsManaged(DataKey::Palettes, meta.value("palettes").toBool());

        updatedWorkspaces.emplace_back(newWorkspace);
        workspacesManager()->setWorkspaces(updatedWorkspaces);

        setCurrentWorkspaceName(name);

        return;
    }

    if (meta["ui_settings"].toBool() != currentWorkspace->isManaged(DataKey::UiSettings)) {
        currentWorkspace->setIsManaged(DataKey::UiSettings, meta["ui_settings"].toBool());
    }

    if (meta["ui_states"].toBool() != currentWorkspace->isManaged(DataKey::UiStates)) {
        currentWorkspace->setIsManaged(DataKey::UiStates, meta["ui_states"].toBool());
    }

    if (meta["ui_toolconfigs"].toBool() != currentWorkspace->isManaged(DataKey::UiToolConfigs)) {
        currentWorkspace->setIsManaged(DataKey::UiToolConfigs, meta["ui_toolconfigs"].toBool());
    }

    if (meta["palettes"].toBool() != currentWorkspace->isManaged(DataKey::Palettes)) {
        currentWorkspace->setIsManaged(DataKey::Palettes, meta["palettes"].toBool());
    }
    workspacesManager()->currentWorkspaceChanged().notify();
}

void WorkspaceActionController::openConfigureWorkspacesDialog()
{
    RetVal<Val> result = interactive()->open("musescore://workspace/select?sync=true");
    if (!result.ret) {
        return;
    }

    std::string selectedWorkspace = result.val.toString();
    setCurrentWorkspaceName(selectedWorkspace);
}

void WorkspaceActionController::setCurrentWorkspaceName(const std::string& workspaceName)
{
    if (configuration()->currentWorkspaceName() == workspaceName || workspaceName.empty()) {
        return;
    }

    configuration()->setCurrentWorkspaceName(workspaceName);
}
