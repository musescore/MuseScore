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
#include "editmenucontroller.h"

using namespace mu::appshell;
using namespace mu::actions;
using namespace mu::uicomponents;

void EditMenuController::init()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const ActionCodeList& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

bool EditMenuController::contains(const ActionCode& actionCode) const
{
    ActionCodeList actions = this->actions();
    return std::find(actions.begin(), actions.end(), actionCode) != actions.end();
}

ActionState EditMenuController::actionState(const ActionCode& actionCode) const
{
    ActionState state;
    state.enabled = actionEnabled(actionCode);

    return state;
}

mu::async::Channel<ActionCodeList> EditMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

ActionCodeList EditMenuController::actions() const
{
    static ActionCodeList actions = {
        "undo",
        "redo",
        "cut",
        "copy",
        "paste",
        "paste-half",
        "paste-double",
        "paste-special",
        "swap",
        "delete",
        "select-all",
        "select-similar",
        "find",
        "preference-dialog"
    };

    return actions;
}

bool EditMenuController::actionEnabled(const ActionCode& actionCode) const
{
    return controller()->actionAvailable(actionCode);
}
