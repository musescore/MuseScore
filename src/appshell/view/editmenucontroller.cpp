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
    notationController()->actionsAvailableChanged().onReceive(this, [this](const ActionCodeList& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });

    applicationController()->actionsAvailableChanged().onReceive(this, [this](const ActionCodeList& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

bool EditMenuController::contains(const ActionCode& actionCode) const
{
    return notationControllerContains(actionCode)
           || applicationControllerContains(actionCode);
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

ActionCodeList EditMenuController::notationControllerActions() const
{
    static ActionCodeList actions = {
        "undo",
        "redo",
        "cut",
        "copy",
        "paste",
        "paste-half",
        "paste-double",
        "swap",
        "delete",
        "select-all",
        "select-section",
        "find"
    };

    return actions;
}

ActionCodeList EditMenuController::applicationControllerActions() const
{
    static ActionCodeList actions = {
        "preference-dialog"
    };

    return actions;
}

bool EditMenuController::notationControllerContains(const ActionCode& actionCode) const
{
    ActionCodeList notationActions = notationControllerActions();
    return std::find(notationActions.begin(), notationActions.end(), actionCode) != notationActions.end();
}

bool EditMenuController::applicationControllerContains(const ActionCode& actionCode) const
{
    ActionCodeList applicationActions = applicationControllerActions();
    return std::find(applicationActions.begin(), applicationActions.end(), actionCode) != applicationActions.end();
}

bool EditMenuController::actionEnabled(const ActionCode& actionCode) const
{
    if (notationControllerContains(actionCode)) {
        return notationController()->actionAvailable(actionCode);
    }

    if (applicationControllerContains(actionCode)) {
        return applicationController()->actionAvailable(actionCode);
    }

    return false;
}
