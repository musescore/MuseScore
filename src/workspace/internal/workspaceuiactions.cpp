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
#include "workspaceuiactions.h"

#include "context/uicontext.h"

using namespace mu::workspace;
using namespace mu::ui;

const UiActionList WorkspaceUiActions::m_actions = {
    UiAction("select-workspace",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Select Workspace")
             ),
    UiAction("configure-workspaces",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Configure Workspace")
             )
};

WorkspaceUiActions::WorkspaceUiActions(std::shared_ptr<WorkspaceActionController> controller)
    : m_controller(controller)
{
}

const UiActionList& WorkspaceUiActions::actionsList() const
{
    return m_actions;
}

bool WorkspaceUiActions::actionEnabled(const UiAction& act) const
{
    if (!m_controller->canReceiveAction(act.code)) {
        return false;
    }

    return true;
}

bool WorkspaceUiActions::actionChecked(const UiAction&) const
{
    return false;
}

mu::async::Channel<mu::actions::ActionCodeList> WorkspaceUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

mu::async::Channel<mu::actions::ActionCodeList> WorkspaceUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}
