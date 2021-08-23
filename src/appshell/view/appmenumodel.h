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
#ifndef MU_APPSHELL_APPMENUMODEL_H
#define MU_APPSHELL_APPMENUMODEL_H

#include "ui/view/abstractmenumodel.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "workspace/iworkspacemanager.h"
#include "iappshellconfiguration.h"
#include "project/irecentprojectsprovider.h"

namespace mu::appshell {
class AppMenuModel : public ui::AbstractMenuModel
{
    Q_OBJECT

    INJECT(appshell, actions::IActionsDispatcher, actionsDispatcher)
    INJECT(appshell, workspace::IWorkspaceManager, workspacesManager)
    INJECT(appshell, IAppShellConfiguration, configuration)
    INJECT(appshell, project::IRecentProjectsProvider, recentProjectsProvider)

public:
    explicit AppMenuModel(QObject* parent = nullptr);

    Q_INVOKABLE void load() override;

private:
    void setupConnections();
    void onActionsStateChanges(const actions::ActionCodeList& codes) override;

    ui::MenuItem fileItem() const;
    ui::MenuItem editItem() const;
    ui::MenuItem viewItem() const;
    ui::MenuItem addItem() const;
    ui::MenuItem formatItem() const;
    ui::MenuItem toolsItem() const;
    ui::MenuItem helpItem() const;
    ui::MenuItem diagnosticItem() const;

    ui::MenuItemList recentScores() const;
    ui::MenuItemList appendClearRecentSection(const ui::MenuItemList& recentScores) const;

    ui::MenuItemList notesItems() const;
    ui::MenuItemList intervalsItems() const;
    ui::MenuItemList tupletsItems() const;
    ui::MenuItemList measuresItems() const;
    ui::MenuItemList framesItems() const;
    ui::MenuItemList textItems() const;
    ui::MenuItemList linesItems() const;
    ui::MenuItemList toolbarsItems() const;
    ui::MenuItemList workspacesItems() const;
};
}

#endif // MU_APPSHELL_APPMENUMODEL_H
