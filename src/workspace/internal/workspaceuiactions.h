//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_WORKSPACE_WORKSPACEUIACTIONS_H
#define MU_WORKSPACE_WORKSPACEUIACTIONS_H

#include "framework/ui/iuiactionsmodule.h"
#include "workspaceactioncontroller.h"
#include "modularity/ioc.h"
#include "context/iuicontextresolver.h"

namespace mu::workspace {
class WorkspaceUiActions : public ui::IUiActionsModule
{
    INJECT(workspace, context::IUiContextResolver, uicontextResolver)
public:
    WorkspaceUiActions(std::shared_ptr<WorkspaceActionController> controller);

    const ui::UiActionList& actionsList() const override;

    bool actionEnabled(const ui::UiAction& act) const override;
    async::Channel<actions::ActionCodeList> actionEnabledChanged() const override;

    bool actionChecked(const ui::UiAction& act) const override;
    async::Channel<actions::ActionCodeList> actionCheckedChanged() const override;

private:
    static const ui::UiActionList m_actions;
    std::shared_ptr<WorkspaceActionController> m_controller;
    async::Channel<actions::ActionCodeList> m_actionEnabledChanged;
    async::Channel<actions::ActionCodeList> m_actionCheckedChanged;
};
}

#endif // MU_WORKSPACE_WORKSPACEUIACTIONS_H
