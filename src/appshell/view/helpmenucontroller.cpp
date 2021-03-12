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
#include "helpmenucontroller.h"

using namespace mu::appshell;
using namespace mu::actions;
using namespace mu::uicomponents;

void HelpMenuController::init()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

bool HelpMenuController::contains(const ActionCode& actionCode) const
{
    ActionCodeList actions = this->actions();
    return std::find(actions.begin(), actions.end(), actionCode) != actions.end();
}

ActionState HelpMenuController::actionState(const ActionCode& actionCode) const
{
    ActionState state;
    state.enabled = actionEnabled(actionCode);

    return state;
}

mu::async::Channel<mu::actions::ActionCodeList> HelpMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

ActionCodeList HelpMenuController::actions() const
{
    static ActionCodeList actions = {
        "show-tours",
        "reset-tours",
        "online-handbook",
        "about",
        "about-qt",
        "about-musicxml",
        "check-update",
        "ask-help",
        "report-bug",
        "leave-feedback",
        "revert-factory"
    };

    return actions;
}

bool HelpMenuController::actionEnabled(const ActionCode& actionCode) const
{
    return controller()->actionAvailable(actionCode);
}
