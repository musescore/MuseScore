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
#include "workspaceactions.h"

using namespace mu::workspace;
using namespace mu::actions;
using namespace mu::shortcuts;

const ActionList WorkspaceActions::m_actions = {
    ActionItem("select-workspace",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Select Workspace")
               ),
    ActionItem("configure-workspaces",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Configure Workspace")
               )
};

const ActionItem& WorkspaceActions::action(const ActionCode& actionCode) const
{
    for (const ActionItem& action : m_actions) {
        if (action.code == actionCode) {
            return action;
        }
    }

    static ActionItem null;
    return null;
}
