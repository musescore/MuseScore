/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "workspacesmenumodel.h"

#include "log.h"

using namespace muse::workspace;
using namespace muse::ui;
using namespace muse::uicomponents;
using namespace muse::actions;

WorkspacesMenuModel::WorkspacesMenuModel(QObject* parent)
    : uicomponents::AbstractMenuModel(parent)
{
}

void WorkspacesMenuModel::load()
{
    AbstractMenuModel::load();

    MenuItemList items;

    IWorkspacePtrList workspaces = workspacesManager()->workspaces();
    IWorkspacePtr currentWorkspace = workspacesManager()->currentWorkspace();

    std::sort(workspaces.begin(), workspaces.end(), [](const IWorkspacePtr& workspace1, const IWorkspacePtr& workspace2) {
        return workspace1->name() < workspace2->name();
    });

    int index = 0;
    for (const IWorkspacePtr& workspace : workspaces) {
        MenuItem* item = new MenuItem(uiActionsRegister()->action("select-workspace"), this);
        item->setId(QString::number(index++));

        UiAction action = item->action();
        action.title = TranslatableString::untranslatable(String::fromStdString(workspace->name()));

        item->setAction(action);
        item->setArgs(ActionData::make_arg1<std::string>(workspace->name()));
        item->setSelectable(true);
        item->setSelected(workspace == currentWorkspace);

        UiActionState state;
        state.enabled = true;
        state.checked = item->selected();
        item->setState(state);

        items << item;
    }

    items << makeSeparator()
          << makeMenuItem("configure-workspaces")
          << makeMenuItem("create-workspace");

    workspacesManager()->currentWorkspaceChanged().onNotify(this, [this]() {
        load();
    });

    workspacesManager()->workspacesListChanged().onNotify(this, [this]() {
        load();
    });

    setItems(items);
}
